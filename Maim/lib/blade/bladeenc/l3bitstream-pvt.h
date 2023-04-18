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

	-	redesigned, reformatted
*/

#ifndef		__L3BITSTREAM_PVT__
#define		__L3BITSTREAM_PVT__





void			encodeMainData
(
		encoder_flags_and_data* flags,

	int						l3_enc[2][2][576],
	III_side_info_t			*side_info,
	III_scalefac_t			*scalefac
);

int				encodeSideInfo (encoder_flags_and_data* flags, III_side_info_t *side_info);

void			write_ancillary_data
(	
	encoder_flags_and_data* flags,
	char					*theData,
	int						lengthInBits
);

void			drain_into_ancillary_data (	encoder_flags_and_data* flags, int lengthInBits);

void			Huffmancodebits
(
	encoder_flags_and_data* flags,
	BitHolder				*ph,
	int						*ix,
	gr_info					*cod_info
);

int				writeHuffmanCode
(
	encoder_flags_and_data* flags,
	BitHolder				*ph,
	int						table,
	int						x,
	int						y
);





#endif		/* __L3BITSTREAM_PVT__ */
