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

	2000-12-11  Andre Piotrowski

	-	reformatted
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>
#include	<assert.h>

#include	"common.h"
#include	"l3side.h"
#include	"loop.h"
#include	"huffman.h"
#include	"l3bitstream.h"
#include	"reservoir.h"





/*
	Layer3 bit reservoir:
	Described in C.1.5.4.2.2 of the IS
*/

// static	int				ResvSize = 0;   /* in bits */
// static	int				ResvMax  = 0;   /* in bits */





void					fixStatic_reservoir (encoder_flags_and_data* flags)
{
	flags->ResvSize = 0;
	flags->ResvMax  = 0;
}





/*
	ResvFrameBegin:
	Called at the beginning of a frame. Updates the maximum
	size of the reservoir, and checks to make sure main_data_begin
	was set properly by the formatter
*/
void					ResvFrameBegin
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps,
	III_side_info_t			*l3_side,
	int						mean_bits,
	int						frameLength
)
{
	layer					*info;
	int						fullFrameBits, mode_gr;
	int						expectedResvSize, resvLimit;


	info = fr_ps->header;
	mode_gr = 2;
	resvLimit = 4088; /* main_data_begin has 9 bits in MPEG 1 */


	/*
		main_data_begin was set by the formatter to the
		expected value for the next call -- this should
		agree with our reservoir size
	*/
	expectedResvSize = l3_side->main_data_begin * 8;
/*	assert (expectedResvSize == ResvSize); */

	fullFrameBits = mean_bits * mode_gr;

	/*
		determine maximum size of reservoir:
		flags->ResvMax + frameLength <= 7680;

		limit max size to resvLimit bits because
		main_data_begin cannot indicate a
		larger value
	*/
	flags->ResvMax = MIN(MAX (0, 7680-frameLength), resvLimit);
}





/*
	ResvMaxBits:
	Called at the beginning of each granule to get the max bit
	allowance for the current granule based on reservoir size
	and perceptual entropy.
*/
int						ResvMaxBits
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps,
	III_side_info_t			*l3_side,
	double					*pe,
	int						mean_bits
)
{
	int						more_bits, max_bits, add_bits, over_bits;


	mean_bits /= fr_ps->stereo;

	max_bits = mean_bits;

	if (flags->ResvMax != 0)
	{
		more_bits = (int) (*pe * 3.1 - mean_bits);

		if (more_bits > 100)
		{
			int		frac = (flags->ResvSize * 6) / 10;

			add_bits = MIN(frac, more_bits);
		}
		else
			add_bits = 0;

		over_bits = flags->ResvSize - ((flags->ResvMax * 8) / 10) - add_bits;
 		if (over_bits > 0)
			add_bits += over_bits;

		max_bits += add_bits;
	}

	if (max_bits > 4095)
		max_bits = 4095;

	return max_bits;
}





/*
	ResvAdjust:
	Called after a granule's bit allocation. Readjusts the size of
	the reservoir to reflect the granule's usage.
*/
void					ResvAdjust
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps,
	gr_info					*cod_info,
	III_side_info_t			*l3_side,
	int						mean_bits
)
{
	flags->ResvSize += (mean_bits / fr_ps->stereo) - cod_info->part2_3_length;
}





/*
	ResvFrameEnd:
	Called after all granules in a frame have been allocated. Makes sure
	that the reservoir size is within limits, possibly by adding stuffing
	bits. Note that stuffing bits are added by increasing a granule's
	part2_3_length. The bitstream formatter will detect this and write the
	appropriate stuffing bits to the bitstream.
*/
void					ResvFrameEnd
(
	encoder_flags_and_data* flags,
	frame_params			*fr_ps,
	III_side_info_t			*l3_side,
	int						mean_bits
)
{
	layer					*info;
	gr_info					*cod_info;
	int						mode_gr, gr, ch, stereo, ancillary_pad, stuffingBits;
	int						over_bits;

	info    = fr_ps->header;
	stereo  = fr_ps->stereo;
	mode_gr = 2;

	ancillary_pad = 0;

	/* just in case mean_bits is odd, this is necessary... */
	if ((stereo == 2)  &&  (mean_bits & 1))
		flags->ResvSize ++;

	stuffingBits = ancillary_pad;
	
	if ((over_bits = flags->ResvSize - flags->ResvMax) > 0)
	{
		stuffingBits += over_bits;
		flags->ResvSize     -= over_bits;
	}

	/* we must be byte aligned */
	if ((over_bits = flags->ResvSize % 8) != 0)
	{
		stuffingBits += over_bits;
		flags->ResvSize     -= over_bits;
	}

	if (stuffingBits)
	{
		/*
			plan a: put all into the first granule
			This was preferred by someone designing a
			real-time decoder...
		*/
		cod_info = &l3_side->gr[0].ch[0].tt;

		if (cod_info->part2_3_length + stuffingBits < 4095)
			cod_info->part2_3_length += stuffingBits;
		else
		{
		    /* plan b: distribute throughout the granules */
		    for (gr = 0;  gr < mode_gr;  gr++)
		    {
				for (ch = 0;  ch < stereo;  ch++)
				{
					int			extraBits, bitsThisGr;
					gr_info		*cod_info = &l3_side->gr[gr].ch[ch].tt;

					if (stuffingBits == 0)
						break;
					extraBits = 4095 - cod_info->part2_3_length;
					bitsThisGr = (extraBits < stuffingBits) ? extraBits : stuffingBits;
					cod_info->part2_3_length += bitsThisGr;
					stuffingBits -= bitsThisGr;
				}
			}
		    /*
				If any stuffing bits remain, we elect to spill them
				into ancillary data. The bitstream formatter will do this if
				l3side->resvDrain is set
		    */
		    l3_side->resvDrain = stuffingBits;
		}
	}
}
