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



	------------    Changes    ------------

	2000-12-10  Andre Piotrowski

	-	reformatted, redesigned
	-	MDCT_CHANGE_LEVEL 5
*/

/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
 *
 * $Id: mdct.h,v 1.1 1996/02/14 04:04:23 rowlands Exp $
 *
 * $Log: mdct.h,v $
 * Revision 1.1  1996/02/14 04:04:23  rowlands
 * Initial revision
 *
 * Received from Mike Coleman
 **********************************************************************/

#ifndef		__MDCT__
#define		__MDCT__





#define MDCT_CHANGE_LEVEL 5










extern	int				fInit_mdct_sub;
extern	int				fInit_mdct;


extern	int				gr_idx[3];





void					mdct_sub
(
	encoder_flags_and_data* flags,
	L3SBS					*sb_sample,
	double					(*mdct_freq)[2][576],
	int						stereo,
	III_side_info_t			*l3_side,
	int						mode_gr
);



#if MDCT_CHANGE_LEVEL < 5

void					mdct
(
	double					*in,
	double					*out,
	int						block_type
);

#else

void					mdct
(
	double					inA[18][32],
	double					inB[18][32],
	int						band,
	double					*out,
	int						block_type
);

#endif





#endif		/* __MDCT__ */
