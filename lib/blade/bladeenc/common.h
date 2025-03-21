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

#if defined(__cplusplus)
extern "C" {

#endif




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



#define		RING_BUFFER				1 // from l3psy.c

// loop.c 
/*	#define	SBLIMIT			32 */
#define	CBLIMIT			21

#define	SFB_LMAX		22
#define	SFB_SMAX		13


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

// encoder


/* Psychoacoustic Model 2 Definitions */

#define BLKSIZE         1024
#define HBLKSIZE        513
#define CBANDS          63


// l3psy


#define		ORG_NUMLINES_NORM		0   /* 0 = don't overwrite long numline entries with short numline entries */ // DON"T CHANGE THIS --awb
#define		ORG_BLOCK_SELECT		0   /* 0 = ISO draft paper states slightly different block selection */
#define		ORG_SHORT_CW_LIMIT		0   /* 0 = ISO draft paper says first element starts at 6/4 = 1 (not 2) */





/*	#define		CBANDS					  63 */
#define		CBANDS_s				42
#define		BLKSIZE_s				256
#define		HBLKSIZE_s				129
#define		TCBMAX_l				63
#define		TCBMAX_s				42
#define		SBMAX_l					21
#define		SBMAX_s					12



/*	#define		switch_pe				1800 */
#define		NORM_TYPE				0
#define		START_TYPE				1
#define		SHORT_TYPE				2
#define		STOP_TYPE				3



/************************************************************************/

#define	SAMPLES_PER_FRAME		1152

#define		MAX_CHANNELS			2
#define		MAX_GRANULES			2


typedef		double					L3SBS[2][3][18][SBLIMIT];   /* [gr][ch] */

typedef		struct
			{
				unsigned int			value;
				int						length;
			}					BitHolderElement;


typedef		struct
			{
				int						max_elements;
				int						nrEntries;
				BitHolderElement		*element;
			}					BitHolder;


typedef		struct BF_FrameData
			{
				int						frameLength;
				int						nGranules;
				int						nChannels;
				BitHolder				header;
				BitHolder				frameSI;
				BitHolder				   channelSI[MAX_CHANNELS];
				BitHolder				  spectrumSI[MAX_GRANULES][MAX_CHANNELS];
				BitHolder				scaleFactors[MAX_GRANULES][MAX_CHANNELS];
				BitHolder				   codedData[MAX_GRANULES][MAX_CHANNELS];
				BitHolder				userSpectrum[MAX_GRANULES][MAX_CHANNELS];
				BitHolder				userFrameData;
			}					BF_FrameData;


typedef		struct BF_FrameResults
			{
				int						SILength;
				int						mainDataLength;
				int						nextBackPtr;
			}					BF_FrameResults;


typedef		struct HeaderDef
			{
				int						size;
				int						frameSize;
				char					data[128];
				struct HeaderDef		*pNext;
			}						Header;


typedef struct loop_flags_and_data_struct {
		int						gr;   /* the current granule */
	int						ch;   /* the current channel */



	III_side_info_t			*side_info;   /* the current side information */
	gr_info					*cod_info;    /* the current coding information */



	double			   *xr_org_l;             /* the initial magnitudes of the spectral values */
	double				  xr34_l[576];        /* the magnitudes powered by 3/4 */
	int					   *ix_l;             /* quantized values */

	double				energy_l[SFB_LMAX];
	double				  xmin_l[SFB_LMAX];   /* the allowed distortion of the scalefactor band */
	double				  xfsf_l[SFB_LMAX];   /* the current distortion of the scalefactor band */
	int					expo16_l[SFB_LMAX];   /* sixteen times the scale factor band exponent */
	int				 *scalefac_l;             /* the current scale factors */
	int			   *scalefac_0_l;             /* scale factors for first granule */

	double			  (*xr_org_s)[3];         /* some short block versions */
	double				(*xr34_s)[3];// = (double (*)[3]) xr34_l;
	int					  (*ix_s)[3];

	double				energy_s[SFB_SMAX][3];
	double				  xmin_s[SFB_SMAX][3];
	double				  xfsf_s[SFB_SMAX][3];
	int					expo16_s[SFB_SMAX][3];
	int				(*scalefac_s)[3];



	int						max_used_sfb_l;
	int						min_used_sfb_s;

	int						end_sfb_l;
	int						end_sfb_s;



	double				    xmax_l[SFB_LMAX];		/* The initial (absolute) maximum magnitude */
	int				   xmax_line_l[SFB_LMAX];		/* of the long bands and their line indices */

	double				    xmax_s[SFB_SMAX][3];	/* Guess ... */
	int				   xmax_line_s[SFB_SMAX][3];



	int						mark_idx_l;				/* speed up - partial quantizing */
	int						mark_tab_l[SFB_LMAX];	/* changed sfb-s                 */ 

	int						mark_idx_s;
	int						mark_tab_s[SFB_SMAX*3*2];	/* changed (sfb,b)-s         */

	int						lo_quant_l [SFB_LMAX];
	int						hi_quant_l [SBMAX_l];

	int						lo_quant_s [SFB_SMAX][3];
	int						hi_quant_s [SFB_SMAX][3];

	int						the_lo_quant;
	int						the_hi_quant;


} loop_flags_and_data;

typedef struct blade_bend_flags_struct {
    float error;

	float butterfly_bubu;
    float butterfly_bubd;
    float butterfly_bdbu;
    float butterfly_bdbd;

    // mdct band step flags
    int mdct_band_step;
    int mdct_invert;

    int mdct_post_h_shift;
    float mdct_post_v_shift;

    int mdct_window_increment;
    int mdct_samp_increment; // Turned off with #define in newmdct. It was buggy and sounded bad.

    float bitrate_squish;

    int mdct_band_reassignments[32];

    float psychoanal_energy[22];
    float psychoanal_threshold[22];
    // we're only plotting the long block bands, for now.

    float mdct_pre_bend[576];
    float mdct_post_bend[576];

    float threshold_bias[22];
    // Likewise, we are currently only biasing the long blocks.

    float freq_gate;

    float mdct_feedback;
    int prev_block_long;

    int in_short_block;
    float feedback_data[2][2][576];
} blade_bend_flags;

typedef struct encoder_flags_and_data_struct {
	blade_bend_flags bends;

	CodecInitIn codec_data;

	// reservoir
	int				ResvSize;// = 0;   /* in bits */
	int				ResvMax;//  = 0;   /* in bits */

	loop_flags_and_data 	loop_flags;

	// codec
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

	int						error_protection;

	III_side_info_t			l3_side;
	CodecInitOut			sOut;

	frame_params			fr_ps;



	char					*pEncodedOutput;
	int						outputBit;

	double			avg_slots_per_frame;

	// l3spy

		int				new_, old, oldest;
	int				flush, sync_flush, syncsize;

#if RING_BUFFER==1
	int				savebuf_start_idx[2];
#endif




	double			*minval, *qthr_l;
	double			*qthr_s, *SNR_s;
	int				*cbw_l, *bu_l, *bo_l;
	int				*cbw_s, *bu_s, *bo_s;
	double			*w1_l, *w2_l;
	double			*w1_s, *w2_s;



	int				cbmax_l, cbmax_s;
	int				*numlines_l;
	int				*numlines_s;

						/* the non-zero entries of norm_l[i] * s3_l[i][j] */
	FLOAT			normed_s3_l [900];   /* a bit more space than needed [799|855|735] */
	int				lo_s3_l     [CBANDS];
	int				hi_s3_l		[CBANDS];

	FLOAT			normed_s3_s [500];   /* a bit more space than needed [445|395|378] */
	int				lo_s3_s     [CBANDS_s];
	int				hi_s3_s		[CBANDS_s];






/* Scale Factor Bands */
	int				blocktype_old[2];



	double			nb_1        [2][CBANDS];
	double			nb_2        [2][CBANDS];

	double			cw          [HBLKSIZE];

	FLOAT			window      [BLKSIZE];
	FLOAT			r			[2][2][6];
	FLOAT			phi_sav		[2][2][6];

	FLOAT			window_s    [BLKSIZE_s];

	double			ratio       [2][SBMAX_l];
	double			ratio_s     [2][SBMAX_s][3];

	// l3bitstream
	int				stereo;
	frame_params	*fr_ps_bitstream;

	int				PartHoldersInitialized;



	BitHolder		       *headerPH;
	BitHolder		      *frameSIPH;
	BitHolder		    *channelSIPH[MAX_CHANNELS];
	BitHolder		   *spectrumSIPH[MAX_GRANULES][MAX_CHANNELS];
	BitHolder		 *scaleFactorsPH[MAX_GRANULES][MAX_CHANNELS];
	BitHolder		    *codedDataPH[MAX_GRANULES][MAX_CHANNELS];
	BitHolder		 *userSpectrumPH[MAX_GRANULES][MAX_CHANNELS];
	BitHolder		*userFrameDataPH;



	BF_FrameData	sFrameData;
	BF_FrameResults	sFrameResults;


	// formatbitstream2
	int				BitsRemaining;
	Header			*pHeaderChain;
	Header			*pFreeHeaderChain;

    // mdct
    int				gr_idx[3];

} encoder_flags_and_data;

#if defined(__cplusplus)
}
#endif

#endif		/* __COMMON__ */
