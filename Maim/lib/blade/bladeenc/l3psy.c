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

#define		RING_BUFFER				1





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


static	int				new_, old, oldest;
static	int				flush, sync_flush, syncsize;

#if RING_BUFFER==1
static	int				savebuf_start_idx[2];
#endif



#if NEW_L3PARM_TABLES

static	double			*minval, *qthr_l;
static	double			*qthr_s, *SNR_s;
static	int				*cbw_l, *bu_l, *bo_l;
static	int				*cbw_s, *bu_s, *bo_s;
static	double			*w1_l, *w2_l;
static	double			*w1_s, *w2_s;

#if ORG_NUMLINES_NORM

static	int				cbmax_l = CBANDS, cbmax_s = CBANDS_s;
static	int				numlines_l  [CBANDS];

static	int				partition_l [HBLKSIZE];
static	int				partition_s [HBLKSIZE_s];
static	double			s3_l        [CBANDS][CBANDS];
static	double			*norm_l, *norm_s;

#else

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

#endif		/* ORG_NUMLINES_NORM */

#else

static	double			minval[CBANDS], qthr_l[CBANDS], norm_l[CBANDS];
static	double			qthr_s[CBANDS_s], norm_s[CBANDS_s], SNR_s[CBANDS_s];
static	int				cbw_l[SBMAX_l],bu_l[SBMAX_l],bo_l[SBMAX_l];
static	int				cbw_s[SBMAX_s],bu_s[SBMAX_s],bo_s[SBMAX_s];
static	double			w1_l[SBMAX_l], w2_l[SBMAX_l];
static	double			w1_s[SBMAX_s], w2_s[SBMAX_s];

static	int				numlines_l  [CBANDS];

static	int				partition_l [HBLKSIZE];
static	int				partition_s [HBLKSIZE_s];
static	double			s3_l        [CBANDS][CBANDS];

#endif		/* NEW_L3PARM_TABLES */



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





#if NEW_L3PARM_TABLES

static	void			L3para_read (int sfreq);

#if !ORG_NUMLINES_NORM
static	void			calc_normed_spreading
(
	int						cbmax,			/* number of lines and rows           */
	const double			bval[],			/* input values to compute the matrix */
	FLOAT					s3_ptr[],		/* the resulting non-zero entries     */
	int						lo_s3[],
	int						hi_s3[],
	const double			norm[]
);
#endif

#else

static	void			L3para_read
(
	int						sfreq,
	int						numlines_l[CBANDS],
	int						partition_l[HBLKSIZE],
	double					minval[CBANDS],
	double					qthr_l[CBANDS],
	double					norm_l[CBANDS],
	double					s3_l[CBANDS][CBANDS],
	int						partition_s[HBLKSIZE_s],
	double					qthr_s[CBANDS_s],
	double					norm_s[CBANDS_s],
	double					SNR_s[CBANDS_s],
	int						cbw_l[SBMAX_l],
	int						bu_l[SBMAX_l],
	int						bo_l[SBMAX_l],
	double					w1_l[SBMAX_l],
	double					w2_l[SBMAX_l],
	int						cbw_s[SBMAX_s],
	int						bu_s[SBMAX_s],
	int						bo_s[SBMAX_s],
	double					w1_s[SBMAX_s],
	double					w2_s[SBMAX_s]
);

#endif





/*____ psycho_anal_init() ___________________________________________________*/

void					psycho_anal_init (double sfreq)
{
	unsigned int			ch, sfb, b, i, j;


	/* reset the r, phi_sav "ring buffer" indices */

	old = 1 - (new_ = oldest = 0);


	/* clear the ratio arrays */

	for (ch = 0;  ch < 2;  ch++)
	{
		for (sfb = 0;  sfb < SBMAX_l;  sfb++)
			ratio[ch][sfb] = 0.0;

		for (sfb = 0;  sfb < SBMAX_s;  sfb++)
			for (b = 0;  b < 3;  b++)
				ratio_s[ch][sfb][b] = 0.0;
	}


	/* clear preecho arrays */

	for (ch = 0;  ch < 2;  ch++)
	{
		for (i = 0;  i < CBANDS;  i++)
		{
			nb_1[ch][i] = 0;
			nb_2[ch][i] = 0;
		}
	}


	/* clear blocktype information */

	for (ch = 0;  ch < 2;  ch++)
		blocktype_old[ch] = NORM_TYPE;


	sync_flush =  768;
	flush      =  576;
	syncsize   = 1344;   /* sync_flush + flush */


#if RING_BUFFER==1
	for (ch = 0;  ch < 2;  ch++)
		savebuf_start_idx[ch] = 0;
#endif


	/* calculate HANN window coefficients */

    for (i = 0;  i < BLKSIZE;  i++)
		window[i] = (FLOAT) (0.5 * (1 - cos (2.0 * PI * (i - 0.5) / BLKSIZE)));

    for (i = 0;  i < BLKSIZE_s;  i++)
		window_s[i] = (FLOAT) (0.5 * (1 - cos (2.0 * PI * (i - 0.5) / BLKSIZE_s)));


	/* reset states used in unpredictability measure */

	for (ch = 0;  ch < 2;  ch++)
	{
		for (i = 0;  i < 2;  i++)
		{
			for (j = 0;  j < 6;  j++)
			{
				      r[ch][i][j] = 0.0;
				phi_sav[ch][i][j] = 0.0;
			}
		}
	}


#if NEW_L3PARM_TABLES
	L3para_read ((int) sfreq);
#else
	L3para_read
	(
		(int) sfreq,
		numlines_l, partition_l, minval, qthr_l, norm_l, s3_l,
		partition_s, qthr_s, norm_s, SNR_s,
		cbw_l, bu_l, bo_l, w1_l, w2_l,
		cbw_s, bu_s, bo_s, w1_s, w2_s
	);
#endif


	/* Set unpredicatiblility of remaining spectral lines to 0.4 */

	for (j = 206;  j < HBLKSIZE;  j++)
		cw[j] = 0.4;
}





/*____ psycho_anal_exit() ___________________________________________________*/

void psycho_anal_exit( void )
{
	/* nothing to do */
}





/*____ psycho_anal() ________________________________________________________*/
									
void					psycho_anal
(
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

#if !ORG_NUMLINES_NORM && NEW_L3PARM_TABLES
	FLOAT					*s3_ptr;
#endif

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
		ratio_d[sfb] = ratio[ch][sfb];

	for (sfb = 0;  sfb < SBMAX_s;  sfb++)
		for (b = 0;  b < 3;  b++)
			ratio_ds[sfb][b] = ratio_s[ch][sfb][b];
	

	if (ch == 0)
		old = 1 - (new_ = oldest = old);


#if ORG_BUFFERS
	/**********************************************************************
	*  Delay signal by sync_flush=768 samples                             *
	**********************************************************************/

#	if RING_BUFFER==0
		for (j = 0;  j < sync_flush;  j++)   /* for long window samples */
			savebuf[j] = savebuf[j+flush];

		for (j = sync_flush;  j < syncsize;  j++)
			savebuf[j] = *buffer++;
#	else
		beg = savebuf_start_idx[ch] = (savebuf_start_idx[ch] + flush) & 2047;

		idx = (beg + sync_flush) & 2047;
		fin = (idx + flush) & 2047;
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
		wsamp_r[j] = window[k++] * savebuf[idx++];
		wsamp_i[j] = window[k++] * savebuf[idx++];
	}
#else
	j = 0;  k = 0;
	idx = beg;
	fin = (idx + BLKSIZE) & 2047;
	if (idx >= fin)
	{
		while (idx < 2048)
		{
			wsamp_r[j] = window[k++] * savebuf[idx++];
			wsamp_i[j] = window[k++] * savebuf[idx++];
			j++;
		}
		idx = 0;
	}
	while (idx < fin)
	{
		wsamp_r[j] = window[k++] * savebuf[idx++];
		wsamp_i[j] = window[k++] * savebuf[idx++];
		j++;
	}
#endif

	fft(wsamp_r, wsamp_i, energy, phi, BLKSIZE);   /* long FFT */

	for (j = 0;  j < 6;  j++)
	{	/* calculate unpredictability measure cw */
		double r1, phi1;
		                     r_prime = 2.0 *       r[ch][old][j] -       r[ch][oldest][j];
		                   phi_prime = 2.0 * phi_sav[ch][old][j] - phi_sav[ch][oldest][j];
		      r[ch][new_][j] = (FLOAT) (  r1 = sqrt((double) energy[j]));
		phi_sav[ch][new_][j] = (FLOAT) (phi1 =                  phi[j] );

		temp3 = r1 + fabs(r_prime);
		if (temp3 != 0.0)
		{
			temp1 = r1 * cos(phi1) - r_prime * cos(phi_prime);
			temp2 = r1 * sin(phi1) - r_prime * sin(phi_prime);
			cw[j] = sqrt(temp1*temp1 + temp2*temp2) / temp3;
		}
		else
			cw[j] = 0;
	}


/**********************************************************************
*     compute unpredicatibility of next 200 spectral lines            *
**********************************************************************/ 

	for (b = 0;  b < 3;  b++)
	{
#if RING_BUFFER==0
		for (j = 0, k = 0, idx = 128*(2 + b);  j < BLKSIZE_s/2;  j++)
		{	/* window data with HANN window */
			wsamp_r[j] = window_s[k++] * savebuf[idx++];
			wsamp_i[j] = window_s[k++] * savebuf[idx++];
		}
#else
		j = 0;  k = 0;
		idx = (beg + 128*(2 + b)) & 2047;
		fin = (idx + BLKSIZE_s) & 2047;
		if (idx >= fin)
		{
			while (idx < 2048)
			{
				wsamp_r[j] = window_s[k++] * savebuf[idx++];
				wsamp_i[j] = window_s[k++] * savebuf[idx++];
				j++;
			}
			idx = 0;
		}
		while (idx < fin)
		{
			wsamp_r[j] = window_s[k++] * savebuf[idx++];
			wsamp_i[j] = window_s[k++] * savebuf[idx++];
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
			cw[j] = sqrt(temp1*temp1 + temp2*temp2) / temp3;
		}
		else
			cw[j] = 0.0;

		cw[j+1] = cw[j+2] = cw[j+3] = cw[j];
	}


/**********************************************************************
*    Calculate the energy and the unpredictability in the threshold   *
*    calculation partitions                                           *
**********************************************************************/

#if ORG_NUMLINES_NORM || !NEW_L3PARM_TABLES

	for (b = 0;  b < cbmax_l;  b++)
	{
		eb[b] = 0.0;
		cb[b] = 0.0;
	}
	for (j = 0;  j < HBLKSIZE;  j++)
	{
		int tp = partition_l[j];
		if (tp >= 0)
		{
			eb[tp] += energy[j];
			cb[tp] += cw[j] * energy[j];
		}
	}

#else

	j = 0;
	for (b = 0;  b < cbmax_l;  b++)
	{
		eb[b] = 0.0;
		cb[b] = 0.0;

		/*
			Calculate the energy and the unpredictability in the threshold
			calculation partitions

			cbmax_l holds the number of valid numlines_l entries
		*/
		k = numlines_l[b];
		do {
			eb[b] += energy[j];
			cb[b] += cw[j] * energy[j];
		} while (j++, --k);
	}

	s3_ptr = normed_s3_l;

#endif


	*pe = 0.0;
	
	for (b = 0;  b < cbmax_l;  b++)
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
#if ORG_NUMLINES_NORM || !NEW_L3PARM_TABLES
		for (k = 0;  k < cbmax_l;  k++)
		{
			ecb += s3_l[b][k] * eb[k];  /* sprdngf for Layer III */
			ctb += s3_l[b][k] * cb[k];
		}
#else
		for (k = lo_s3_l[b];  k < hi_s3_l[b];  k++)
		{
			ecb += *s3_ptr   * eb[k];  /* sprdngf for Layer III */
			ctb += *s3_ptr++ * cb[k];
		}
#endif


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
		SNR_l = MAX (minval[b], 23.0 * tbb + 6.0);   /* 29*tbb + 6*(1-tbb) */
	
		/* calculate the threshold for each partition */
#if ORG_NUMLINES_NORM || !NEW_L3PARM_TABLES
	    nb = ecb * norm_l[b] * exp(-SNR_l * LN_TO_LOG10);
#else
	    nb = ecb * exp(-SNR_l * LN_TO_LOG10);   /* our ecb is already normed */
#endif

		/*
			pre-echo control
		*/
		thr[b] = MAX (qthr_l[b], MIN(nb, nb_2[ch][b]));
		nb_2[ch][b] = MIN(2.0 * nb, 16.0 * nb_1[ch][b]);
	    nb_1[ch][b] = nb;


		/*
			calculate percetual entropy

			thr[b] -> thr[b]+1.0 : for non sound portition
		*/
		if (eb[b] > thr[b])
			*pe += numlines_l[b] * log((eb[b]+1.0) / (thr[b]+1.0));
	}
	

#define switch_pe  1800
	

	if (*pe < switch_pe)
	{
		/* no attack : use long blocks */

		if (blocktype_old[ch] == SHORT_TYPE)
			blocktype = STOP_TYPE;
		else   /* NORM_TYPE, STOP_TYPE */
			blocktype = NORM_TYPE;


		/* threshold calculation (part 2) */

		for (sfb = 0;  sfb < SBMAX_l;  sfb++)
		{
			int		bu = bu_l[sfb];
			int		bo = bo_l[sfb];
			double	en = w1_l[sfb] * eb[bu] + w2_l[sfb] * eb[bo];

			for (b = bu+1;  b < bo;  b++)
				en += eb[b];

			if (en != 0.0)
			{
				double	thm = w1_l[sfb] * thr[bu] + w2_l[sfb] * thr[bo];

				for (b = bu+1;  b < bo;  b++)
					thm += thr[b];

				ratio[ch][sfb] = thm / en;
			}
			else
				ratio[ch][sfb] = 0.0;
		}
	}
	else
	{
		/* attack : use short blocks */
		blocktype = SHORT_TYPE;
#if ORG_BLOCK_SELECT
		if (blocktype_old[ch] == NORM_TYPE)
			blocktype_old[ch] = START_TYPE;
		else   /* SHORT_TYPE, STOP_TYPE */
			blocktype_old[ch] = SHORT_TYPE;
#else   /* ISO */
		if (blocktype_old[ch] == SHORT_TYPE)
			blocktype_old[ch] = SHORT_TYPE;
		else   /* NORM_TYPE, STOP_TYPE */
			blocktype_old[ch] = START_TYPE;
#endif


		/* threshold calculation for short blocks */

		for (sblock = 0;  sblock < 3;  sblock++)
		{
#if ORG_NUMLINES_NORM || !NEW_L3PARM_TABLES

			for (b = 0;  b < cbmax_s;  b++)
				eb[b] = 0.0;

			for (j = 0;  j < HBLKSIZE_s;  j++)
				eb[partition_s[j]] += energy_s[sblock][j];

#else

			j = 0;
			for (b = 0;  b < cbmax_s;  b++)
			{
				eb[b] = 0.0;

				/*
					Calculate the energy and the unpredictability in the threshold
					calculation partitions

					cbmax_s holds the number of valid numlines_s entries
				*/
				k = numlines_s[b];
				do {
					eb[b] += energy_s[sblock][j];
				} while (j++, --k);
			}

			s3_ptr = normed_s3_s;
#endif

			for (b = 0;  b < cbmax_s;  b++)
			{
				FLOAT					nb;
				FLOAT					ecb = 0.0;

#if ORG_NUMLINES_NORM || !NEW_L3PARM_TABLES
				for (k = 0;  k < cbmax_s;  k++)
					ecb += s3_l[b][k] * eb[k];

				nb = ecb * norm_l[b] * exp((double) SNR_s[b] * LN_TO_LOG10);
#else
				for (k = lo_s3_s[b];  k < hi_s3_s[b];  k++)
					ecb += *s3_ptr++ * eb[k];

				nb = ecb * exp((double) SNR_s[b] * LN_TO_LOG10);   /* our ecb is already normed */
#endif
				thr[b] = MAX(qthr_s[b], nb);
			}

			for (sfb = 0;  sfb < SBMAX_s;  sfb++)
			{
				int		bu = bu_s[sfb];
				int		bo = bo_s[sfb];
				double	en = w1_s[sfb] * eb[bu] + w2_s[sfb] * eb[bo];

				for (b = bu+1;  b < bo;  b++)
					en += eb[b];
				if (en != 0.0)
				{
					double	thm = w1_s[sfb] * thr[bu] + w2_s[sfb] * thr[bo];

					for (b = bu+1;  b < bo;  b++)
						thm += thr[b];

					ratio_s[ch][sfb][sblock] = thm / en;
				}
				else
					ratio_s[ch][sfb][sblock] = 0.0;
			}
		}
	} 
	
	cod_info->block_type = blocktype_old[ch];
	blocktype_old[ch] = blocktype;

	if ( cod_info->block_type == NORM_TYPE )
	    cod_info->window_switching_flag = 0;
	else
	    cod_info->window_switching_flag = 1;

	cod_info->mixed_block_flag = 0;
}





/*____ L3para_read() __________________________________________________________*/

#if NEW_L3PARM_TABLES

static void				L3para_read (int sfreq)
{
	int						sfreq_idx;
	l3_parm_block			*parm;
	double					*bval_l, *bval_s;

#if ORG_NUMLINES_NORM
	int						cbmax_l, cbmax_s;
	int						i, j, k;
#else
	double					*norm_l, *norm_s;
#endif


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
	cbmax_l    = parm->long_data.cbmax_l;

#if ORG_NUMLINES_NORM
	for (i = 0, j = 0;  i < cbmax_l;  i++)
	{
		numlines_l[i] = parm->long_data.numlines_l[i];

		for (k = 0;  k < numlines_l[i];  k++)
			partition_l[j++] = i;
	}
#else
	numlines_l = parm->long_data.numlines_l;
#endif
		
	minval     = parm->long_data.minval;
	qthr_l     = parm->long_data.qthr_l;
	norm_l     = parm->long_data.norm_l;
	bval_l     = parm->long_data.bval_l;


	/*
		Compute the normed spreading function norm_l[i] * s3_l[i][j]
	*/
#if ORG_NUMLINES_NORM
	for (i = 0;  i < cbmax_l;  i++)
	{
		double x, temp, tempx, tempy;

		for (j = 0;  j < cbmax_l;  j++)
		{
/*			tempx = (bval_l[i]-bval_l[j]) * 1.05; */
			if (j >= i)
				tempx = (bval_l[i]-bval_l[j]) * 3.0;
			else
				tempx = (bval_l[i]-bval_l[j]) * 1.5;
/*			if (j >= i)  tempx = (bval_l[j]-bval_l[i]) * 3.0;
			else         tempx = (bval_l[j]-bval_l[i]) * 1.5; */
			if (tempx > 0.5  &&  tempx < 2.5)
			{
				temp = tempx - 0.5;
				x = 8.0 * temp * (temp-2.0);
			}
			else  x = 0.0;
			tempx += 0.474;
			tempy = 15.811389 + 7.5*tempx - 17.5*sqrt(1.0+tempx*tempx);
			if (tempy <= -60.0)  s3_l[i][j] = 0.0;
			else                 s3_l[i][j] = exp((x + tempy) * LN_TO_LOG10);
		}
	}
#else
	calc_normed_spreading (cbmax_l, bval_l, normed_s3_l, lo_s3_l, hi_s3_l, norm_l);
#endif


	/*
		Read short block data
	*/
	cbmax_s    = parm->short_data.cbmax_s;

#if ORG_NUMLINES_NORM
	for (i = 0, j = 0;  i < cbmax_s;  i++)
	{
		numlines_l[i] = parm->short_data.numlines_s[i];

		for (k = 0;  k < numlines_l[i];  k++)
			partition_s[j++] = i;
	}
#else
	numlines_s = parm->short_data.numlines_s;
#endif

	qthr_s     = parm->short_data.qthr_s;
	norm_s     = parm->short_data.norm_s;
	SNR_s      = parm->short_data.SNR_s;
	bval_s     = parm->short_data.bval_s;


#if !ORG_NUMLINES_NORM

	/*
		Compute the normed spreading function norm_s[i] * s3_s[i][j]
	*/
	calc_normed_spreading (cbmax_s, bval_s, normed_s3_s, lo_s3_s, hi_s3_s, norm_s);

#endif


	/*
		Read long block data for converting threshold
		calculation partitions to scale factor bands
	*/
	cbw_l = parm->long_thres.cbw_l;
	bu_l  = parm->long_thres.bu_l;
	bo_l  = parm->long_thres.bo_l;
	w1_l  = parm->long_thres.w1_l;
	w2_l  = parm->long_thres.w2_l;


	/*
		Read short block data for converting threshold
		calculation partitions to scale factor bands
	*/
	cbw_s = parm->short_thres.cbw_s;
	bu_s  = parm->short_thres.bu_s;
	bo_s  = parm->short_thres.bo_s;
	w1_s  = parm->short_thres.w1_s;
	w2_s  = parm->short_thres.w2_s;
}

#else		/* NEW_L3PARM_TABLES */

static	void			L3para_read
(
	int						sfreq,
	int						numlines_l[CBANDS],
	int						partition_l[HBLKSIZE],
	double					minval[CBANDS],
	double					qthr_l[CBANDS],
	double					norm_l[CBANDS],
	double					s3_l[CBANDS][CBANDS],
	int						partition_s[HBLKSIZE_s],
	double					qthr_s[CBANDS_s],
	double					norm_s[CBANDS_s],
	double					SNR_s[CBANDS_s],
	int						cbw_l[SBMAX_l],
	int						bu_l[SBMAX_l],
	int						bo_l[SBMAX_l],
	double					w1_l[SBMAX_l],
	double					w2_l[SBMAX_l],
	int						cbw_s[SBMAX_s],
	int						bu_s[SBMAX_s],
	int						bo_s[SBMAX_s],
	double					w1_s[SBMAX_s],
	double					w2_s[SBMAX_s]
)
{
	static	double			bval_l[CBANDS];
	int						cbmax_tp;

	int						sbmax;
	int						i, j, k, k2;


	psyDataElem				*rpa1;
	psyDataElem2			*rpa2;
	psyDataElem3			*rpa3;


/* Read long block data */

	switch (sfreq)
	{
		case 32000:  rpa1 = psy_longBlock_32000_58;  cbmax_tp = 59;  break;
		case 44100:  rpa1 = psy_longBlock_44100_62;  cbmax_tp = 63;  break;
		case 48000:  rpa1 = psy_longBlock_48000_61;  cbmax_tp = 62;  break;
		default   :  return;  /* Just to avoid compiler warnings */
	}

	for (i = 0, k2 = 0;  i < cbmax_tp;  i++)
	{
		numlines_l[i] = rpa1->lines;
		minval[i]     = rpa1->minVal;
		qthr_l[i]     = rpa1->qthr;
		norm_l[i]     = rpa1->norm;
		bval_l[i]     = rpa1->bVal;
		rpa1++;

		for (k = 0;  k < numlines_l[i];  k++)
			partition_l[k2++] = i;
	}

		
/************************************************************************
 * Now compute the spreading function, s[j][i], the value of the spread-*
 * ing function, centered at band j, for band i, store for later use    *
 ************************************************************************/

	for (i = 0;  i < cbmax_tp;  i++)
	{
		double x, temp, tempx, tempy;

		for (j = 0;  j < cbmax_tp;  j++)
		{
/*			tempx = (bval_l[i]-bval_l[j]) * 1.05; */
			if (j >= i)
				tempx = (bval_l[i]-bval_l[j]) * 3.0;
			else
				tempx = (bval_l[i]-bval_l[j]) * 1.5;
/*			if (j >= i)  tempx = (bval_l[j]-bval_l[i]) * 3.0;
			else         tempx = (bval_l[j]-bval_l[i]) * 1.5; */
			if (tempx > 0.5  &&  tempx < 2.5)
			{
				temp = tempx - 0.5;
				x = 8.0 * temp * (temp-2.0);
			}
			else  x = 0.0;
			tempx += 0.474;
			tempy = 15.811389 + 7.5*tempx - 17.5*sqrt(1.0+tempx*tempx);
			if (tempy <= -60.0)  s3_l[i][j] = 0.0;
			else                 s3_l[i][j] = exp((x + tempy) * LN_TO_LOG10);
		}
	}


/* Read short block data */

	switch (sfreq)
	{
		case 32000:  rpa2 = psy_shortBlock_32000_41;  cbmax_tp = 42;  break;
		case 44100:  rpa2 = psy_shortBlock_44100_38;  cbmax_tp = 39;  break;
		case 48000:  rpa2 = psy_shortBlock_48000_37;  cbmax_tp = 38;  break;
		default   :  return;  /* Just to avoid compiler warnings */
	}

	for (i = 0, k2 = 0;  i < cbmax_tp;  i++)
	{
		numlines_l[i] = rpa2->lines;
		qthr_s[i]     = rpa2->qthr;
		norm_s[i]     = rpa2->norm;
		SNR_s[i]      = rpa2->snr;
		rpa2++;

		for (k = 0;  k < numlines_l[i];  k++)
			partition_s[k2++] = i;
	}


/* Read long block data for converting threshold calculation
   partitions to scale factor bands */

	switch (sfreq)
	{
		case 32000:  rpa3 = psy_data3_32000_20;  break;
		case 44100:  rpa3 = psy_data3_44100_20;  break;
		case 48000:  rpa3 = psy_data3_48000_20;  break;
		default   :  return;  /* Just to avoid compiler warnings */
	}
	sbmax = SBMAX_l;

	for (i = 0;  i < sbmax;  i++)
	{
		cbw_l[i] = rpa3->cbw;
		bu_l[i] = rpa3->bu;
		bo_l[i] = rpa3->bo;
		w1_l[i] = rpa3->w1;
		w2_l[i] = rpa3->w2;
		rpa3++;		
	}


/* Read short block data for converting threshold calculation
   partitions to scale factor bands */

	switch (sfreq)
	{
		case 32000:  rpa3 = psy_data4_32000_11;  break;
		case 44100:  rpa3 = psy_data4_44100_11;  break;
		case 48000:  rpa3 = psy_data4_48000_11;  break;
		default   :  return;  /* Just to avoid compiler warnings */
	}
	sbmax = SBMAX_s;

	for (i = 0;  i < sbmax;  i++)
	{
		cbw_s[i] = rpa3->cbw;
		bu_s[i] = rpa3->bu;
		bo_s[i] = rpa3->bo;
		w1_s[i] = rpa3->w1;
		w2_s[i] = rpa3->w2;
		rpa3++;		
	}	
}

#endif		/* NEW_L3PARM_TABLES */





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
