/*
			(c) Copyright 1998, 1999 - Tord Jansson
			=======================================

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
*/


/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
 *
 * $Id: l3bitstream.h,v 1.1 1996/02/14 04:04:23 rowlands Exp $
 *
 * $Log: l3bitstream.h,v $
 * Revision 1.1  1996/02/14 04:04:23  rowlands
 * Initial revision
 *
 * Received from Mike Coleman
 **********************************************************************/

#ifndef		__L3BITSTREAM__
#define		__L3BITSTREAM__





#include "common.h"
#include "encoder.h"





void					III_format_bitstream
(
	int						bitsPerFrame,
	frame_params			*in_fr_ps,
	int						l3_enc[2][2][576],
	III_side_info_t			*l3_side,
	III_scalefac_t			*scalefac,
	double					(*xr)[2][576],
	char					*ancillary,
	int						anc_bits
);

void					III_FlushBitstream (void);

void					fixStatic_l3bitstream (void);





#endif		/* __L3BITSTREAM__ */
