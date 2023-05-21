/*
			(c) Copyright 1998-2001 - Tord Jansson
			=======================================

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

	2000-11-06  Andre Piotrowski

	-	speed up: complete new, much faster functions 'create_ana_filter()', 'windowFilterSubband()'
	-	          'WIND_SB_CHANGE_LEVEL' 3 requires a new 'enwindow[]' (see 'tables.c')

	2000-11-22	ap

	-	bug fix: initWindowFilterSubband() -- Dividing the enwindow-entries is allowed to be done once only!

	2000-12-11  ap

	-	speed up: WIND_SB_CHANGE_LEVEL 4

	2001-01-12  ap

	-	bug fix: fixed some typo relevant in case of ORG_BUFFERS == 1
*/

#include "common.h"
#include "encoder.h"





/*  ========================================================================================  */
/*      rebuffer_audio                                                                        */
/*  ========================================================================================  */

#if ORG_BUFFERS

void rebuffer_audio
(
	short					buffer[2][1152],
	short					*insamp,
	unsigned int			samples_read,
	int						stereo
)
{
	unsigned int			j;

	if (stereo == 2)
	{ 
		for (j = 0;  j < samples_read/2;  j++)
		{
			buffer[0][j] = insamp[2*j];
			buffer[1][j] = insamp[2*j+1];
		}
	}
	else
	{		
		for (j = 0;  j < samples_read;  j++)
		{
			buffer[0][j] = insamp[j];
			buffer[1][j] = 0;
		}
	}

	for( ;  j < 1152;  j++)
	{
		buffer[0][j] = 0;
		buffer[1][j] = 0;
	}

	return;
}

#else

#define		FLUSH					1152
#define		DELAY					 768

void rebuffer_audio
(
	const short				*insamp,
	FLOAT					buffer[2][2048],
	int						*buffer_idx,
	unsigned int			samples_read,
	int						stereo
)
{
	int						idx, med, fin;

	/* flush the last two read granules */
	*buffer_idx = (*buffer_idx + FLUSH) & 2047;
	idx = (*buffer_idx + DELAY) & 2047;
	fin = (idx + FLUSH) & 2047;

	if (stereo == 2)
	{
		med = (idx + samples_read/2) & 2047;
		if (idx >= med)
		{
			while (idx < 2048)
			{
				buffer[0][idx] = *insamp++;
				buffer[1][idx] = *insamp++;
				idx++;
			}
			idx = 0;
		}
		while (idx < med)
		{
			buffer[0][idx] = *insamp++;
			buffer[1][idx] = *insamp++;
			idx++;
		}
	}
	else
	{		
		med = (idx + samples_read) & 2047;
		if (idx >= med)
		{
			while (idx < 2048)
			{
				buffer[0][idx] = *insamp++;
				buffer[1][idx] = 0;
				idx++;
			}
			idx = 0;
		}
		while (idx < med)
		{
			buffer[0][idx] = *insamp++;
			buffer[1][idx] = 0;
			idx++;
		}
	}

	if (idx != fin)
	{
		if (idx > fin)
		{
			while (idx < 2048)
			{
				buffer[0][idx] = 0;
				buffer[1][idx] = 0;
				idx++;
			}
			idx = 0;
		}
		while (idx < fin)
		{
			buffer[0][idx] = 0;
			buffer[1][idx] = 0;
			idx++;
		}
	}
}

#endif





#if ORG_BUFFERS
#define WIND_SB_CHANGE_LEVEL	3
#else
#define WIND_SB_CHANGE_LEVEL	4
#endif



#if WIND_SB_CHANGE_LEVEL==4

	typedef double 			filter_matrix[SBLIMIT/4][32];

	static	filter_matrix	m;

	/*  ========================================================================================  */
	/*      create_ana_filter                                                                     */
	/*  ========================================================================================  */
	/*
		Calculates the analysis filterbank coefficients and rounds to the 9th decimal place
		accuracy of the filterbank tables in the ISO document.
		The coefficients are stored in #new_filter#


		Originally was computed

			filter[i][j]  :=  cos (PI64 * ((2*i+1) * (16-j)))

		for i = 0..SBLIMIT-1 and j = 0..63 .


		But for j = 0..15 is 16-j = -(16-(32-j)) which implies

		(1)		filter[i][j]  =  filter[i][32-j] .

		(We know that filter[i][16] = cos(0) = 1 , but that is not so interesting.)

		Furthermore, for j = 33..48 we get

				   filter[i][96-j]
				=  cos (PI64 * (2*i+1) * (j-80))
				=  cos (PI * (2*i+1) + PI64 * (2*i+1) * (16-j))
		(2)		=  cos (PI * (2*i+1)) * cos (PI64 * (2*i+1) * (16-j))   // sin (PI * (2*i+1)) = 0
				=  -cos (PI64 * (2*i+1) * (16-j))
				=  -filter[i][j] ,

		especially is filter[i][48] = 0 .


		On the other hand there is

				   filter[31-i][j]
				=  cos (PI64 * (2*(31-i)+1) * (16-j))
				=  cos (PI64 * (64-(2*i+1)) * (16-j))
				=  cos (PI * (16-j) - PI64 * (2*i+1) * (16-j))
				=  cos (PI * (16-j)) * cos (PI64 * (2*i+1) * (16-j))   // sin (PI * (16-j)) = 0
				=  (-1)^^j * cos (PI64 * (2*i+1) * (16-j))
				=  (-1)^^j * filter[i][j] .


		There is a third somewhat more complication "symmetry":

				   filter[15-i][j]
				=  cos (PI64 * (2*(15-i)+1) * (16-j))
				=  cos (PI64 * (32-(2*i+1)) * (16-j))
				=  cos (PI/2 * (16-j) - PI64 * (2*i+1) * (16-j))
				=  cos (PI/2 * (16-j)) * cos (PI64 * (2*i+1) * (16-j)) + sin (PI/2 * (16-j)) * sin (PI64 * (2*i+1) * (16-j))
				=  cos (PI/2 * (16-j)) * cos (PI64 * (2*i+1) * (16-j)) + sin (PI/2 * (16-j)) * cos (PI64 * (2*i+1) * (-1)^^i * (16-(j+32)))

				=  (-1)^^(  j  /2    ) * filter[i][j   ]   for even j
				=  (-1)^^((j+1)/2 + i) * filter[i][j+32]   for odd  j  with  j <  32
				=  (-1)^^((j-1)/2 + i) * filter[i][j-32]   for odd  j  with  j >= 32


		For these reasons we create a filter of the eighth size only and set

			new_filter[i][j]  :=  filter[i][j]       for i = 0..SBLIMIT/4-1 and j =  0..16
			new_filter[i][j]  :=  filter[i][j+16]    for i = 0..SBLIMIT/4-1 and j = 17..31
	*/

	static	void create_ana_filter (double new_filter[SBLIMIT/4][32])
	{
		register int			i, j;
		double					t;

		for (i = 0;  i < SBLIMIT/4;  i++)
		{
			for (j = 0;  j <= 16;  j++)
			{
				t = 1e9 * cos (PI64 * (double) ((2*i+1) * (16-j)));
				if (t >= 0)
					modf (t + 0.5, &t);
				else
					modf (t - 0.5, &t);
				new_filter[i][j] = t * 1e-9;
			}
			for (j = 17;  j < 32;  j++)
			{
				t = 1e9 * cos (PI64 * (double) ((2*i+1) * j));   /* (16-(j+16)) */
				if (t >= 0)
					modf (t + 0.5, &t);
				else
					modf (t - 0.5, &t);
				new_filter[i][j] = t * 1e-9;
			}
		}
	}

	/*  ========================================================================================  */
	/*      windowFilterSubband                                                                   */
	/*  ========================================================================================  */
	/*
		Calculates the analysis filter bank coefficients

		The windowed samples #z# is filtered by the digital filter matrix #m# to produce the
		subband samples #s#. This done by first selectively picking out values from the windowed
		samples, and then multiplying them by the filter matrix, producing 32 subband samples.
	*/

	void  windowFilterSubband
	(
		const FLOAT				*buffer,
		int						buffer_idx,
		double					s[SBLIMIT]
	)
	{
		int						i, k;

		double					t, u, v, w;

		double					z[32];
		double					*p;
		double					*pEnw;

		pEnw = enwindow;

		for (k = 0;  k <= 16;  k++)
		{
			t =  buffer[(buffer_idx+511) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+447) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+383) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+319) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+255) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+191) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+127) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+ 63) & 2047] * *pEnw++;
			z[k] = t;

			buffer_idx--;
		}			

		for (k = 15;  k >= 0;  k--)
		{
			t =  buffer[(buffer_idx+511) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+447) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+383) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+319) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+255) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+191) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+127) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+ 63) & 2047] * *pEnw++;
			z[k] += t;

			buffer_idx--;
		}			

		for (k = 17;  k < 32;  k++)
		{
			t =  buffer[(buffer_idx+511) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+447) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+383) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+319) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+255) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+191) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+127) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+ 63) & 2047] * *pEnw++;
			z[k] = t;

			buffer_idx--;
		}			

		/* normally y[48] was computed like the other entries, */
		/* but this entry is not needed because m[i][48] = 0.  */
		buffer_idx--; /* But we have to increase the pointers. */
		pEnw += 8;

		for (k = 31;  k > 16;  k--)
		{
			t =  buffer[(buffer_idx+511) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+447) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+383) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+319) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+255) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+191) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+127) & 2047] * *pEnw++;
			t += buffer[(buffer_idx+ 63) & 2047] * *pEnw++;
			z[k] -= t;

			buffer_idx--;
		}			

		for (i = 0;  i < SBLIMIT/4;  i++)
		{
			p = m[i];

			t = u = v = w = 0.0;
			for (k = 0;  k < 16;  k += 4)
			{
				t += p[k   ] * z[k  ];
				v += p[k+ 2] * z[k+2];
				u += p[k+ 1] * z[k+1] + p[k+ 3] * z[k+3];
				w -= p[k+17] * z[k+1] - p[k+19] * z[k+3];
			}
			for ( ;  k < 32;  k += 4)
			{
				t += p[k   ] * z[k  ];
				v += p[k+ 2] * z[k+2];
				u += p[k+ 1] * z[k+1] + p[k+ 3] * z[k+3];
				w += p[k-15] * z[k+1] - p[k-13] * z[k+3];
			}

			s[          i] = (t + v) + u;
			s[SBLIMIT-1-i] = (t + v) - u;
			if (i & 1)
			{
				s[SBLIMIT/2-1-i] = (t - v) - w;
				s[SBLIMIT/2  +i] = (t - v) + w;
			}
			else
			{
				s[SBLIMIT/2-1-i] = (t - v) + w;
				s[SBLIMIT/2  +i] = (t - v) - w;
			}
		}	 
	}

#elif WIND_SB_CHANGE_LEVEL==3

	#define imax			(SBLIMIT/4)
	typedef double 			filter_matrix[imax][32];

	static	int				half[2];
	static	int				off[2];
	static	double			x[2][512];
	static	filter_matrix	m;

	/*  ========================================================================================  */
	/*      create_ana_filter                                                                     */
	/*  ========================================================================================  */
	/*
		Calculates the analysis filterbank coefficients and rounds to the 9th decimal place
		accuracy of the filterbank tables in the ISO document.
		The coefficients are stored in #new_filter#


		Originally was computed

			filter[i][j]  :=  cos (PI64 * ((2*i+1) * (16-j)))

		for i = 0..SBLIMIT-1 and j = 0..63 .


		But for j = 0..15 is 16-j = -(16-(32-j)) which implies

		(1)		filter[i][j]  =  filter[i][32-j] .

		(We know that filter[i][16] = cos(0) = 1 , but that is not so interesting.)

		Furthermore, for j = 33..48 we get

				   filter[i][96-j]
				=  cos (PI64 * (2*i+1) * (j-80))
				=  cos (PI * (2*i+1) + PI64 * (2*i+1) * (16-j))
		(2)		=  cos (PI * (2*i+1)) * cos (PI64 * (2*i+1) * (16-j))   // sin (PI * (2*i+1)) = 0
				=  -cos (PI64 * (2*i+1) * (16-j))
				=  -filter[i][j] ,

		especially is filter[i][48] = 0 .


		On the other hand there is

				   filter[31-i][j]
				=  cos (PI64 * (2*(31-i)+1) * (16-j))
				=  cos (PI64 * (64-(2*i+1)) * (16-j))
				=  cos (PI * (16-j) - PI64 * (2*i+1) * (16-j))
				=  cos (PI * (16-j)) * cos (PI64 * (2*i+1) * (16-j))   // sin (PI * (16-j)) = 0
				=  (-1)^^j * cos (PI64 * (2*i+1) * (16-j))
				=  (-1)^^j * filter[i][j] .


		There is a third somewhat more complication "symmetry":

				   filter[15-i][j]
				=  cos (PI64 * (2*(15-i)+1) * (16-j))
				=  cos (PI64 * (32-(2*i+1)) * (16-j))
				=  cos (PI/2 * (16-j) - PI64 * (2*i+1) * (16-j))
				=  cos (PI/2 * (16-j)) * cos (PI64 * (2*i+1) * (16-j)) + sin (PI/2 * (16-j)) * sin (PI64 * (2*i+1) * (16-j))
				=  cos (PI/2 * (16-j)) * cos (PI64 * (2*i+1) * (16-j)) + sin (PI/2 * (16-j)) * cos (PI64 * (2*i+1) * (-1)^^i * (16-(j+32)))

				=  (-1)^^(  j  /2    ) * filter[i][j   ]   for even j
				=  (-1)^^((j+1)/2 + i) * filter[i][j+32]   for odd  j  with  j <  32
				=  (-1)^^((j-1)/2 + i) * filter[i][j-32]   for odd  j  with  j >= 32


		For these reasons we create a filter of the eighth size only and set

			new_filter[i][j]  :=  filter[i][j]       for i = 0..SBLIMIT/4-1 and j =  0..16
			new_filter[i][j]  :=  filter[i][j+16]    for i = 0..SBLIMIT/4-1 and j = 17..31
	*/

	static	void create_ana_filter (double new_filter[imax][32])
	{
		register int			i, j;
		double					t;

		for (i = 0;  i < imax;  i++)
		{
			for (j = 0;  j <= 16;  j++)
			{
				t = 1e9 * cos (PI64 * (double) ((2*i+1) * (16-j)));
				if (t >= 0)
					modf (t + 0.5, &t);
				else
					modf (t - 0.5, &t);
				new_filter[i][j] = t * 1e-9;
			}
			for (j = 17;  j < 32;  j++)
			{
				t = 1e9 * cos (PI64 * (double) ((2*i+1) * j));   /* (16-(j+16)) */
				if (t >= 0)
					modf (t + 0.5, &t);
				else
					modf (t - 0.5, &t);
				new_filter[i][j] = t * 1e-9;
			}
		}
	}

	/*  ========================================================================================  */
	/*      windowFilterSubband                                                                   */
	/*  ========================================================================================  */
	/*
		Calculates the analysis filter bank coefficients

		The windowed samples #z# is filtered by the digital filter matrix #m# to produce the
		subband samples #s#. This done by first selectively picking out values from the windowed
		samples, and then multiplying them by the filter matrix, producing 32 subband samples.
	*/

	void  windowFilterSubband
	(
		short					*pBuffer,
		int						ch,
		double					s[SBLIMIT]
	)
	{
		int						i, k;
		int						a;

		double					t, u, v, w;

		double					z[32];
		double					*pm;
		double					*px;
		double					*pEnw;


		px = x[ch] + half[ch];
		a = off[ch];

		/* replace 32 oldest samples with 32 new samples */

		for (i = 0;  i < 32;  i++)
			px[a + (31-i)*8] = (double) pBuffer[i]/*/SCALE*/;


		pEnw = enwindow;

		for (k = 0;  k <= 16;  k++)
		{
			t =  px[(a+0) & 7] * *pEnw++;
			t += px[(a+1) & 7] * *pEnw++;
			t += px[(a+2) & 7] * *pEnw++;
			t += px[(a+3) & 7] * *pEnw++;
			t += px[(a+4) & 7] * *pEnw++;
			t += px[(a+5) & 7] * *pEnw++;
			t += px[(a+6) & 7] * *pEnw++;
			t += px[(a+7) & 7] * *pEnw++;
			z[k] = t;

			px += 8;
		}			

		for (k = 15;  k > 0;  k--)
		{
			t =  px[(a+0) & 7] * *pEnw++;
			t += px[(a+1) & 7] * *pEnw++;
			t += px[(a+2) & 7] * *pEnw++;
			t += px[(a+3) & 7] * *pEnw++;
			t += px[(a+4) & 7] * *pEnw++;
			t += px[(a+5) & 7] * *pEnw++;
			t += px[(a+6) & 7] * *pEnw++;
			t += px[(a+7) & 7] * *pEnw++;
			z[k] += t;

			px += 8;
		}			

		half[ch] ^= 256;   /* toggling 0 or 256 */
		if (half[ch])
		{
			off[ch] = (a + 7) & 7;
		}
		else
		{
			px = x[ch];
			a = (a + 1) & 7;
		}

		/* k = 0; */
		{
			t =  px[(a+0) & 7] * *pEnw++;
			t += px[(a+1) & 7] * *pEnw++;
			t += px[(a+2) & 7] * *pEnw++;
			t += px[(a+3) & 7] * *pEnw++;
			t += px[(a+4) & 7] * *pEnw++;
			t += px[(a+5) & 7] * *pEnw++;
			t += px[(a+6) & 7] * *pEnw++;
			t += px[(a+7) & 7] * *pEnw++;
			z[k] += t;

			px += 8;
		}			

		for (k = 17;  k < 32;  k++)
		{
			t =  px[(a+0) & 7] * *pEnw++;
			t += px[(a+1) & 7] * *pEnw++;
			t += px[(a+2) & 7] * *pEnw++;
			t += px[(a+3) & 7] * *pEnw++;
			t += px[(a+4) & 7] * *pEnw++;
			t += px[(a+5) & 7] * *pEnw++;
			t += px[(a+6) & 7] * *pEnw++;
			t += px[(a+7) & 7] * *pEnw++;
			z[k] = t;

			px += 8;
		}			

		/* normally y[48] was computed like the other entries, */
		/* but this entry is not needed because m[i][48] = 0.  */
		px += 8;      /* But we have to increase the pointers. */
		pEnw += 8;

		for (k = 31;  k > 16;  k--)
		{
			t =  px[(a+0) & 7] * *pEnw++;
			t += px[(a+1) & 7] * *pEnw++;
			t += px[(a+2) & 7] * *pEnw++;
			t += px[(a+3) & 7] * *pEnw++;
			t += px[(a+4) & 7] * *pEnw++;
			t += px[(a+5) & 7] * *pEnw++;
			t += px[(a+6) & 7] * *pEnw++;
			t += px[(a+7) & 7] * *pEnw++;
			z[k] -= t;

			px += 8;
		}			

		for (i = 0;  i < imax;  i++)
		{
			pm = m[i];

			t = u = v = w = 0.0;
			for (k = 0;  k < 16;  k += 4)
			{
				t += pm[k   ] * z[k  ];
				v += pm[k+ 2] * z[k+2];
				u += pm[k+ 1] * z[k+1] + pm[k+ 3] * z[k+3];
				w -= pm[k+17] * z[k+1] - pm[k+19] * z[k+3];
			}
			for (   ;  k < 32;  k += 4)
			{
				t += pm[k   ] * z[k  ];
				v += pm[k+ 2] * z[k+2];
				u += pm[k+ 1] * z[k+1] + pm[k+ 3] * z[k+3];
				w += pm[k-15] * z[k+1] - pm[k-13] * z[k+3];
			}

			s[          i] = (t + v) + u;
			s[SBLIMIT-1-i] = (t + v) - u;
			if (i & 1)
			{
				s[SBLIMIT/2-1-i] = (t - v) - w;
				s[SBLIMIT/2  +i] = (t - v) + w;
			}
			else
			{
				s[SBLIMIT/2-1-i] = (t - v) + w;
				s[SBLIMIT/2  +i] = (t - v) - w;
			}
		}	 
	}

#elif WIND_SB_CHANGE_LEVEL==2

	typedef double 			filter_matrix[SBLIMIT/2][32];

	static	int				off[2];
	static	int				half[2];
	static	double			x[2][512];
	static	filter_matrix	m;

	/*  ========================================================================================  */
	/*      create_ana_filter                                                                     */
	/*  ========================================================================================  */
	/*
		Calculates the analysis filterbank coefficients and rounds to the 9th decimal place
		accuracy of the filterbank tables in the ISO document.
		The coefficients are stored in #new_filter#


		Originally was computed

			filter[i][j]  :=  cos (PI64 * ((2*i+1) * (16-j)))

		for i = 0..SBLIMIT-1 and j = 0..63 .


		But for j = 0..15 is 16-j = -(16-(32-j)) which implies

		(1)		filter[i][j]  =  filter[i][32-j] .

		(We know that filter[i][16] = cos(0) = 1 , but that is not so interesting.)

		Furthermore, for j = 33..48 we get

				   filter[i][96-j]
				=  cos (PI64 * (2*i+1) * (j-80))
				=  cos (PI * (2*i+1) + PI64 * (2*i+1) * (16-j))
		(2)		=  cos (PI * (2*i+1)) * cos (PI64 * (2*i+1) * (16-j))   // sin (PI * (2*i+1)) = 0
				=  -cos (PI64 * (2*i+1) * (16-j))
				=  -filter[i][j] ,

		especially is filter[i][48] = 0 .


		On the other hand there is

				   filter[31-i][j]
				=  cos (PI64 * (2*(31-i)+1) * (16-j))
				=  cos (PI64 * (64-(2*i+1)) * (16-j))
				=  cos (PI * (16-j) - PI64 * (2*i+1) * (16-j))
				=  cos (PI * (16-j)) * cos (PI64 * (2*i+1) * (16-j))   // sin (PI * (16-j)) = 0
				=  (-1)^^j * cos (PI64 * (2*i+1) * (16-j))
				=  (-1)^^j * filter[i][j] .


		For these reasons we create a filter of the quarter size only and set

			new_filter[i][j]  :=  filter[i][j]       for i = 0..SBLIMIT/2-1 and j =  0..16
			new_filter[i][j]  :=  filter[i][j+16]    for i = 0..SBLIMIT/2-1 and j = 17..31
	*/

	static	void create_ana_filter (double new_filter[SBLIMIT/2][32])
	{
		register int			i, j;
		double					t;

		for (i = 0;  i < SBLIMIT/2;  i++)
		{
			for (j = 0;  j < =16;  j++)
			{
				t = 1e9 * cos (PI64 * (double) ((2*i+1) * (16-j)));
				if (t >= 0)
					modf (t + 0.5, &t);
				else
					modf (t - 0.5, &t);
				new_filter[i][j] = t * 1e-9;
			}
			for (j = 17;  j < 32;  j++)
			{
				t = 1e9 * cos (PI64 * (double) ((2*i+1) * j));   /* (16-(j+16)) */
				if (t >= 0)
					modf (t + 0.5, &t);
				else
					modf (t - 0.5, &t);
				new_filter[i][j] = t * 1e-9;
			}
		}
	}

	/*  ========================================================================================  */
	/*      windowFilterSubband                                                                   */
	/*  ========================================================================================  */
	/*
		Calculates the analysis filter bank coefficients

		The windowed samples #z# is filtered by the digital filter matrix #m# to produce the
		subband samples #s#. This done by first selectively picking out values from the windowed
		samples, and then multiplying them by the filter matrix, producing 32 subband samples.
	*/

	void  windowFilterSubband
	(
		short					*pBuffer,
		int						ch,
		double					s[SBLIMIT]
	)
	{
		int						i, k;
		int						a;

		double					t, u;

		double					z[32];   /* y[64]; */
		double					*pm;
		double					*px;
		double					*pEnw;


		px = x[ch] + half[ch];
		a = off[ch];

		/* replace 32 oldest samples with 32 new samples */

		for (i = 0;  i < 32;  i++)
			px[a + (31-i)*8] = (double) pBuffer[i] /*/SCALE*/;


		/*
			for k = 0..15 we compute

				z[k]  :=  y[k] + y[32-k]

			and we set

				z[16|  :=  y[16] .
		*/

		pEnw = enwindow;

		for (k = 0;  k <= 16;  k++)   /* for (j = 0;  j < =16;  j++) */
		{
			t =  px[(a+0) & 7] * pEnw[64*0];
			t += px[(a+1) & 7] * pEnw[64*1];
			t += px[(a+2) & 7] * pEnw[64*2];
			t += px[(a+3) & 7] * pEnw[64*3];
			t += px[(a+4) & 7] * pEnw[64*4];
			t += px[(a+5) & 7] * pEnw[64*5];
			t += px[(a+6) & 7] * pEnw[64*6];
			t += px[(a+7) & 7] * pEnw[64*7];
			z[k] = t;   /* y[j] = t; */

			px += 8;
			pEnw++;
		}			

		for (k = 15;  k > 0;  k--)   /* for (j = 17;  j < 32;  j++) */
		{
			t =  px[(a+0) & 7] * pEnw[64*0];
			t += px[(a+1) & 7] * pEnw[64*1];
			t += px[(a+2) & 7] * pEnw[64*2];
			t += px[(a+3) & 7] * pEnw[64*3];
			t += px[(a+4) & 7] * pEnw[64*4];
			t += px[(a+5) & 7] * pEnw[64*5];
			t += px[(a+6) & 7] * pEnw[64*6];
			t += px[(a+7) & 7] * pEnw[64*7];
			z[k] += t;   /* y[j] = t; */

			px += 8;
			pEnw++;
		}			

		half[ch] ^= 256;   /* toggling 0 or 256 */
		if (half[ch])
		{
			off[ch] = (a + 7) & 7;   /*offset is modulo (HAN_SIZE-1)*/
		}
		else
		{
			px = x[ch];
			a = (a + 1) & 7;
		}

		/* k = 0; not needed, actual value of k is 0 */   /* j = 32; */
		{
			t =  px[(a+0) & 7] * pEnw[64*0];
			t += px[(a+1) & 7] * pEnw[64*1];
			t += px[(a+2) & 7] * pEnw[64*2];
			t += px[(a+3) & 7] * pEnw[64*3];
			t += px[(a+4) & 7] * pEnw[64*4];
			t += px[(a+5) & 7] * pEnw[64*5];
			t += px[(a+6) & 7] * pEnw[64*6];
			t += px[(a+7) & 7] * pEnw[64*7];
			z[k] += t;   /* y[j] = t; */

			px += 8;
			pEnw++;
		}			

		/*
			for k = 17..31 we compute

				z[k]  :=  y[k+16] - y[80-k]
		*/

		for (k = 17;  k < 32;  k++)   /* for (j = 33;  j < 47;  j++) */
		{
			t =  px[(a+0) & 7] * pEnw[64*0];
			t += px[(a+1) & 7] * pEnw[64*1];
			t += px[(a+2) & 7] * pEnw[64*2];
			t += px[(a+3) & 7] * pEnw[64*3];
			t += px[(a+4) & 7] * pEnw[64*4];
			t += px[(a+5) & 7] * pEnw[64*5];
			t += px[(a+6) & 7] * pEnw[64*6];
			t += px[(a+7) & 7] * pEnw[64*7];
			z[k] = t;   /* y[j] = t; */

			px += 8;
			pEnw++;
		}			

		/* normally y[48] was computed like the other entries, */
		/* but this entry is not needed because m[i][48] = 0.  */
		px += 8;      /* But we have to increase the pointers. */
		pEnw++;

		for (k = 31;  k > 16;  k--)   /* for (j = 49;  j < 64;  j++) */
		{
			t =  px[(a+0) & 7] * pEnw[64*0];
			t += px[(a+1) & 7] * pEnw[64*1];
			t += px[(a+2) & 7] * pEnw[64*2];
			t += px[(a+3) & 7] * pEnw[64*3];
			t += px[(a+4) & 7] * pEnw[64*4];
			t += px[(a+5) & 7] * pEnw[64*5];
			t += px[(a+6) & 7] * pEnw[64*6];
			t += px[(a+7) & 7] * pEnw[64*7];
			z[k] -= t;   /* y[j] = t; */

			px += 8;
			pEnw++;
		}			

		pm = m[0];                           /* pm = m[0];                      */
		for (i = 0;  i < SBLIMIT/2;  i++)    /* for (i = 0;  i < SBLIMIT;  i++) */
		{

			t = u = 0.0;                     /*    t = 0.0;                     */
			for (k = 0;  k < 32; )           /*    for (j = 0;  j < 64;  j++)   */
			{
				t += *pm++ * z[k++];         /*       t += *pm++ * y[j];        */
				u += *pm++ * z[k++];
			}

			s[          i] = t + u;          /*    s[i] = t;                    */
			s[SBLIMIT-1-i] = t - u;
		}	 
	}

#elif WIND_SB_CHANGE_LEVEL==1

	typedef double 			filter_matrix[SBLIMIT][64];

	static	int				off[2];
	static	int				half[2];
	static	double			x[2][512];
	static	filter_matrix	m;

	/*  ========================================================================================  */
	/*      create_ana_filter                                                                     */
	/*  ========================================================================================  */
	/*
		Calculates the analysis filterbank coefficients and rounds to the 9th decimal place
		accuracy of the filterbank tables in the ISO document.
		The coefficients are stored in #filter#
	*/

	static	void create_ana_filter (filter_matrix filter)
	{
		register int			i, j;
		double					t;

		for (i = 0;  i < SBLIMIT;  i++)
		{
			for (j = 0;  j < 64;  j++)
			{
				t = 1e9 * cos (PI64 * (double) ((2*i+1) * (16-j)));
				if (t >= 0)
					modf (t + 0.5, &t);
				else
					modf (t - 0.5, &t);
				filter[i][j] = t * 1e-9;
			}
		}
	}

	/*  ========================================================================================  */
	/*      windowFilterSubband                                                                   */
	/*  ========================================================================================  */
	/*
		Calculates the analysis filter bank coefficients

		The windowed samples #y# is filtered by the digital filter matrix #m# to produce the
		subband samples #s#. This done by first selectively picking out values from the windowed
		samples, and then multiplying them by the filter matrix, producing 32 subband samples.
	*/

	void  windowFilterSubband
	(
		short					*pBuffer,
		int						ch,
		double					s[SBLIMIT]
	)
	{
		int						i, j;
		int						a;

		double					t;

		double					y[64];
		double					*pm;
		double					*px;
		double					*pEnw;


		px = x[ch] + half[ch];
		a = off[ch];

		/* replace 32 oldest samples with 32 new samples */

		for (i = 0;  i < 32;  i++)
			px[a + (31-i)*8] = (double) pBuffer[i] /*/SCALE*/;


		pEnw = enwindow;

		for (j = 0;  j < 32;  j++)
		{
			t =  px[(a+0) & 7] * pEnw[64*0];
			t += px[(a+1) & 7] * pEnw[64*1];
			t += px[(a+2) & 7] * pEnw[64*2];
			t += px[(a+3) & 7] * pEnw[64*3];
			t += px[(a+4) & 7] * pEnw[64*4];
			t += px[(a+5) & 7] * pEnw[64*5];
			t += px[(a+6) & 7] * pEnw[64*6];
			t += px[(a+7) & 7] * pEnw[64*7];
			y[j] = t;

			px += 8;
			pEnw++;
		}			

		half[ch] ^= 256;   /* toggling 0 or 256 */
		if (half[ch])
		{
			off[ch] = (a + 7) & 7;   /*offset is modulo (HAN_SIZE-1)*/
		}
		else
		{
			px = x[ch];
			a = (a + 1) & 7;
		}

		for (j = 32;  j < 64;  j++)
		{
			t =  px[(a+0) & 7] * pEnw[64*0];
			t += px[(a+1) & 7] * pEnw[64*1];
			t += px[(a+2) & 7] * pEnw[64*2];
			t += px[(a+3) & 7] * pEnw[64*3];
			t += px[(a+4) & 7] * pEnw[64*4];
			t += px[(a+5) & 7] * pEnw[64*5];
			t += px[(a+6) & 7] * pEnw[64*6];
			t += px[(a+7) & 7] * pEnw[64*7];
			y[j] = t;

			px += 8;
			pEnw++;
		}			

		pm = m[0];
		for (i = 0;  i < SBLIMIT;  i++)
		{
			t = 0.0;
			for (j = 0;  j < 64;  j++)
				t += *pm++ * y[j];

			s[i] = t;
		}	 
	}

#endif





/*  ========================================================================================  */
/*      initWindowFilterSubband                                                               */
/*  ========================================================================================  */

void initWindowFilterSubband (void)
{
	static	int				initialized = 0;

	int						i;

	if (!initialized)
	{
		create_ana_filter (m);

		for (i = 0;  i < 512;  i++)
			enwindow[i] /= SCALE;

		initialized = 1;
	}

#if ORG_BUFFERS
	half[0] = half[1] = 0;
	off[0] = off[1] = 0;
	memset (x, 0, sizeof(x));
#endif
}





