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

	-	redesigned, reformatted, slightly optimized
*/

#include	<stdlib.h>
#include	<assert.h>

#include	"system.h"
#include	"common.h"

#include	"l3psy.h"
#include	"loop.h"
#include	"formatbitstream2.h"
#include	"huffman.h"

#include	"l3bitstream.h" /* the public interface */
#include	"l3bitstream-pvt.h"




#if 0
static	int				stereo = 1;
static	frame_params	*fr_ps  = NULL;

static	int				PartHoldersInitialized = 0;



static	BitHolder		       *headerPH;
static	BitHolder		      *frameSIPH;
static	BitHolder		    *channelSIPH[MAX_CHANNELS];
static	BitHolder		   *spectrumSIPH[MAX_GRANULES][MAX_CHANNELS];
static	BitHolder		 *scaleFactorsPH[MAX_GRANULES][MAX_CHANNELS];
static	BitHolder		    *codedDataPH[MAX_GRANULES][MAX_CHANNELS];
static	BitHolder		 *userSpectrumPH[MAX_GRANULES][MAX_CHANNELS];
static	BitHolder		*userFrameDataPH;



static	BF_FrameData	sFrameData;
static	BF_FrameResults	sFrameResults;
#endif




/*
	III_format_bitstream()

	This is called after a frame of audio has been quantized and coded.
	It will write the encoded audio to the bitstream. Note that
	from a layer3 encoder's perspective the bit stream is primarily
	a series of main_data() blocks, with header and side information
	inserted at the proper locations to maintain framing. (See Figure A.7
	in the IS).
*/

void					III_format_bitstream
(
	encoder_flags_and_data* flags,
	int						bitsPerFrame,
	frame_params			*in_fr_ps,
	int						l3_enc[2][2][576],
	III_side_info_t			*l3_side,
	III_scalefac_t			*scalefac,
	double					(*xr)[2][576],
	char					*ancillary,
	int						ancillary_bits
)
{
	int						gr, ch, i, mode_gr;

	flags->fr_ps_bitstream = in_fr_ps;
	flags->stereo = flags->fr_ps_bitstream->stereo;
	mode_gr = 2;

	if (!flags->PartHoldersInitialized)
	{
		 flags->headerPH = initBitHolder (&flags->sFrameData.header, 16*2);
		flags->frameSIPH = initBitHolder (&flags->sFrameData.frameSI, 4*2);

		for (ch = 0;  ch < MAX_CHANNELS;  ch++)
			flags->channelSIPH[ch] = initBitHolder (&flags->sFrameData.channelSI[ch], 8*2);

		for (gr = 0;  gr < MAX_GRANULES;  gr++)
		{
			for (ch = 0;  ch < MAX_CHANNELS;  ch++)
			{
				  flags->spectrumSIPH[gr][ch] = initBitHolder (&flags->sFrameData.  spectrumSI[gr][ch],  32*2);
				flags->scaleFactorsPH[gr][ch] = initBitHolder (&flags->sFrameData.scaleFactors[gr][ch],  64*2);
				   flags->codedDataPH[gr][ch] = initBitHolder (&flags->sFrameData.   codedData[gr][ch], 576*2);
				flags->userSpectrumPH[gr][ch] = initBitHolder (&flags->sFrameData.userSpectrum[gr][ch],   4*2);
			}
		}
		flags->userFrameDataPH = initBitHolder (&flags->sFrameData.userFrameData, 8*2);

		flags->PartHoldersInitialized = 1;
	}

#if 1
	for (gr = 0;  gr < mode_gr;  gr++)
	{
		for (ch = 0;  ch < flags->stereo;  ch++)
		{
			int						*pi = &l3_enc[gr][ch][0];
			double					*pr =     &xr[gr][ch][0];

			for (i = 0;  i < 576;  i++, pr++, pi++)
				if (*pr < 0  &&  *pi > 0)
					*pi *= -1;
		}
	}
#endif

	encodeSideInfo (flags, l3_side);
	encodeMainData (flags, l3_enc, l3_side, scalefac);
	write_ancillary_data (flags, ancillary, ancillary_bits);

	if (l3_side->resvDrain)
		drain_into_ancillary_data (flags, l3_side->resvDrain);

	flags->sFrameData.frameLength = bitsPerFrame;
	flags->sFrameData.nGranules   = mode_gr;
	flags->sFrameData.nChannels   = flags->stereo;

	writeFrame (flags, &flags->sFrameData, &flags->sFrameResults);

	/* we set this here -- it will be tested in the next loops iteration */
	l3_side->main_data_begin = flags->sFrameResults.nextBackPtr;
}





void					III_FlushBitstream (	encoder_flags_and_data* flags)
{
	int						gr, ch;

	if (flags->PartHoldersInitialized)
	{
		exitBitHolder (&flags->sFrameData.header);
		exitBitHolder (&flags->sFrameData.frameSI);

		for (ch = 0;  ch < MAX_CHANNELS;  ch++)
			exitBitHolder (&flags->sFrameData.channelSI[ch]);


		for (gr = 0;  gr < MAX_GRANULES;  gr++)
		{
			for (ch = 0;  ch < MAX_CHANNELS;  ch++)
			{
				exitBitHolder (&flags->sFrameData.  spectrumSI[gr][ch]);
				exitBitHolder (&flags->sFrameData.scaleFactors[gr][ch]);
				exitBitHolder (&flags->sFrameData.   codedData[gr][ch]);
				exitBitHolder (&flags->sFrameData.userSpectrum[gr][ch]);
			}
		}
		exitBitHolder (&flags->sFrameData.userFrameData);

		flags->PartHoldersInitialized = 0;
	}

	/* BF_FlushBitstream (frameData, frameResults); */
}





static	unsigned		slen1_tab[16] = { 0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4 };
static	unsigned		slen2_tab[16] = { 0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3 };





void			encodeMainData
(
	encoder_flags_and_data* flags,
	int						l3_enc[2][2][576],
	III_side_info_t			*side_info,
	III_scalefac_t			*scalefac
)
{
	int						gr, ch, sfb, b, mode_gr;

	mode_gr = 2;


	for (gr = 0;  gr < mode_gr;  gr++)
		for (ch = 0;  ch < flags->stereo;  ch++)
			flags->scaleFactorsPH[gr][ch]->nrEntries = 0;


	for (gr = 0;  gr < mode_gr;  gr++)
		for (ch = 0;  ch < flags->stereo;  ch++)
			flags->codedDataPH[gr][ch]->nrEntries = 0;


	for (gr = 0;  gr < 2;  gr++)
	{
		for (ch = 0;  ch < flags->stereo;  ch++)
		{
			BitHolder				*ph = flags->scaleFactorsPH[gr][ch];		
			gr_info					*cod_info = &side_info->gr[gr].ch[ch].tt;
			unsigned				slen1 = slen1_tab[cod_info->scalefac_compress];
			unsigned				slen2 = slen2_tab[cod_info->scalefac_compress];
			int						*ix = &l3_enc[gr][ch][0];

			if (cod_info->window_switching_flag  &&  cod_info->block_type == SHORT_TYPE)
			{
				if (cod_info->mixed_block_flag)
				{
					for (sfb = 0;  sfb < 8;  sfb++)
						addBits (ph, scalefac->l[gr][ch][sfb], slen1);

					for (sfb = 3;  sfb < 6;  sfb++)
						for (b = 0;  b < 3;  b++)
							addBits (ph, scalefac->s[gr][ch][sfb][b], slen1);
				}
				else
				{
					for (sfb = 0;  sfb < 6;  sfb++)
						for (b = 0;  b < 3;  b++)
							addBits (ph, scalefac->s[gr][ch][sfb][b], slen1);
				}

				for (sfb = 6;  sfb < 12;  sfb++)
					for (b = 0;  b < 3;  b++)
						addBits (ph, scalefac->s[gr][ch][sfb][b], slen2);
			}
			else if (gr == 0)
			{
				for (sfb = 0;  sfb < 11;  sfb++)
					addBits (ph, scalefac->l[gr][ch][sfb], slen1);

				for (sfb = 11;  sfb < 21;  sfb++)
					addBits (ph, scalefac->l[gr][ch][sfb], slen2);
			}
			else
			{
				if (!side_info->scfsi[ch][0])
					for (sfb = 0;  sfb < 6;  sfb++)
						addBits (ph, scalefac->l[gr][ch][sfb], slen1);

				if (!side_info->scfsi[ch][1])
					for (sfb = 6;  sfb < 11;  sfb++)
						addBits (ph, scalefac->l[gr][ch][sfb], slen1);

				if (!side_info->scfsi[ch][2])
					for (sfb = 11;  sfb < 16;  sfb++)
						addBits (ph, scalefac->l[gr][ch][sfb], slen2);

				if (!side_info->scfsi[ch][3])
					for (sfb = 16;  sfb < 21;  sfb++)
						addBits (ph, scalefac->l[gr][ch][sfb], slen2);
			}

			Huffmancodebits (flags, flags->codedDataPH[gr][ch], ix, cod_info);
		} /* for ch */
	} /* for gr */
} /* main_data */





/*____ encodeSideInfo() _____________________________________________________*/

int				encodeSideInfo (encoder_flags_and_data* flags, III_side_info_t *side_info)
{
	int						gr, ch, scfsi_band, region, b, bits_sent, mode_gr;
	layer					*info = flags->fr_ps_bitstream->header;

	mode_gr =  2;


	flags->headerPH->nrEntries = 0;

	addBits (flags->headerPH, 0xfff                   , 12);
	addBits (flags->headerPH, 1                       ,  1);
	addBits (flags->headerPH, 4 - 3                   ,  2);   /* 4 - Layer */
	addBits (flags->headerPH, !info->error_protection ,  1);
	addBits (flags->headerPH, info->bitrate_index     ,  4);
	addBits (flags->headerPH, info->sampling_frequency,  2);
	addBits (flags->headerPH, info->padding           ,  1);
	addBits (flags->headerPH, info->extension         ,  1);
	addBits (flags->headerPH, info->mode              ,  2);
	addBits (flags->headerPH, info->mode_ext          ,  2);
	addBits (flags->headerPH, info->copyright         ,  1);
	addBits (flags->headerPH, info->original          ,  1);
	addBits (flags->headerPH, info->emphasis          ,  2);

	bits_sent = 32;

	if (info->error_protection)
	{
		addBits (flags->headerPH, 0, 16);   /* Just a dummy add. Real CRC calculated & inserted in writeSideInfo() */
		bits_sent += 16;
	}


	flags->frameSIPH->nrEntries = 0;

	addBits (flags->frameSIPH, side_info->main_data_begin, 9);

	if (flags->stereo == 2)
		addBits (flags->frameSIPH, side_info->private_bits, 3);
	else
		addBits (flags->frameSIPH, side_info->private_bits, 5);


	for (ch = 0;  ch < flags->stereo;  ch++)
	{
		BitHolder				*ph = flags->channelSIPH[ch];

		ph->nrEntries = 0;

		for (scfsi_band = 0;  scfsi_band < 4;  scfsi_band++)
			addBits (ph, side_info->scfsi[ch][scfsi_band], 1);
	}


	for (gr = 0;  gr < 2;  gr++)
	{
		for (ch = 0;  ch < flags->stereo;  ch++)
		{
			BitHolder				*ph = flags->spectrumSIPH[gr][ch];
			gr_info					*cod_info = &side_info->gr[gr].ch[ch].tt;

			ph->nrEntries = 0;

			addBits (ph, cod_info->part2_3_length       , 12);
			addBits (ph, cod_info->big_values           ,  9);
			addBits (ph, cod_info->global_gain          ,  8);
			addBits (ph, cod_info->scalefac_compress    ,  4);
			addBits (ph, cod_info->window_switching_flag,  1);

			if (cod_info->window_switching_flag)
			{   
				addBits (ph, cod_info->block_type      , 2);
				addBits (ph, cod_info->mixed_block_flag, 1);

				for (region = 0; region < 2; region++)
					addBits (ph, cod_info->table_select[region], 5);

				for (b = 0;  b < 3;  b++)
					addBits (ph, cod_info->subblock_gain[b], 3);
			}
			else
			{
				for (region = 0; region < 3; region++)
					addBits (ph, cod_info->table_select[region], 5);

				addBits (ph, cod_info->region0_count, 4);
				addBits (ph, cod_info->region1_count, 3);
			}

			addBits (ph, cod_info->preflag           , 1);
			addBits (ph, cod_info->scalefac_scale    , 1);
			addBits (ph, cod_info->count1table_select, 1);
		}
	}


	if (flags->stereo == 2)
		bits_sent += 256;
	else
		bits_sent += 136;


	return bits_sent;
}





/*____ write_ancillary_data() _______________________________________________*/

void			write_ancillary_data
(
	encoder_flags_and_data* flags,
	char					*theData,
	int						lengthInBits
)
{
	int						bytesToSend   = lengthInBits / 8;
	int						remainingBits = lengthInBits % 8;
	unsigned				wrd;

	flags->userFrameDataPH->nrEntries = 0;

	while (bytesToSend--)
	{
		wrd = *theData++;
		addBits (flags->userFrameDataPH, wrd, 8);
	}
	if (remainingBits)
	{
		/* right-justify remaining bits */
		wrd = *theData >> (8 - remainingBits);
		addBits (flags->userFrameDataPH, wrd, remainingBits);
	}
}





/*
	Some combinations of bitrate, Fs, and stereo make it impossible to stuff
	out a frame using just main_data, due to the limited number of bits to
	indicate main_data_length. In these situations, we put stuffing bits into
	the ancillary data...
*/
void			drain_into_ancillary_data (	encoder_flags_and_data* flags, int lengthInBits)
{
	int						wordsToSend   = lengthInBits / 32;
	int						remainingBits = lengthInBits % 32;

	/*
		flags->userFrameDataPH->part->nrEntries set by call to write_ancillary_data()
	*/

	while (wordsToSend--)
		addBits (flags->userFrameDataPH, 0, 32);
	if (remainingBits)
		addBits (flags->userFrameDataPH, 0, remainingBits); 
}





/*
	Note the discussion of huffmancodebits() on pages 28
	and 29 of the IS, as well as the definitions of the side
	information on pages 26 and 27.
*/
void			Huffmancodebits
(
	encoder_flags_and_data* flags,
	BitHolder				*ph,
	int						*ix,
	gr_info					*cod_info
)
{
	int						sfb, window, line, start, end;
	int						stuffingBits, table;

	int						bitsWritten = 0;
	int						bigvalues   = cod_info->big_values * 2;


	/*
		Within each scalefactor band, data is given for successive time windows,
		beginning with window 0 and ending with window 2 (in case of short blocks!
		--- there is only one long block window). Within each window, the quantized
		values are then arranged in order of increasing frequency...
	*/

	/* 1: Write the bigvalues */
	if (bigvalues)
	{
		if (cod_info->window_switching_flag  &&  (cod_info->block_type == SHORT_TYPE))
		{
			int							(*ix_s)[3] = (int (*)[3]) ix;
			int							*scalefac = &blade_sfBandIndex[flags->fr_ps_bitstream->header->sampling_frequency].s[0];

			table = cod_info->table_select[0];
			if (table)
			{
				if (cod_info->mixed_block_flag)  /* Mixed blocks long, short */
				{
					for (line=0; line<36; line+=2)   /* cod_info->address1 = 36 */
						bitsWritten += writeHuffmanCode (flags, ph, table, ix[line], ix[line+1]);
				}
				else
				{
					for (sfb=0; sfb<3; sfb++)   /* (cod_info->region0_count + 1) / 3 = 3 */
					{
						start = scalefac[sfb];
						end   = scalefac[sfb+1];

						for (window=0; window<3; window++)
							for (line=start; line<end; line+=2)
								bitsWritten += writeHuffmanCode (flags, ph, table, ix_s[line][window], ix_s[line+1][window]);
					}
				}
			}

			table = cod_info->table_select[1];
			if (table)
			{
				for (sfb=3; sfb<SFB_SMAX; sfb++)   /* scalefac[3] = 12 --- 12*3 = 36 */
				{
					start = scalefac[sfb];
					end   = scalefac[sfb+1]; 

					for (window=0; window<3; window++)
						for (line=start; line<end; line+=2)
							bitsWritten += writeHuffmanCode (flags, ph, table, ix_s[line][window], ix_s[line+1][window]);
				}
			}
		}
		else   /* Long blocks */
		{
			int							region1Start = (cod_info->address1 > bigvalues) ? bigvalues : cod_info->address1;
			int							region2Start = (cod_info->address2 > bigvalues) ? bigvalues : cod_info->address2;

			table = cod_info->table_select[0];
			if (table)
				for (line=0; line<region1Start; line+=2)
					bitsWritten += writeHuffmanCode (flags, ph, table, ix[line], ix[line+1]);

			table = cod_info->table_select[1];
			if (table)
				for (line=region1Start; line<region2Start; line+=2)
					bitsWritten += writeHuffmanCode (flags, ph, table, ix[line], ix[line+1]);

			table = cod_info->table_select[2];
			if (table)
				for (line=region2Start; line<bigvalues; line+=2)
					bitsWritten += writeHuffmanCode (flags, ph, table, ix[line], ix[line+1]);
		}
	}

	/* 2: Write count1 area */
	if (cod_info->count1)
	{
		struct huffcodetab		*h = blade_ht + (cod_info->count1table_select + 32);

		int						*pos = ix + bigvalues;
		int						*end = ix + bigvalues + (cod_info->count1 * 4);

		while (pos < end)
		{
			int						len, v, w, x, y;
			int						bits = 0;
			int						p    = 0;

			v = *pos++;  if (v)  bits++, p |= 1;
			w = *pos++;  if (w)  bits++, p |= 2;
			x = *pos++;  if (x)  bits++, p |= 4;
			y = *pos++;  if (y)  bits++, p |= 8;

			addBits (ph, h->table[p], len = h->hlen[p]);

			if (v)  addBits (ph, v<0, 1);
			if (w)  addBits (ph, w<0, 1);
			if (x)  addBits (ph, x<0, 1);
			if (y)  addBits (ph, y<0, 1);

			bitsWritten += bits+len;
		}
	}


	stuffingBits = cod_info->part2_3_length - cod_info->part2_length - bitsWritten;
	if (stuffingBits)
	{
		int						stuffingWords = stuffingBits / 32;
		int						remainingBits = stuffingBits % 32;

		/*
			Due to the nature of the Huffman code
			tables, we will pad with ones
		*/
		while (stuffingWords--)
			addBits (ph, ~0, 32);
		if (remainingBits)
			addBits (ph, ~0, remainingBits);

		bitsWritten += stuffingBits;
	}
}





/*
	Implements the pseudocode of page 98 of the IS

	aaaaaaaaaaaaaaargh --- why don«t write the code immediately?
*/

int				writeHuffmanCode
(
	encoder_flags_and_data* flags,
	BitHolder				*ph,
	int						table,
	int						x,
	int						y
)
{
	struct huffcodetab		*h = blade_ht + table;

	unsigned				signx = (x <= 0) ? (x = -x, 1) : 0;
	unsigned				signy = (y <= 0) ? (y = -y, 1) : 0;

	unsigned				code, cbits, idx;

assert (table);
/*
	if (table == 0)
		return 0;
*/

	if (table > 15)   /* ESC-table is used */
	{
		unsigned				linbits = h->linbits;
		unsigned				ext     = 0;
		unsigned				xbits   = 0;

		if (x)  {if (x > 14)  {ext =                    (x-15);  xbits += linbits;  x = 15;}  ext = (ext << 1) | signx;  xbits++;}
		if (y)  {if (y > 14)  {ext = (ext << linbits) | (y-15);  xbits += linbits;  y = 15;}  ext = (ext << 1) | signy;  xbits++;}

		idx   = x * h->ylen + y;
		code  = h->table[idx];
		cbits = h->hlen[idx];

		addBits (ph, code, cbits);
		addBits (ph, ext , xbits);

		return cbits + xbits;
	}
	else   /* No ESC-words */
	{
		idx   = x * h->ylen + y;
		code  = h->table[idx];
		cbits = h->hlen[idx];

		if (x)  {code = (code << 1) | signx;  cbits++;}
		if (y)  {code = (code << 1) | signy;  cbits++;}

		addBits (ph, code, cbits);

		return cbits;
	}
}



