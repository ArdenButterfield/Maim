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



	------------    Changes    ------------

	2000-12-11  Andre Piotrowski

	-	reformatted
*/

#ifndef		__SAMPLEIN__
#define		__SAMPLEIN__





typedef		enum FileType
			{
				WAV,
				AIFF,
				RAW
			} SI_FileType;

typedef		enum SampleType
			{
				STEREO,
				INVERSE_STEREO,
				DOWNMIX_MONO,
				LEFT_CHANNEL_MONO,
				RIGHT_CHANNEL_MONO
			} SI_OutFormat;

typedef		struct SplInDef
			{
				/* These may be set from the outside after openInput */

				SI_OutFormat			outputType;
				int						outputFreq;

				/* These may NOT be set from the outside */	
				
				unsigned int			length;        /* Length in samples */
				int						errcode;					
				FILE					* fp;
				unsigned int			samplesLeft;

				/* Input format. Output is always 16-bit signed. */

				int						freq;          /* Hz */
				int						nChannels;     /* TRUE = Read sample is in stereo */
				int						channelBits;   /* Bits per channel of sample (normally 8 or 16) */
				int						sampleBits;    /* bits * nChannels */
				int						fSign;         /* Signed/unsigned */
				int						byteorder;     /* LITTLE_ENDIAN or BIG_ENDIAN */
				SI_FileType				filetype;      /* WAV or AIFF */

				/* The following is only used by some formats */

				char					preReadBuffer[256];
				int						nPreReadBytes;
				
			} SI_Stream;





int						openInput (SI_Stream *pStream, char *pFileName);
int						readSamples (SI_Stream *pStream, unsigned int nSamples, short *wpSamples);
int						closeInput (SI_Stream *pStream);





#endif		/* __SAMPLEIN__ */
