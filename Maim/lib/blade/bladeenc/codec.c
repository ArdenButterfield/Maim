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

#include	"common.h"
#include	"l3psy.h"
#include	"mdct.h"
#include	"reservoir.h"
#include	"formatbitstream2.h"
#include	"l3bitstream.h"
#include	"loop.h"
#include	"encoder.h"
#include	"codec.h"





extern	int				fInit_fft;





/************************************************************************/

#define	SAMPLES_PER_FRAME		1152

static	L3SBS					l3_sb_sample;

static	layer					info;



#if ORG_BUFFERS
static	short					buffer[2][1152];
/*	static	float					snr32[32]; */
static	short					sam[2][2048];
#else
static	FLOAT					buffer[2][2048];
static	int						buffer_idx;
#endif



static	int						whole_SpF;

static	double					frac_SpF, slot_lag;

static	int						stereo, error_protection;

static	III_side_info_t			l3_side;
static	CodecInitOut			sOut;

static	frame_params			fr_ps;



char					*pEncodedOutput;
int						outputBit;

volatile double			avg_slots_per_frame;





/*____ codecInit() ____________________________________________________________*/

CodecInitOut			*codecInit (CodecInitIn *psIn)
{
	int						j;

	/* Read psIn */

	switch (psIn->frequency)
	{
		case 48000:  info.sampling_frequency = 1;  break;
	  	case 44100:  info.sampling_frequency = 0;  break;
	  	case 32000:  info.sampling_frequency = 2;  break;
	  	default   :  return FALSE;
	}

	switch (psIn->mode)
	{
		case  0:  info.mode = MPG_MD_STEREO      ;  info.mode_ext = 0;  break;
		case  2:  info.mode = MPG_MD_DUAL_CHANNEL;  info.mode_ext = 0;  break;
		case  3:  info.mode = MPG_MD_MONO        ;  info.mode_ext = 0;  break;
		default:  return FALSE;
	}

	j = 0;
	while (j < 15  &&  bitratex[1][j] != psIn->bitrate)
		j++;
	info.bitrate_index    = j;

	info.version 	      = 1;   /* Default: MPEG-1 */
	info.emphasis 	      = psIn->emphasis;
	info.extension 	      = psIn->fPrivate;
	info.copyright 	      = psIn->fCopyright;
	info.original 	      = psIn->fOriginal;
	info.error_protection = psIn->fCRC;


/*_______ Static-fix _______________*/


	fInit_mdct_sub = 0;
	fInit_mdct = 0;

	fInit_fft = 0;

	fInit_iteration_loop = 0;
/*	DON'T DO THAT ALL THE TIME
	fInit_huffman_read_flag = 0;
*/

	fixStatic_loop();

	l3_side.main_data_begin = 0;
	fixStatic_reservoir();


/*___________________________________*/

    psycho_anal_init (psIn->frequency);
	initWindowFilterSubband ();
	initFormatBitstream ();

/*     clear buffers */
	memset ((char *) l3_sb_sample, 0, sizeof(l3_sb_sample));
    memset((char *) buffer, 0, sizeof(buffer));
/*    memset((char *) snr32, 0, sizeof(snr32));*/
#if ORG_BUFFERS
    memset((char *) sam, 0, sizeof(sam));
#endif



    fr_ps.header 	  = &info;
    fr_ps.tab_num 	  = -1;   /* no table loaded */
    fr_ps.alloc 	  = NULL;
    fr_ps.actual_mode = info.mode;
    fr_ps.stereo 	  = (info.mode == MPG_MD_MONO) ? 1 : 2;
	fr_ps.sblimit 	  = SBLIMIT;
    fr_ps.jsbound 	  = SBLIMIT;

    
    stereo = fr_ps.stereo;
    error_protection = info.error_protection;

    avg_slots_per_frame =
		((double) SAMPLES_PER_FRAME / s_freq[1][info.sampling_frequency]) *
		((double) bitratex[1][info.bitrate_index] / 8.0);
    whole_SpF = (int) avg_slots_per_frame;
    frac_SpF  = avg_slots_per_frame - (double) whole_SpF;
    slot_lag  = -frac_SpF;

/*    if (frac_SpF == 0)
    	info.padding = 0;
*/
	genNoisePowTab();

/*________________________*/


	if( stereo != 2 )
		sOut.nSamples = SAMPLES_PER_FRAME;
	else
		sOut.nSamples = SAMPLES_PER_FRAME*2;

	sOut.bufferSize = 2048;

	return  &sOut;			/* How many samples we want in each chunk... */
}





/*____ codecEncodeChunk() _____________________________________________________*/

unsigned int			codecEncodeChunk
(
	int						nSamples,
	short					*pSamples,
	char					*pDest
)
{
	static	double				xr[2][2][576];
	static	double				xr_dec[2][2][576];
	static	double				pe[2][2];
	static	int					l3_enc[2][2][576];
	static	III_psy_ratio		ratio;
	static	III_scalefac_t		scalefac;
	int							gr, ch;
	int							mean_bits, sideinfo_len;
	int							bitsPerFrame;
	int							j;

/*
	static	int					fFirst = TRUE;

	if (fFirst)
	{
		memset ((char *) &xr      , 0, sizeof(xr));
		memset ((char *) &xr_dec  , 0, sizeof(xr_dec));
		memset ((char *) &pe      , 0, sizeof(pe));
		memset ((char *) &l3_enc  , 0, sizeof(l3_enc));
		memset ((char *) &ratio   , 0, sizeof(ratio));
		memset ((char *) &l3_side , 0, sizeof(l3_side));
		memset ((char *) &scalefac, 0, sizeof(scalefac));

		fFirst = FALSE;
	}
*/


/* rebuffer audio */

#if ORG_BUFFERS
	rebuffer_audio (buffer, pSamples, nSamples, stereo);
#else
	rebuffer_audio (pSamples, buffer, &buffer_idx, nSamples, stereo);
#endif


/* psychoacoustic model */

	for (gr = 0;  gr < 2;  gr++)
		for (ch = 0;  ch < stereo;  ch++)
			psycho_anal
			(
#if ORG_BUFFERS
				&buffer[ch][gr*576],
				&sam[ch][0],
#else
		    	buffer[ch],
		    	(buffer_idx+gr*576) & 2047,
#endif
				ch,
				3,
				/*snr32,*/
				&ratio.l[gr][ch][0],
				&ratio.s[gr][ch][0],
				&pe[gr][ch],
				&l3_side.gr[gr].ch[ch].tt
			);


/* polyphase filtering */

	for (gr = 0;  gr < 2;  gr++)
	{
		int		gr_plus_1 = gr_idx[gr+1];

		for (ch = 0;  ch < stereo;  ch++)
		{
			for (j = 0;  j < 18;  j++)
			{
				windowFilterSubband
				(
#if ORG_BUFFERS
					&buffer[ch][gr*18*32+32*j],
					ch,
#else
					buffer[ch],
					(buffer_idx+768-480+gr*18*32+32*j) & 2047,
#endif
					l3_sb_sample[ch][gr_plus_1][j]
				);
			}
		}
	}


/* apply mdct to the polyphase outputs */

	mdct_sub (&l3_sb_sample, xr, stereo, &l3_side, 2);


	pEncodedOutput = pDest;
	outputBit = 8;
	pEncodedOutput[0] = 0;


	if (frac_SpF != 0)
	{
		if (slot_lag > (frac_SpF-1.0))
		{
			slot_lag -= frac_SpF;
			info.padding = 0;
		}
		else
		{
			info.padding = 1;
			slot_lag += (1-frac_SpF);
		}
	}

	bitsPerFrame = 8 * whole_SpF + (info.padding * 8);


/* determine the mean bitrate for main data */

	sideinfo_len = 32;

	if (stereo == 1)
		sideinfo_len += 136;
	else
		sideinfo_len += 256;

	if (info.error_protection)
		sideinfo_len += 16;

	mean_bits = (bitsPerFrame - sideinfo_len) / 2;


/* bit and noise allocation */

	iteration_loop
	(
		pe,
		xr,
		&ratio,
		&l3_side,
		l3_enc,
		mean_bits,
		stereo,
		xr_dec,
		&scalefac,
		&fr_ps,
		0,
		bitsPerFrame
	);


/* write the frame to the bitstream */

	III_format_bitstream
	(
		bitsPerFrame,
		&fr_ps,
		l3_enc,
		&l3_side,
		&scalefac,
		xr,
		NULL,
		0
	);


	return  pEncodedOutput - pDest;
}





/*____ codecExit() ____________________________________________________________*/

unsigned int			codecExit (char *pDest)
{
	pEncodedOutput = pDest;
	outputBit = 8;
	pEncodedOutput[0] = 0;

	psycho_anal_exit ();
	exitFormatBitstream ();
	III_FlushBitstream ();

	return pEncodedOutput - pDest;
}





/*____ codecFlush() _________________________________________________________*/

unsigned int			codecFlush (char *pDest)
{
	pEncodedOutput = pDest;
	outputBit = 8;
	pEncodedOutput[0] = 0;

	flushFrame ();

	whole_SpF = (int) avg_slots_per_frame;
	frac_SpF  = avg_slots_per_frame - (double) whole_SpF;
	slot_lag  = -frac_SpF;

	l3_side.main_data_begin = 0;
	fixStatic_reservoir ();

	return pEncodedOutput - pDest;
}



