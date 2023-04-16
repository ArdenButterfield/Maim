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

	2000-11-22  Andre Piotrowski

	-	bug fix: mem_alloc() - no lomger a need to allocate block*2 bytes
*/

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/

#include	<string.h> /* 1995-07-11 shn */
#include	<ctype.h>
#include	<stdlib.h>

#include    "common.h"





/***********************************************************************
*
*  Global Variable Definitions
*
***********************************************************************/

/* 1: MPEG-1, 0: MPEG-2 LSF, 1995-07-11 shn */
double					s_freq[2][4] =
{
	{22.05, 24, 16, 0},
	{44.1 , 48, 32, 0}
};

/* 1: MPEG-1, 0: MPEG-2 LSF, 1995-07-11 shn */
int						bitratex[2][15] =
{
	{   0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160},
	{   0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320}
};





/*******************************************************************************
*
*  Allocate number of bytes of memory equal to "block".
*
*******************************************************************************/

void					*mem_alloc (unsigned int block, char *item)
{
	void					*ptr;

	ptr = (void *) malloc (block);

	memset (ptr, 0, block);

	return ptr;
}





/****************************************************************************
*
*  Free memory pointed to by "*ptr_addr".
*
*****************************************************************************/

void					mem_free (void **ptr_addr)
{
	if (*ptr_addr != NULL)
	{
		free (*ptr_addr);
		*ptr_addr = NULL;
	}
}



