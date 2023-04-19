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

	2000-11-10  Andre Piotrowski

	-	reformatted, used 'static' functions, global variables, less parameters

	2000-11-11  ap

	-	a lot of functions redesigned to let the bladetable become superfluous:
			genNoisePowTab(), iteration_loop(), calc_noise(), preemphasis(), amp_scalefac_bands()
	-	bladTabValue() replaced by cutting_crew()

	2000-11-22  ap

	-	bug fix:  module - reset fInit_huffman_read_flag

	2000-11-28  ap
	-	speed up	:  implemented partial quantizing

	2000-11-30  ap
	-	speed up	:  implemented faster Huffman coding
	-	integration	:  improved (optional) Huffman coding
							[choosing better tables]
	-	integration	:  improved (optional) binary search
							[in fact, this is a BUG FIX (original dist10 bug)]

	2000-12-02  ap
	-	bug fix		:  original dist10's outer_loop could cause an endless loop (huff_bits <= 0).
	-	speed up	:  faster part2_length/scale_bitcount calculation
	-	integration	:  improved (optional) scalefactor compression
							[smart preflag switching and choosing best compression]

	2000-12-03  ap
	-	integration	:  improved (optional) quantanf algorithm
							[calculating exact quantizer step boundaries]
	-	integration	:  improved (optional) preemphasing/amplifying algorithm
							[preemphase only if iteration==1, according to ISO]
	-	integration	:	improved (optional) outer_loop algorithm
							[amplify the bands, marked as "should be amplified"]

	2000-12-10  ap
	-	definitely killed SCFSI

	2001-01-12  ap

	-	use some explicit type casting to avoid compiler warnings
	-	clear some backward compatability flags for 0.93.10

	2001-04-07  ap
	-	implemented flag CHECK_TJ_OVERFLOW to check for huffman table overflow

	2001-05-02  ap
	-	bug fix: outer_loop() - correct handling of flag tjBitOverflow2
*/





/*  ========================================================================================  */
/*              keeping backward compatability                                                */
/*  ========================================================================================  */

#define		ORG_HUFFMAN_CODING		0   /* 0 = use better Huffman tables for shorter code */
#define		ORG_BINARY_SEARCH		0   /* 0 = use a correct implemented binary search */
#define		ORG_QUANTANF_INIT		0   /* 0 = use better quantization start value */ // DON"T CHANGE THIS ONE
#define		ORG_PREEMPHASING		0   /* 0 = use a more ISO-like preemphasing algorithm */
#define		ORG_SCF_COMPRESS		0   /* 0 = choose better scalefactor compression tables and smart preemphasing */
#define		ORG_OUTER_LOOP			0   /* 0 = differ between marked as "been amplified" and "should be amplified" */
#define		ORG_HIGHEST_SFB			1   /* 0 = cut off highest frequencies (in last scale factor band) */

#define		CHECK_TJ_OVERFLOW		1   /* 1 = check for huffman table overflow */





#define		infinity				99999999





#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>
#include	<assert.h>

#include	"system.h"
#include	"common.h"

#include	"l3side.h"
#include	"l3psy.h"
#include	"huffman.h"
#include	"l3bitstream.h"
#include	"reservoir.h"
#include	"loop.h"
#include	"loop-pvt.h"





#if ORG_HUFFMAN_CODING
static	void			tiny_single_Huffman_2   /* Escape tables */
(
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 15... */
	unsigned				*choice,
	unsigned				*sum
);
#endif



static	int				amplify_short (encoder_flags_and_data* flags);
static	int				amplify_long
(
	encoder_flags_and_data* flags,
	int						iteration
);





int						my_nint (double in)
{

	if (in < 0)
		return (int)(in - 0.5);
	else
		return (int)(in + 0.5);
}





/*
	Here are MPEG1 Table B.8 and MPEG2 Table B.1
	-- Layer III scalefactor bands.
	Index into this using a method such as:
	idx  = fr_ps->header->sampling_frequency
	       + (fr_ps->header->version * 3)
*/

struct scalefac_struct blade_sfBandIndex[3] =
{
	{ /* Table B.8.b: 44.1 kHz */
		{0,4,8,12,16,20,24,30,36,44,52,62,74,90,110,134,162,196,238,288,342,418,576},
		{0,4,8,12,16,22,30,40,52,66,84,106,136,192}
	},
	{ /* Table B.8.c: 48 kHz */
		{0,4,8,12,16,20,24,30,36,42,50,60,72,88,106,128,156,190,230,276,330,384,576},
		{0,4,8,12,16,22,28,38,50,64,80,100,126,192}
	},
	{ /* Table B.8.a: 32 kHz */
		{0,4,8,12,16,20,24,30,36,44,54,66,82,102,126,156,194,240,296,364,448,550,576},
		{0,4,8,12,16,22,30,42,58,78,104,138,180,192}
	}
};



/*
	The following table is used to implement the scalefactor
	partitioning for MPEG2 as described in section
	2.4.3.2 of the IS. The indexing corresponds to the
	way the tables are presented in the IS:

	[table_number][row_in_table][column of nr_of_sfb]
*/

static unsigned nr_of_sfb_block[6][3][4] =
{
	{
		{ 6, 5, 5, 5},
		{ 9, 9, 9, 9},
		{ 6, 9, 9, 9}
	},
	{
		{6, 5,  7, 3},
		{9, 9, 12, 6},
		{6, 9, 12, 6}
	},
	{
		{11,10, 0, 0},
		{18,18, 0, 0},
		{15,18, 0, 0}
	},
	{
		{ 7, 7, 7, 0},
		{12,12,12, 0},
		{ 6,15,12, 0}
	},
	{
		{ 6, 6, 6, 3},
		{12, 9, 9, 6},
		{ 6,12, 9, 6}
	},
	{
		{ 8, 8, 5, 0},
		{15,12, 9, 0},
		{ 6,18, 9, 0}
	}
};



/* Table B.6: layer3 preemphasis */
int  blade_pretab[21] =
{
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 
	1, 2, 2, 3, 3, 3, 2
};



/* This is the scfsi_band table from 2.4.2.7 of the IS */
int scfsi_band_long[5] = { 0, 6, 11, 16, 21 };



int						*scalefac_band_long;
int						*scalefac_band_short;

int						fInit_iteration_loop;

#if ORG_BINARY_SEARCH || ORG_QUANTANF_INIT || CHECK_TJ_OVERFLOW
int						tjBitOverflow2;
#endif


/* We need this initialization here since some compilers chokes on the
   the declaration if it constains an initialization that point directly
   into a struct.
*/

void fixStatic_loop( encoder_flags_and_data* flags )
{
	scalefac_band_long  = &blade_sfBandIndex[0].l[0];
	scalefac_band_short = &blade_sfBandIndex[0].s[0];
}





/*  ========================================================================  */
/*              generating the power tables                                   */
/*  ========================================================================  */



#define	BOT						200
#define POW216_MAX				(BOT/2 * 16)
#define POW216_MIN				(-25 * 16)



static	double					pow216_space[POW216_MAX-POW216_MIN+1];
static	double					*pow216 = pow216_space - POW216_MIN;
static	double					noisePowTab[8191+15];



void genNoisePowTab (encoder_flags_and_data* data)
{
	int	i;

	for (i = POW216_MIN;  i <= POW216_MAX;  i++)
		pow216[i] = pow (2.0, (double)i/16.0);

	for (i = 0;  i < 8191+15;  i++)
		noisePowTab[i] = pow (i, 4.0/3.0);
}





/*  ========================================================================  */
/*              static variables                                              */
/*  ========================================================================  */


// static	int						gr;   /* the current granule */
// static	int						ch;   /* the current channel */



// static	III_side_info_t			*side_info;   /* the current side information */
// static	gr_info					*cod_info;    /* the current coding information */



// static	double			   *xr_org_l;             /* the initial magnitudes of the spectral values */
// static	double				  xr34_l[576];        /* the magnitudes powered by 3/4 */
// static	int					   *ix_l;             /* quantized values */

// static	double				energy_l[SFB_LMAX];
// static	double				  xmin_l[SFB_LMAX];   /* the allowed distortion of the scalefactor band */
// static	double				  xfsf_l[SFB_LMAX];   /* the current distortion of the scalefactor band */
// static	int					expo16_l[SFB_LMAX];   /* sixteen times the scale factor band exponent */
// static	int				 *scalefac_l;             /* the current scale factors */
// static	int			   *scalefac_0_l;             /* scale factors for first granule */

// static	double			  (*xr_org_s)[3];         /* some short block versions */
// static	double				(*xr34_s)[3] = (double (*)[3]) xr34_l;
// static	int					  (*ix_s)[3];

// static	double				energy_s[SFB_SMAX][3];
// static	double				  xmin_s[SFB_SMAX][3];
// static	double				  xfsf_s[SFB_SMAX][3];
// static	int					expo16_s[SFB_SMAX][3];
// static	int				(*scalefac_s)[3];



// static	int						max_used_sfb_l;
// static	int						min_used_sfb_s;

// static	int						end_sfb_l;
// static	int						end_sfb_s;



// static	double				    xmax_l[SFB_LMAX];		/* The initial (absolute) maximum magnitude */
// static	int				   xmax_line_l[SFB_LMAX];		/* of the long bands and their line indices */

// static	double				    xmax_s[SFB_SMAX][3];	/* Guess ... */
// static	int				   xmax_line_s[SFB_SMAX][3];



// static	int						mark_idx_l;				/* speed up - partial quantizing */
// static	int						mark_tab_l[SFB_LMAX];	/* changed sfb-s                 */ 

// static	int						mark_idx_s;
// static	int						mark_tab_s[SFB_SMAX*3*2];	/* changed (sfb,b)-s         */

#if !ORG_QUANTANF_INIT

// static	int						lo_quant_l [SFB_LMAX];
// static	int						hi_quant_l [SBMAX_l];

// static	int						lo_quant_s [SFB_SMAX][3];
// static	int						hi_quant_s [SFB_SMAX][3];

// static	int						the_lo_quant;
// static	int						the_hi_quant;

// numerical constants: these can stay
static	double					log_2, cc, dd;

#endif






/*  ========================================================================  */
/*              iteration_loop                                                */
/*  ========================================================================  */

void					iteration_loop
(
	encoder_flags_and_data* flags,
	double					pe[][2],
	double					xr_org[2][2][576],
	III_psy_ratio			*ratio,
	III_side_info_t			*l3_side,
	int						l3_enc[2][2][576],
	int						mean_bits,
	int						stereo,
	double					xr_dec[2][2][576],
	III_scalefac_t			*scalefac,
	frame_params			*fr_ps,
	int						ancillary_pad,
	int						bitsPerFrame
)
{
	int						max_bits;
	int						i, sfb, b, scfsi_band;
	int						mode_gr;

	int						*main_data_begin;
	layer					*info;

	int						start, end;

	double					total_energy, temp, x;



	flags->loop_flags.side_info = l3_side;

	main_data_begin = &(flags->loop_flags.side_info)->main_data_begin;
	info = fr_ps->header;


	(flags->loop_flags.side_info)->resvDrain = 0;

	if (!fInit_iteration_loop)
	{
		*main_data_begin = 0;
		fInit_iteration_loop = 1;

#if !ORG_QUANTANF_INIT
		log_2 = log(2.0);
		cc = 4.0/3.0 * log(8205.0 - 0.5 + 0.0946) / log_2;
		dd = 4.0/3.0 * log(   1.0 - 0.5 + 0.0946) / log_2;
#endif
	}
	mode_gr = 2;

	scalefac_band_long  = &blade_sfBandIndex[info->sampling_frequency].l[0];
	scalefac_band_short = &blade_sfBandIndex[info->sampling_frequency].s[0];


	ResvFrameBegin (flags, fr_ps, (flags->loop_flags.side_info), mean_bits, bitsPerFrame);

	for (flags->loop_flags.gr = 0;  flags->loop_flags.gr < mode_gr; flags->loop_flags.gr++)
	{
		for (flags->loop_flags.ch = 0;  flags->loop_flags.ch < stereo;  flags->loop_flags.ch++)
		{
			flags->loop_flags.xr_org_l = xr_org[flags->loop_flags.gr][flags->loop_flags.ch];
			flags->loop_flags.xr_org_s = (double (*)[3]) flags->loop_flags.xr_org_l;

			flags->loop_flags.ix_l = l3_enc[flags->loop_flags.gr][flags->loop_flags.ch];
			flags->loop_flags.ix_s = (int (*)[3]) flags->loop_flags.ix_l;

			(flags->loop_flags.cod_info) = &(flags->loop_flags.side_info)->gr[flags->loop_flags.gr].ch[flags->loop_flags.ch].tt;

			flags->loop_flags.scalefac_l = scalefac->l[flags->loop_flags.gr][flags->loop_flags.ch];  flags->loop_flags.scalefac_0_l = scalefac->l[0][flags->loop_flags.ch];
			flags->loop_flags.scalefac_s = scalefac->s[flags->loop_flags.gr][flags->loop_flags.ch];



			/* reset of iteration variables */

			for (scfsi_band = 0;  scfsi_band < 4;  scfsi_band++)
				(flags->loop_flags.cod_info)->slen[scfsi_band] = 0;

			flags->loop_flags.cod_info->sfb_partition_table = &nr_of_sfb_block[0][0][0];
			flags->loop_flags.cod_info->part2_3_length      = 0;
			flags->loop_flags.cod_info->big_values          = 0;
			flags->loop_flags.cod_info->count1              = 0;
			flags->loop_flags.cod_info->scalefac_compress   = 0;
			flags->loop_flags.cod_info->table_select[0]     = 0;
			flags->loop_flags.cod_info->table_select[1]     = 0;
			flags->loop_flags.cod_info->table_select[2]     = 0;
			flags->loop_flags.cod_info->subblock_gain[0]    = 0;
			flags->loop_flags.cod_info->subblock_gain[1]    = 0;
			flags->loop_flags.cod_info->subblock_gain[2]    = 0;
			flags->loop_flags.cod_info->region0_count       = 0;
			flags->loop_flags.cod_info->region1_count       = 0;
			flags->loop_flags.cod_info->part2_length        = 0;
			flags->loop_flags.cod_info->preflag             = 0;
			flags->loop_flags.cod_info->scalefac_scale      = 0;
			flags->loop_flags.cod_info->quantizerStepSize   = 0.0;
			flags->loop_flags.cod_info->count1table_select  = 0;



/*  ========    gr_deco    ========  */

	if (flags->loop_flags.cod_info->window_switching_flag && (flags->loop_flags.cod_info->block_type == SHORT_TYPE))
	{
		if (flags->loop_flags.cod_info->mixed_block_flag)
		{
			/*
				In mixed blocks there come first 8 long scale factor band areas covering
				the place normally used by the first 3 short scale factor band areas.
			*/
			flags->loop_flags.max_used_sfb_l = flags->loop_flags.cod_info->sfb_lmax = 8;
			flags->loop_flags.min_used_sfb_s = flags->loop_flags.cod_info->sfb_smax = 3;

			/* The following values don«t need to be set again and again ... */
			flags->loop_flags.cod_info->region0_count =  7;   /* scalefac_band_long[7+1   ] =  36               */
			flags->loop_flags.cod_info->region1_count = 13;   /* scalefac_band_long[7+13+2] = 576  (no region2) */
		}
		else
		{
			flags->loop_flags.max_used_sfb_l = flags->loop_flags.cod_info->sfb_lmax = 0;  /* No long blocks */
			flags->loop_flags.min_used_sfb_s = flags->loop_flags.cod_info->sfb_smax = 0;

			/* The following values don«t need to be set again and again ... */
			flags->loop_flags.cod_info->region0_count =  8;   /*                   scalefac_band_short[(8+1   )/3] =  12  ( 12*3 =  36) */
			flags->loop_flags.cod_info->region1_count = 36;   /* 36? should be 29: scalefac_band_short[(8+29+2)/3] = 192  (192*3 = 576) */
										    /* probably meant  : scalefac_band_short[36/3 + 1  ] = 192  (192*3 = 576) */
		                                    /* 2000-02-27 AP      no effect on output because block_type != NORM_TYPE */
		}

		/* to access the entire array we need the last scalefac_band_short area */
		flags->loop_flags.end_sfb_l = flags->loop_flags.max_used_sfb_l; /*flags->loop_flags.cod_info->sfb_lmax;*/
		flags->loop_flags.end_sfb_s = SFB_SMAX;

		/* The following values don«t need to be set again and again ... */
		flags->loop_flags.cod_info->count1     =   0;         /* (zero_region-bigv_region) / 4; */
		flags->loop_flags.cod_info->big_values = 288;         /*              bigv_region  / 2; */

		flags->loop_flags.cod_info->count1table_select = 1;   /* sum0 == sum1 == 0 */

		flags->loop_flags.cod_info->address1 =  36;           /* choose one of the region0_count formulas above */
		flags->loop_flags.cod_info->address2 = 576;           /* bigv_region; */
		flags->loop_flags.cod_info->address3 =   0;
	}
	else
	{
		flags->loop_flags.max_used_sfb_l = flags->loop_flags.cod_info->sfb_lmax = SBMAX_l;
		flags->loop_flags.min_used_sfb_s = flags->loop_flags.cod_info->sfb_smax = SBMAX_s;  /* No short blocks */

		/* to access the entire array we need the last scalefac_band_long area */
		flags->loop_flags.end_sfb_l = SFB_LMAX;
		flags->loop_flags.end_sfb_s = flags->loop_flags.min_used_sfb_s; /*flags->loop_flags.cod_info->sfb_smax;*/
	}


			/* reset of iteration variables */

			for (sfb = 0;  sfb < flags->loop_flags.max_used_sfb_l/*SFB_LMAX-1*/;  sfb++)
				flags->loop_flags.scalefac_l[sfb] = 0;
			for (sfb = flags->loop_flags.min_used_sfb_s/*0*/;  sfb < SFB_SMAX-1;  sfb++)
				for (b = 0;  b < 3;  b++)
					flags->loop_flags.scalefac_s[sfb][b] = 0;

/*  ========    calc_xmin and start of quantanf_init    ========  */
/*
	Calculate the allowed distortion for each scalefactor band,
	as determined by the psychoacoustic model.
	xmin(sb) = ratio(sb) * energy(sb) / bandwidth(sb)
*/

#if ORG_QUANTANF_INIT
	log_sum = 0.0;
#endif
	total_energy = 0.0;

	for (sfb = 0;  sfb < flags->loop_flags.end_sfb_l;  sfb++)
	{
		start = scalefac_band_long[sfb];
		end   = scalefac_band_long[sfb+1];

		flags->loop_flags.expo16_l[sfb] = 0;

		flags->loop_flags.xmax_l[sfb] = 0.0;
		flags->loop_flags.xmax_line_l[sfb] = start;

		temp = 0.0;
#if !ORG_HIGHEST_SFB
		if (sfb < flags->loop_flags.max_used_sfb_l)
		{
#endif
			for (i = start;  i < end;  i++)
			{
				if ((x = fabs(flags->loop_flags.xr_org_l[i])) != 0.0)
				{
					flags->loop_flags.xr34_l[i] = sqrt(x * sqrt(x));
					temp += x*x;
#	if ORG_QUANTANF_INIT
					log_sum += log(x);
#	endif
					if (x > flags->loop_flags.xmax_l[sfb])
					{
						flags->loop_flags.xmax_l[sfb] = x;
						flags->loop_flags.xmax_line_l[sfb] = i;
					}
				}
				else
					flags->loop_flags.xr34_l[i] = 0.0;
			}
#if !ORG_HIGHEST_SFB
		}
		else   /* cut off the (highest frequency) entries in the unused scale factor band */
		{
			for (i = start;  i < end;  i++)
				flags->loop_flags.xr34_l[i] = 0.0;
		}
#endif
		total_energy += flags->loop_flags.energy_l[sfb] = temp;

		if (sfb < flags->loop_flags.max_used_sfb_l)
			flags->loop_flags.xmin_l[sfb] = ratio->l[flags->loop_flags.gr][flags->loop_flags.ch][sfb] * temp;
	}

	for (sfb = flags->loop_flags.min_used_sfb_s;  sfb < flags->loop_flags.end_sfb_s;  sfb++)
	{
		start = scalefac_band_short[sfb];
		end   = scalefac_band_short[sfb+1];

		for (b = 0;  b < 3;  b++)
		{
			flags->loop_flags.expo16_s[sfb][b] = 0;

			flags->loop_flags.xmax_s[sfb][b] = 0.0;
			flags->loop_flags.xmax_line_s[sfb][b] = start;

			temp = 0.0;
#if !ORG_HIGHEST_SFB
			if (sfb < SBMAX_s)
			{
#endif
				for (i = start;  i < end;  i++)
				{
					if ((x = fabs(flags->loop_flags.xr_org_s[i][b])) != 0.0)
					{
						flags->loop_flags.xr34_s[i][b] = sqrt(x * sqrt(x));
						temp += x*x;
#if ORG_QUANTANF_INIT
						log_sum += log(x);
#endif
						if (x > flags->loop_flags.xmax_s[sfb][b])
						{
							flags->loop_flags.xmax_s[sfb][b] = x;
							flags->loop_flags.xmax_line_s[sfb][b] = i;
						}
					}
					else
						flags->loop_flags.xr34_s[i][b] = 0.0;
				}
#if !ORG_HIGHEST_SFB
			}
			else   /* cut off the (highest frequency) entries in the unused scale factor band */
			{
				for (i = start;  i < end;  i++)
					flags->loop_flags.xr34_s[i][b] = 0.0;
			}
#endif
			total_energy += flags->loop_flags.energy_s[sfb][b] = temp;

			if (sfb < SFB_SMAX-1)
				flags->loop_flags.xmin_s[sfb][b] = ratio->s[flags->loop_flags.gr][flags->loop_flags.ch][sfb][b] * temp;
		}
	}


/*  ========    calc_scfsi    ========  */

	/* None of the granules contains short blocks */
	if (!flags->loop_flags.cod_info->window_switching_flag || (flags->loop_flags.cod_info->block_type != SHORT_TYPE))
	{
		if (flags->loop_flags.gr == 1)
		{
			for (scfsi_band = 0;  scfsi_band < 4;  scfsi_band++)
				(flags->loop_flags.side_info)->scfsi[flags->loop_flags.ch][scfsi_band] = 0;
		}
	}



			/* calculation of number of available bit( per granule ) */
			max_bits = ResvMaxBits (flags, fr_ps, (flags->loop_flags.side_info), &pe[flags->loop_flags.gr][flags->loop_flags.ch], mean_bits);



			/* all spectral values zero ? */
			if (total_energy != 0.0)
			{



#if ORG_QUANTANF_INIT

/*  ========    quantanf_init (remaining)    ========  */

#define system_const			   8.0
#define minlimit				-100.0

	temp = my_nint (system_const * (log_sum/288.0 - log(total_energy/576.0)));
	if (temp < minlimit)
		temp = minlimit;
	/*
		SS 19-12-96. Starting value of
		global_gain or quantizerStepSize 
		has to be reduced for iteration_loop
	*/
	temp -= 70.0;

				flags->loop_flags.cod_info->quantizerStepSize = temp;

#else   /* ORG_QUANTANF_INIT */

	double					xmax, the_xmax;

	flags->loop_flags.the_lo_quant = -infinity;   /* "-infinity" */
	flags->loop_flags.the_hi_quant = -infinity;   /* the real maximum for high_quant is about +4 ! */

	the_xmax = -1.0;

	for (sfb = 0;  sfb < flags->loop_flags.end_sfb_l;  sfb++)
	{
		xmax = flags->loop_flags.xmax_l[sfb];
		if (xmax == 0.0)
		{
			flags->loop_flags.lo_quant_l[sfb] = -infinity;
			flags->loop_flags.hi_quant_l[sfb] = -infinity;
		}
		else
		{
			flags->loop_flags.lo_quant_l[sfb] = floor (4.0 * (log(xmax)/log_2 - cc)) + 1;
			flags->loop_flags.hi_quant_l[sfb] = floor (4.0 * (log(xmax)/log_2 - dd)) + 1;

			if (xmax > the_xmax)
			{
				the_xmax = xmax;
				flags->loop_flags.the_lo_quant = flags->loop_flags.lo_quant_l[sfb];
				flags->loop_flags.the_hi_quant = flags->loop_flags.hi_quant_l[sfb];
			}
		}
	}

	for (sfb = flags->loop_flags.min_used_sfb_s;  sfb < flags->loop_flags.end_sfb_s;  sfb++)
	{
		for (b = 0;  b < 3;  b++)
		{
			xmax = flags->loop_flags.xmax_s[sfb][b];
			if (xmax == 0.0)
			{
				flags->loop_flags.lo_quant_s[sfb][b] = -infinity;
				flags->loop_flags.hi_quant_s[sfb][b] = -infinity;
			}
			else
			{
				flags->loop_flags.lo_quant_s[sfb][b] = floor (4.0 * (log(xmax)/log_2 - cc) /* - 8 * cod_info->subblock_gain[b] */) + 1;
				flags->loop_flags.hi_quant_s[sfb][b] = floor (4.0 * (log(xmax)/log_2 - dd) /* - 8 * cod_info->subblock_gain[b] */) + 1;

				if (xmax > the_xmax)
				{
					the_xmax = xmax;
					flags->loop_flags.the_lo_quant = flags->loop_flags.lo_quant_s[sfb][b];
					flags->loop_flags.the_hi_quant = flags->loop_flags.hi_quant_s[sfb][b];
				}
			}
		}		
	}


	/*
		Try the power table at its least boundary
		I«ve never reached this deep before!
	*/
	assert (flags->loop_flags.the_lo_quant > -POW216_MAX);

	flags->loop_flags.cod_info->quantizerStepSize = flags->loop_flags.the_lo_quant;

#endif   /* ORG_QUANTANF_INIT */



				flags->loop_flags.cod_info->part2_3_length = outer_loop (flags, max_bits, fr_ps);
			}

			ResvAdjust (flags, fr_ps, flags->loop_flags.cod_info, (flags->loop_flags.side_info), mean_bits);

			flags->loop_flags.cod_info->global_gain = my_nint (flags->loop_flags.cod_info->quantizerStepSize + 210.0);
/*			assert (cod_info->global_gain < 256); */
		}	/* for ch */
	}	/* for gr */

	ResvFrameEnd (flags, fr_ps, (flags->loop_flags.side_info), mean_bits);
}





/*  ========================================================================  */
/*              outer_loop                                                    */
/*  ========================================================================  */
/*
	The outer iteration loop controls the masking conditions
	of all scalefactorbands. It computes the best scalefac and
	global gain. This module calls the inner iteration loop
*/

static	int				outer_loop
(
	encoder_flags_and_data* flags,
	int						max_bits,
	frame_params			*fr_ps
)
{
	int						scalesave_l[SFB_LMAX-1];
	int						scalesave_s[SFB_SMAX-1][3];
	int						bits, huff_bits, save_preflag, save_compress, save_part2_length;
	int						sfb, b, over, iteration;


	/* reset the pointers of our changed sfb [(sfb,b)] indices list */
	flags->loop_flags.mark_idx_l = flags->loop_flags.mark_idx_s = 0;


#if 0
	cod_info->preflag           = 0;   /* assignments are all done in iteration_loop()                      */
	cod_info->scalefac_compress = 0;   /* just to show what«s going on ...                                  */
	cod_info->part2_length      = 0;   /* == part2_length(fr_ps) because of slen1_tab[0] = slen2_tab[0] = 0 */
#endif

	huff_bits = max_bits /* - cod_info->part2_length */;   /* try first without scaling */


	iteration = 1;

	bits = bin_search_StepSize (flags, max_bits, flags->loop_flags.cod_info->quantizerStepSize);  /* speeds things up a bit */

	while (1)
	{
		for (sfb = 0;  sfb < SFB_LMAX-1;  sfb++)  /* save scaling factors */
			scalesave_l[sfb] = flags->loop_flags.scalefac_l[sfb];

		for (sfb = 0;  sfb < SFB_SMAX-1;  sfb++)
			for (b = 0;  b < 3;  b++)
				scalesave_s[sfb][b] = flags->loop_flags.scalefac_s[sfb][b];

		save_preflag      = flags->loop_flags.cod_info->preflag;
		save_compress     = flags->loop_flags.cod_info->scalefac_compress;
		save_part2_length = flags->loop_flags.cod_info->part2_length;

		calc_noise (flags);  /* distortion calculation */

		over = amplify (flags, iteration);

#if ORG_OUTER_LOOP
/*
	misplaced break condition in original dist10:
	-	There is one flag only for both, marking a scalefactor band as
		"should be amplified" or marking it as "is amplified", namely
		a corresponding scalefactor greater than zero!
	-	The desired amplification of the scalefactors bands marked as
		"should be amplified" is actually not done before the next call
		to quantize(), respectively partial_quantize().
	-	Thus, it can happen that all scalefactor bands are marked, but
		not all of the marked bands are amplified.
	-	Since loop_break() doesn't know that, the outer loop frequently
		gets terminated to early.
*/
		if (loop_break (flags))
			break;
#endif

		huff_bits = max_bits - needed_bits_for_storing_scalefactors (flags, fr_ps);
		if (huff_bits < 0)
			break;   /* not enough space to store the scale factors */


		/* We have to wait checking this break condition */
		/* until needed_bits_for_storing_scalefactors()  */
		/* set the scalefac compress index!!!            */
		if (over == 0)
			break;   /* no more bands to amplify */


		iteration++;

		/*
			Most of the times, only a few bands will be changed,
			so why quantize the whole area?
		*/
		partial_quantize (flags);
#if ORG_BINARY_SEARCH || ORG_QUANTANF_INIT || CHECK_TJ_OVERFLOW
		if (tjBitOverflow2)
			bits = infinity;
		else
			bits = count_bits (flags);
#else
		bits = count_bits (flags);
#endif

		while (bits > huff_bits)
		{
			flags->loop_flags.cod_info->quantizerStepSize += 1.0;
#if ORG_BINARY_SEARCH || ORG_QUANTANF_INIT || CHECK_TJ_OVERFLOW
			tjBitOverflow2 = FALSE;
			quantize (flags);
			if (tjBitOverflow2)
				bits = infinity;
			else
				bits = count_bits (flags);
#else
			quantize ();
			bits = count_bits (flags);
#endif
		}

#if !ORG_OUTER_LOOP
		/*
			A break would mean to restore the parameters of the last iteration,
			but we like to accept the current state. If you want to avoid the
			'goto', you have to to take the long way home and place the loop
			break condition in front of the call to calc_noise().
		*/
		if (loop_break(flags))
			goto take_that_and_party;
#endif
	}


	flags->loop_flags.cod_info->preflag           = save_preflag;
	flags->loop_flags.cod_info->scalefac_compress = save_compress;
	flags->loop_flags.cod_info->part2_length      = save_part2_length;

	for (sfb = 0;  sfb < SFB_LMAX-1;  sfb++)
		flags->loop_flags.scalefac_l[sfb] = scalesave_l[sfb];    

	for (sfb = 0;  sfb < SFB_SMAX-1;  sfb++)
		for (b = 0;  b  < 3;  b++)
			flags->loop_flags.scalefac_s[sfb][b] = scalesave_s[sfb][b];

take_that_and_party:
	flags->loop_flags.cod_info->part2_3_length    = flags->loop_flags.cod_info->part2_length + bits;


	return flags->loop_flags.cod_info->part2_3_length;
}





/*  ========================================================================================  */
/*      needed_bits_for_storing_scalefactors                                                  */
/*  ========================================================================================  */
/*
	counts the bits needed to code the scale factors (cod_info->part2_length)
	and the compression index (cod_info->scalefac_compress).

	If there is no suitable index, it returns "infinity".
*/

static	int				needed_bits_for_storing_scalefactors
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps
)
{
#if 0
	static	int				      slen1[16] = {  0,  0,  0,  0,  3,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4 };
	static	int				      slen2[16] = {  0,  1,  2,  3,  0,  1,  2,  3,  1,  2,  3,  1,  2,  3,  2,  3 };
#endif

	/* 2^^slen1[k] */
	static	int				 pow2_slen1[16] = {  1,  1,  1,  1,  8,  2,  2,  2,  4,  4,  4,  8,  8,  8, 16, 16};
	/* 2^^slen2[k] */
	static	int				 pow2_slen2[16] = {  1,  2,  4,  8,  1,  2,  4,  8,  2,  4,  8,  2,  4,  8,  4,  8};

	/* (8+9) * slen1[k] + (9+9) * slen2[k] */
	static	int				part2_len_m[16] = {  0, 18, 36, 54, 51, 35, 53, 71, 52, 70, 88, 69, 87,105,104,122};
	/* (9+9) * slen1[k] + (9+9) * slen2[k] */
	static	int				part2_len_s[16] = {  0, 18, 36, 54, 54, 36, 54, 72, 54, 72, 90, 72, 90,108,108,126};
	/* (6+5) * slen1[k] + (5+5) * slen2[k] */
	static	int				part2_len_l[16] = {  0, 10, 20, 30, 33, 21, 31, 41, 32, 42, 52, 43, 53, 63, 64, 74};

	int						sfb, b, k;
	int						max_slen1, max_slen2, *table;


	max_slen1 = max_slen2 = 0;

	if (flags->loop_flags.cod_info->window_switching_flag  &&  (flags->loop_flags.cod_info->block_type == SHORT_TYPE))
	{
		if (flags->loop_flags.cod_info->mixed_block_flag)
		{
			table = part2_len_m;

			for (sfb = 0;  sfb < 8;  sfb++)
				if (flags->loop_flags.scalefac_l[sfb] > max_slen1)
					max_slen1 = flags->loop_flags.scalefac_l[sfb];

			for (sfb = 3;  sfb < 6;  sfb++)
				for (b = 0;  b < 3;  b++)
					if (flags->loop_flags.scalefac_s[sfb][b] > max_slen1)
						max_slen1 = flags->loop_flags.scalefac_s[sfb][b];
		}
		else
		{
			table = part2_len_s;

			for (sfb = 0;  sfb < 6;  sfb++)
				for (b = 0;  b < 3;  b++)
					if (flags->loop_flags.scalefac_s[sfb][b] > max_slen1)
						max_slen1 = flags->loop_flags.scalefac_s[sfb][b];
		}

		for (sfb = 6;  sfb < 12/*SBMAX_s*/;  sfb++)
			for (b = 0;  b < 3;  b++)
				if (flags->loop_flags.scalefac_s[sfb][b] > max_slen2)
					max_slen2 = flags->loop_flags.scalefac_s[sfb][b];
	}
	else
	{
		table = part2_len_l;

		for (sfb = 0;  sfb < 11;  sfb++)
			if (flags->loop_flags.scalefac_l[sfb] > max_slen1)
				max_slen1 = flags->loop_flags.scalefac_l[sfb];


#if ORG_SCF_COMPRESS && !ORG_PREEMPHASING
		/* This was seen in LAME */
		if (!flags->loop_flags.cod_info->preflag)
		{
			for (sfb = 11;  sfb < SBMAX_l;  sfb++)
				if (flags->loop_flags.scalefac_l[sfb] < (1 + flags->loop_flags.cod_info->scalefac_scale) * blade_pretab[sfb])
					break;

			if (sfb == SBMAX_l)
			{
				for (sfb = 11;  sfb < SBMAX_l;  sfb++)
					flags->loop_flags.scalefac_l[sfb] -= (1 + flags->loop_flags.cod_info->scalefac_scale) * blade_pretab[sfb];
				flags->loop_flags.cod_info->preflag = 1;
			}
		}
#endif


		for (sfb = 11;  sfb < 21/*SBMAX_l*/;  sfb++)
			if (flags->loop_flags.scalefac_l[sfb] > max_slen2)
				max_slen2 = flags->loop_flags.scalefac_l[sfb];
	}


	flags->loop_flags.cod_info->part2_length = infinity;

	for (k = 0;  k < 16;  k++)
	{
		if (max_slen1 < pow2_slen1[k]  &&  max_slen2 < pow2_slen2[k])
		{
#if ORG_SCF_COMPRESS
			flags->loop_flags.cod_info->scalefac_compress = k;
			flags->loop_flags.cod_info->part2_length      = table[k];
			break;
#else
			if (flags->loop_flags.cod_info->part2_length > table[k])
			{
				flags->loop_flags.cod_info->scalefac_compress = k;
				flags->loop_flags.cod_info->part2_length      = table[k];
			}
#endif
		}
	}


	return flags->loop_flags.cod_info->part2_length;
}





/*  ========================================================================================  */
/*              calc_noise                                                                    */
/*  ========================================================================================  */
/*
	calculates the distortion introduced by the qunatization
	in each scale factor band.
*/
static	void			calc_noise (encoder_flags_and_data* flags)
{
	int						i, b, sfb, start, end, off;
	double					f, sum, temp;


	off = -4 * (int)flags->loop_flags.cod_info->quantizerStepSize;


	for (sfb = 0;  sfb < flags->loop_flags.max_used_sfb_l;  sfb++)
	{
		if (flags->loop_flags.ix_l[flags->loop_flags.xmax_line_l[sfb]] == 0)   /* quantized values all zero? */
		{
			flags->loop_flags.xfsf_l[sfb] = flags->loop_flags.energy_l[sfb];   /* see calculation of flags->loop_flags.xmin_l */
		}
		else
		{
			start = scalefac_band_long[sfb];
			end   = scalefac_band_long[sfb+1];

			sum = 0.0;

			f = pow216[flags->loop_flags.expo16_l[sfb] + off];

			for (i = start;  i < end;  i++)
			{
				temp = fabs(flags->loop_flags.xr_org_l[i]) - noisePowTab[flags->loop_flags.ix_l[i]] / f;
				sum += temp * temp;
			}

			flags->loop_flags.xfsf_l[sfb] = sum;
		}
	}

	for (b = 0;  b < 3;  b++)
	{
		off = -4 * ((int)flags->loop_flags.cod_info->quantizerStepSize + 8 * flags->loop_flags.cod_info->subblock_gain[b]);

		for (sfb = flags->loop_flags.min_used_sfb_s;  sfb < SFB_SMAX-1;  sfb++)
		{
			if (flags->loop_flags.ix_s[flags->loop_flags.xmax_line_s[sfb][b]] == 0)   /* quantized values all zero? */
			{
				flags->loop_flags.xfsf_s[sfb][b] = flags->loop_flags.energy_s[sfb][b];   /* see calculation of xmin_s */
			}
			else
			{
				start = scalefac_band_short[sfb];
				end   = scalefac_band_short[sfb+1];

				sum = 0.0;

				f = pow216[flags->loop_flags.expo16_s[sfb][b] + off];

				for (i = start;  i < end;  i++)
				{
	 				temp = fabs(flags->loop_flags.xr_org_s[i][b]) - noisePowTab[flags->loop_flags.ix_s[i][b]] / f;
					sum += temp * temp;
				}       

				flags->loop_flags.xfsf_s[sfb][b] = sum;
			}
		}
	}
}





/*  ========================================================================================  */
/*              loop_break                                                                    */
/*  ========================================================================================  */
/*
	returns zero if there is a scalefac which has not been amplified.
	Otherwise it returns one. 
*/

static	int				loop_break (encoder_flags_and_data* flags)
{
	int						sfb, b;

	for (sfb = 0;  sfb < flags->loop_flags.cod_info->sfb_lmax;  sfb++)
		if (flags->loop_flags.scalefac_l[sfb] == 0)
			return 0;

	for (sfb = flags->loop_flags.min_used_sfb_s;  sfb < 12;  sfb++)
		for (b = 0;  b < 3;  b++)
			if (flags->loop_flags.scalefac_s[sfb][b] == 0)
				return 0;

	return 1;
}





/*  ========================================================================================  */
/*              preemphasing and amplifying                                                   */
/*  ========================================================================================  */
/*
	Preemphasing: see ISO 11172-3  section  C.1.5.4.3.4
	Amplifying  : see ISO 11172-3  section  C.1.5.4.3.5

	amplifying the scalefactor bands that violate the masking threshold.
*/

static	int				amplify
(
	encoder_flags_and_data* flags,
	int						iteration
)
{
	if (flags->loop_flags.cod_info->window_switching_flag  &&  flags->loop_flags.cod_info->block_type == SHORT_TYPE)
		return amplify_short (flags);
	else
		return amplify_long (flags, iteration);
}





static	int				amplify_short (encoder_flags_and_data* flags)
{
	
	int						sfb, b, over, expo16_off;

	expo16_off = 16 * (1 + flags->loop_flags.cod_info->scalefac_scale) / 2;
	over = 0;

#ifdef	MIXED_BLOCKS
	for (sfb = 0;  sfb < flags->loop_flags.max_used_sfb_l;  sfb++)
	{
		if (flags->loop_flags.xfsf_l[sfb] > flags->loop_flags.xmin_l[sfb])
		{
			flags->loop_flags.scalefac_l[sfb]++;
			flags->loop_flags.expo16_l[sfb] += expo16_off;
			over++;
			flags->loop_flags.mark_tab_l[flags->loop_flags.mark_idx_l++] = sfb;
		}
	}
#endif

	for (sfb = flags->loop_flags.min_used_sfb_s;  sfb < SBMAX_s;  sfb++)
	{
		for (b = 0;  b < 3;  b++)
		{
			if (flags->loop_flags.xfsf_s[sfb][b] > flags->loop_flags.xmin_s[sfb][b])
			{
				flags->loop_flags.scalefac_s[sfb][b]++;
				flags->loop_flags.expo16_s[sfb][b] += expo16_off;
				over++;
				flags->loop_flags.mark_tab_s[flags->loop_flags.mark_idx_s++] = sfb;
				flags->loop_flags.mark_tab_s[flags->loop_flags.mark_idx_s++] = b;
			}
		}
	}

	return over;
}





static	int				amplify_long
(
	encoder_flags_and_data* flags,
	int						iteration
)
{
	int						pre_expo_off[SFB_LMAX];

	int						sfb, stop_at, over = 0;
	int						expo16_off;


	stop_at = flags->loop_flags.max_used_sfb_l;

	expo16_off = 16 * (1 + flags->loop_flags.cod_info->scalefac_scale) / 2;


	/*
		Preemphasis is switched on if in all the upper four scalefactor
		bands the actual distortion exceeds the threshold after the
		first call of the inner loop.

		Original bug of dist10 - preemphasis() didn't know 'iteration'!!!
	*/
#if !ORG_PREEMPHASING
	if (iteration == 1)
#endif
		if (!flags->loop_flags.cod_info->preflag)
		{	
			for (sfb = flags->loop_flags.max_used_sfb_l-4;  sfb < flags->loop_flags.max_used_sfb_l;  sfb++)
				if (flags->loop_flags.xfsf_l[sfb] <= flags->loop_flags.xmin_l[sfb])
					goto no_preemphasing;

			flags->loop_flags.cod_info->preflag = 1;

			stop_at = 11;   /* blade_pretab[sfb] = 0  for  sfb = 0..10 */

			for (sfb = stop_at;  sfb < flags->loop_flags.max_used_sfb_l;  sfb++)
			{
				flags->loop_flags.expo16_l[sfb] += pre_expo_off[sfb] = expo16_off * blade_pretab[sfb];

				flags->loop_flags.mark_tab_l[flags->loop_flags.mark_idx_l++] = sfb;
			}
		}


no_preemphasing:


	for (sfb = 0;  sfb < stop_at;  sfb++)
	{
		if (flags->loop_flags.xfsf_l[sfb] > flags->loop_flags.xmin_l[sfb])
		{
			over++;
			flags->loop_flags.expo16_l[sfb] += expo16_off;
			flags->loop_flags.scalefac_l[sfb]++;

			flags->loop_flags.mark_tab_l[flags->loop_flags.mark_idx_l++] = sfb;
		}
	}
	for (sfb = stop_at;  sfb < flags->loop_flags.max_used_sfb_l;  sfb++)   /* The just preemphased bands have to be treated differently */
	{
		if (flags->loop_flags.xfsf_l[sfb] > flags->loop_flags.xmin_l[sfb] * pow216[2*pre_expo_off[sfb]])
		{
			over++;
			flags->loop_flags.expo16_l[sfb] += expo16_off;
			flags->loop_flags.scalefac_l[sfb]++;
		}
	}


	return over;
}





/*  ========================================================================  */
/*              quantize                                                      */
/*  ========================================================================  */
/*
	Quantization of the vector xr ( -> ix)
*/

static	int INLINE		cutting_crew (encoder_flags_and_data* flags, FLOAT in)
{
	
	int						retVal;

	retVal = (int) (in + 0.4054);

#if ORG_BINARY_SEARCH || ORG_QUANTANF_INIT || CHECK_TJ_OVERFLOW
	if (retVal > 8191+14)
		tjBitOverflow2 = TRUE;
#endif

	return retVal;
}



static	void quantize (	encoder_flags_and_data* flags)
{
	int						sfb, i, b, start, end;
	double					f, z, y;

	for (sfb = 0;  sfb < flags->loop_flags.end_sfb_l;  sfb++)
	{
		start = scalefac_band_long[sfb];
		end   = scalefac_band_long[sfb+1];

			        /* (flags->loop_flags.expo16_l[sfb] - 16/4 * quant_step) * 3/4 */
			f = pow216[(flags->loop_flags.expo16_l[sfb]/4 - (int)flags->loop_flags.cod_info->quantizerStepSize) * 3];

			for (i = start;  i < end;  i += 2)
			{
				z = flags->loop_flags.xr34_l[i  ] * f;
				y = flags->loop_flags.xr34_l[i+1] * f;
				flags->loop_flags.ix_l[i  ] = cutting_crew (flags, z);
				flags->loop_flags.ix_l[i+1] = cutting_crew (flags, y);
			}
	}

	for (sfb = flags->loop_flags.min_used_sfb_s;  sfb < flags->loop_flags.end_sfb_s;  sfb++)
	{
		start = scalefac_band_short[sfb];
		end   = scalefac_band_short[sfb+1];

		for (b = 0;  b < 3;  b++)
		{
				        /* (expo_s[sfb][b] - 16/4 * (quant_step + 8 * flags->loop_flags.cod_info->subblock_gain[b])) * 3/4 */
				f = pow216[(flags->loop_flags.expo16_s[sfb][b] / 4 - (int)flags->loop_flags.cod_info->quantizerStepSize - 8 * flags->loop_flags.cod_info->subblock_gain[b]) * 3];

				for (i = start;  i < end;  i += 2)
				{
					z = flags->loop_flags.xr34_s[i  ][b] * f;
					y = flags->loop_flags.xr34_s[i+1][b] * f;
					flags->loop_flags.ix_s[i  ][b] = cutting_crew (flags, z);
					flags->loop_flags.ix_s[i+1][b] = cutting_crew (flags, y);
				}
		}
	}
}



static	void			partial_quantize (	encoder_flags_and_data* flags)
{
	int						sfb, i, b, start, end;
	double					f, z, y;

	while (flags->loop_flags.mark_idx_l)
	{
		sfb = flags->loop_flags.mark_tab_l[--flags->loop_flags.mark_idx_l];

		start = scalefac_band_long[sfb];
		end   = scalefac_band_long[sfb+1];

		        /* (flags->loop_flags.expo16_l[sfb] - 16/4 * quant_step) * 3/4 */
		f = pow216[(flags->loop_flags.expo16_l[sfb]/4 - (int)flags->loop_flags.cod_info->quantizerStepSize) * 3];

		for (i = start;  i < end;  i += 2)
		{
			z = flags->loop_flags.xr34_l[i  ] * f;
			y = flags->loop_flags.xr34_l[i+1] * f;
			flags->loop_flags.ix_l[i  ] = cutting_crew (flags, z);
			flags->loop_flags.ix_l[i+1] = cutting_crew (flags, y);
		}
	}

	while (flags->loop_flags.mark_idx_s)
	{
		b   = flags->loop_flags.mark_tab_s[--flags->loop_flags.mark_idx_s];
		sfb = flags->loop_flags.mark_tab_s[--flags->loop_flags.mark_idx_s];

		start = scalefac_band_short[sfb];
		end   = scalefac_band_short[sfb+1];

		        /* (expo_16s[sfb][b] - 16/4 * (quant_step + 8 * flags->loop_flags.cod_info->subblock_gain[b])) * 3/4 */
		f = pow216[(flags->loop_flags.expo16_s[sfb][b] / 4 - (int)flags->loop_flags.cod_info->quantizerStepSize - 8 * flags->loop_flags.cod_info->subblock_gain[b]) * 3];

		for (i = start;  i < end;  i += 2)
		{
			z = flags->loop_flags.xr34_s[i  ][b] * f;
			y = flags->loop_flags.xr34_s[i+1][b] * f;
			flags->loop_flags.ix_s[i  ][b] = cutting_crew (flags, z);
			flags->loop_flags.ix_s[i+1][b] = cutting_crew (flags, y);
		}
	}
}





/*  ========================================================================  */
/*              count_bits                                                    */
/*  ========================================================================  */

struct
{
	unsigned region0_count;
	unsigned region1_count;
} subdv_table[ 23 ] =
{
	{0, 0}, /*  0 bands */
	{0, 0}, /*  1 bands */
	{0, 0}, /*  2 bands */
	{0, 0}, /*  3 bands */
	{0, 0}, /*  4 bands */
	{0, 1}, /*  5 bands */
	{1, 1}, /*  6 bands */
	{1, 1}, /*  7 bands */
	{1, 2}, /*  8 bands */
	{2, 2}, /*  9 bands */
	{2, 3}, /* 10 bands */
	{2, 3}, /* 11 bands */
	{3, 4}, /* 12 bands */
	{3, 4}, /* 13 bands */
	{3, 4}, /* 14 bands */
	{4, 5}, /* 15 bands */
	{4, 5}, /* 16 bands */
	{4, 6}, /* 17 bands */
	{5, 6}, /* 18 bands */
	{5, 6}, /* 19 bands */
	{5, 7}, /* 20 bands */
	{6, 7}, /* 21 bands */
	{6, 7}, /* 22 bands */
};



/*
	Calculation of rzero, count1, big_values
	(Partitions ix into big values, quadruples and zeros).

	Determines the number of bits to encode the quadruples.

	Presumable subdivides the bigvalue region which will
	use separate Huffman tables.

	Select huffman code tables for bigvalues regions

	Count the number of bits necessary to code the bigvalues region.
*/

static	int				count_bits (encoder_flags_and_data* flags)
{
	flags->loop_flags.cod_info->table_select[0] = 0;
	flags->loop_flags.cod_info->table_select[1] = 0;
	flags->loop_flags.cod_info->table_select[2] = 0;

	if (flags->loop_flags.cod_info->window_switching_flag && (flags->loop_flags.cod_info->block_type == SHORT_TYPE))
		return count_bits_short (flags);
	else
		return count_bits_long (flags);
}





static	int				count_bits_short (encoder_flags_and_data* flags)
{
	unsigned int			bits = 0;

	/*
		Within each scalefactor band, data is given for successive
		time windows, beginning with window 0 and ending with window 2.
		Within each window, the quantized values are then arranged in
		order of increasing frequency...
	*/
	int						sfb, b;
	unsigned int			max, temp;


	/*
		the first part --- 8 long blocks or 3 short blocks
	*/

	max = 0;
#ifdef MIXED_BLOCKS
	if (flags->loop_flags.cod_info->mixed_block_flag)
	{
		for (sfb = 0;  sfb < 8;  sfb++)
			if ((temp = flags->loop_flags.ix_l[flags->loop_flags.xmax_line_l[sfb]]) > max)
				max = temp;
		choose_table_long (flags, 0, 36, max, &flags->loop_flags.cod_info->table_select[0], &bits);
	}
	else
#endif
	{
		for (sfb = 0;  sfb < 3;  sfb++)
			for (b = 0;  b < 3;  b++)
				if ((temp = flags->loop_flags.ix_s[flags->loop_flags.xmax_line_s[sfb][b]][b]) > max)
					max = temp;
		choose_table_short (flags, 0, 3, max, &flags->loop_flags.cod_info->table_select[0], &bits);
	}


	/*
		the second part --- short blocks only
	*/

	max = 0;
	for (sfb = 3;  sfb < SFB_SMAX;  sfb++)
		for (b = 0;  b < 3;  b++)
			if ((temp = flags->loop_flags.ix_s[flags->loop_flags.xmax_line_s[sfb][b]][b]) > max)
				max = temp;
	choose_table_short (flags, 3, SFB_SMAX, max, &flags->loop_flags.cod_info->table_select[1], &bits);

	return bits;
}





static	int				count_bits_long (encoder_flags_and_data* flags)
{
	int						zero_region;
	int						bigv_region;

	unsigned				bits = 0;
	int						sum0 = 0;
	int						sum1 = 0;

	int						sfb_anz, index0, index1, sfb, i;
	unsigned				max, temp;

	int						p;


	for (zero_region = 576;  zero_region > 1;  zero_region -= 2)
		     if (flags->loop_flags.ix_l[zero_region-1])  break;
		else if (flags->loop_flags.ix_l[zero_region-2])  break;

	for (bigv_region = zero_region;  bigv_region > 3;  bigv_region -= 4)
	{
		     if (flags->loop_flags.ix_l[bigv_region-1] > 1)  break;
		else if (flags->loop_flags.ix_l[bigv_region-2] > 1)  break;
		else if (flags->loop_flags.ix_l[bigv_region-3] > 1)  break;
		else if (flags->loop_flags.ix_l[bigv_region-4] > 1)  break;

		p = 0;
		if (flags->loop_flags.ix_l[bigv_region-1])  bits++, p |= 8;
		if (flags->loop_flags.ix_l[bigv_region-2])  bits++, p |= 4;
		if (flags->loop_flags.ix_l[bigv_region-3])  bits++, p |= 2;
		if (flags->loop_flags.ix_l[bigv_region-4])  bits++, p |= 1;

		sum0 += blade_ht[32].hlen[p];
		sum1 += blade_ht[33].hlen[p];
	}

	flags->loop_flags.cod_info->count1     = (zero_region-bigv_region) / 4;
	flags->loop_flags.cod_info->big_values =              bigv_region  / 2;

	if (sum0 < sum1)
	{
		bits += sum0;
		flags->loop_flags.cod_info->count1table_select = 0;
	}
	else
	{
		bits += sum1;
		flags->loop_flags.cod_info->count1table_select = 1;
	}

	if (bigv_region)
	{
		sfb_anz = 1;
		while (scalefac_band_long[sfb_anz] < bigv_region)
			sfb_anz++;

		if (flags->loop_flags.cod_info->window_switching_flag)   /* START_TYPE, STOP_TYPE */
		{
			index0 = (flags->loop_flags.cod_info->region0_count =  7) + 1;
			          flags->loop_flags.cod_info->region1_count = 13;
			index1 = sfb_anz - index0;  if (index0 + index1 < 22)  index1++;

			flags->loop_flags.cod_info->address1 =  36;
			flags->loop_flags.cod_info->address2 = bigv_region;
			flags->loop_flags.cod_info->address3 =   0;
		}
		else   /* NORM_TYPE */  
		{
			index0 = (flags->loop_flags.cod_info->region0_count = subdv_table[sfb_anz].region0_count) + 1;
			index1 = (flags->loop_flags.cod_info->region1_count = subdv_table[sfb_anz].region1_count) + 1;

			flags->loop_flags.cod_info->address1 = scalefac_band_long[index0];
			flags->loop_flags.cod_info->address2 = scalefac_band_long[index0 + index1];
			flags->loop_flags.cod_info->address3 = bigv_region;
		}

		if (flags->loop_flags.cod_info->address1 > 0)
		{
			max = 0;
			for (sfb = 0;  sfb < index0;  sfb++)
				if ((temp = flags->loop_flags.ix_l[flags->loop_flags.xmax_line_l[sfb]]) > max)
					max = temp;
			choose_table_long (flags, 0, flags->loop_flags.cod_info->address1, max, &flags->loop_flags.cod_info->table_select[0], &bits);
		}

		if (flags->loop_flags.cod_info->address2 > flags->loop_flags.cod_info->address1)
		{
			max = 0;
			for (sfb = index0;  sfb < index0+index1;  sfb++)
				if ((temp = flags->loop_flags.ix_l[flags->loop_flags.xmax_line_l[sfb]]) > max)
					max = temp;
			choose_table_long (flags, flags->loop_flags.cod_info->address1, flags->loop_flags.cod_info->address2, max, &flags->loop_flags.cod_info->table_select[1], &bits);
		}

		if (bigv_region > flags->loop_flags.cod_info->address2)
		{
			max = 0;
			for (sfb = index0+index1;  sfb < sfb_anz-1;  sfb++)
				if ((temp = flags->loop_flags.ix_l[flags->loop_flags.xmax_line_l[sfb]]) > max)
					max = temp;
			for (i = scalefac_band_long[sfb_anz-1]; i < bigv_region;  i++)
				if ((temp = flags->loop_flags.ix_l[i]) > max)
					max = temp;
			choose_table_long (flags, flags->loop_flags.cod_info->address2, bigv_region, max, &flags->loop_flags.cod_info->table_select[2], &bits);
		}
	}
	else
	{	/* no big_values region */
		flags->loop_flags.cod_info->region0_count = 0;
		flags->loop_flags.cod_info->region1_count = 0;

		flags->loop_flags.cod_info->address1 = 0;
		flags->loop_flags.cod_info->address2 = 0;
		flags->loop_flags.cod_info->address3 = 0;
	}


	return bits;
}





/*  ========================================================================  */
/*              bin_search_step_size                                          */
/*  ========================================================================  */
/*
	The following optional code written by Seymour Shlien
	will speed up the outer_loop code which is called
	by iteration_loop. When BIN_SEARCH is defined, the
	outer_loop function precedes the call to the function inner_loop
	with a call to bin_search gain defined below, which
	returns a good starting quantizerStepSize.

	The function count_bits() [a sequence of statements, originally part of inner_loop()]
	was completely rewritten.


	changed the behaviour:
	now, it returns the found number of bits <= desired_rate
*/

static	int				bin_search_StepSize
(
	encoder_flags_and_data* flags,
	int						desired_rate,
	double					start
)
{
	int						bits;

	int						top = start;
#if ORG_BINARY_SEARCH || ORG_QUANTANF_INIT
	int						bot = 200;
#else
	int						bot = flags->loop_flags.the_hi_quant;
#endif
	int						next = start;

#if ORG_BINARY_SEARCH

	int						last;

	do
	{
		last = next;
		next = (top + bot) / 2;
		flags->loop_flags.cod_info->quantizerStepSize = next;

		tjBitOverflow2 = FALSE;
		quantize (flags);
		if (tjBitOverflow2)
			bits = infinity;
		else
			bits = count_bits (flags);

		if (bits > desired_rate) 
			top = next;
		else 
			bot = next;
	}
	while ((bits != desired_rate)  &&  (abs(last-next) > 1));

#else   /* ORG_BINARY_SEARCH */

	do
	{
		next = top + (bot - top) / 2;
		flags->loop_flags.cod_info->quantizerStepSize = next;

#if ORG_BINARY_SEARCH || ORG_QUANTANF_INIT || CHECK_TJ_OVERFLOW
		tjBitOverflow2 = FALSE;
		quantize (flags);
		if (tjBitOverflow2)
			bits = infinity;
		else
			bits = count_bits (flags);
#else
		quantize (flags);
		bits = count_bits (flags);
#endif

		if (bits > desired_rate) 
			top = next + 1;
		else 
			bot = next;
	}
	while (top < bot);

#endif   /* ORG_BINARY_SEARCH */

	if (bits > desired_rate)
	{
		flags->loop_flags.cod_info->quantizerStepSize = next+1;
#if ORG_BINARY_SEARCH || ORG_QUANTANF_INIT || CHECK_TJ_OVERFLOW
		tjBitOverflow2 = FALSE;
		quantize (flags);
assert(! tjBitOverflow2);
#else
		quantize (flags);
#endif
		bits = count_bits (flags);
assert(bits <= desired_rate);
	}

	return bits;

}





/*  ========================================================================================  */
/*              choose_table_long                                                             */
/*  ========================================================================================  */
/*
	Choose the Huffman table that will encode ix[start..end] with the fewest
	bits and increases the bit_sum by the amount of these bits.

	Note: This code contains knowledge about the sizes and characteristics
	of the Huffman tables as defined in the IS (Table B.7), and will not work
	with any arbitrary tables.
*/
static	void			choose_table_long
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				max,
	unsigned				*table,
	unsigned				*bit_sum
)
{
	unsigned				choice0, choice1;


	if (max == 0)
	{
		*table = 0;
		return;
	}

	if (max < 15)
	{
		choice0 = 1;  /* we can start with 1 because blade_ht[0].xlen == 0 <= max */
		while (blade_ht[choice0].xlen <= max)
			choice0++;

		switch (choice0)
		{
			case  1:  single_Huffman (flags, start, end,/* 1 */      table, bit_sum);  break;
			case  2:  double_Huffman (flags, start, end,  2,  3,     table, bit_sum);  break;
			case  5:  double_Huffman (flags, start, end,  5,  6,     table, bit_sum);  break;
			case  7:  triple_Huffman (flags, start, end,  7,  8,  9, table, bit_sum);  break;
			case 10:  triple_Huffman (flags, start, end, 10, 11, 12, table, bit_sum);  break;
			case 13:  double_Huffman (flags, start, end, 13, 15,     table, bit_sum);  break;
		}
	}
#if !ORG_HUFFMAN_CODING   /* no part of original BladeEnc */
	else if (max == 15)
	{
		triple_Huffman_2 (flags, start, end,/* 13, 15, 24, */ table, bit_sum);
	}
#endif
	else
	{
		max -= 15;

#if ORG_HUFFMAN_CODING
		choice0 = 15;  while (blade_ht[choice0].linmax < max)  choice0++;
#else
		choice0 = 16;  while (blade_ht[choice0].linmax < max)  choice0++;
#endif

assert(choice0 < 24);
		choice1 = 24;  while (blade_ht[choice1].linmax < max)  choice1++;
assert(choice1 < 32);

#if ORG_HUFFMAN_CODING
		double_Huffman_2 (flags, start, end, choice1, choice0, table, bit_sum);
#else
		double_Huffman_2 (flags, start, end, choice0, choice1, table, bit_sum);
#endif
	}
}





/*  ========================================================================================  */
/*              choose_table_short                                                            */
/*  ========================================================================================  */
/*
	Choose the Huffman table that will encode ix[start_sfb..end_sfb][0..2]
	with the fewest bits and increases the bit_sum by the amount of these bits.

	Note: This code contains knowledge about the sizes and characteristics
	of the Huffman tables as defined in the IS (Table B.7), and will not work
	with any arbitrary tables.
*/

static	void 			choose_table_short
(
	encoder_flags_and_data* flags,
	unsigned				start_sfb,
	unsigned				end_sfb,
	unsigned				max,
	unsigned				*table,
	unsigned				*bit_sum
)
{
	unsigned				choice0;
#if !ORG_HUFFMAN_CODING
	unsigned				choice1;
#endif
	int						start, end;

	start = 3 * scalefac_band_short[start_sfb];
	end   = 3 * scalefac_band_short[  end_sfb];

	if (max == 0)
	{
		*table = 0;
		return;
	}

	if (max < 15)
	{
		choice0 = 1;  /* we can start with 1 because blade_ht[0].xlen == 0 <= max */
		while (blade_ht[choice0].xlen <= max)
			choice0++;

#if ORG_HUFFMAN_CODING
			          tiny_single_Huffman (flags, start, end, choice0,    table, bit_sum);
#else
		switch (choice0)
		{
			case  1:  tiny_single_Huffman (flags, start, end,/* 1 */      table, bit_sum);  break;
			case  2:  tiny_double_Huffman (flags, start, end,  2,  3,     table, bit_sum);  break;
			case  5:  tiny_double_Huffman (flags, start, end,  5,  6,     table, bit_sum);  break;
			case  7:  tiny_triple_Huffman (flags, start, end,  7,  8,  9, table, bit_sum);  break;
			case 10:  tiny_triple_Huffman (flags, start, end, 10, 11, 12, table, bit_sum);  break;
			case 13:  tiny_double_Huffman (flags, start, end, 13, 15,     table, bit_sum);  break;
		}
#endif
	}
#if !ORG_HUFFMAN_CODING   /* no part of original BladeEnc */
	else if (max == 15)
	{
		tiny_triple_Huffman_2 (flags, start, end,/* 13, 15, 24, */ table, bit_sum);
	}
#endif
	else
	{
		max -= 15;

#if ORG_HUFFMAN_CODING

		choice0 = 15;  while (blade_ht[choice0].linmax < max)  choice0++;
assert(choice0 < 24);
		tiny_single_Huffman_2 (flags,start, end, choice0, table, bit_sum);

#else

		choice0 = 16;  while (blade_ht[choice0].linmax < max)  choice0++;
assert(choice0 < 24);
		choice1 = 24;  while (blade_ht[choice1].linmax < max)  choice1++;
assert(choice1 < 32);
		tiny_double_Huffman_2 (flags, start, end, choice0, choice1, table, bit_sum);

#endif
	}
}





/*  ========================================================================================  */
/*      Huffmania                                                                             */
/*  ========================================================================================  */



/*
	That case, we don«t need to decide which is the best table.
*/

static	void			single_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
/*	unsigned				table0, == 1 */
	unsigned				*choice,
	unsigned				*sum
)
{
/*
	int						v;
*/
	unsigned				bits0, signs, idx;

	static	struct huffcodetab		*h0 = blade_ht + /* table0 */ 1;   /* static because of the constant!!! */

#if 0   /* not needed */
	static	unsigned				ylen = h0->ylen;   /* == 2 */
#endif

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = signs = 0;

	while (pos < fin)
	{
		idx = 0;
#if 0
		v = *pos++;  if (v)  {signs++;  idx = v /* * ylen */ + v;}
		v = *pos++;  if (v)  {signs++;  idx += v;}
#else
		if (*pos++)  {signs++;  idx = 2;}
		if (*pos++)  {signs++;  idx++;}
#endif
		bits0 += h0->hlen[idx];
	}

	*choice = /* table0 */ 1;
	*sum += bits0 + signs;
}



#if ORG_HUFFMAN_CODING
static	void			tiny_single_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v0, v1, v2;
	unsigned				bits0, signs, idx0, idx1, idx2;

	struct huffcodetab		*h0 = blade_ht + table0;

	unsigned				ylen = h0->ylen;

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = signs = 0;

	while (pos < fin)
	{
		idx0 = idx1 = idx2 = 0;

		v0 = *pos++;  if (v0)  {signs++;  idx0 = v0 * ylen;}
		v1 = *pos++;  if (v1)  {signs++;  idx1 = v1 * ylen;}
		v2 = *pos++;  if (v2)  {signs++;  idx2 = v2 * ylen;}
		v0 = *pos++;  if (v0)  {signs++;  idx0 += v0;}
		v1 = *pos++;  if (v1)  {signs++;  idx1 += v1;}
		v2 = *pos++;  if (v2)  {signs++;  idx2 += v2;}

		bits0 += h0->hlen[idx0] + h0->hlen[idx1] + h0->hlen[idx2];
	}

	*choice = table0;
	*sum += bits0 + signs;
}
#else
static	void tiny_single_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
/*	unsigned				table0 == 1 */
	unsigned				*choice,
	unsigned				*sum
)
{
/*
	int						v0, v1, v2;
*/
	unsigned				bits0, signs, idx0, idx1, idx2;

	static	struct huffcodetab		*h0 = blade_ht + /* table0 */ 1;   /* static because of the constant!!! */

#if 0   /* not needed */
	static	unsigned				ylen = h0->ylen;   /* == 2 --- static because of the constant!!! */
#endif

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = signs = 0;

	while (pos < fin)
	{
		idx0 = idx1 = idx2 = 0;

		if (*pos++)  {signs++;  idx0 = 2;}
		if (*pos++)  {signs++;  idx1 = 2;}
		if (*pos++)  {signs++;  idx2 = 2;}
		if (*pos++)  {signs++;  idx0++;}
		if (*pos++)  {signs++;  idx1++;}
		if (*pos++)  {signs++;  idx2++;}

		bits0 += h0->hlen[idx0] + h0->hlen[idx1] + h0->hlen[idx2];
	}

	*choice = /* table0 */ 1;
	*sum += bits0 + signs;
}
#endif



#if ORG_HUFFMAN_CODING
static	void tiny_single_Huffman_2   /* Escape tables */
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 15... */
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v0, v1, v2;
	unsigned				bits0, signs, xbits, idx0, idx1, idx2;

	struct huffcodetab		*h0 = blade_ht + table0;

#if 0   /* not needed */
	static	unsigned				ylen = h0->ylen;   /* == h1->ylen == 16 --- static because of the constant!!! */
#endif
	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = signs = xbits = 0;

	while (pos < fin)
	{
		idx0 = idx1 = idx2 = 0;

		v0 = *pos++;  if (v0)  {if (v0 > 14)  {v0 = 15;  xbits++;}  signs++;  idx0  = v0 /* * ylen */ << 4;}
		v1 = *pos++;  if (v1)  {if (v1 > 14)  {v1 = 15;  xbits++;}  signs++;  idx1  = v1 /* * ylen */ << 4;}
		v2 = *pos++;  if (v2)  {if (v2 > 14)  {v2 = 15;  xbits++;}  signs++;  idx2  = v2 /* * ylen */ << 4;}
		v0 = *pos++;  if (v0)  {if (v0 > 14)  {v0 = 15;  xbits++;}  signs++;  idx0 += v0;}
		v1 = *pos++;  if (v1)  {if (v1 > 14)  {v1 = 15;  xbits++;}  signs++;  idx1 += v1;}
		v2 = *pos++;  if (v2)  {if (v2 > 14)  {v2 = 15;  xbits++;}  signs++;  idx2 += v2;}

		bits0 += h0->hlen[idx0] + h0->hlen[idx1] + h0->hlen[idx2];
	}

	bits0 += xbits * h0->linbits;

	*choice = table0;
	*sum += bits0 + signs;
}
#endif





/*
	The following function is called for the most maximum values below 16 (respectively 15)
*/

static	void			double_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 2, 5, 13 */
	unsigned				table1,   /* 3, 6, 15 */
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v;
	unsigned				bits0, bits1, signs, idx;

	struct huffcodetab		*h0 = blade_ht + table0;
	struct huffcodetab		*h1 = blade_ht + table1;

	unsigned				ylen = h0->ylen;   /* == h1->ylen */

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = bits1 = signs = 0;

	while (pos < fin)
	{
		idx = 0;
		v = *pos++;  if (v)  {signs++;  idx = v * ylen;}
		v = *pos++;  if (v)  {signs++;  idx += v;}
		bits0 += h0->hlen[idx];
		bits1 += h1->hlen[idx];
	}

	if (bits0 < bits1)
	{
		*choice = table0;
		*sum += bits0 + signs;
	}
	else
	{
		*choice = table1;
		*sum += bits1 + signs;
	}
}



static	void			tiny_double_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 2, 5, 13 */
	unsigned				table1,   /* 3, 6, 15 */
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v0, v1, v2;
	unsigned				bits0, bits1, signs, idx0, idx1, idx2;

	struct huffcodetab		*h0 = blade_ht + table0;
	struct huffcodetab		*h1 = blade_ht + table1;

	unsigned				ylen = h0->ylen;   /* == h1->ylen */

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = bits1 = signs = 0;

	while (pos < fin)
	{
		idx0 = idx1 = idx2 = 0;
		v0 = *pos++;  if (v0)  {signs++;  idx0 = v0 * ylen;}
		v1 = *pos++;  if (v1)  {signs++;  idx1 = v1 * ylen;}
		v2 = *pos++;  if (v2)  {signs++;  idx2 = v2 * ylen;}
		v0 = *pos++;  if (v0)  {signs++;  idx0 += v0;}
		v1 = *pos++;  if (v1)  {signs++;  idx1 += v1;}
		v2 = *pos++;  if (v2)  {signs++;  idx2 += v2;}
		bits0 += h0->hlen[idx0] + h0->hlen[idx1] + h0->hlen[idx2];
		bits1 += h1->hlen[idx0] + h1->hlen[idx1] + h1->hlen[idx2];
	}

	if (bits0 < bits1)
	{
		*choice = table0;
		*sum += bits0 + signs;
	}
	else
	{
		*choice = table1;
		*sum += bits1 + signs;
	}
}



/*
	poor men«s brave tailor --- only three at a blow...
*/

static	void			triple_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 7, 10 */
	unsigned				table1,   /* 8, 11 */
	unsigned				table2,   /* 9, 12 */
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v;
	unsigned				bits0, bits1, bits2, signs, idx;

	struct huffcodetab		*h0 = blade_ht + table0;
	struct huffcodetab		*h1 = blade_ht + table1;
	struct huffcodetab		*h2 = blade_ht + table2;

	unsigned				ylen = h0->ylen;   /* == h1->ylen == h2->ylen */

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = bits1 = bits2 = signs = 0;

	while (pos < fin)
	{
		idx = 0;
		v = *pos++;  if (v)  {signs++;  idx = v * ylen;}
		v = *pos++;  if (v)  {signs++;  idx += v;}
		bits0 += h0->hlen[idx];
		bits1 += h1->hlen[idx];
		bits2 += h2->hlen[idx];
	}

	if (bits0 < bits1  &&  bits0 < bits2)
	{
		*choice = table0;
		*sum += bits0 + signs;
	}
	else if (bits1 < bits2)
	{
		*choice = table1;
		*sum += bits1 + signs;
	}
	else
	{
		*choice = table2;
		*sum += bits2 + signs;
	}
}



static	void			tiny_triple_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 7, 10 */
	unsigned				table1,   /* 8, 11 */
	unsigned				table2,   /* 9, 12 */
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v0, v1, v2;
	unsigned				bits0, bits1, bits2, signs, idx0, idx1, idx2;

	struct huffcodetab		*h0 = blade_ht + table0;
	struct huffcodetab		*h1 = blade_ht + table1;
	struct huffcodetab		*h2 = blade_ht + table2;

	unsigned				ylen = h0->ylen;   /* == h1->ylen == h2->ylen */

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = bits1 = bits2 = signs = 0;

	while (pos < fin)
	{
		idx0 = idx1 = idx2 = 0;
		v0 = *pos++;  if (v0)  {signs++;  idx0 = v0 * ylen;}
		v1 = *pos++;  if (v1)  {signs++;  idx1 = v1 * ylen;}
		v2 = *pos++;  if (v2)  {signs++;  idx2 = v2 * ylen;}
		v0 = *pos++;  if (v0)  {signs++;  idx0 += v0;}
		v1 = *pos++;  if (v1)  {signs++;  idx1 += v1;}
		v2 = *pos++;  if (v2)  {signs++;  idx2 += v2;}
		bits0 += h0->hlen[idx0] + h0->hlen[idx1] + h0->hlen[idx2];
		bits1 += h1->hlen[idx0] + h1->hlen[idx1] + h1->hlen[idx2];
		bits2 += h2->hlen[idx0] + h2->hlen[idx1] + h2->hlen[idx2];
	}

	if (bits0 < bits1  &&  bits0 < bits2)
	{
		*choice = table0;
		*sum += bits0 + signs;
	}
	else if (bits1 < bits2)
	{
		*choice = table1;
		*sum += bits1 + signs;
	}
	else
	{
		*choice = table2;
		*sum += bits2 + signs;
	}
}





/*
	The escape table 24 deals with linbits=4 instead of linbits=0 in case of table 13 and 15.
	Nevertheless, sometimes it produces the better result...
	Furthermore we take advantage because of the constant table numbers.
*/

static	void			triple_Huffman_2
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
/*	unsigned				table0,   == 13 */
/*	unsigned				table1,   == 15 */
/*	unsigned				table2,   == 24 */
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v;
	unsigned				bits0, bits1, bits2, signs, idx;

	static	struct huffcodetab		*h0 = blade_ht + /* table0 */ 13;   /* all static declarations because of the constant values!!! */
	static	struct huffcodetab		*h1 = blade_ht + /* table1 */ 15;
	static	struct huffcodetab		*h2 = blade_ht + /* table2 */ 24;

#if 0   /* not needed */
	static	unsigned				ylen = h0->ylen;   /* == h1->ylen == h2->ylen */   /* == 16 */
#endif

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = bits1 = bits2 = signs = 0;

	while (pos < fin)
	{
		idx = 0;
		v = *pos++;  if (v)  {if (v == 15)  bits2 += /* h2->linbits */ 4;  signs++;  idx = v /* * ylen */ << 4;}
		v = *pos++;  if (v)  {if (v == 15)  bits2 += /* h2->linbits */ 4;  signs++;  idx += v;}
		bits0 += h0->hlen[idx];
		bits1 += h1->hlen[idx];
		bits2 += h2->hlen[idx];
	}

	if (bits0 < bits1  &&  bits0 < bits2)
	{
		*choice = /* table0 */ 13;
		*sum += bits0 + signs;
	}
	else if (bits1 < bits2)
	{
		*choice = /* table1 */ 15;
		*sum += bits1 + signs;
	}
	else
	{
		*choice = /* table2 */ 24;
		*sum += bits2 + signs;
	}
}



static	void			tiny_triple_Huffman_2
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
/*	unsigned				table0,   == 13 */
/*	unsigned				table1,   == 15 */
/*	unsigned				table2,   == 24 */
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v0, v1, v2;
	unsigned				bits0, bits1, bits2, signs, idx0, idx1, idx2;

	static	struct huffcodetab		*h0 = blade_ht + /* table0 */ 13;   /* all static declarations because of the constant values!!! */
	static	struct huffcodetab		*h1 = blade_ht + /* table1 */ 15;
	static	struct huffcodetab		*h2 = blade_ht + /* table2 */ 24;

#if 0   /* not needed */
	static	unsigned				ylen = h0->ylen;   /* == h1->ylen == h2->ylen */   /* == 16 */
#endif

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = bits1 = bits2 = signs = 0;

	while (pos < fin)
	{
		idx0 = idx1 = idx2 = 0;
		v0 = *pos++;  if (v0)  {if (v0 == 15)  bits2 += /* h2->linbits */ 4;  signs++;  idx0 = v0 /* * ylen */ << 4;}
		v1 = *pos++;  if (v1)  {if (v1 == 15)  bits2 += /* h2->linbits */ 4;  signs++;  idx1 = v1 /* * ylen */ << 4;}
		v2 = *pos++;  if (v2)  {if (v2 == 15)  bits2 += /* h2->linbits */ 4;  signs++;  idx2 = v2 /* * ylen */ << 4;}
		v0 = *pos++;  if (v0)  {if (v0 == 15)  bits2 += /* h2->linbits */ 4;  signs++;  idx0 += v0;}
		v1 = *pos++;  if (v1)  {if (v1 == 15)  bits2 += /* h2->linbits */ 4;  signs++;  idx1 += v1;}
		v2 = *pos++;  if (v2)  {if (v2 == 15)  bits2 += /* h2->linbits */ 4;  signs++;  idx2 += v2;}
		bits0 += h0->hlen[idx0] + h0->hlen[idx1] + h0->hlen[idx2];
		bits1 += h1->hlen[idx0] + h1->hlen[idx1] + h1->hlen[idx2];
		bits2 += h2->hlen[idx0] + h2->hlen[idx1] + h2->hlen[idx2];
	}

	if (bits0 < bits1  &&  bits0 < bits2)
	{
		*choice = /* table0 */ 13;
		*sum += bits0 + signs;
	}
	else if (bits1 < bits2)
	{
		*choice = /* table1 */ 15;
		*sum += bits1 + signs;
	}
	else
	{
		*choice = /* table2 */ 24;
		*sum += bits2 + signs;
	}
}





/*
	In case of two escape tables, we esepecially have to take care for
	the possibly different linbits values...
*/

static	void			double_Huffman_2   /* Escape tables */
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 16... */
	unsigned				table1,   /* 24... */
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v;
	unsigned				bits0, bits1, signs, xbits, idx;

	struct huffcodetab		*h0 = blade_ht + table0;
	struct huffcodetab		*h1 = blade_ht + table1;

#if 0   /* not needed */
	static	unsigned				ylen = h0->ylen;   /* == h1->ylen */   /* == 16 */
#endif
	unsigned				linbits0 = h0->linbits;
	unsigned				linbits1 = h1->linbits;

	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = bits1 = signs = xbits = 0;

	while (pos < fin)
	{
		idx = 0;
		v = *pos++;  if (v)  {if (v > 14)  {v = 15;  xbits++;/*bits0 += linbits0;  bits1 += linbits1;*/}  signs++;  idx = v /* * ylen */ << 4;}
		v = *pos++;  if (v)  {if (v > 14)  {v = 15;  xbits++;/*bits0 += linbits0;  bits1 += linbits1;*/}  signs++;  idx += v;}
		bits0 += h0->hlen[idx];
		bits1 += h1->hlen[idx];
	}
	bits0 += xbits * linbits0;
	bits1 += xbits * linbits1;

	if (bits0 < bits1)
	{
		*choice = table0;
		*sum += bits0 + signs;
	}
	else
	{
		*choice = table1;
		*sum += bits1 + signs;
	}
}



static	void			tiny_double_Huffman_2   /* Escape tables */
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 16... */
	unsigned				table1,   /* 24... */
	unsigned				*choice,
	unsigned				*sum
)
{
	int						v0, v1, v2;
	unsigned				bits0, bits1, signs, xbits, idx0, idx1, idx2;

	struct huffcodetab		*h0 = blade_ht + table0;
	struct huffcodetab		*h1 = blade_ht + table1;

#if 0   /* not needed */
	static	unsigned				ylen = h0->ylen;   /* == h1->ylen == 16 --- static because of the constant!!! */
#endif
	int						*pos = flags->loop_flags.ix_l + start;
	int						*fin = flags->loop_flags.ix_l + end;

	bits0 = bits1 = signs = xbits = 0;

	while (pos < fin)
	{
		idx0 = idx1 = idx2 = 0;

		v0 = *pos++;  if (v0)  {if (v0 > 14)  {v0 = 15;  xbits++;}  signs++;  idx0  = v0 /* * ylen */ << 4;}
		v1 = *pos++;  if (v1)  {if (v1 > 14)  {v1 = 15;  xbits++;}  signs++;  idx1  = v1 /* * ylen */ << 4;}
		v2 = *pos++;  if (v2)  {if (v2 > 14)  {v2 = 15;  xbits++;}  signs++;  idx2  = v2 /* * ylen */ << 4;}
		v0 = *pos++;  if (v0)  {if (v0 > 14)  {v0 = 15;  xbits++;}  signs++;  idx0 += v0;}
		v1 = *pos++;  if (v1)  {if (v1 > 14)  {v1 = 15;  xbits++;}  signs++;  idx1 += v1;}
		v2 = *pos++;  if (v2)  {if (v2 > 14)  {v2 = 15;  xbits++;}  signs++;  idx2 += v2;}

		bits0 += h0->hlen[idx0] + h0->hlen[idx1] + h0->hlen[idx2];
		bits1 += h1->hlen[idx0] + h1->hlen[idx1] + h1->hlen[idx2];
	}

	bits0 += xbits * h0->linbits;
	bits1 += xbits * h1->linbits;

	if (bits0 < bits1)
	{
		*choice = table0;
		*sum += bits0 + signs;
	}
	else
	{
		*choice = table1;
		*sum += bits1 + signs;
	}
}



