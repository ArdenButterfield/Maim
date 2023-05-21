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

	2000-11-04  Andre Piotrowski

	-	speed up: don«t calculate the unneeded phi values!

	2000-11-22	ap

	-	speed up: slightly improved way to handle the 'special case'

	2000-12-05  ap

	-	speed up: implemented prepacking

	2000-12-11  ap

	-	speed up: avoid data reordering

	2000-12-12  ap

	-	moved fft configuration switches to "l3psy.h"

	2001-01-12  ap

	-	use some explicit type casting to avoid compiler warnings

	2001-04-06  ap

	-	implemented radix-4 FFT
*/

/*****************************************************************************
 * FFT computes fast fourier transform of BLKSIZE samples of data            *
 *   based on the decimation-in-frequency algorithm described in "Digital    *
 *   Signal Processing" by Oppenheim and Schafer, refer to pages 304         *
 *   (flow graph) and 330-332 (Fortran program in problem 5).                *
 *                                                                           *
 *   required constants:                                                     *
 *         PI          3.14159265358979                                      *
 *         BLKSIZE     should be 2^^M for a positive integer M               *
 *                                                                           *
 *****************************************************************************/

#include	"common.h"
#include	"encoder.h"
#include	"l3psy.h"





/* The switches have been moved to "l3psy.h" */





int						fInit_fft;





#if NORMAL_FFT   /* handling sign exceptions */
#define P +
#define M -
#else
#define P -
#define M +
#endif


void fft (FLOAT x_real[], FLOAT x_imag[], FLOAT energy[], FLOAT phi[], int N)
{
 	int						i, j, k, off;
#if USED_VALUES_ONLY
	int						min_i, max_i;
#endif
	int						m, mr;
	double					xr0, xr1;
	double					xi0, xi1;
	double					ur0, ur1, ur2, ur3;
	double					ui0, ui1, ui2, ui3;
	double					tr0, tr1, tr2, tr3;
	double					ti0, ti1, ti2, ti3;
	double					er1, er2, er3;
	double					ei1, ei2, ei3;
	double					t_real, t_imag;
	double					sqrt05;
	static double			w_real[BLKSIZE*7/8], w_imag[BLKSIZE*7/8];
	int						N_ORG;

#if !REORDER_DATA
	static	int				swap_l[BLKSIZE/2+1];
	static	int				swap_s[BLKSIZE_s/2+1];
	int						*pSwap, a, b;
#endif

	double	t1, t2, t3, t4, t5, t6;

	if (fInit_fft == 0)
	{
		for (i = 0;  i < BLKSIZE*7/8;  i++)
		{
			w_real[i] =   cos(2*PI*i/BLKSIZE);
/* M */		w_imag[i] = M sin(2*PI*i/BLKSIZE);
		}

		sqrt05 = sqrt (0.5);

#if !REORDER_DATA
		j = 0;
		for (i = 0;  i < BLKSIZE/2-1;  i++)
		{
			swap_l[i] = j;  k = BLKSIZE/4;  while (k <= j) {j -= k;  k >>= 1;}  j += k;
		}
		swap_l[i] = i;  swap_l[i+1] = i+1;

		j = 0;
		for (i = 0;  i < BLKSIZE_s/2-1;  i++)
		{
			swap_s[i] = j;  k = BLKSIZE_s/4;  while (k <= j) {j -= k;  k >>= 1;}  j += k;
		}
		swap_s[i] = i;  swap_s[i+1] = i+1;
#endif

		fInit_fft++;
	}


#if REAL_SEQUENCE
	N_ORG = N;
	N >>= 1;
#if !PREPACKED
	/* packing the sequence to the half length */
	for (i = 0;  i < N;  i++)
	{
		x_real[i] = x_real[2*i];
		x_imag[i] = x_real[2*i+1];
	}
#endif
#endif



	off = BLKSIZE/N;

	for (m = N;  m >= 4;  m = mr)
	{
		mr = m >> 2;

			for (i = 0;  i < N;  i += m)
			{
				ur0 = x_real[i     ];  ui0 = x_imag[i     ];
				ur1 = x_real[i+mr  ];  ui1 = x_imag[i+mr  ];
				ur2 = x_real[i+mr*2];  ui2 = x_imag[i+mr*2];
				ur3 = x_real[i+mr*3];  ui3 = x_imag[i+mr*3];

				xr0 = ur0 + ur2;  xi0 = ui0 + ui2;
				xr1 = ur1 + ur3;  xi1 = ui1 + ui3;
				tr0 = xr0 + xr1;  ti0 = xi0 + xi1;
				tr2 = xr0 - xr1;  ti2 = xi0 - xi1;

				xr0 = ur0 - ur2;  xi0 = ui0 - ui2;
/* P */			xr1 = P (ui1 - ui3);  xi1 = P (ur3 - ur1);
				tr1 = xr0 + xr1;  ti1 = xi0 + xi1;
				tr3 = xr0 - xr1;  ti3 = xi0 - xi1;

				x_real[i     ] = tr0;  x_imag[i     ] = ti0;
				x_real[i+mr*2] = tr1;  x_imag[i+mr*2] = ti1;
				x_real[i+mr*1] = tr2;  x_imag[i+mr*1] = ti2;
				x_real[i+mr*3] = tr3;  x_imag[i+mr*3] = ti3;
			}

		k = off;
		for (j = 1;  j < mr;  j++)
		{
			er1 = w_real[  k];  ei1 = w_imag[  k];
			er2 = w_real[2*k];  ei2 = w_imag[2*k];
			er3 = w_real[3*k];  ei3 = w_imag[3*k];

			for (i = j;  i < N;  i += m)
			{
				ur0 = x_real[i     ];  ui0 = x_imag[i     ];
				ur1 = x_real[i+mr  ];  ui1 = x_imag[i+mr  ];
				ur2 = x_real[i+mr*2];  ui2 = x_imag[i+mr*2];
				ur3 = x_real[i+mr*3];  ui3 = x_imag[i+mr*3];

				xr0 = ur0 + ur2;  xi0 = ui0 + ui2;
				xr1 = ur1 + ur3;  xi1 = ui1 + ui3;
				tr0 = xr0 + xr1;  ti0 = xi0 + xi1;
				tr2 = xr0 - xr1;  ti2 = xi0 - xi1;

				xr0 = ur0 - ur2;  xi0 = ui0 - ui2;
/* P */			xr1 = P (ui1 - ui3);  xi1 = P (ur3 - ur1);
				tr1 = xr0 + xr1;  ti1 = xi0 + xi1;
				tr3 = xr0 - xr1;  ti3 = xi0 - xi1;

				x_real[i     ] = tr0;                    x_imag[i     ] = ti0;
				x_real[i+mr*2] = tr1 * er1 - ti1 * ei1;  x_imag[i+mr*2] = tr1 * ei1 + ti1 * er1;
				x_real[i+mr*1] = tr2 * er2 - ti2 * ei2;  x_imag[i+mr*1] = tr2 * ei2 + ti2 * er2;
				x_real[i+mr*3] = tr3 * er3 - ti3 * ei3;  x_imag[i+mr*3] = tr3 * ei3 + ti3 * er3;
			}
			k += off;
		}
		off <<= 2;
	}

	if (m == 2)
	{
		for (i = 0;  i < N;  i += 2)
		{
			xr0 = x_real[i+1];  x_real[i+1] = x_real[i] - xr0;  x_real[i] += xr0;
			xi0 = x_imag[i+1];  x_imag[i+1] = x_imag[i] - xi0;  x_imag[i] += xi0;
		}
	}


#if REORDER_DATA
	/* this section reorders the data to the correct ordering */
	j = 0;
	for (i = 0;  i < N-1;  i++)
	{
		if (i < j)
		{
			t_real = x_real[j];
			t_imag = x_imag[j];
			x_real[j] = x_real[i];
			x_imag[j] = x_imag[i];
			x_real[i] = (FLOAT) t_real;
			x_imag[i] = (FLOAT) t_imag;
		}
		k=N/2;
		while (k <= j)
		{
			j -= k;
			k >>= 1;
		}
		j += k;
	}
#else
	/*
		We don«t reorder the data to the correct ordering,
		but access the data by the bit reverse order index array.
	*/
	pSwap = (N_ORG == BLKSIZE) ? swap_l : swap_s;
#endif


#if REAL_SEQUENCE
	/* unpacking the sequence */
	t_real = x_real[0];
	t_imag = x_imag[0];
	x_real[0] = (FLOAT) (t_real+t_imag);
	x_imag[0] = 0.0;
	x_real[N] = (FLOAT) (t_real-t_imag);
	x_imag[N] = 0.0;

	k = off = BLKSIZE/N_ORG;
	for (i = 1;  i < N/2;  i++)
	{
#if REORDER_DATA
#define		a	i
#define		b	(N-i)
#else
		a = pSwap[i];
		b = pSwap[N-i];
#endif
		t1 = x_real[a] + x_real[b];
		t2 = x_real[a] - x_real[b];
		t3 = x_imag[a] + x_imag[b];
		t4 = x_imag[a] - x_imag[b];
		t5 = t2*w_imag[k] + t3*w_real[k];
		t6 = t3*w_imag[k] - t2*w_real[k];

		x_real[a] = (FLOAT) (t1+t5) / 2.0;
		x_imag[a] = (FLOAT) (t6+t4) / 2.0;
		x_real[b] = (FLOAT) (t1-t5) / 2.0;
		x_imag[b] = (FLOAT) (t6-t4) / 2.0;

		k += off;
	}
	/* x_real[N/2] doesn«t change */
	/* x_imag[N/2] changes the sign in case of a normal fft */
#if (NORMAL_FFT)
#if REORDER_DATA
	x_imag[i] = -x_imag[i];
#else
	x_imag[pSwap[i]] *= -1.0;
#endif   /* REORDER_DATA */
#endif   /* NORMAL_FFT */
	N = N_ORG;
#endif   /* REAL_SEQUENCE */


	/* calculating the energy and phase, phi */
#if USED_VALUES_ONLY
	if (N == BLKSIZE)
	{
		min_i = LONG_FFT_MIN_IDX;
		max_i = LONG_FFT_MAX_IDX;
	}
	else
	{
		min_i = SHORT_FFT_MIN_IDX;
		max_i = SHORT_FFT_MAX_IDX;
	}
#endif
#if REAL_SEQUENCE
	for (i = 0;  i <= N/2;  i++)
#else
	for (i = 0;  i < N;  i++)
#endif
	{
#if REORDER_DATA
#define		a	i
#else
		a = pSwap[i];
#endif
		energy[i] = x_real[a]*x_real[a] + x_imag[a]*x_imag[a];
		if(energy[i] <= 0.0005)
		{
			energy[i] = (FLOAT) 0.0005;        /* keep the identity */
			x_real[a] = (FLOAT) sqrt(0.0005);  /* energy[i] * cos(phi[i]) == x_real[i] */
			x_imag[a] = 0.0;                   /* energy[i] * sin(phi[i]) == x_imag[i] */
		}
#if USED_VALUES_ONLY
		if (i >= min_i  &&  i <= max_i)
#endif
		phi[i] = (FLOAT) atan2((double) x_imag[a], (double) x_real[a]);
	}
}


#undef P
#undef M



