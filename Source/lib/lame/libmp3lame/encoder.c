/*
 *      LAME MP3 encoding engine
 *
 *      Copyright (c) 1999 Mark Taylor
 *      Copyright (c) 2000-2002 Takehiro Tominaga
 *      Copyright (c) 2000-2011 Robert Hegemann
 *      Copyright (c) 2001 Gabriel Bouvigne
 *      Copyright (c) 2001 John Dahlstrom
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* $Id: encoder.c,v 1.111 2011/05/07 16:05:17 rbrito Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <math.h>


#include "lame.h"
#include "machine.h"
#include "encoder.h"
#include "util.h"
#include "lame_global_flags.h"
#include "newmdct.h"
#include "psymodel.h"
#include "lame-analysis.h"
#include "bitstream.h"
#include "VbrTag.h"
#include "quantize_pvt.h"



/*
 * auto-adjust of ATH, useful for low volume
 * Gabriel Bouvigne 3 feb 2001
 *
 * modifies some values in
 *   gfp->internal_flags->ATH
 *   (gfc->ATH)
 */
static void
adjust_ATH(lame_internal_flags const *const gfc)
{
    SessionConfig_t const *const cfg = &gfc->cfg;
    FLOAT   gr2_max, max_pow;

    if (gfc->ATH->use_adjust == 0) {
        gfc->ATH->adjust_factor = 1.0; /* no adjustment */
        return;
    }

    /* jd - 2001 mar 12, 27, jun 30 */
    /* loudness based on equal loudness curve; */
    /* use granule with maximum combined loudness */
    max_pow = gfc->ov_psy.loudness_sq[0][0];
    gr2_max = gfc->ov_psy.loudness_sq[1][0];
    if (cfg->channels_out == 2) {
        max_pow += gfc->ov_psy.loudness_sq[0][1];
        gr2_max += gfc->ov_psy.loudness_sq[1][1];
    }
    else {
        max_pow += max_pow;
        gr2_max += gr2_max;
    }
    if (cfg->mode_gr == 2) {
        max_pow = Max(max_pow, gr2_max);
    }
    max_pow *= 0.5;     /* max_pow approaches 1.0 for full band noise */

    /* jd - 2001 mar 31, jun 30 */
    /* user tuning of ATH adjustment region */
    max_pow *= gfc->ATH->aa_sensitivity_p;

    /*  adjust ATH depending on range of maximum value
     */

    /* jd - 2001 feb27, mar12,20, jun30, jul22 */
    /* continuous curves based on approximation */
    /* to GB's original values. */
    /* For an increase in approximate loudness, */
    /* set ATH adjust to adjust_limit immediately */
    /* after a delay of one frame. */
    /* For a loudness decrease, reduce ATH adjust */
    /* towards adjust_limit gradually. */
    /* max_pow is a loudness squared or a power. */
    if (max_pow > 0.03125) { /* ((1 - 0.000625)/ 31.98) from curve below */
        if (gfc->ATH->adjust_factor >= 1.0) {
            gfc->ATH->adjust_factor = 1.0;
        }
        else {
            /* preceding frame has lower ATH adjust; */
            /* ascend only to the preceding adjust_limit */
            /* in case there is leading low volume */
            if (gfc->ATH->adjust_factor < gfc->ATH->adjust_limit) {
                gfc->ATH->adjust_factor = gfc->ATH->adjust_limit;
            }
        }
        gfc->ATH->adjust_limit = 1.0;
    }
    else {              /* adjustment curve */
        /* about 32 dB maximum adjust (0.000625) */
        FLOAT const adj_lim_new = 31.98 * max_pow + 0.000625;
        if (gfc->ATH->adjust_factor >= adj_lim_new) { /* descend gradually */
            gfc->ATH->adjust_factor *= adj_lim_new * 0.075 + 0.925;
            if (gfc->ATH->adjust_factor < adj_lim_new) { /* stop descent */
                gfc->ATH->adjust_factor = adj_lim_new;
            }
        }
        else {          /* ascend */
            if (gfc->ATH->adjust_limit >= adj_lim_new) {
                gfc->ATH->adjust_factor = adj_lim_new;
            }
            else {      /* preceding frame has lower ATH adjust; */
                /* ascend only to the preceding adjust_limit */
                if (gfc->ATH->adjust_factor < gfc->ATH->adjust_limit) {
                    gfc->ATH->adjust_factor = gfc->ATH->adjust_limit;
                }
            }
        }
        gfc->ATH->adjust_limit = adj_lim_new;
    }
}

/***********************************************************************
 *
 *  some simple statistics
 *
 *  bitrate index 0: free bitrate -> not allowed in VBR mode
 *  : bitrates, kbps depending on MPEG version
 *  bitrate index 15: forbidden
 *
 *  mode_ext:
 *  0:  LR
 *  1:  LR-i
 *  2:  MS
 *  3:  MS-i
 *
 ***********************************************************************/

static void
updateStats(lame_internal_flags * const gfc)
{
    SessionConfig_t const *const cfg = &gfc->cfg;
    EncResult_t *eov = &gfc->ov_enc;
    int     gr, ch;
    assert(0 <= eov->bitrate_index && eov->bitrate_index < 16);
    assert(0 <= eov->mode_ext && eov->mode_ext < 4);

    /* count bitrate indices */
    eov->bitrate_channelmode_hist[eov->bitrate_index][4]++;
    eov->bitrate_channelmode_hist[15][4]++;

    /* count 'em for every mode extension in case of 2 channel encoding */
    if (cfg->channels_out == 2) {
        eov->bitrate_channelmode_hist[eov->bitrate_index][eov->mode_ext]++;
        eov->bitrate_channelmode_hist[15][eov->mode_ext]++;
    }
    for (gr = 0; gr < cfg->mode_gr; ++gr) {
        for (ch = 0; ch < cfg->channels_out; ++ch) {
            int     bt = gfc->l3_side.tt[gr][ch].block_type;
            if (gfc->l3_side.tt[gr][ch].mixed_block_flag)
                bt = 4;
            eov->bitrate_blocktype_hist[eov->bitrate_index][bt]++;
            eov->bitrate_blocktype_hist[eov->bitrate_index][5]++;
            eov->bitrate_blocktype_hist[15][bt]++;
            eov->bitrate_blocktype_hist[15][5]++;
        }
    }
}


static float apply_v_shift(float v, float v_shift) 
{
    float v_mag, v_sign;
    v_mag = fabsf(v);
    v_sign = (0 < v) - (v < 0);
    if ((v_shift > 0) && (v != 0)) {
        v = (v_mag + (v_shift)) * v_sign;
    // } else if (v_shift < 0) {
    //     v = fmaxf(0, v_mag + v_shift) * v_sign;
    } else if ((v_shift < 0) && (v_mag < -v_shift)) {
        v = 0;
    }
    return v;
}



static void
lame_encode_frame_init(lame_internal_flags * gfc, const sample_t *const inbuf[2])
{
    SessionConfig_t const *const cfg = &gfc->cfg;

    int     ch, gr;

    if (gfc->lame_encode_frame_init == 0) {
        sample_t primebuff0[286 + 1152 + 576];
        sample_t primebuff1[286 + 1152 + 576];
        int const framesize = 576 * cfg->mode_gr;
        /* prime the MDCT/polyphase filterbank with a short block */
        int     i, j;
        gfc->lame_encode_frame_init = 1;
        memset(primebuff0, 0, sizeof(primebuff0));
        memset(primebuff1, 0, sizeof(primebuff1));
        for (i = 0, j = 0; i < 286 + 576 * (1 + cfg->mode_gr); ++i) {
            if (i < framesize) {
                primebuff0[i] = 0;
                if (cfg->channels_out == 2)
                    primebuff1[i] = 0;
            }
            else {
                primebuff0[i] = inbuf[0][j];
                if (cfg->channels_out == 2)
                    primebuff1[i] = inbuf[1][j];
                ++j;
            }
        }
        /* polyphase filtering / mdct */
        for (gr = 0; gr < cfg->mode_gr; gr++) {
            for (ch = 0; ch < cfg->channels_out; ch++) {
                gfc->l3_side.tt[gr][ch].block_type = SHORT_TYPE;
            }
        }
        mdct_sub48(gfc, primebuff0, primebuff1);

        /* check FFT will not use a negative starting offset */
#if 576 < FFTOFFSET
# error FFTOFFSET greater than 576: FFT uses a negative offset
#endif
        /* check if we have enough data for FFT */
        assert(gfc->sv_enc.mf_size >= (BLKSIZE + framesize - FFTOFFSET));
        /* check if we have enough data for polyphase filterbank */
        assert(gfc->sv_enc.mf_size >= (512 + framesize - 32));
    }

}







/************************************************************************
*
* encodeframe()           Layer 3
*
* encode a single frame
*
************************************************************************
lame_encode_frame()


                       gr 0            gr 1
inbuf:           |--------------|--------------|--------------|


Polyphase (18 windows, each shifted 32)
gr 0:
window1          <----512---->
window18                 <----512---->

gr 1:
window1                         <----512---->
window18                                <----512---->



MDCT output:  |--------------|--------------|--------------|

FFT's                    <---------1024---------->
                                         <---------1024-------->



    inbuf = buffer of PCM data size=MP3 framesize
    encoder acts on inbuf[ch][0], but output is delayed by MDCTDELAY
    so the MDCT coefficints are from inbuf[ch][-MDCTDELAY]

    psy-model FFT has a 1 granule delay, so we feed it data for the 
    next granule.
    FFT is centered over granule:  224+576+224
    So FFT starts at:   576-224-MDCTDELAY

    MPEG2:  FFT ends at:  BLKSIZE+576-224-MDCTDELAY      (1328)
    MPEG1:  FFT ends at:  BLKSIZE+2*576-224-MDCTDELAY    (1904)

    MPEG2:  polyphase first window:  [0..511]
                      18th window:   [544..1055]          (1056)
    MPEG1:            36th window:   [1120..1631]         (1632)
            data needed:  512+framesize-32

    A close look newmdct.c shows that the polyphase filterbank
    only uses data from [0..510] for each window.  Perhaps because the window
    used by the filterbank is zero for the last point, so Takehiro's
    code doesn't bother to compute with it.

    FFT starts at 576-224-MDCTDELAY (304)  = 576-FFTOFFSET

*/

typedef FLOAT chgrdata[2][2];


int
lame_encode_mp3_frame(       /* Output */
                         lame_internal_flags * gfc, /* Context */
                         sample_t const *inbuf_l, /* Input */
                         sample_t const *inbuf_r, /* Input */
                         unsigned char *mp3buf, /* Output */
                         int mp3buf_size)
{                       /* Output */
    SessionConfig_t const *const cfg = &gfc->cfg;
    int     mp3count;
    III_psy_ratio masking_LR[2][2]; /*LR masking & energy */
    III_psy_ratio masking_MS[2][2]; /*MS masking & energy */
    III_psy_ratio (*masking)[2]; /*pointer to selected maskings */
    const sample_t *inbuf[2];

    FLOAT   tot_ener[2][4];
    FLOAT   ms_ener_ratio[2] = { .5, .5 };
    FLOAT   pe[2][2] = { {0., 0.}, {0., 0.} }, pe_MS[2][2] = { {
    0., 0.}, {
    0., 0.}};
    FLOAT (*pe_use)[2];

    int     ch, gr;

    inbuf[0] = inbuf_l;
    inbuf[1] = inbuf_r;

    if (gfc->lame_encode_frame_init == 0) {
        /*first run? */
        lame_encode_frame_init(gfc, inbuf);

    }


    /********************** padding *****************************/
    /* padding method as described in 
     * "MPEG-Layer3 / Bitstream Syntax and Decoding"
     * by Martin Sieler, Ralph Sperschneider
     *
     * note: there is no padding for the very first frame
     *
     * Robert Hegemann 2000-06-22
     */
    gfc->ov_enc.padding = FALSE;
    if ((gfc->sv_enc.slot_lag -= gfc->sv_enc.frac_SpF) < 0) {
        gfc->sv_enc.slot_lag += cfg->samplerate_out;
        gfc->ov_enc.padding = TRUE;
    }



    /****************************************
    *   Stage 1: psychoacoustic model       *
    ****************************************/

    gfc->bendFlagsAndData->in_short_block = 0;

    int     blocktype[2];
    {
        /* psychoacoustic model
         * psy model has a 1 granule (576) delay that we must compensate for
         * (mt 6/99).
         */
        int     ret;
        const sample_t *bufp[2] = {0, 0}; /* address of beginning of left & right granule */
        

        for (gr = 0; gr < cfg->mode_gr; gr++) {

            for (ch = 0; ch < cfg->channels_out; ch++) {
                bufp[ch] = &inbuf[ch][576 + gr * 576 - FFTOFFSET];
            }
            ret = L3psycho_anal_vbr(gfc, bufp, gr,
                                    masking_LR, masking_MS,
                                    pe[gr], pe_MS[gr], tot_ener[gr], blocktype);
            if (ret != 0)
                return -4;

            if (cfg->mode == JOINT_STEREO) {
                ms_ener_ratio[gr] = tot_ener[gr][2] + tot_ener[gr][3];
                if (ms_ener_ratio[gr] > 0)
                    ms_ener_ratio[gr] = tot_ener[gr][3] / ms_ener_ratio[gr];
            }

            /* block type flags */
            for (ch = 0; ch < cfg->channels_out; ch++) {
                gr_info *const cod_info = &gfc->l3_side.tt[gr][ch];
                cod_info->block_type = blocktype[ch];
                cod_info->mixed_block_flag = 0;
                if (blocktype[ch] != NORM_TYPE) {
                    gfc->bendFlagsAndData->in_short_block = 1;
                }
            }
        }
    }

    /* auto-adjust of ATH, useful for low volume */
    adjust_ATH(gfc);


    /****************************************
    *   Stage 2: MDCT                       *
    ****************************************/

    /* polyphase filtering / mdct */
    mdct_sub48(gfc, inbuf[0], inbuf[1]);


#if 0
    if ((blocktype[0] == NORM_TYPE) && 
        (blocktype[1] == NORM_TYPE) && 
        (gfc->bendFlagsAndData->prev_block_long)) {
        float wet = gfc->bendFlagsAndData->mdct_feedback;
        float dry = 1 - wet;
        for (int gr = 0; gr < 2; ++gr) {
            for (int ch = 0; ch < 2; ++ch) {
                for (int s = 0; s < 576; ++s) {
                    gfc->bendFlagsAndData->feedback_data[gr][ch][s] = 
                        dry * gfc->l3_side.tt[gr][ch].xr[s] + 
                        wet * gfc->bendFlagsAndData->feedback_data[gr][ch][s];
                    gfc->l3_side.tt[gr][ch].xr[s] = gfc->bendFlagsAndData->feedback_data[gr][ch][s];
                }
            }
        }
        printf("\n");
    } else if ((blocktype[0] == NORM_TYPE) && (blocktype[1] == NORM_TYPE)) {
        gfc->bendFlagsAndData->prev_block_long = 1;
        for (int gr = 0; gr < 2; ++gr) {
            for (int ch = 0; ch < 2; ++ch) {
                for (int s = 0; s < 576; ++s) {
                    gfc->bendFlagsAndData->feedback_data[gr][ch][s] = gfc->l3_side.tt[gr][ch].xr[s];
                }
            }
        }

    } else {
        gfc->bendFlagsAndData->prev_block_long = 0;
    }
    
#endif
    
    // TESTT: print output of mdct
    // scale y: boring. shift y: grating tone
    // shift x: robot voice
    int h_shift = gfc->bendFlagsAndData->mdct_post_h_shift;
    float v_shift = gfc->bendFlagsAndData->mdct_post_v_shift;
    float* pre_bend = gfc->bendFlagsAndData->mdct_pre_bend;
    float* post_bend = gfc->bendFlagsAndData->mdct_post_bend;
    float v;
    for (int i = 0; i < 576; ++i) {
        pre_bend[i] = 0;
    }
    for (gr = 0; gr < cfg->mode_gr; gr++) {
        for (ch = 0; ch < cfg->channels_out; ch++) {
            for (int i = 0; i < 576; ++i) {
                pre_bend[i] += gfc->l3_side.tt[gr][ch].xr[i];
            }
        }
    }
    for (int i = 0; i < 576; ++i) {
        pre_bend[i] /= 4;
    }
    if (h_shift < 0) {
        for (gr = 0; gr < cfg->mode_gr; gr++) {
            for (ch = 0; ch < cfg->channels_out; ch++) {
                for (int i = 0; i < 576; ++i) {
                    v = gfc->l3_side.tt[gr][ch].xr[(i+576-h_shift)%576];
                    gfc->l3_side.tt[gr][ch].xr[i] = apply_v_shift(v, v_shift);
                }
            }
        }
    } else {
        for (gr = 0; gr < cfg->mode_gr; gr++) {
            for (ch = 0; ch < cfg->channels_out; ch++) {
                for (int i = 576-1; i >= 0; --i) {
                    v = gfc->l3_side.tt[gr][ch].xr[(i+576-h_shift)%576];
                    gfc->l3_side.tt[gr][ch].xr[i] = apply_v_shift(v, v_shift);
                }
            }
        }
    }

    float wet = gfc->bendFlagsAndData->mdct_feedback;
    float dry = 1 - wet;
    float m;
    for (int gr = 0; gr < 2; ++gr) {
        for (int ch = 0; ch < 2; ++ch) {
            for (int s = 0; s < 576; ++s) {
                float input = gfc->l3_side.tt[gr][ch].xr[s];
                if (isnan(input) || isinf(input)) {
                    input = 0;
                }
                m = fabsf(input);
                float fed = gfc->bendFlagsAndData->feedback_data[gr][ch][s];
                gfc->bendFlagsAndData->feedback_data[gr][ch][s] = (isnan(fed) || isinf(fed)) ? 0 :
                                                                  dry * m + wet * fed;
                gfc->l3_side.tt[gr][ch].xr[s] = (gfc->l3_side.tt[gr][ch].xr[s] > 0) ?
                    gfc->bendFlagsAndData->feedback_data[gr][ch][s] :
                    -gfc->bendFlagsAndData->feedback_data[gr][ch][s];
            }
        }
    }
    printf("\n");
    for (int i = 0; i < 576; ++i) {
        post_bend[i] = 0;
    }
    for (gr = 0; gr < cfg->mode_gr; gr++) {
        for (ch = 0; ch < cfg->channels_out; ch++) {
            for (int i = 0; i < 576; ++i) {
                post_bend[i] += gfc->l3_side.tt[gr][ch].xr[i];
            }
        }
    }
    for (int i = 0; i < 576; ++i) {
        post_bend[i] /= 4;
    }

    // Result: some small numbers, between -1ish and 1, mostly very small magnitude, and then
    // all zeros above the lowpass filter cutoff
    


    /****************************************
    *   Stage 3: MS/LR decision             *
    ****************************************/

    /* Here will be selected MS or LR coding of the 2 stereo channels */
    gfc->ov_enc.mode_ext = MPG_MD_LR_LR;

    if (cfg->force_ms) {
        gfc->ov_enc.mode_ext = MPG_MD_MS_LR;
    }
    else if (cfg->mode == JOINT_STEREO) {
        /* ms_ratio = is scaled, for historical reasons, to look like
           a ratio of side_channel / total.
           0 = signal is 100% mono
           .5 = L & R uncorrelated
         */

        /* [0] and [1] are the results for the two granules in MPEG-1,
         * in MPEG-2 it's only a faked averaging of the same value
         * _prev is the value of the last granule of the previous frame
         * _next is the value of the first granule of the next frame
         */

        FLOAT   sum_pe_MS = 0;
        FLOAT   sum_pe_LR = 0;
        for (gr = 0; gr < cfg->mode_gr; gr++) {
            for (ch = 0; ch < cfg->channels_out; ch++) {
                sum_pe_MS += pe_MS[gr][ch];
                sum_pe_LR += pe[gr][ch];
            }
        }

        /* based on PE: M/S coding would not use much more bits than L/R */
        if (sum_pe_MS <= 1.00 * sum_pe_LR) {

            gr_info const *const gi0 = &gfc->l3_side.tt[0][0];
            gr_info const *const gi1 = &gfc->l3_side.tt[cfg->mode_gr - 1][0];

            if (gi0[0].block_type == gi0[1].block_type && gi1[0].block_type == gi1[1].block_type) {

                gfc->ov_enc.mode_ext = MPG_MD_MS_LR;
            }
        }
    }

    /* bit and noise allocation */
    if (gfc->ov_enc.mode_ext == MPG_MD_MS_LR) {
        masking = (III_psy_ratio (*)[2])masking_MS; /* use MS masking */
        pe_use = pe_MS;
        if ((blocktype[0] == NORM_TYPE) && (blocktype[1] == NORM_TYPE)) {
            for (int i = 0; i < 22; ++i) {
                masking_MS[0][0].thm.l[i] *= gfc->bendFlagsAndData->threshold_bias[i];
                masking_MS[0][1].thm.l[i] *= gfc->bendFlagsAndData->threshold_bias[i];
                masking_MS[1][0].thm.l[i] *= gfc->bendFlagsAndData->threshold_bias[i];
                masking_MS[1][1].thm.l[i] *= gfc->bendFlagsAndData->threshold_bias[i];
            }
        }

    }
    else {
        masking = (III_psy_ratio (*)[2])masking_LR; /* use LR masking */
        pe_use = pe;
        if ((blocktype[0] == NORM_TYPE) && (blocktype[1] == NORM_TYPE)) {
            for (int i = 0; i < 22; ++i) {
                masking_LR[0][0].thm.l[i] *= gfc->bendFlagsAndData->threshold_bias[i];
                masking_LR[0][1].thm.l[i] *= gfc->bendFlagsAndData->threshold_bias[i];
                masking_LR[1][0].thm.l[i] *= gfc->bendFlagsAndData->threshold_bias[i];
                masking_LR[1][1].thm.l[i] *= gfc->bendFlagsAndData->threshold_bias[i];
            }
        }

    }

    if (!(gfc->bendFlagsAndData->in_short_block)) {
        for (int i = 0; i < 22; ++i) {
            gfc->bendFlagsAndData->psychoanal_energy[i] = ((*masking)[0].en.l[i] + (*masking)[1].en.l[i])/2;
            gfc->bendFlagsAndData->psychoanal_threshold[i] = ((*masking)[0].thm.l[i] + (*masking)[1].thm.l[i])/2;
        }
    }

    /*
    Sometimes small mag float values, but sometimes very big float values? whats up here?
    is it a short block long block thing?
    for (int i = 0; i < SBMAX_l; ++i) {
        printf("%f ", (*masking)[0].en.l[i]);
    }
    for (int )
    printf("\n");
    */
    
    /* copy data for MP3 frame analyzer */
    if (cfg->analysis && gfc->pinfo != NULL) {
        for (gr = 0; gr < cfg->mode_gr; gr++) {
            for (ch = 0; ch < cfg->channels_out; ch++) {
                gfc->pinfo->ms_ratio[gr] = 0;
                gfc->pinfo->ms_ener_ratio[gr] = ms_ener_ratio[gr];
                gfc->pinfo->blocktype[gr][ch] = gfc->l3_side.tt[gr][ch].block_type;
                gfc->pinfo->pe[gr][ch] = pe_use[gr][ch];
                memcpy(gfc->pinfo->xr[gr][ch], &gfc->l3_side.tt[gr][ch].xr[0], sizeof(FLOAT) * 576);
                /* in psymodel, LR and MS data was stored in pinfo.  
                   switch to MS data: */
                if (gfc->ov_enc.mode_ext == MPG_MD_MS_LR) {
                    gfc->pinfo->ers[gr][ch] = gfc->pinfo->ers[gr][ch + 2];
                    memcpy(gfc->pinfo->energy[gr][ch], gfc->pinfo->energy[gr][ch + 2],
                           sizeof(gfc->pinfo->energy[gr][ch]));
                }
            }
        }
    }


    /****************************************
    *   Stage 4: quantization loop          *
    ****************************************/

    if (cfg->vbr == vbr_off || cfg->vbr == vbr_abr) {
        static FLOAT const fircoef[9] = {
            -0.0207887 * 5, -0.0378413 * 5, -0.0432472 * 5, -0.031183 * 5,
            7.79609e-18 * 5, 0.0467745 * 5, 0.10091 * 5, 0.151365 * 5,
            0.187098 * 5
        };

        int     i;
        FLOAT   f;

        for (i = 0; i < 18; i++)
            gfc->sv_enc.pefirbuf[i] = gfc->sv_enc.pefirbuf[i + 1];

        f = 0.0;
        for (gr = 0; gr < cfg->mode_gr; gr++)
            for (ch = 0; ch < cfg->channels_out; ch++)
                f += pe_use[gr][ch];
        gfc->sv_enc.pefirbuf[18] = f;

        f = gfc->sv_enc.pefirbuf[9];
        for (i = 0; i < 9; i++)
            f += (gfc->sv_enc.pefirbuf[i] + gfc->sv_enc.pefirbuf[18 - i]) * fircoef[i];

        f = (670 * 5 * cfg->mode_gr * cfg->channels_out) / f;
        for (gr = 0; gr < cfg->mode_gr; gr++) {
            for (ch = 0; ch < cfg->channels_out; ch++) {
                pe_use[gr][ch] *= f;
            }
        }
    }

    gfc->iteration_loop(gfc, (const FLOAT (*)[2])pe_use, ms_ener_ratio, masking);


    /****************************************
    *   Stage 5: bitstream formatting       *
    ****************************************/


    /*  write the frame to the bitstream  */
    (void) format_bitstream(gfc);

    /* copy mp3 bit buffer into array */
    mp3count = copy_buffer(gfc, mp3buf, mp3buf_size, 1);


    if (cfg->write_lame_tag) {
        AddVbrFrame(gfc);
    }

    if (cfg->analysis && gfc->pinfo != NULL) {
        int     framesize = 576 * cfg->mode_gr;
        for (ch = 0; ch < cfg->channels_out; ch++) {
            int     j;
            for (j = 0; j < FFTOFFSET; j++)
                gfc->pinfo->pcmdata[ch][j] = gfc->pinfo->pcmdata[ch][j + framesize];
            for (j = FFTOFFSET; j < 1600; j++) {
                gfc->pinfo->pcmdata[ch][j] = inbuf[ch][j - FFTOFFSET];
            }
        }
        gfc->sv_qnt.masking_lower = 1.0;

        set_frame_pinfo(gfc, masking);
    }

    ++gfc->ov_enc.frame_number;

    updateStats(gfc);

    return mp3count;
}
