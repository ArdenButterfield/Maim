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

	2000-12-06  Andre Piotrowski

	-	reformatted
*/
#ifndef		__TABLES__
#define		__TABLES__





#define		NEW_L3PARM_TABLES		1 // DON"T CHANGE THIS -- awb
#define		CALC_NORM				0





#define		CBANDS					63
#define		CBANDS_s				42





#if NEW_L3PARM_TABLES


	typedef		struct
				{
					int						cbmax_l;   /* number of valid entries */
					int						numlines_l[CBANDS];
					double					minval    [CBANDS];
					double					qthr_l    [CBANDS];
#if !CALC_NORM   /* do we compute more exact values? */
					double					norm_l    [CBANDS];
#endif
					double					bval_l    [CBANDS];
				}						l3_long_data;


	typedef		struct
				{
					int						cbmax_s;   /* number of valid entries */
					int						numlines_s[CBANDS_s];
					double					qthr_s    [CBANDS_s];
#if !CALC_NORM   /* do we compute more exact values? */
					double					norm_s    [CBANDS_s];
#endif
					double					SNR_s     [CBANDS_s];
					double					bval_s    [CBANDS_s];
				}						l3_short_data;


	typedef		struct
				{
					int						cbw_l     [CBANDS];
					int						bu_l      [CBANDS];
					int						bo_l      [CBANDS];
					double					w1_l      [CBANDS];
					double					w2_l      [CBANDS];
				}						l3_long_thres;


	typedef		struct
				{
					int						cbw_s     [CBANDS_s];
					int						bu_s      [CBANDS_s];
					int						bo_s      [CBANDS_s];
					double					w1_s      [CBANDS_s];
					double					w2_s      [CBANDS_s];
				}						l3_short_thres;


	typedef		struct
				{
					l3_long_data			long_data;
					l3_short_data			short_data;
					l3_long_thres			long_thres;
					l3_short_thres			short_thres;
				}						l3_parm_block;


	extern		l3_parm_block			l3_parm[3];
    extern      int                     gibbon[3][22];


#else   /* NEW_L3PARM_TABLES */


	typedef		struct
				{
				/*	int						no; */
					int						lines;
					double					minVal;
					double					qthr;
					double					norm;
					double					bVal;
				}						psyDataElem;


	typedef		struct
				{
				/*	int						no; */
					int						lines;
					double					qthr;
					double					norm;
					double					snr;
					double					bVal;
				}						psyDataElem2;

	typedef		struct
				{
					int						cbw;
					int						bu;
					int						bo;
					float					w1;
					float					w2;
				}						psyDataElem3;


	extern		psyDataElem				psy_longBlock_48000_61[62];
	extern		psyDataElem				psy_longBlock_44100_62[63];
	extern		psyDataElem				psy_longBlock_32000_58[59];

	extern		psyDataElem2			psy_shortBlock_48000_37[38];
	extern		psyDataElem2			psy_shortBlock_44100_38[39];
	extern		psyDataElem2			psy_shortBlock_32000_41[42];

	extern		psyDataElem3			psy_data3_48000_20[21];
	extern		psyDataElem3			psy_data3_44100_20[21];
	extern		psyDataElem3			psy_data3_32000_20[21];

	extern		psyDataElem3			psy_data4_48000_11[12];
	extern		psyDataElem3			psy_data4_44100_11[12];
	extern		psyDataElem3			psy_data4_32000_11[12];


#endif   /* NEW_L3PARM_TABLES */


extern		double					enwindow[512];
extern		char					aHuffcode[1498][36];





#endif		/* __TABLES__ */
