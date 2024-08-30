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

	-	reformatted, redesigned
*/

#ifndef		__ENCODER__
#define		__ENCODER__





#define		ORG_BUFFERS				0   /* 1 = use original buffer concept */





/***********************************************************************
*
*  Encoder Definitions
*
***********************************************************************/





/***********************************************************************
*
*  Encoder Function Prototype Declarations
*
***********************************************************************/


void					initWindowFilterSubband (void);




#if ORG_BUFFERS

	void					rebuffer_audio
	(
		short					buffer[2][1152],
		short					*insamp,
		unsigned int			samples_read,
		int						stereo
	);
	void					windowFilterSubband
	(
		short					*pBuffer,
		int						ch,
		double					s[SBLIMIT]
	);

#else

	void					rebuffer_audio
	(
		const short				*insamp,
		FLOAT					buffer[2][2048],
		int						*buffer_idx,
		unsigned int			samples_read,
		int						stereo
	);
	void					windowFilterSubband
	(
		const FLOAT				*buffer,
		int						buffer_idx,
		double					s[SBLIMIT]
	);

#endif





#endif		/* __ENCODER__ */
