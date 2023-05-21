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

#ifndef		__HUFFMAN__
#define		__HUFFMAN__


struct huffcodetab {
	unsigned int			xlen;           /* max. x-index                           */
	unsigned int			ylen;           /* max. y-index                           */
	unsigned int			linbits;        /* number of linbits                      */
	unsigned int			linmax;         /* max number to be stored in linbits     */
	int						ref;            /* a positive value indicates a reference */
	unsigned int			*table;         /* pointer to array[xlen][ylen]           */
	unsigned char			*hlen;          /* pointer to array[xlen][ylen]           */
};


extern struct huffcodetab	blade_ht[34];
/* global memory block               */
/* array of all huffcodtable headers */
/* 0..31 Huffman code table 0..31    */
/* 32,33 count1-tables               */


#endif		/* __HUFFMAN__ */
