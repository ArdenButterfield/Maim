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





typedef		struct
			{
				int						frequency;   /* 48000, 44100 and 32000 allowed. */
				int						mode;        /* 0 = Stereo, 2 = Dual Channel, 3 = Mono */
				int						bitrate;     
				int						emphasis;    /* 0 = None, 1 = 50/15 microsec, 3 = CCITT J.17 */
				int						fPrivate;               
				int						fCRC;
				int						fCopyright;
				int						fOriginal;
			}						CodecInitIn;


typedef		struct
			{
				int						nSamples;
				int						bufferSize;
			}						CodecInitOut;



extern	CodecInitOut	*codecInit (CodecInitIn *psInitData);

extern	unsigned int	codecEncodeChunk (int nSamples, short *pSamples, char *pOutput);

extern	unsigned int	codecExit (char *pOutput);

extern	unsigned int	codecFlush (char *pOutput);





#endif		/* __CODEC__ */
