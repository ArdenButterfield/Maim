/*
			(c) Copyright 1998-2000 - Tord Jansson
			======================================

		This file is part of the BladeEnc MP3 Encoder, based on
		ISO's reference code for MPEG Layer 3 compression, and might
		contain smaller or larger sections that are directly taken
		from ISO's reference code.

		All changes to the ISO reference code herein are either
		copyrighted by Tord Jansson (tord.jansson@swipnet.se)
		or sublicensed to Tord Jansson by a third party.

	BladeEnc is free software; you can redistribute this file
	and/or modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.



	------------    CHANGES    ------------

	2000-11-22  Andre Piotrowski

	-	big fix: module - original dist10 error - l3_sb_sample has to be of type 'L3SBS' not 'SBS'.
	-	bug fix: codecInit() - don't reset fInit-huffman_read_flag - read_huffcodetab() is a memory eater!

	2000-12-11  ap

	-	reformatted
	-	speed up: use single buffer
*/

#include	<stdlib.h>
#include	<assert.h>

#include	"codec.h"










/*____ codecInit() ____________________________________________________________*/

CodecInitOut			*codecInit (encoder_flags_and_data* flags, CodecInitIn *psIn)
{
	int						j;

	/* Read psIn */

	flags->PartHoldersInitialized = 0;
	flags->loop_flags.xr34_s = (double (*)[3]) flags->loop_flags.xr34_l;

	switch (psIn->frequency)
	{
		case 48000:  flags->info.sampling_frequency = 1;  break;
	  	case 44100:  flags->info.sampling_frequency = 0;  break;
	  	case 32000:  flags->info.sampling_frequency = 2;  break;
	  	default   :  return FALSE;
	}

	switch (psIn->mode)
	{
		case  0:  flags->info.mode = MPG_MD_STEREO      ;  flags->info.mode_ext = 0;  break;
		case  2:  flags->info.mode = MPG_MD_DUAL_CHANNEL;  flags->info.mode_ext = 0;  break;
		case  3:  flags->info.mode = MPG_MD_MONO        ;  flags->info.mode_ext = 0;  break;
		default:  return FALSE;
	}

	j = 0;
	while (j < 15  &&  bitratex[1][j] != psIn->bitrate)
		j++;
	flags->info.bitrate_index    = j;

	flags->info.version 	      = 1;   /* Default: MPEG-1 */
	flags->info.emphasis 	      = psIn->emphasis;
	flags->info.extension 	      = psIn->fPrivate;
	flags->info.copyright 	      = psIn->fCopyright;
	flags->info.original 	      = psIn->fOriginal;
	flags->info.error_protection = psIn->fCRC;


/*_______ Static-fix _______________*/


	fInit_mdct_sub = 0;
	fInit_mdct = 0;

	fInit_fft = 0;

	fInit_iteration_loop = 0;
/*	DON'T DO THAT ALL THE TIME
	fInit_huffman_read_flag = 0;
*/

	fixStatic_loop(flags);

	flags->l3_side.main_data_begin = 0;
	fixStatic_reservoir(flags);


/*___________________________________*/

    psycho_anal_init (flags, psIn->frequency);
	initWindowFilterSubband ();
	initFormatBitstream (flags);

/*     clear buffers */
	memset ((char *) flags->l3_sb_sample, 0, sizeof(flags->l3_sb_sample));
    memset((char *) flags->buffer, 0, sizeof(flags->buffer));
/*    memset((char *) snr32, 0, sizeof(snr32));*/
#if ORG_BUFFERS
    memset((char *) flags->sam, 0, sizeof(flags->sam));
#endif



    flags->fr_ps.header 	  = &flags->info;
    flags->fr_ps.tab_num 	  = -1;   /* no table loaded */
    flags->fr_ps.alloc 	  = NULL;
    flags->fr_ps.actual_mode = flags->info.mode;
    flags->fr_ps.stereo 	  = (flags->info.mode == MPG_MD_MONO) ? 1 : 2;
	flags->fr_ps.sblimit 	  = SBLIMIT;
    flags->fr_ps.jsbound 	  = SBLIMIT;

    
    flags->stereo = flags->fr_ps.stereo;
    flags->error_protection = flags->info.error_protection;

    flags->avg_slots_per_frame =
		((double) SAMPLES_PER_FRAME / s_freq[1][flags->info.sampling_frequency]) *
		((double) bitratex[1][flags->info.bitrate_index] / 8.0);
    flags->whole_SpF = (int) flags->avg_slots_per_frame;
    flags->frac_SpF  = flags->avg_slots_per_frame - (double) flags->whole_SpF;
    flags->slot_lag  = -flags->frac_SpF;

/*    if (flags->frac_SpF == 0)
    	flags->info.padding = 0;
*/
	genNoisePowTab(flags);

/*________________________*/


	if( flags->stereo != 2 )
		flags->sOut.nSamples = SAMPLES_PER_FRAME;
	else
		flags->sOut.nSamples = SAMPLES_PER_FRAME*2;

	flags->sOut.bufferSize = 2048;

	return  &flags->sOut;			/* How many samples we want in each chunk... */
}





/*____ codecEncodeChunk() _____________________________________________________*/
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

unsigned int			codecEncodeChunk
(
	encoder_flags_and_data* flags,
	int						nSamples,
	short					*pSamples,
	char					*pDest
)
{
	double				xr[2][2][576];
	double				xr_dec[2][2][576];
	double				pe[2][2];
	int					l3_enc[2][2][576];
	III_psy_ratio		ratio;
	III_scalefac_t		scalefac;
	int							gr, ch;
	int							mean_bits, sideinfo_len;
	int							bitsPerFrame;
	int							j;

/*
	int					fFirst = TRUE;

	if (fFirst)
	{
		memset ((char *) &xr      , 0, sizeof(xr));
		memset ((char *) &xr_dec  , 0, sizeof(xr_dec));
		memset ((char *) &pe      , 0, sizeof(pe));
		memset ((char *) &l3_enc  , 0, sizeof(l3_enc));
		memset ((char *) &ratio   , 0, sizeof(ratio));
		memset ((char *) &flags->l3_side , 0, sizeof(flags->l3_side));
		memset ((char *) &scalefac, 0, sizeof(scalefac));

		fFirst = FALSE;
	}
*/


/* rebuffer audio */

#if ORG_BUFFERS
	rebuffer_audio (flags->buffer, pSamples, nSamples, flags->stereo);
#else
	rebuffer_audio (pSamples, flags->buffer, &flags->buffer_idx, nSamples, flags->stereo);
#endif


/* psychoacoustic model */

	for (gr = 0;  gr < 2;  gr++)
		for (ch = 0;  ch < flags->stereo;  ch++)
			psycho_anal
			( flags,
#if ORG_BUFFERS
				&flags->buffer[ch][gr*576],
				&flags->sam[ch][0],
#else
		    	flags->buffer[ch],
		    	(flags->buffer_idx+gr*576) & 2047,
#endif
				ch,
				3,
				/*snr32,*/
				&ratio.l[gr][ch][0],
				&ratio.s[gr][ch][0],
				&pe[gr][ch],
				&flags->l3_side.gr[gr].ch[ch].tt
			);


/* polyphase filtering */

	for (gr = 0;  gr < 2;  gr++)
	{
		int		gr_plus_1 = gr_idx[gr+1];

		for (ch = 0;  ch < flags->stereo;  ch++)
		{
			for (j = 0;  j < 18;  j++)
			{
				windowFilterSubband
				(
#if ORG_BUFFERS
					&flags->buffer[ch][gr*18*32+32*j],
					ch,
#else
					flags->buffer[ch],
					(flags->buffer_idx+768-480+gr*18*32+32*j) & 2047,
#endif
					flags->l3_sb_sample[ch][gr_plus_1][j]
				);
			}
		}
	}


/* apply mdct to the polyphase outputs */

	mdct_sub (flags, &flags->l3_sb_sample, xr, flags->stereo, &flags->l3_side, 2);

    float* pre_bend = flags->bends.mdct_pre_bend;
    float* post_bend = flags->bends.mdct_post_bend;
    for (int i = 0; i < 576; ++i) {
        pre_bend[i] = 0;
    }
    for (gr = 0; gr < 2; gr++) {
        for (ch = 0; ch < 2; ch++) {
            for (int i = 0; i < 576; ++i) {
                pre_bend[i] += xr[gr][ch][i];
            }
        }
    }
    for (int i = 0; i < 576; ++i) {
        pre_bend[i] /= 4;
    }


    int h_shift = flags->bends.mdct_post_h_shift;
    float v_shift = flags->bends.mdct_post_v_shift;
    float v;
    if (h_shift < 0) {
        for (gr = 0; gr < 2; gr++) {
            for (ch = 0; ch < 2; ch++) {
                for (int i = 0; i < 576; ++i) {
                    v = xr[gr][ch][(i+576-h_shift)%576];
                    xr[gr][ch][i] = apply_v_shift(v, v_shift);
                }
            }
        }
    } else {
        for (gr = 0; gr < 2; gr++) {
            for (ch = 0; ch < 2; ch++) {
                for (int i = 576-1; i >= 0; --i) {
                    v = xr[gr][ch][(i+576-h_shift)%576];
                    xr[gr][ch][i] = apply_v_shift(v, v_shift);
                }
            }
        }
    }

    float wet = flags->bends.mdct_feedback;
    float dry = 1 - wet;
    float m;
    for (int gr = 0; gr < 2; ++gr) {
        for (int ch = 0; ch < 2; ++ch) {
            for (int s = 0; s < 576; ++s) {
            	m = fabs(xr[gr][ch][s]);
                if (isnan(m) || isinf(m)) {
                    m = 0;
                }
                float fed = flags->bends.feedback_data[gr][ch][s];

                flags->bends.feedback_data[gr][ch][s] = (isnan(fed) || isinf(fed)) ? 0 :
                    dry * m + 
                    wet * flags->bends.feedback_data[gr][ch][s];

                xr[gr][ch][s] = (xr[gr][ch][s] > 0) ? 
                	flags->bends.feedback_data[gr][ch][s] : 
                	-flags->bends.feedback_data[gr][ch][s];

            }
        }
    }

    for (int i = 0; i < 576; ++i) {
        post_bend[i] = 0;
    }
    for (gr = 0; gr < 2; gr++) {
        for (ch = 0; ch < 2; ch++) {
            for (int i = 0; i < 576; ++i) {
                post_bend[i] += xr[gr][ch][i];
            }
        }
    }
    for (int i = 0; i < 576; ++i) {
        post_bend[i] /= 4;
    }



	flags->pEncodedOutput = pDest;
	flags->outputBit = 8;
	flags->pEncodedOutput[0] = 0;


	if (flags->frac_SpF != 0)
	{
		if (flags->slot_lag > (flags->frac_SpF-1.0))
		{
			flags->slot_lag -= flags->frac_SpF;
			flags->info.padding = 0;
		}
		else
		{
			flags->info.padding = 1;
			flags->slot_lag += (1-flags->frac_SpF);
		}
	}

	bitsPerFrame = 8 * flags->whole_SpF + (flags->info.padding * 8);


/* determine the mean bitrate for main data */

	sideinfo_len = 32;

	if (flags->stereo == 1)
		sideinfo_len += 136;
	else
		sideinfo_len += 256;

	if (flags->info.error_protection)
		sideinfo_len += 16;
	
	mean_bits = (bitsPerFrame - sideinfo_len) / 2;


/* bit and noise allocation */

	iteration_loop
	(
		flags,
		pe,
		xr,
		&ratio,
		&flags->l3_side,
		l3_enc,
		mean_bits,
		flags->stereo,
		xr_dec,
		&scalefac,
		&flags->fr_ps,
		0,
		bitsPerFrame
	);


/* write the frame to the bitstream */

	III_format_bitstream
	(
		flags,
		bitsPerFrame,
		&flags->fr_ps,
		l3_enc,
		&flags->l3_side,
		&scalefac,
		xr,
		NULL,
		0
	);


	return  flags->pEncodedOutput - pDest;
}





/*____ codecExit() ____________________________________________________________*/

unsigned int			codecExit (encoder_flags_and_data* flags, char *pDest)
{
	flags->pEncodedOutput = pDest;
	flags->outputBit = 8;
	flags->pEncodedOutput[0] = 0;

	psycho_anal_exit (flags);
	exitFormatBitstream (flags);
	III_FlushBitstream (flags);

	return flags->pEncodedOutput - pDest;
}





/*____ codecFlush() _________________________________________________________*/

unsigned int			codecFlush (encoder_flags_and_data* flags, char *pDest)
{
	flags->pEncodedOutput = pDest;
	flags->outputBit = 8;
	flags->pEncodedOutput[0] = 0;

	flushFrame (flags);

	flags->whole_SpF = (int) flags->avg_slots_per_frame;
	flags->frac_SpF  = flags->avg_slots_per_frame - (double) flags->whole_SpF;
	flags->slot_lag  = -flags->frac_SpF;

	flags->l3_side.main_data_begin = 0;
	fixStatic_reservoir (flags);

	return flags->pEncodedOutput - pDest;
}



