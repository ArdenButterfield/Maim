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

	2000-11-04  Andre Piotrowski

	-	some cosmetics
	-	included "l3psy.h" to use the defined block type constants
	-	negligible speed up of mdct_sub()
	-	speed up of mdct()

	2000-12-10  ap

	-	speed up of mdct_sub() and mdct() - MDCT_CHANGE_LEVEL 5
*/

/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
 *
 * $Id: mdct.c,v 1.1 1996/02/14 04:04:23 rowlands Exp $
 *
 * $Log: mdct.c,v $
 * Revision 1.1  1996/02/14 04:04:23  rowlands
 * Initial revision
 *
 * Received from Mike Coleman
 **********************************************************************/

#include "common.h"
#include <assert.h>
#include "l3side.h"
#include "l3psy.h"
#include "mdct.h"





/*
	This is table B.9: coefficients for aliasing reduction
*/
static	double			c[8] = { -0.6, -0.535, -0.33, -0.185, -0.095, -0.041, -0.0142, -0.0037 };





int						gr_idx[3] = {0,1,2};





#if 0
static	double			ca[8];
#endif
static	double			cs[8];





int	fInit_mdct_sub;

void mdct_sub
(
	L3SBS					(*sb_sample),
	double					(*mdct_freq)[2][576],
	int						stereo,
	III_side_info_t			*l3_side,
	int						mode_gr
)
{
	gr_info					*cod_info;
#if MDCT_CHANGE_LEVEL < 5
	double					mdct_in[36];
#endif
	int						gr, ch, band, k, j;
	double					bu, bd;
	int						block_type;
	double					(*mdct_enc_long)[2][32][18] = (double (*)[2][32][18]) mdct_freq;

	if (!fInit_mdct_sub)
	{
		/* prepare the aliasing reduction butterflies */
		for (k = 0;  k < 8;  k++)
		{
			double sq = sqrt (1.0 + c[k]*c[k]);
#if 0
			ca[k] = c[k] / sq;
#endif
			cs[k] = 1.0 / sq;
		}
		fInit_mdct_sub++;
	}


	for (gr = 0;  gr < mode_gr;  gr++)
	{
		int		pre_gr = gr_idx[gr  ];
		int		cur_gr = gr_idx[gr+1];

		for (ch = 0;  ch < stereo;  ch++)
		{
			double (*mdct_enc)[18] = mdct_enc_long[gr][ch];

			cod_info = &l3_side->gr[gr].ch[ch].tt;
			block_type = cod_info->block_type;

			/* Compensate for inversion in the analysis filter */
			for (k = 1;  k < 18;  k++)
				if (k & 1)
					for (band = 1;  band < 32;  band++)
						if (band & 1)
							(*sb_sample)[ch][cur_gr][k][band] *= -1.0;
			/*
				Perform imdct of 18 previous subband samples
				+ 18 current subband samples
			*/
			for (band = 0;  band < 32;  band++)
			{
#if MDCT_CHANGE_LEVEL < 5
				for (k = 0;  k < 18;  k++)
				{
					mdct_in[k]    = (*sb_sample)[ch][pre_gr][k][band];
					mdct_in[k+18] = (*sb_sample)[ch][cur_gr][k][band];
				}
				if (cod_info->mixed_block_flag  &&  (band < 2))
					block_type = NORM_TYPE;   /* AND WHEN WILL THE BLOCK_TYPE BE SWITCHED BACK? */

			                /* &mdct_enc[gr][ch][band][0] */
				mdct (mdct_in, mdct_enc/*[gr][ch]*/[band], block_type);
#else
				mdct ((*sb_sample)[ch][pre_gr], (*sb_sample)[ch][cur_gr], band, mdct_enc[band], block_type);
#endif
			}

			/*
				Perform aliasing reduction butterfly
				on long blocks
			*/

			if (block_type != SHORT_TYPE)
				for (band = 0;  band < 31;  band++)
					for (k = 0;  k < 8;  k++)
					{
#if 1	/* This is faster because the calculation can be done more sequential */
						bu = (mdct_enc[band  ][17-k] + mdct_enc[band+1][   k] * c[k]) * cs[k];
						bd = (mdct_enc[band+1][   k] - mdct_enc[band  ][17-k] * c[k]) * cs[k];
						mdct_enc[band  ][17-k] = bu;
						mdct_enc[band+1][   k] = bd;
#else
						bu = mdct_enc[band  ][17-k] * cs[k] + mdct_enc[band+1][   k] * ca[k];
						bd = mdct_enc[band+1][   k] * cs[k] - mdct_enc[band  ][17-k] * ca[k];
						mdct_enc[band  ][17-k] = bu;
						mdct_enc[band+1][   k] = bd;
#endif
					}
		}
	}

	/*
		Save latest granule's subband samples to be used in
		the next mdct call
	*/
	j = gr_idx[mode_gr];
	for (k = mode_gr;  k > 0;  k--)
		gr_idx[k] = gr_idx[k-1];
	gr_idx[0] = j;
}





/*-------------------------------------------------------------------*/
/*                                                                   */
/*   Function: Calculation of the MDCT                               */
/*   In the case of long blocks ( block_type 0,1,3 ) there are       */
/*   36 coefficents in the time domain and 18 in the frequency       */
/*   domain.                                                         */
/*   In the case of short blocks (block_type 2 ) there are 3         */
/*   transformations with short length. This leads to 12 coefficents */
/*   in the time and 6 in the frequency domain. In this case the     */
/*   results are stored side by side in the vector out[].            */
/*                                                                   */
/*   New layer3                                                      */
/*                                                                   */
/*-------------------------------------------------------------------*/

int	fInit_mdct;



#if MDCT_CHANGE_LEVEL == 5

void mdct
(
	double					inA[18][32],
	double					inB[18][32],
	int						band,
	double					*out,
	int						block_type
)
{
	static	double			cos_l[18][18];
	static	double			cos_s[ 6][ 6];

	static	double			winNorm [18];
	static	double			winShort[ 6];

	int						k, m, N;
	double					t1, t2, t3, t4, t5, t6, u1, u2;


	if (!fInit_mdct)
   	{
		N = 36;
		for (k = 0;  k < N/2;  k++)  winNorm [k] = sin (PI/36 * (k+0.5));
		for (k = 0;  k < N/4;  k++)  for (m = 0;  m < N/2;  m++)  cos_l[k][m] = cos((PI/(2*N)) * (2* k     +1+N/2) * (2*m+1)) / (N/4);
		for (     ;  k < N/2;  k++)  for (m = 0;  m < N/2;  m++)  cos_l[k][m] = cos((PI/(2*N)) * (2*(k+N/4)+1+N/2) * (2*m+1)) / (N/4);

		N = 12;
		for (k = 0;  k < N/2;  k++)  winShort[k] = sin (PI/12 * (k+0.5));
		for (k = 0;  k < N/4;  k++)  for (m = 0;  m < N/2;  m++)  cos_s[k][m] = cos((PI/(2*N)) * (2*k      +1+N/2) * (2*m+1)) / (N/4);
		for (     ;  k < N/2;  k++)  for (m = 0;  m < N/2;  m++)  cos_s[k][m] = cos((PI/(2*N)) * (2*(k+N/4)+1+N/2) * (2*m+1)) / (N/4);

		fInit_mdct++;
	}

	for (m = 0;  m < 18;  m++)  out[m] = 0.0;
	switch (block_type)
	{
		case  NORM_TYPE:
			for (k = 0;  k < 9;  k++)
			{
				t1 = winNorm [   k] * inA[k][band] - winNorm [17-k] * inA[17-k][band];
				t2 = winNorm [17-k] * inB[k][band] + winNorm [   k] * inB[17-k][band];
				for (m = 0;  m < 18;  m++)
					out[m] += t1 * cos_l[k][m] + t2 * cos_l[k+9][m];
			}
			break;

		case START_TYPE:
			for (k = 0;  k < 6;  k++)
			{
				t1 = winNorm [   k] * inA[k][band] - winNorm [17-k] * inA[17-k][band];
				t2 =                  inB[k][band];
				for (m = 0;  m < 18;  m++)
					out[m] += t1 * cos_l[k][m] + t2 * cos_l[k+9][m];
			}
			for (k =  6;  k <  9;  k++)
			{
				t1 = winNorm [   k] * inA[k][band] - winNorm [17-k] * inA[17-k][band];
				t2 = winShort[11-k] * inB[k][band] + winShort[k- 6] * inB[17-k][band];
				for (m = 0;  m < 18;  m++)
					out[m] += t1 * cos_l[k][m] + t2 * cos_l[k+9][m];
			}
			break;

		case  STOP_TYPE:
			for (k =  0;  k <  6;  k++)
			{
				t1 =                                                 -inA[17-k][band];
				t2 = winNorm [17-k] * inB[k][band] + winNorm [   k] * inB[17-k][band];
				for (m = 0;  m < 18;  m++)
					out[m] += t1 * cos_l[k][m] + t2 * cos_l[k+9][m];
			}
			for (k =  6;  k <  9;  k++)
			{
				t1 = winShort[k- 6] * inA[k][band] - winShort[11-k] * inA[17-k][band];
				t2 = winNorm [17-k] * inB[k][band] + winNorm [   k] * inB[17-k][band];
				for (m = 0;  m < 18;  m++)
					out[m] += t1 * cos_l[k][m] + t2 * cos_l[k+9][m];
			}
			break;

		case SHORT_TYPE:
			for (k =  0;  k <  3;  k++)
			{
				u1 = winShort[k];  u2 = winShort[5-k];
				t1 = u1 * inA[k+ 6][band] - u2 * inA[11-k][band];
				t2 = u2 * inA[k+12][band] + u1 * inA[17-k][band];
				t3 = u1 * inA[k+12][band] - u2 * inA[17-k][band];
				t4 = u2 * inB[k   ][band] + u1 * inB[ 5-k][band];
				t5 = u1 * inB[k   ][band] - u2 * inB[ 5-k][band];
				t6 = u2 * inB[k+ 6][band] + u1 * inB[11-k][band];
				for (m = 0;  m <  6;  m++)
				{
					u1 = cos_s[k][m];  u2 = cos_s[k+3][m];
					out[3*m  ] += t1 * u1 + t2 * u2;
					out[3*m+1] += t3 * u1 + t4 * u2;
					out[3*m+2] += t5 * u1 + t6 * u2;
				}
			}
	}
}

#elif MDCT_CHANGE_LEVEL == 4   /* reduce number of multiplications to nearly the half once more! (cos_x[9+k][m] = -cos_x[9-k][m] and cos_x[35+k][m] = cos_x[35-k][m]) */

void mdct
(
	double					*in,
	double					*out,
	int						block_type
)
{
	static	double			cos_l[36][18];
	static	double			cos_s[12][ 6];

	static	double			winNorm [36];
	static	double			winShort[12];
	static	double			*winStart = winShort-18;
	static	double			*winStop  = winShort- 6;

	int						l, k, m, N;
	double					temp;


	if (!fInit_mdct)
	{
		N = 36;
		for (k = 0;  k < N;  k++)  winNorm [k] = sin (PI/36 * (k+0.5));
		for (k = 0;  k < N;  k++)  for (m = 0;  m < N/2;  m++)  cos_l[k][m] = cos((PI/(2*N)) * (2*k+1+N/2) * (2*m+1)) / (N/4);

		N = 12;
		for (k = 0;  k < N;  k++)  winShort[k] = sin (PI/12 * (k+0.5));
		for (k = 0;  k < N;  k++)  for (m = 0;  m < N/2;  m++)  cos_s[k][m] = cos((PI/(2*N)) * (2*k+1+N/2) * (2*m+1)) / (N/4);

		fInit_mdct++;
	}

	for (m = 0;  m < 18;  m++)  out[m] = 0.0;
	switch (block_type)
	{
		case  NORM_TYPE:
			for (k =  0;  k <  9;  k++)  {temp = (winNorm [k] * in[k] - winNorm [17-k] * in[17-k]);  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			for (k = 18;  k < 27;  k++)  {temp = (winNorm [k] * in[k] + winNorm [53-k] * in[53-k]);  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			break;

		case START_TYPE:
			for (k =  0;  k <  9;  k++)  {temp = (winNorm [k] * in[k] - winNorm [17-k] * in[17-k]);  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			for (k = 18;  k < 24;  k++)  {temp =               in[k];  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			for (k = 24;  k < 27;  k++)  {temp = (winStart[k] * in[k] + winStart[53-k] * in[53-k]);  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			break;

		case  STOP_TYPE:
			for (k =  6;  k <  9;  k++)  {temp = (winStop [k] * in[k] - winStop [17-k] * in[17-k]);  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			for (k = 12;  k < 18;  k++)  {temp =               in[k];  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			for (k = 18;  k < 27;  k++)  {temp = (winNorm [k] * in[k] + winNorm [53-k] * in[53-k]);  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			break;

		case SHORT_TYPE:
			for (l = 0;  l < 3;  l++)
			{
				for (k =  0;  k <  3;  k++)  {temp = (winShort[k] * in[k+6+l*6] - winShort[ 5-k] * in[ 5-k+6+l*6]);  for (m = 0;  m <  6;  m++)  out[3*m+l] += temp * cos_s[k][m];}
				for (k =  6;  k <  9;  k++)  {temp = (winShort[k] * in[k+6+l*6] + winShort[17-k] * in[17-k+6+l*6]);  for (m = 0;  m <  6;  m++)  out[3*m+l] += temp * cos_s[k][m];}
			}
	}
}

#elif MDCT_CHANGE_LEVEL == 3   /* reduce number of multiplications to nearly the half (win_x[k]*in[k] is constant in m-loop)! flip cos_x components for faster access */

void mdct
(
	double					*in,
	double					*out,
	int						block_type
)
{
	static	double			cos_l[36][18];
	static	double			cos_s[12][ 6];

	static	double			winNorm [36];
	static	double			winShort[12];
	static	double			*winStart = winShort-18;
	static	double			*winStop  = winShort- 6;

	int						l, k, m, N;
	double					temp;


	if (!fInit_mdct)
	{
		N = 36;
		for (k = 0;  k < N;  k++)  winNorm [k] = sin (PI/36 * (k+0.5));
		for (k = 0;  k < N;  k++)  for (m = 0;  m < N/2;  m++)  cos_l[k][m] = cos((PI/(2*N)) * (2*k+1+N/2) * (2*m+1)) / (N/4);

		N = 12;
		for (k = 0;  k < N;  k++)  winShort[k] = sin (PI/12 * (k+0.5));
		for (k = 0;  k < N;  k++)  for (m = 0;  m < N/2;  m++)  cos_s[k][m] = cos((PI/(2*N)) * (2*k+1+N/2) * (2*m+1)) / (N/4);

		fInit_mdct++;
	}

	for (m = 0;  m < 18;  m++)  out[m] = 0.0;
	switch (block_type)
	{
		case  NORM_TYPE:
			for (k =  0;  k < 36;  k++)  {temp = winNorm [k] * in[k];  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			break;

		case START_TYPE:
			for (k =  0;  k < 18;  k++)  {temp = winNorm [k] * in[k];  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			for (k = 18;  k < 24;  k++)  {temp =               in[k];  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			for (k = 24;  k < 30;  k++)  {temp = winStart[k] * in[k];  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			break;

		case  STOP_TYPE:
			for (k =  6;  k < 12;  k++)  {temp = winStop [k] * in[k];  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			for (k = 12;  k < 18;  k++)  {temp =               in[k];  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			for (k = 18;  k < 36;  k++)  {temp = winNorm [k] * in[k];  for (m = 0;  m < 18;  m++)  out[m] += temp * cos_l[k][m];}
			break;

		case SHORT_TYPE:
			for (l = 0;  l < 3;  l++)
				for (k =  0;  k < 12;  k++)  {temp = winShort[k] * in[k+6+l*6];  for (m = 0;  m <  6;  m++)  out[3*m+l] += temp * cos_s[k][m];}
	}
}

#elif MDCT_CHANGE_LEVEL == 2   /* avoid calculating of some redundant values; take care for some special values */

void mdct
(
	double					*in,
	double					*out,
	int						block_type
)
{
	static	double			cos_l[18][36];
	static	double			cos_s[ 6][12];

	static	double			winNorm [36];
	static	double			winShort[12];
	static	double			*winStart = winShort-18;
	static	double			*winStop  = winShort- 6;

	int						l, k, m, N;
	double					sum;


	if (!fInit_mdct)
	{
		for (k =  0;  k < 36;  k++)  winNorm [k] = sin (PI/36 * (k+0.5));
		for (k =  0;  k < 12;  k++)  winShort[k] = sin (PI/12 * (k+0.5));

		N = 36;  for (m = 0;  m < N/2;  m++)  for (k = 0;  k < N;  k++)  cos_l[m][k] = cos((PI/(2*N)) * (2*k+1+N/2) * (2*m+1)) / (N/4);
		N = 12;  for (m = 0;  m < N/2;  m++)  for (k = 0;  k < N;  k++)  cos_s[m][k] = cos((PI/(2*N)) * (2*k+1+N/2) * (2*m+1)) / (N/4);

		fInit_mdct++;
	}

	switch (block_type)
	{
		case  NORM_TYPE:
			for (m = 0;  m < 18;  m++)
			{
				sum = 0.0;
				for (k =  0;  k < 36;  k++) sum += winNorm [k] * in[k] * cos_l[m][k];
				out[m] = sum;
			}
			break;

		case START_TYPE:
			for (m = 0;  m < 18;  m++)
			{
				sum = 0.0;
				for (k =  0;  k < 18;  k++) sum += winNorm [k] * in[k] * cos_l[m][k];
				for (k = 18;  k < 24;  k++) sum +=               in[k] * cos_l[m][k];
				for (k = 24;  k < 30;  k++) sum += winStart[k] * in[k] * cos_l[m][k];
				out[m] = sum;
			}
			break;

		case  STOP_TYPE:
			for (m = 0;  m < 18;  m++)
			{
				sum = 0.0;
				for (k =  6;  k < 12;  k++) sum += winStop [k] * in[k] * cos_l[m][k];
				for (k = 12;  k < 18;  k++) sum +=               in[k] * cos_l[m][k];
				for (k = 18;  k < 36;  k++) sum += winNorm [k] * in[k] * cos_l[m][k];
				out[m] = sum;
			}
			break;

		case SHORT_TYPE:
			for (l = 0;  l < 3;  l++)
			{
				for (m = 0;  m <  6;  m++)
				{
					sum = 0.0;
					for (k = 0;  k < 12;  k++) sum += winShort[k] * in[k+6+l*6] * cos_s[m][k];
					out[3*m+l] = sum;
				}
			}
	}
}

#elif MDCT_CHANGE_LEVEL == 1   /* reformatted for better overview; use block type constants */

void mdct
(
	double					*in,
	double					*out,
	int						block_type
)
{
	static	double			cos_l[18][36];
	static	double			cos_s[ 6][12];

	static	double			winNorm [36];
	static	double			winShort[12];
	static	double			winStart[36];
	static	double			winStop [36];

	int						l, k, m, N;
	double					sum;


	if (!fInit_mdct)
	{
		/* type 0 -- NORM_TYPE */
		for (k =  0;  k < 36;  k++)  winNorm [k] = sin (PI/36 * (k+0.5));
		/* type 1 -- START_TYPE */
		for (k =  0;  k < 18;  k++)  winStart[k] = sin (PI/36 * (k+0.5));
		for (k = 18;  k < 24;  k++)  winStart[k] = 1.0;
		for (k = 24;  k < 30;  k++)  winStart[k] = sin (PI/12 * (k+0.5 - 18));
		for (k = 30;  k < 36;  k++)  winStart[k] = 0.0;
		/* type 3 -- STOP_TYPE */
		for (k =  0;  k <  6;  k++)  winStop [k] = 0.0;
		for (k =  6;  k < 12;  k++)  winStop [k] = sin (PI/12 * (k+0.5 -  6));
		for (k = 12;  k < 18;  k++)  winStop [k] = 1.0;
		for (k = 18;  k < 36;  k++)  winStop [k] = sin (PI/36 * (k+0.5));
		/* type 2 -- SHORT_TYPE */
		for (k =  0;  k < 12;  k++)  winShort[k] = sin (PI/12 * (k+0.5));

		N = 12;  for (m = 0;  m < N/2;  m++)  for (k = 0;  k < N;  k++)  cos_s[m][k] = cos((PI/(2*N)) * (2*k+1+N/2) * (2*m+1)) / (N/4);
		N = 36;  for (m = 0;  m < N/2;  m++)  for (k = 0;  k < N;  k++)  cos_l[m][k] = cos((PI/(2*N)) * (2*k+1+N/2) * (2*m+1)) / (N/4);

		fInit_mdct++;
	}

	switch (block_type)
	{
	case  NORM_TYPE:  N = 36;                             for (m = 0;  m < N/2;  m++)  {sum = 0.0;  for (k = 0;  k < N;  k++) sum += winNorm [k] * in[k      ] * cos_l[m][k];  out[  m  ] = sum;}  break;
	case START_TYPE:  N = 36;                             for (m = 0;  m < N/2;  m++)  {sum = 0.0;  for (k = 0;  k < N;  k++) sum += winStart[k] * in[k      ] * cos_l[m][k];  out[  m  ] = sum;}  break;
	case  STOP_TYPE:  N = 36;                             for (m = 0;  m < N/2;  m++)  {sum = 0.0;  for (k = 0;  k < N;  k++) sum += winStop [k] * in[k      ] * cos_l[m][k];  out[  m  ] = sum;}  break;
	case SHORT_TYPE:  N = 12;  for (l = 0;  l < 3;  l++) {for (m = 0;  m < N/2;  m++)  {sum = 0.0;  for (k = 0;  k < N;  k++) sum += winShort[k] * in[k+6+l*6] * cos_s[m][k];  out[3*m+l] = sum;}}
	}
}

#endif /* MDCT_CHANGE_LEVEL */
