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

	2000-12-04  Andre Piotrowski

	-	reformatted, slimmed
*/



/**********************************************************************
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Encoder - Lower Sampling Frequency Extension
 *
 * $Id: l3side.h,v 1.1 1996/02/14 04:04:23 rowlands Exp $
 *
 * $Log: l3side.h,v $
 * Revision 1.1  1996/02/14 04:04:23  rowlands
 * Initial revision
 *
 * Received from Mike Coleman
 **********************************************************************/
/**********************************************************************
 *   date   programmers                comment                        *
 * 25. 6.92  Toshiyuki Ishino          Ver 1.0                        *
 * 29.10.92  Masahiro Iwadare          Ver 2.0                        *
 * 17. 4.93  Masahiro Iwadare          Updated for IS Modification    *
 *                                                                    *
 *********************************************************************/

#ifndef		__L3SIDE__
#define		__L3SIDE__





/* #define CBLIMIT_SHORT 12 */





/*	void					l3deco (void); */





/* Layer III side information. */

/*	typedef		double					D576[576]; */
/*	typedef		int						I576[576]; */
/*	typedef		double					D192_3[192][3]; */
/*	typedef		int						I192_3[192][3]; */
/*	typedef		char					C192_3[192][3]; */



/*	typedef		struct
				{
					unsigned				cbl_max;
					unsigned				cbs_start;
				}						III_cb_info; */

typedef		struct
			{
				double					l[2][2][21];
				double					s[2][2][12][3];
			}						III_psy_ratio;

/*	typedef		struct
				{
					double					l[2][2][21];
					double					s[2][2][12][3];
				}						III_psy_xmin;*/

/*	typedef		struct
				{
					double					xr[576];
					double					xr_s[3][192];
					double					xmin[21];
					double					xmin_s[3][192];
					int						ix[576];
					int						ix_s[3][192];
				}						III_input3;*/ /* ch */

typedef		struct
			{
				unsigned				part2_3_length;
				unsigned				big_values;
				unsigned				count1;
				unsigned				global_gain;
				unsigned				scalefac_compress;
				unsigned				window_switching_flag;
				unsigned				block_type;
				unsigned				mixed_block_flag;
				unsigned				table_select[3];
				int /*	unsigned */		subblock_gain[3];
				unsigned				region0_count;
				unsigned				region1_count;
				unsigned				preflag;
				unsigned				scalefac_scale;
				unsigned				count1table_select;
				unsigned				part2_length;
				unsigned				sfb_lmax;
				unsigned				sfb_smax;
				unsigned				address1;
				unsigned				address2;
				unsigned				address3;
				double					quantizerStepSize;
				/* added for LSF */
				unsigned				*sfb_partition_table;
				unsigned				slen[4];
			}						gr_info;

typedef		struct
			{
				int						main_data_begin; /* unsigned -> int */
				unsigned				private_bits;
				int						resvDrain;
				unsigned				scfsi[2][4];
				struct
				{
					struct gr_info_s
					{
						gr_info					tt;
					}						ch[2];
				}						gr[2];
			} III_side_info_t;





/* Layer III scale factors. */

typedef		struct
			{
				int						l[2][2][22];      /*         [cb] */
				int						s[2][2][13][3];   /* [window][cb] */
			}						III_scalefac_t;   /* [gr][ch] */





#endif		/* __L3SIDE__ */
