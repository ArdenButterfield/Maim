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

	-	reformatted
*/

/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
 *
 * $Id: reservoir.h,v 1.1 1996/02/14 04:04:23 rowlands Exp $
 *
 * $Log: reservoir.h,v $
 * Revision 1.1  1996/02/14 04:04:23  rowlands
 * Initial revision
 *
 * Received from Mike Coleman
 **********************************************************************/
/*
  Revision History:

  Date        Programmer                Comment
  ==========  ========================= ===============================
  1995/09/06  mc@fivebats.com           created

*/

#ifndef		__RESERVOIR__
#define		__RESERVOIR__





void					BladeResvFrameBegin
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps,
	III_side_info_t			*l3_side,
	int						mean_bits,
	int						frameLength
);

int						BladeResvMaxBits
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps,
	III_side_info_t			*l3_side,
	double					*pe,
	int						mean_bits
);

void					BladeResvAdjust
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps,
	gr_info					*cod_info,
	III_side_info_t			*l3_side,
	int						mean_bits
);

void					BladeResvFrameEnd
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps,
	III_side_info_t			*l3_side,
	int						mean_bits
);

void					fixStatic_reservoir (encoder_flags_and_data* flags);





#endif		/* __RESERVOIR__ */
