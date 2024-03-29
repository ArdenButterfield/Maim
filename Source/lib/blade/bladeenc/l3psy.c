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

	2000-12-05  Andre Piotrowski

	-	speed up: implemented prepacking of fft-data

	2000-12-11  ap

	-	speed up:  faster psycho_anal()
	-	optional bug fix: integrated better norm calclulation and block selecting

	2000-12-12  ap

	-	use SHORT_FFT_MIN_IDX to allow switching of "ORG_SHORT_CW_LIMIT" in "l3psy.h"

	2001-01-12  ap

	-	use some explicit type casting to avoid compiler warnings
*/




#include	<stdlib.h>

#include	"common.h"
#include	"tables.h"
#include	"encoder.h"
#include	"l3side.h"
#include	"l3psy.h"





/*____ Global Static Variables ______________________________________________*/

/* The static variables "r", "phi_sav", "new_", "old" and "oldest" have    */
/* to be remembered for the unpredictability measure.  For "r" and        */
/* "phi_sav", the first index from the left is the channel select and     */
/* the second index is the "age" of the data.                             */


#if 0
static	int				new_, old, oldest;
static	int				flush, sync_flush, syncsize;

#if RING_BUFFER==1
static	int				savebuf_start_idx[2];
#endif




static	double			*minval, *qthr_l;
static	double			*qthr_s, *SNR_s;
static	int				*cbw_l, *bu_l, *bo_l;
static	int				*cbw_s, *bu_s, *bo_s;
static	double			*w1_l, *w2_l;
static	double			*w1_s, *w2_s;

static	int				cbmax_l, cbmax_s;
static	int				*numlines_l;
static	int				*numlines_s;

						/* the non-zero entries of norm_l[i] * s3_l[i][j] */
static	FLOAT			normed_s3_l [900];   /* a bit more space than needed [799|855|735] */
static	int				lo_s3_l     [CBANDS];
static	int				hi_s3_l		[CBANDS];

static	FLOAT			normed_s3_s [500];   /* a bit more space than needed [445|395|378] */
static	int				lo_s3_s     [CBANDS_s];
static	int				hi_s3_s		[CBANDS_s];



/* Scale Factor Bands */
static	int				blocktype_old[2];



static	double			nb_1        [2][CBANDS];
static	double			nb_2        [2][CBANDS];

static	double			cw          [HBLKSIZE];

static	FLOAT			window      [BLKSIZE];
static	FLOAT			r			[2][2][6];
static	FLOAT			phi_sav		[2][2][6];

static	FLOAT			window_s    [BLKSIZE_s];

static	double			ratio       [2][SBMAX_l];
static	double			ratio_s     [2][SBMAX_s][3];
#endif



static	void			L3para_read (encoder_flags_and_data* flags, int sfreq);

static	void			calc_normed_spreading
(
	encoder_flags_and_data* flags,
	int						cbmax,			/* number of lines and rows           */
	const double			bval[],			/* input values to compute the matrix */
	FLOAT					s3_ptr[],		/* the resulting non-zero entries     */
	int						lo_s3[],
	int						hi_s3[],
	const double			norm[]
);


/*____ psycho_anal_init() ___________________________________________________*/

void					psycho_anal_init (encoder_flags_and_data* flags, double sfreq)
{
	unsigned int			ch, sfb, b, i, j;


	/* reset the r, phi_sav "ring buffer" indices */

	flags->old = 1 - (flags->new_ = flags->oldest = 0);


	/* clear the ratio arrays */

	for (ch = 0;  ch < 2;  ch++)
	{
		for (sfb = 0;  sfb < SBMAX_l;  sfb++)
			flags->ratio[ch][sfb] = 0.0;

		for (sfb = 0;  sfb < SBMAX_s;  sfb++)
			for (b = 0;  b < 3;  b++)
				flags->ratio_s[ch][sfb][b] = 0.0;
	}


	/* clear preecho arrays */

	for (ch = 0;  ch < 2;  ch++)
	{
		for (i = 0;  i < CBANDS;  i++)
		{
			flags->nb_1[ch][i] = 0;
			flags->nb_2[ch][i] = 0;
		}
	}


	/* clear blocktype information */

	for (ch = 0;  ch < 2;  ch++)
		flags->blocktype_old[ch] = NORM_TYPE;


	flags->sync_flush =  768;
	flags->flush      =  576;
	flags->syncsize   = 1344;   /* sync_flush + flush */


#if RING_BUFFER==1
	for (ch = 0;  ch < 2;  ch++)
		flags->savebuf_start_idx[ch] = 0;
#endif


	/* calculate HANN window coefficients */

    for (i = 0;  i < BLKSIZE;  i++)
		flags->window[i] = (FLOAT) (0.5 * (1 - cos (2.0 * PI * (i - 0.5) / BLKSIZE)));

    for (i = 0;  i < BLKSIZE_s;  i++)
		flags->window_s[i] = (FLOAT) (0.5 * (1 - cos (2.0 * PI * (i - 0.5) / BLKSIZE_s)));


	/* reset states used in unpredictability measure */

	for (ch = 0;  ch < 2;  ch++)
	{
		for (i = 0;  i < 2;  i++)
		{
			for (j = 0;  j < 6;  j++)
			{
				      flags->r[ch][i][j] = 0.0;
				flags->phi_sav[ch][i][j] = 0.0;
			}
		}
	}


	L3para_read (flags, (int) sfreq);


	/* Set unpredicatiblility of remaining spectral lines to 0.4 */

	for (j = 206;  j < HBLKSIZE;  j++)
		flags->cw[j] = 0.4;
}

/*____ psycho_anal_exit() ___________________________________________________*/

void psycho_anal_exit( encoder_flags_and_data* flags )
{
	/* nothing to do */
}





/*____ psycho_anal() ________________________________________________________*/
									
void					psycho_anal
(
	encoder_flags_and_data* flags,
#if ORG_BUFFERS
	short int				*buffer,
	short int				savebuf[2048],
#else
	FLOAT					*buffer,
	int						buffer_idx,
#endif
	int						ch,
	int						lay,
/*	float					snr32[32], */
	double					ratio_d[SBMAX_l],
	double					ratio_ds[SBMAX_s][3],
	double					*pe,
	gr_info					*cod_info
)
{
	int						blocktype;
	unsigned int			sfb, b, j, k;
	double					r_prime, phi_prime; /* not FLOAT */
	double					temp1, temp2, temp3;

	FLOAT					*s3_ptr;

	int						sblock;

	double					thr         [CBANDS];
	double					eb          [CBANDS];
	FLOAT					cb          [CBANDS];
	FLOAT					wsamp_r     [HBLKSIZE];
	FLOAT					wsamp_i     [HBLKSIZE];

	FLOAT					energy      [HBLKSIZE];
	FLOAT					phi         [6];
	FLOAT					energy_s    [3][BLKSIZE_s];
	FLOAT					phi_s       [3][52];

#if ORG_BUFFERS
#if RING_BUFFER==1
	int						beg, idx, fin;
#endif
#else
#	define					savebuf		buffer
#	define					beg			buffer_idx
	int						idx, fin;
#endif

	
	for (sfb = 0;  sfb < SBMAX_l;  sfb++)
		ratio_d[sfb] = flags->ratio[ch][sfb];

	for (sfb = 0;  sfb < SBMAX_s;  sfb++)
		for (b = 0;  b < 3;  b++)
			ratio_ds[sfb][b] = flags->ratio_s[ch][sfb][b];
	

	if (ch == 0)
		flags->old = 1 - (flags->new_ = flags->oldest = flags->old);


#if ORG_BUFFERS
	/**********************************************************************
	*  Delay signal by sync_flush=768 samples                             *
	**********************************************************************/

#	if RING_BUFFER==0
		for (j = 0;  j < flags->sync_flush;  j++)   /* for long window samples */
			savebuf[j] = savebuf[j+flags->flush];

		for (j = flags->sync_flush;  j < flags->syncsize;  j++)
			savebuf[j] = *buffer++;
#	else
		beg = flags->savebuf_start_idx[ch] = (flags->savebuf_start_idx[ch] + flags->flush) & 2047;

		idx = (beg + flags->sync_flush) & 2047;
		fin = (idx + flags->flush) & 2047;
		if (idx >= fin)
		{
			while (idx < 2048)
				savebuf[idx++] = *buffer++;
			idx = 0;
		}
		while (idx < fin)
			savebuf[idx++] = *buffer++;
#	endif
#endif


/**********************************************************************
*    compute unpredicatability of first six spectral lines            * 
**********************************************************************/

#if RING_BUFFER==0
	for (j = 0, k = 0, idx = 0;  j < BLKSIZE/2;  j++)
	{
		wsamp_r[j] = flags->window[k++] * savebuf[idx++];
		wsamp_i[j] = flags->window[k++] * savebuf[idx++];
	}
#else
	j = 0;  k = 0;
	idx = beg;
	fin = (idx + BLKSIZE) & 2047;
	if (idx >= fin)
	{
		while (idx < 2048)
		{
			wsamp_r[j] = flags->window[k++] * savebuf[idx++];
			wsamp_i[j] = flags->window[k++] * savebuf[idx++];
			j++;
		}
		idx = 0;
	}
	while (idx < fin)
	{
		wsamp_r[j] = flags->window[k++] * savebuf[idx++];
		wsamp_i[j] = flags->window[k++] * savebuf[idx++];
		j++;
	}
#endif

	fft(wsamp_r, wsamp_i, energy, phi, BLKSIZE);   /* long FFT */

	for (j = 0;  j < 6;  j++)
	{	/* calculate unpredictability measure cw */
		double r1, phi1;
		                     r_prime = 2.0 *       flags->r[ch][flags->old][j] -       flags->r[ch][flags->oldest][j];
		                   phi_prime = 2.0 * flags->phi_sav[ch][flags->old][j] - flags->phi_sav[ch][flags->oldest][j];
		      flags->r[ch][flags->new_][j] = (FLOAT) (  r1 = sqrt((double) energy[j]));
		flags->phi_sav[ch][flags->new_][j] = (FLOAT) (phi1 =                  phi[j] );

		temp3 = r1 + fabs(r_prime);
		if (temp3 != 0.0)
		{
			temp1 = r1 * cos(phi1) - r_prime * cos(phi_prime);
			temp2 = r1 * sin(phi1) - r_prime * sin(phi_prime);
			flags->cw[j] = sqrt(temp1*temp1 + temp2*temp2) / temp3;
		}
		else
			flags->cw[j] = 0;
	}


/**********************************************************************
*     compute unpredicatibility of next 200 spectral lines            *
**********************************************************************/ 

	for (b = 0;  b < 3;  b++)
	{
#if RING_BUFFER==0
		for (j = 0, k = 0, idx = 128*(2 + b);  j < BLKSIZE_s/2;  j++)
		{	/* window data with HANN window */
			wsamp_r[j] = flags->window_s[k++] * savebuf[idx++];
			wsamp_i[j] = flags->window_s[k++] * savebuf[idx++];
		}
#else
		j = 0;  k = 0;
		idx = (beg + 128*(2 + b)) & 2047;
		fin = (idx + BLKSIZE_s) & 2047;
		if (idx >= fin)
		{
			while (idx < 2048)
			{
				wsamp_r[j] = flags->window_s[k++] * savebuf[idx++];
				wsamp_i[j] = flags->window_s[k++] * savebuf[idx++];
				j++;
			}
			idx = 0;
		}
		while (idx < fin)
		{
			wsamp_r[j] = flags->window_s[k++] * savebuf[idx++];
			wsamp_i[j] = flags->window_s[k++] * savebuf[idx++];
			j++;
		}
#endif

		fft (wsamp_r, wsamp_i, energy_s[b], phi_s[b], BLKSIZE_s);   /* short FFT*/
	}
 
	for (j = 6, k = SHORT_FFT_MIN_IDX;  j < 206;  j += 4, k++)
	{	/* calculate unpredictability measure cw */
		double r1, phi1;

	 	  r_prime = 2.0 * sqrt((double) energy_s[0][k]) - sqrt((double) energy_s[2][k]);
		phi_prime = 2.0 *                  phi_s[0][k]  -                  phi_s[2][k];
		       r1 = sqrt((double) energy_s[1][k]);
		     phi1 =                  phi_s[1][k];

		temp3 = r1 + fabs(r_prime);
		if (temp3 != 0.0)
		{
			temp1 = r1 * cos(phi1) - r_prime * cos(phi_prime);
			temp2 = r1 * sin(phi1) - r_prime * sin(phi_prime);
			flags->cw[j] = sqrt(temp1*temp1 + temp2*temp2) / temp3;
		}
		else
			flags->cw[j] = 0.0;

		flags->cw[j+1] = flags->cw[j+2] = flags->cw[j+3] = flags->cw[j];
	}


/**********************************************************************
*    Calculate the energy and the unpredictability in the threshold   *
*    calculation partitions                                           *
**********************************************************************/


	j = 0;
	for (b = 0;  b < flags->cbmax_l;  b++)
	{
		eb[b] = 0.0;
		cb[b] = 0.0;

		/*
			Calculate the energy and the unpredictability in the threshold
			calculation partitions

			cbmax_l holds the number of valid numlines_l entries
		*/
		k = flags->numlines_l[b];
		do {
			eb[b] += energy[j];
			cb[b] += flags->cw[j] * energy[j];
		} while (j++, --k);
	}

	s3_ptr = flags->normed_s3_l;



	*pe = 0.0;
	
	for (b = 0;  b < flags->cbmax_l;  b++)
	{
		FLOAT					nb;
		FLOAT					ecb = 0.0;
		double					ctb = 0.0;
		double					SNR_l;
		double					cbb, tbb;


		/*
			convolve the partitioned energy and unpredictability
			with the spreading function, normed_s3_l[b][k]
		*/
		for (k = flags->lo_s3_l[b];  k < flags->hi_s3_l[b];  k++)
		{
			ecb += *s3_ptr   * eb[k];  /* sprdngf for Layer III */
			ctb += *s3_ptr++ * cb[k];
		}


		/*
			calculate the tonality of each threshold calculation partition
			calculate the SNR in each threshhold calculation partition
		*/
		if (ecb != 0.0)
		{
			cbb = ctb / ecb;
			if (cbb < 0.01)
				cbb = 0.01;
			tbb = -0.299 - 0.43 * log(cbb);   /* conv1=-0.299, conv2=-0.43 */
			tbb = MIN(MAX (0.0, tbb), 1.0) ;  /* 0<=tbb<=1 */
		}
		else
			tbb = 0.0;  /* cbb==0 => -0.299-0.43*cbb<0 => tbb=0*/

		/* TMN=29.0,NMT=6.0 for all calculation partitions */
		SNR_l = MAX (flags->minval[b], 23.0 * tbb + 6.0);   /* 29*tbb + 6*(1-tbb) */
	
		/* calculate the threshold for each partition */
	    nb = ecb * exp(-SNR_l * LN_TO_LOG10);   /* our ecb is already normed */

		/*
			pre-echo control
		*/
		thr[b] = MAX (flags->qthr_l[b], MIN(nb, flags->nb_2[ch][b]));
		flags->nb_2[ch][b] = MIN(2.0 * nb, 16.0 * flags->nb_1[ch][b]);
	    flags->nb_1[ch][b] = nb;


		/*
			calculate percetual entropy

			thr[b] -> thr[b]+1.0 : for non sound portition
		*/
		if (eb[b] > thr[b])
			*pe += flags->numlines_l[b] * log((eb[b]+1.0) / (thr[b]+1.0));
	}
	

#define switch_pe  1800
	

	if (*pe < switch_pe)
	{
		/* no attack : use long blocks */

		if (flags->blocktype_old[ch] == SHORT_TYPE)
			blocktype = STOP_TYPE;
		else   /* NORM_TYPE, STOP_TYPE */
			blocktype = NORM_TYPE;


		/* threshold calculation (part 2) */

		for (sfb = 0;  sfb < SBMAX_l;  sfb++)
		{
			int		bu = flags->bu_l[sfb];
			int		bo = flags->bo_l[sfb];
			double	en = flags->w1_l[sfb] * eb[bu] + flags->w2_l[sfb] * eb[bo];

			for (b = bu+1;  b < bo;  b++)
				en += eb[b];

			flags->bends.psychoanal_energy[sfb] = en;
			if (en != 0.0)
			{
				double	thm = flags->w1_l[sfb] * thr[bu] + flags->w2_l[sfb] * thr[bo];

				for (b = bu+1;  b < bo;  b++)
					thm += thr[b];

				thm *= flags->bends.threshold_bias[sfb];
				flags->bends.psychoanal_threshold[sfb] = thm;

				flags->ratio[ch][sfb] = thm / en;
			}
			else {
				flags->ratio[ch][sfb] = 0.0;
				flags->bends.psychoanal_threshold[sfb] = 0;
			}
		}
	}
	else
	{
		/* attack : use short blocks */
		blocktype = SHORT_TYPE;
#if ORG_BLOCK_SELECT
		if (flags->blocktype_old[ch] == NORM_TYPE)
			flags->blocktype_old[ch] = START_TYPE;
		else   /* SHORT_TYPE, STOP_TYPE */
			flags->blocktype_old[ch] = SHORT_TYPE;
#else   /* ISO */
		if (flags->blocktype_old[ch] == SHORT_TYPE)
			flags->blocktype_old[ch] = SHORT_TYPE;
		else   /* NORM_TYPE, STOP_TYPE */
			flags->blocktype_old[ch] = START_TYPE;
#endif


		/* threshold calculation for short blocks */

		for (sblock = 0;  sblock < 3;  sblock++)
		{

			j = 0;
			for (b = 0;  b < flags->cbmax_s;  b++)
			{
				eb[b] = 0.0;

				/*
					Calculate the energy and the unpredictability in the threshold
					calculation partitions

					cbmax_s holds the number of valid flags->numlines_s entries
				*/
				k = flags->numlines_s[b];
				do {
					eb[b] += energy_s[sblock][j];
				} while (j++, --k);
			}

			s3_ptr = flags->normed_s3_s;

			for (b = 0;  b < flags->cbmax_s;  b++)
			{
				FLOAT					nb;
				FLOAT					ecb = 0.0;

				for (k = flags->lo_s3_s[b];  k < flags->hi_s3_s[b];  k++)
					ecb += *s3_ptr++ * eb[k];

				nb = ecb * exp((double) flags->SNR_s[b] * LN_TO_LOG10);   /* our ecb is already normed */
				thr[b] = MAX(flags->qthr_s[b], nb);
			}

			for (sfb = 0;  sfb < SBMAX_s;  sfb++)
			{
				int		bu = flags->bu_s[sfb];
				int		bo = flags->bo_s[sfb];
				double	en = flags->w1_s[sfb] * eb[bu] + flags->w2_s[sfb] * eb[bo];

				for (b = bu+1;  b < bo;  b++)
					en += eb[b];
				if (en != 0.0)
				{
					double	thm = flags->w1_s[sfb] * thr[bu] + flags->w2_s[sfb] * thr[bo];

					for (b = bu+1;  b < bo;  b++)
						thm += thr[b];

					flags->ratio_s[ch][sfb][sblock] = thm / en;
				}
				else
					flags->ratio_s[ch][sfb][sblock] = 0.0;
			}
		}
	} 
	
	cod_info->block_type = flags->blocktype_old[ch];
	flags->blocktype_old[ch] = blocktype;

	if ( cod_info->block_type == NORM_TYPE )
	    cod_info->window_switching_flag = 0;
	else
	    cod_info->window_switching_flag = 1;

	cod_info->mixed_block_flag = 0;
}





/*____ L3para_read() __________________________________________________________*/

static void				L3para_read (encoder_flags_and_data* flags, int sfreq)
{
	int						sfreq_idx;
	l3_parm_block			*parm;
	double					*bval_l, *bval_s;

	double					*norm_l, *norm_s;


	/*
		Set parameter block
	*/
	switch (sfreq)
	{
		case 32000:  sfreq_idx = 2;  break;
		case 44100:  sfreq_idx = 0;  break;
		case 48000:  sfreq_idx = 1;  break;
		default   :  return;  /* Just to avoid compiler warnings */
	}
	parm = l3_parm + sfreq_idx;


	/*
		Read long block data
	*/
	flags->cbmax_l    = parm->long_data.cbmax_l;

	flags->numlines_l = parm->long_data.numlines_l;
		
	flags->minval     = parm->long_data.minval;
	flags->qthr_l     = parm->long_data.qthr_l;
	norm_l     = parm->long_data.norm_l;
	bval_l     = parm->long_data.bval_l;


	/*
		Compute the normed spreading function norm_l[i] * s3_l[i][j]
	*/
	calc_normed_spreading (flags, flags->cbmax_l, bval_l, flags->normed_s3_l, flags->lo_s3_l, flags->hi_s3_l, norm_l);


	/*
		Read short block data
	*/
	flags->cbmax_s    = parm->short_data.cbmax_s;

	flags->numlines_s = parm->short_data.numlines_s;

	flags->qthr_s     = parm->short_data.qthr_s;
	norm_s     = parm->short_data.norm_s;
	flags->SNR_s      = parm->short_data.SNR_s;
	bval_s     = parm->short_data.bval_s;


#if !ORG_NUMLINES_NORM

	/*
		Compute the normed spreading function norm_s[i] * s3_s[i][j]
	*/
	calc_normed_spreading (flags, flags->cbmax_s, bval_s, flags->normed_s3_s, flags->lo_s3_s, flags->hi_s3_s, norm_s);

#endif


	/*
		Read long block data for converting threshold
		calculation partitions to scale factor bands
	*/
	flags->cbw_l = parm->long_thres.cbw_l;
	flags->bu_l  = parm->long_thres.bu_l;
	flags->bo_l  = parm->long_thres.bo_l;
	flags->w1_l  = parm->long_thres.w1_l;
	flags->w2_l  = parm->long_thres.w2_l;


	/*
		Read short block data for converting threshold
		calculation partitions to scale factor bands
	*/
	flags->cbw_s = parm->short_thres.cbw_s;
	flags->bu_s  = parm->short_thres.bu_s;
	flags->bo_s  = parm->short_thres.bo_s;
	flags->w1_s  = parm->short_thres.w1_s;
	flags->w2_s  = parm->short_thres.w2_s;
}



#if !ORG_NUMLINES_NORM && NEW_L3PARM_TABLES

/*  ========================================================================================  */
/*              calc_normed_spreading                                                         */
/*  ========================================================================================  */
/*
	Compute the normed spreading function,
	the normed value of the spreading function,
	centered at band j, for band i, store for later use

	Since this is a band matrix, we store only the non-zero entries
	in linear order in the single dimension array normed_s3.

	The array has to be accessed in linear order, too, starting with line 0,
	up to line cbmax-1. For line b, the current entries represent

		norm[b] * s3[b][lo_s3[b]]  ...  norm[b] * s3[b][hi_s3[b]-1]

	Normally, we could easily compute the norm [building the reciprocal of the line sum].
	Alas, dist10 uses somewhat (strange and) different, that made our norm differring too
	much at the last few lines. Thus, we renounce and use the original values.
*/

static	void			calc_normed_spreading
(
	encoder_flags_and_data* flags,
	int						cbmax,			/* number of lines and rows           */
	const double			bval[],			/* input values to compute the matrix */
	FLOAT					s3_ptr[],		/* the resulting non-zero entries     */
	int						lo_s3[],
	int						hi_s3[],
	const double			norm[]
)
{
	double					arg, x, y;
	double					s3[CBANDS];
	int						i, j;
	int						non_zero_part;



	for (i = 0;  i < cbmax;  i++)
	{
		non_zero_part = FALSE;
		hi_s3[i] = cbmax;   /* we preset this value for the case that the line ends with a non-zero entry */

		for (j = 0;  j < cbmax;  j++)
		{
			if (j >= i)
				arg = (bval[i] - bval[j]) * 3.0;
			else
				arg = (bval[i] - bval[j]) * 1.5;

			if (arg > 0.5  &&  arg < 2.5)
				x = 8.0 * (arg - 0.5) * (arg - 2.5);
			else
				x = 0.0;

			arg += 0.474;

			y = 15.811389 + 7.5 * arg - 17.5 * sqrt(1.0 + arg * arg);

			if (y <= -60.0)
			{
				if (non_zero_part)   /* only zeroes will follow */
				{
					hi_s3[i] = j;
					break;   /* so cut the computing for this line */
				}
			}
			else
			{
				s3[j] = exp((x + y) * LN_TO_LOG10);

				if (! non_zero_part)
				{
					lo_s3[i] = j;
					non_zero_part = TRUE;   /* the first non-zero entry ends the non_zero_part */
				}
			}
		}

		for (j = lo_s3[i];  j < hi_s3[i];  j++)
			*s3_ptr++ = s3[j] * norm[i];
	}
}

#endif		/* ORG_NUMLINES_NORM */
