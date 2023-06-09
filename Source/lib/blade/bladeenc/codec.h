/*
			(c) Copyright 1998-2000 - Tord Jansson
			======================================

		This file is part of the BladeEnc MP3 Encoder, based on
		ISO's reference code for MPEG Layer 3 compression.

		This file doesn't contain any of the ISO reference code and
		is copyright Tord Jansson (tord.jansson@swipnet.se).

	BladeEnc is free software; you can redistribute this file
	and/or modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.



	------------    CHANGES    ------------

	2000-12-10  Andre Piotrowski

	-	reformatted
*/

#ifndef		__CODEC__
#define		__CODEC__



#include	"common.h"
#include	"l3psy.h"
#include	"mdct.h"
#include	"reservoir.h"
#include	"formatbitstream2.h"
#include	"l3bitstream.h"
#include	"loop.h"
#include	"encoder.h"

extern	int				fInit_fft;








extern	CodecInitOut	*codecInit (encoder_flags_and_data* flags, CodecInitIn *psInitData);

extern	unsigned int	codecEncodeChunk (encoder_flags_and_data* flags, int nSamples, short *pSamples, char *pOutput);

extern	unsigned int	codecExit (encoder_flags_and_data* flags, char *pOutput);

extern	unsigned int	codecFlush (encoder_flags_and_data* flags, char *pOutput);





#endif		/* __CODEC__ */
