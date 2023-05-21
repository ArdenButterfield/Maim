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

	2000-02-15  Pirandot

	-	reformatted, used 'static', less parameters

	2000-12-03  ap
	-	adapted according to changes to "loop.h"
*/

#ifndef		__LOOP_PVT__
#define		__LOOP_PVT__





static	int				outer_loop
(
	encoder_flags_and_data* flags,
	int						max_bits,
	frame_params			*fr_ps
);

static	int				needed_bits_for_storing_scalefactors
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps
);

static	void			calc_noise (encoder_flags_and_data* flags);

static	int				loop_break (encoder_flags_and_data* flags);

static	int				amplify
(
	encoder_flags_and_data* flags,
	int						iteration
);

static	int INLINE		cutting_crew (encoder_flags_and_data* flags, FLOAT in);

static	void			quantize (encoder_flags_and_data* flags);

static	void			partial_quantize (encoder_flags_and_data* flags);

static	int				bin_search_StepSize
(
	encoder_flags_and_data* flags,
	int						desired_rate,
	double					start
);



/*  ------------    bit counting    --------------------------------------------------------  */



static	int				count_bits (encoder_flags_and_data* flags);

static	int				count_bits_short (encoder_flags_and_data* flags);

static	int				count_bits_long (encoder_flags_and_data* flags);

static	void			choose_table_long
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				max,
	unsigned				*table,
	unsigned				*bit_sum
);

static	void			choose_table_short
(
	encoder_flags_and_data* flags,
	unsigned				start_sfb,
	unsigned				end_sfb,
	unsigned				max,
	unsigned				*table,
	unsigned				*bit_sum
);



/*  ------------    Huffmania    -----------------------------------------------------------  */



static	void			single_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
/*	unsigned				table0, == 1 */
	unsigned				*choice,
	unsigned				*sum
);

static	void			double_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 2, 5, 13 */
	unsigned				table1,   /* 3, 6, 15 */
	unsigned				*choice,
	unsigned				*sum
);

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
);

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
);

static	void			double_Huffman_2   /* Escape tables */
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 16... */
	unsigned				table1,   /* 24... */
	unsigned				*choice,
	unsigned				*sum
);



/*  ------------    the struggle continues    ----------------------------------------------  */



static	void			tiny_single_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
#if ORG_HUFFMAN_CODING
	unsigned				table0,
#else
/*	unsigned				table0, == 1 */
#endif
	unsigned				*choice,
	unsigned				*sum
);

static	void			tiny_double_Huffman
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 2, 5, 13 */
	unsigned				table1,   /* 3, 6, 15 */
	unsigned				*choice,
	unsigned				*sum
);

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
);

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
);

static	void			tiny_double_Huffman_2   /* Escape tables */
(
	encoder_flags_and_data* flags,
	unsigned				start,
	unsigned				end,
	unsigned				table0,   /* 16... */
	unsigned				table1,   /* 24... */
	unsigned				*choice,
	unsigned				*sum
);





#endif		/* __LOOP_PVT__ */
