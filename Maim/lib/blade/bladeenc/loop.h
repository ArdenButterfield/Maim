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

	2000-12-11  Andre Piotrowski

	-	reformatted, redesigned
*/

/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
 *
 * $Id: loop.h,v 1.1 1996/02/14 04:04:23 rowlands Exp $
 *
 * $Log: loop.h,v $
 * Revision 1.1  1996/02/14 04:04:23  rowlands
 * Initial revision
 *
 * Received from Mike Coleman
 **********************************************************************/

/**********************************************************************
 *   date   programmers                comment                        *
 * 25. 6.92  Toshiyuki Ishino          Ver 1.0                        *
 * 29.10.92  Masahiro Iwadare          Ver 2.0                        *
 * 17. 4.93  Masahiro Iwadare          Updated for IS Modification    *
 *                                                                    *
 *********************************************************************/

#ifndef		__LOOP__
#define		__LOOP__




#include "common.h"
#include "l3side.h"









extern	int				fInit_iteration_loop;
/* NOT NEEDED
extern	int				fInit_huffman_read_flag;
*/


extern	int				blade_pretab[];


struct	scalefac_struct
{
	int l[23];
	int s[14];
};

extern	struct scalefac_struct		blade_sfBandIndex[];  /* Table B.8 -- in loop.c */





void					iteration_loop
(
	encoder_flags_and_data* flags,
	double					pe[][2],
	double					xr_org[2][2][576],
	III_psy_ratio			*ratio,
	III_side_info_t			*l3_side,
	int						l3_enc[2][2][576],
	int						mean_bits,
	int						stereo,
	double					xr_dec[2][2][576],
	III_scalefac_t			*scalefac,
	frame_params			*fr_ps,
	int						ancillary_pad,
	int						bitsPerFrame
);

int						my_nint (double in);

void					fixStatic_loop (encoder_flags_and_data* flags);

void					genNoisePowTab (encoder_flags_and_data* flags);





#endif		/* __LOOP__ */
