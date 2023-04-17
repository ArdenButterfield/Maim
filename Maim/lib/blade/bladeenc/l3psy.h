/*
			(c) Copyright 1998-2001 - Tord Jansson
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



	------------    Changes    ------------

	2000-12-11  Andre Piotrowski

	-	reformatted

	2000-12-12  ap

	-	removed fft() prototype from "encoder.h" to this file
	-	removed the fft configuration switches from "subs.c" to this file
	-	implemented ORG_SHORT_CW_LIMIT

	2001-01-12  ap

	-	bug fix: include encoder.h for definition of ORG_BUFFERS
	-	clear some backward compatability flags for 0.93.10
*/

/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
 *
 * $Id: l3psy.h,v 1.1 1996/02/14 04:04:23 rowlands Exp $
 *

 * $Log: l3psy.h,v $
 * Revision 1.1  1996/02/14 04:04:23  rowlands
 * Initial revision
 *
 * Received from Mike Coleman
 **********************************************************************/

#ifndef		__L3PSY__
#define 	__L3PSY__



#include	"encoder.h"





/*  ========================================================================================  */
/*              keeping backward compatability                                                */
/*  ========================================================================================  */






void					psycho_anal
(
#if ORG_BUFFERS
	short int				*buffer,
	short int				savebuf[2048],
#else
	FLOAT					*buffer,
	int						buffer_idx,
#endif
	int						ch,
	int						lay,
/*	FLOAT					snr32[32], */
	double					ratio_d[SBMAX_l],
	double					ratio_ds[SBMAX_s][3],
	double					*pe,
	gr_info					*cod_info
);

void					psycho_anal_init (double sfreq);

void					psycho_anal_exit (void);





void					fft (FLOAT[], FLOAT[], FLOAT[], FLOAT[], int);



/*
	Switches for configuring the fft in "subs.c"
*/

/* Do we want to reorder the data? */
#define		REORDER_DATA        0   /* no need to do that */

/* To get the inverse fft, set NORMAL_FFT to 0 */
#define		NORMAL_FFT			1   /* si */

/* Try this to speed up the fft when using non-complex sequences */
#define		REAL_SEQUENCE		1   /* no complex values needed */

/* We assume, the real data gets prepacked externally */
#define		PREPACKED			1   /* yes */

/* No one needs all the phi values in layer 3 !!! */
#define		USED_VALUES_ONLY	1
#define		LONG_FFT_MIN_IDX	0
#define		LONG_FFT_MAX_IDX	5
#if ORG_SHORT_CW_LIMIT
#define		SHORT_FFT_MIN_IDX	2    /* (  6+2)/4     */
#define		SHORT_FFT_MAX_IDX	51   /* (206+2)/4 - 1 */
#else   /* according to ISO */
#define		SHORT_FFT_MIN_IDX	1    /*   6/4     */
#define		SHORT_FFT_MAX_IDX	50   /* 206/4 - 1 */
#endif





#endif		/* __L3PSY__ */
