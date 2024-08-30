/*
			(c) Copyright 1998-2001 - Tord Jansson
			======================================

		This file is part of the BladeEnc MP3 Encoder, based on
		ISO's reference code for MPEG Layer 3 compression.

		This file doesn't contain any of the ISO reference code and
		is copyright Tord Jansson (tord.jansson@swipnet.se).

	BladeEnc is free software; you can redistribute this file
	and/or modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.



	------------    Changes    ------------

	2000-12-05  Andre Piotrowski

	-	reformatted, slightly optimized

	2001-01-12  ap

	-	use some explicit type casting to avoid compiler warnings
*/

#include	<stdio.h>
#include	<stdlib.h>

#if defined(_MSC)
#include	<malloc.h>
#endif

#include	"common.h"
#include	"formatbitstream2.h"


/*____ Structure Definitions _________________________________________________*/

// header struct moved to common.h




/*____ Function Prototypes ___________________________________________________*/

int				writeMainDataBits (encoder_flags_and_data* flags, BF_FrameData *psFrame, BitHolder *psBH);
void			putbits (	encoder_flags_and_data* flags, unsigned int val, int n);
int				generateHeader (encoder_flags_and_data* flags, BF_FrameData *psFrame);




#if 0
/*____ Static Data ___________________________________________________________*/

static	int				BitsRemaining     = 0;
static	Header			*pHeaderChain     = NULL;
static	Header			*pFreeHeaderChain = NULL;
#endif




/*____ initFormatBitstream() ________________________________________________*/

void					initFormatBitstream (encoder_flags_and_data* flags)
{
	flags->BitsRemaining    = 0;
	flags->pHeaderChain     = NULL;
	flags->pFreeHeaderChain = NULL;
}





/*____ exitFormatBitstream() _________________________________________________*/

void					exitFormatBitstream (encoder_flags_and_data* flags)
{
	Header					*psFree;

	/* Fill out the last frame with 0xFF */

	flushFrame (flags);

	/* Dealocate Headers */

	while (flags->pHeaderChain != NULL)
	{
		psFree = flags->pHeaderChain;
		flags->pHeaderChain = psFree->pNext;
		free (psFree);
	}

	while (flags->pFreeHeaderChain != NULL)
	{
		psFree = flags->pFreeHeaderChain;
		flags->pFreeHeaderChain = psFree->pNext;
		free (psFree);
	}
}





/*____ flushFrame() _________________________________________________________*/

void					flushFrame (encoder_flags_and_data* flags)
{
	/* Fill out the last frame with 0xFF */

	while (flags->BitsRemaining > 32)
	{
		putbits (flags, 0xFFFFFFFF, 32);
			flags->BitsRemaining -= 32;
	}

	if (flags->BitsRemaining > 0)
		putbits (flags, 0xFFFFFFFF, flags->BitsRemaining);

	flags->BitsRemaining = 0;
}





/*____ initBitHolder() ______________________________________________________*/

BitHolder				*initBitHolder
(
	BitHolder				*wp,
	int						elements
)
{
	wp->element      = (BitHolderElement *) malloc (sizeof(BitHolderElement) * elements);
	wp->max_elements = elements;
	wp->nrEntries    = 0;

	return wp;
}





/*____ exitBitHolder() ______________________________________________________*/

void					exitBitHolder (BitHolder * wp)
{
	mem_free ((void **)&wp->element);
/*
	if (wp->element != NULL)
	{
		free (wp->element);
		wp->element = NULL;
	}
*/
}





/*____ addBits() ____________________________________________________________*/

void					addBits
(
	BitHolder				*wp,
	unsigned int			value,
	int						length
)
{
	if (length != 0)
	{
		if (wp->nrEntries == wp->max_elements)
		{
			printf ("ERROR: BitHolder overflow!\n");
			exit (-1);
		}

		value &= 0xFFFFFFFF >> (32-length);

		wp->element[wp->nrEntries].value  = value;
		wp->element[wp->nrEntries].length = length;
		wp->nrEntries++;
	}
}





/*____ writeFrame() _________________________________________________________*/

void					writeFrame
(
	encoder_flags_and_data* flags,
	BF_FrameData			*psFrame,
	BF_FrameResults			*results
)
{
	int						bits;
	int						gr, ch;

	int						sizeRemainHeaders, sizeRemainFrames;
	Header					*psHeader;


	/* Generate and save header, return size of SideInfo.*/

	results->SILength = generateHeader (flags, psFrame);


	/* Put the bits and compute size of mainData */

	bits = 0;
	for (gr = 0;  gr < psFrame->nGranules;  gr++)
	{
		for (ch = 0;  ch < psFrame->nChannels;  ch++)
		{
			bits += writeMainDataBits (flags, psFrame, &psFrame->scaleFactors[gr][ch]);
			bits += writeMainDataBits (flags, psFrame, &psFrame->   codedData[gr][ch]);
			bits += writeMainDataBits (flags, psFrame, &psFrame->userSpectrum[gr][ch]);
		}
	}
	bits += writeMainDataBits (flags, psFrame, &psFrame->userFrameData);

	results->mainDataLength = bits;


	/* calculate nextBackPointer */

	sizeRemainHeaders = 0;
	sizeRemainFrames  = 0;

	for (psHeader = flags->pHeaderChain;  psHeader != NULL;  psHeader = psHeader->pNext)
	{
		sizeRemainHeaders += psHeader->size;
		sizeRemainFrames  += psHeader->frameSize;
	}


	results->nextBackPtr = (flags->BitsRemaining / 8) + sizeRemainFrames - sizeRemainHeaders;	/* BitsRemaining must be dividable by 8 */
}





/*=============================================================================
									>>> Static Functions <<<
=============================================================================*/





/*____ writeBitHolder() _____________________________________________________*/

void			writeBitHolder (	encoder_flags_and_data* flags, BitHolder *part)
{
	BitHolderElement		*ep;
	int						i;

	ep = part->element;
	for (i = 0;  i < part->nrEntries;  i++, ep++)
		putbits (flags, ep->value, ep->length);
}





/*____ calcCRC() ____________________________________________________________*/

int				calcCRC
(
	char					*pData,
	int						size
)
{
	int						i;
	int						crc = 0xffff;
	int						masking;
	int						carry;
	int						data;

	for (i = 2;  i < size;  i++)
	{
		if (i != 4  &&  i != 5)
		{
			masking = 1 << 8;
			data    = pData[i];
			while (masking >>= 1)
			{
				carry = crc & 0x8000;
				crc <<= 1;
				/*
					if ( (carry == 0) != ((data & masking) == 0) )
				*/
				if (!carry ^ !(data & masking))
					crc ^= 0x8005;
			}
		}
	}

	crc &= 0xffff;
	return crc;
}





/*____ generateHeader() ____________________________________________________*/

int				generateHeader (encoder_flags_and_data* flags, BF_FrameData *psFrame)
{
	int						gr, ch;
	int						crc;
	Header					*psHeader;
	Header					**wpLink;
	char					*pOldEncodedOutput;


	/* Get a Free Header structure */

	if( flags->pFreeHeaderChain == NULL )
		psHeader = (Header *) malloc( sizeof( Header ) );
	else
	{
		psHeader = flags->pFreeHeaderChain;
		flags->pFreeHeaderChain = psHeader->pNext;
	}
	psHeader->pNext = NULL;
	for( wpLink = &(flags->pHeaderChain) ; * wpLink != NULL ; wpLink = &((*wpLink)->pNext) )
	{}   /* avoid compiler warning */
	*wpLink = psHeader;


	/* Generate the Header */

	pOldEncodedOutput = flags->pEncodedOutput;
	flags->pEncodedOutput = psHeader->data;
	flags->pEncodedOutput[0] = 0;										/* Need to be cleared since we OR in data... */


	writeBitHolder (flags, &psFrame->header);
	writeBitHolder (flags, &psFrame->frameSI);

	for (ch = 0;  ch < psFrame->nChannels;  ch++)
		writeBitHolder (flags, &psFrame->channelSI[ch]);

	for (gr = 0;  gr < psFrame->nGranules;  gr++)
		for (ch = 0;  ch < psFrame->nChannels;  ch++)
			writeBitHolder (flags, &psFrame->spectrumSI[gr][ch]);


	/* Checksum generation (if CRC enabled).*/

	if (!(psHeader->data[1] & 0x1))
	{
		crc = calcCRC (psHeader->data, flags->pEncodedOutput - psHeader->data);
		psHeader->data[4] = (char) (crc >> 8);
		psHeader->data[5] = (char) crc;
	}


	psHeader->size      = flags->pEncodedOutput - psHeader->data;
	psHeader->frameSize = psFrame->frameLength / 8;

	flags->pEncodedOutput = pOldEncodedOutput;


	return	psHeader->size * 8;
}





/*____ writeHeader() _______________________________________________________*/

int				writeHeader (encoder_flags_and_data* flags)
{
	Header					*psHeader;

	psHeader = flags->pHeaderChain;
	memcpy (flags->pEncodedOutput, psHeader->data, psHeader->size);
	flags->pEncodedOutput += psHeader->size;
	*(flags->pEncodedOutput) = 0;

	flags->pHeaderChain = psHeader->pNext;
	psHeader->pNext = flags->pFreeHeaderChain;
	flags->pFreeHeaderChain = psHeader;

	return	(psHeader->frameSize - psHeader->size) * 8;
}





/*____ writeMainDataBits() __________________________________________________*/

int				writeMainDataBits
(
	encoder_flags_and_data* flags,
	BF_FrameData			*psFrame,   /* avoid compiler warning */
	BitHolder				*psBH
)
{
	BitHolderElement		*psElem = psBH->element;
	int						i, bits = 0;
	unsigned int			val;
	int						nBits;

	for (i = 0;  i < psBH->nrEntries;  i++, psElem++)
	{
		val   = psElem->value;
		nBits = psElem->length;

		if (flags->BitsRemaining == 0)
			flags->BitsRemaining = writeHeader (flags);

		if (nBits > flags->BitsRemaining)
		{
			nBits -= flags->BitsRemaining;
			putbits (flags, val >> nBits, flags->BitsRemaining);
			flags->BitsRemaining = writeHeader (flags);
		}

		putbits (flags, val, nBits);
		flags->BitsRemaining -= nBits;

		bits += psElem->length;
	}

	return	bits;
}





/*____ putbits() _____________________________________________________________*/


/*write n bits into the bit stream */

void			putbits
(
	encoder_flags_and_data* flags,
	unsigned int			val,
	int						n
)
{

	if (n == 0)
		return;

	while (n >= flags->outputBit)
	{
		n -= flags->outputBit;

		*(flags->pEncodedOutput) |= val >> n;

		flags->outputBit = 8;
		flags->pEncodedOutput++;
		*(flags->pEncodedOutput) = 0;
	}

	if (n > 0)   /* n < outputBit */
	{
		flags->outputBit -= n;

		*(flags->pEncodedOutput) |= val << flags->outputBit;
	}
}



