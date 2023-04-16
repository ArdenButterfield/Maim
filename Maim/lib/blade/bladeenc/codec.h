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





/************************************************************************/

#define	SAMPLES_PER_FRAME		1152

typedef struct encoder_flags_and_data_struct {
	L3SBS					l3_sb_sample;

	layer					info;



#if ORG_BUFFERS
	short					buffer[2][1152];
	/*	static	float					snr32[32]; */
	short					sam[2][2048];
#else
	FLOAT					buffer[2][2048];
	int						buffer_idx;
#endif



	int						whole_SpF;

	double					frac_SpF, slot_lag;

	int						stereo, error_protection;

	III_side_info_t			l3_side;
	CodecInitOut			sOut;

	frame_params			fr_ps;



	char					*pEncodedOutput;
	int						outputBit;

	double			avg_slots_per_frame;
} encoder_flags_and_data;



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



extern	CodecInitOut	*codecInit (encoder_flags_and_data* flags, CodecInitIn *psInitData);

extern	unsigned int	codecEncodeChunk (encoder_flags_and_data* flags, int nSamples, short *pSamples, char *pOutput);

extern	unsigned int	codecExit (encoder_flags_and_data* flags, char *pOutput);

extern	unsigned int	codecFlush (encoder_flags_and_data* flags, char *pOutput);





#endif		/* __CODEC__ */
