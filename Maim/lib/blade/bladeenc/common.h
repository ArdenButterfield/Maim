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

	2000-02-07  Andre Piotrowski

	-	reformatted
*/

#ifndef		__COMMON__
#define		__COMMON__





/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/

#include	<stdio.h>
#include	<string.h>
#include	<math.h>

#include	"tables.h"





#define		MIN(A, B)				((A) < (B) ? (A) : (B))
#define		MAX(A, B)				((A) > (B) ? (A) : (B))





#ifndef EXIT_SUCCESS
#define		EXIT_SUCCESS			0
#endif

#ifndef EXIT_FAILURE
#define		EXIT_FAILURE			1
#endif





/***********************************************************************
*
*  Global Definitions
*
***********************************************************************/

/* General Definitions */

#define		FLOAT					float
#ifndef FALSE
#define		FALSE					0
#endif

#ifndef TRUE
#define		TRUE					1
#endif

#define		NULL_CHAR				'\0'

#define		MAX_U_32_NUM			0xFFFFFFFF

#ifndef PI
#define		PI						3.14159265358979
#endif
#define		PI4						PI/4
#define		PI64					PI/64

#define		LN_TO_LOG10				0.2302585093

#define		VOL_REF_NUM				0
#define		MPEG_AUDIO_ID			1
#define		MPEG_PHASE2_LSF			0	/* 1995-07-11 SHN */

#if 0
#define		MONO					1
#define		STEREO					2
#endif

#define		BITS_IN_A_BYTE			8
#define		WORD					16
#define		MAX_NAME_SIZE			81
#define		SBLIMIT					32
#define		SSLIMIT					18
#define		FFT_SIZE				1024
#define		HAN_SIZE				512
#define		SCALE_BLOCK				12
#define		SCALE_RANGE				64
#define		SCALE					32768
#define		CRC16_POLYNOMIAL		0x8005


/* MPEG Header Definitions - Mode Values */

#define		MPG_MD_STEREO			0
#define		MPG_MD_DUAL_CHANNEL		2
#define		MPG_MD_MONO				3


/* Mode Extention */

#define		MPG_MD_LR_LR			0
#define		MPG_MD_LR_I				1
#define		MPG_MD_MS_LR			2
#define		MPG_MD_MS_I				3


/* "bit_stream.h" Definitions */

#define		MINIMUM					4    /* Minimum size of the buffer in bytes */
#define		MAX_LENGTH				32   /* Maximum length of word written or
							                read from bit stream */

#define		READ_MODE				0
#define		WRITE_MODE				1
#define		ALIGNING				8



#define		BUFFER_SIZE				4096





/***********************************************************************
*
*  Global Type Definitions
*
***********************************************************************/


/* Structure for Reading Layer II Allocation Tables from File */

typedef		struct
			{
				unsigned int			steps;
				unsigned int			bits;
				unsigned int			group;
				unsigned int			quant;
			} sb_alloc, *alloc_ptr;

typedef		sb_alloc				al_table[SBLIMIT][16];


/* Header Information Structure */

typedef		struct
			{
				int						version;
				int						error_protection;
				int						bitrate_index;
				int						sampling_frequency;
				int						padding;
				int						extension;
				int						mode;
				int						mode_ext;
				int						copyright;
				int						original;
				int						emphasis;
/*				int						freq; */
			} layer, *the_layer;


/* Parent Structure Interpreting some Frame Parameters in Header */

typedef		struct
			{
				layer					*header;       /* raw header information */
				int						actual_mode;   /* when writing IS, may forget if 0 chs */
				al_table				*alloc;        /* bit allocation table read in */
				int						tab_num;       /* number of table as loaded */
				int						stereo;        /* 1 for mono, 2 for stereo */
				int						jsbound;       /* first band of joint stereo coding */
				int						sblimit;       /* total number of sub bands */
			} frame_params;



enum byte_order { order_unknown, order_bigEndian, order_littleEndian };
extern enum byte_order NativeByteOrder;


/* "bit_stream.h" Type Definitions */

typedef		struct bit_stream_struc
			{
				FILE					*pt;            /* pointer to bit stream device */
				unsigned char			*buf;           /* bit stream buffer */
				int						buf_size;       /* size of buffer (in number of bytes) */
				int						totbit;         /* bit counter of bit stream */
				int						buf_byte_idx;   /* pointer to top byte in buffer */
				int						buf_bit_idx;    /* pointer to top bit of top byte in buffer */
				int						mode;           /* bit stream open in read or write mode */
				int						eob;            /* end of buffer index */
				int						eobs;           /* end of bit stream flag */
				char					format;         /* format of file in rd mode (BINARY/ASCII) */
			} Bit_stream_struc;





#include	"l3side.h"

/***********************************************************************
*
*  Global Variable External Declarations
*
***********************************************************************/

/*
extern	char			*mode_names[4]; 
extern	char			*layer_names[3]; 
extern	char			*version_names[2];
*/
extern	double			s_freq[2][4];
extern	int				bitratex[2][15];





/***********************************************************************
*
*  Global Function Prototype Declarations
*
***********************************************************************/

/* The following functions are in the file "common.c" */

extern	void			*mem_alloc (unsigned int block, char *item);
extern	void			mem_free (void **ptr_addr);





#endif		/* __COMMON__ */
