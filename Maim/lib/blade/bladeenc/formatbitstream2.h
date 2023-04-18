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

	2000-12-15  Andre Piotrowski

	-	reformatted
*/

#ifndef		__FORMATBITSTREAM2__
#define		__FORMATBITSTREAM2__










/*____ Structure Definitions ________________________________________________*/






/*____ Function Prototypes __________________________________________________*/

BitHolder				*initBitHolder (BitHolder *wp, int elements);
void					exitBitHolder (BitHolder *wp);
void					addBits (BitHolder *wp, unsigned int value, int length);
void					writeFrame (encoder_flags_and_data* flags, BF_FrameData *psFrame, BF_FrameResults *results);
void					initFormatBitstream (encoder_flags_and_data* flags);
void					exitFormatBitstream (encoder_flags_and_data* flags);
void					flushFrame (encoder_flags_and_data* flags);





#endif   /* __FORMATBITSTREAM2__ */
