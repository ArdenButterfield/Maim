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

	2000-11-22  Andre Piotrowski

	-	bug fix: removeJobQueueEntry() -- caught a wrong condition -- harmless, because never called that way
	-	feature: CONCAT != 0 allows to concatenate files encoded with '-noGap'

	2000-12-11  ap

	-	include "arglink.h" (now containing the prototype for 'mystrupr()')

	2001-01-12  ap

	-	version number

	2001-01-19	Tord Jansson

	-	Changed all return values to positive (and different) values.


*/


#include <stdio.h>
#include <math.h>

#include "blade.h"

int main()
{
	float left[1152];
	float right[1152];
	char out[10000];
	int numout;
	int i = 0;
	encoder_flags_and_data* flags = blade_init(44100, 320);
	for (int round = 0; round < 1000; ++round){
		for (i = 0; i < 1152; ++i) {
			left[i] = sin((float)((round * 1152 + i) / 1000.f));
			right[i] = sin((float)((round * 1152 + i) / 1000.f));
		}
		numout = blade_encode_chunk(flags, left, right, out);
		for (int c = 0; c < numout; ++c) {
			putchar(out[c]);
		}
	}
	blade_deinit(flags);
}

#if 0
#include	<stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <time.h>

#include    "system.h"
#include	"common.h"
#include	"codec.h"
#include    "samplein.h"
#include	"arglink.h"
#include	"bladesys.h"





#if SYSTEM == MAC_OS

#include	<SIOUX.h>
#include	<Events.h>

	int						main_  (int argc, char* argv[]);
	void					DoEvent (EventRecord *i);

	extern int gFriendliness;

#endif





enum
{
	PROG_OFF,
	PROG_BOTH_PERC_ETA,
	PROG_FILE_GRAPH,
	PROG_BATCH_GRAPH,
	PROG_BOTH_GRAPH,
	PROG_BOTH_SAMPLES,
	PROG_BOTH_ETA,
	PROG_FILE_RLEN,
	PROG_FILE_NORETURN
};





/*____ Structure Definitions __________________________________________________*/

typedef		struct JobDef			Job;

			struct JobDef
			{
				CodecInitIn			sCodec;
				SI_Stream			sInput;
				Job					*psNext;
				char				bDeleteSource;
				char				bNoGap;
				char				bConcat;
				char				outputFilename[MAX_NAMELEN];
				char				sourceFilename[MAX_NAMELEN];
			};





/*____ Function Prototypes ____________________________________________________*/

void					quit (int returnValue);

void					timerCalibrate (void);
time_t					timerStart (void);
float					timerStop (time_t startTime);

void					updateProgressIndicator
(
	time_t					startBatch,
	double					batchLen,
	double					batchDone,
	time_t					startFile,
	double					fileLen,
	double					fileDone, 
	time_t					currTime,
	int						progType,
	Job						*psJob
);

void					setOutputDir (char *pPath);

argLink					*readGlobalSwitches (argLink *pArgLink);
argLink					*readLocalSwitches (argLink *pArgLink, Job *psJob);

void					sortJobs (Job **psJob);
int						validateJobs (Job *psJob);

int						printUsage (void);

int						addCommandlineJob (argLink **pArgLink);
int						clearJobQueue (void);
int						removeJobQueueEntry (Job *psJob);

/* unused prototypes!? */
int						addFileToBatch (char *pFilename, char *pOutputFilename);
int						readL3EncCommandline (int argc, char *argv[]);





/*____ Static Data ____________________________________________________________*/



/* Parameters set through the commandline. */

int						wantedBitrate	   = -1;   /* -1 = Unspecified. */
int						wantedCRC		   = FALSE;
int						wantedPrivate	   = FALSE;
int						wantedCopyright    = FALSE;
int						wantedOriginal	   = TRUE;
SI_OutFormat			wantedInputType    = STEREO;
int						wantedChannelSwap  = FALSE;

int						wantedDeleteSource = FALSE;
int						wantedNoGap        = FALSE;
int						wantedSort         = FALSE;
int						wantedConcat       = FALSE;

#ifdef WAIT_KEY
#define		DFLT_WANTED_QUIT		FALSE
#else
#define		DFLT_WANTED_QUIT		FALSE
#endif 
int						wantedQuit         = DFLT_WANTED_QUIT;

int						wantedQuiet        = FALSE;
int						wantedSTDOUT       = FALSE;
int						fPreparedSTDIN     = FALSE;

char					prioString[256];
char					*pPrioString       = NULL;

Job						*psJobQueue        = NULL;

char					outputDir[MAX_NAMELEN];

FILE					*textout;

int						timerCalibrationValue;   /* Only used if PRECISE_TIMER is defined. */

int						progType           = PROG_BOTH_PERC_ETA;   /* Style of the progress bar */


int						rawFrequency       = 44100;
int						rawChannels        = 2;
int						rawBits            = 16;
int						rawSigned          = TRUE;
int						rawByteorder       = BYTEORDER;





/*____ main() _________________________________________________________________*/

#if SYSTEM == MAC_OS
int						main_ (int argc, char* argv[])   /* Because the real main is somewhere else */
#else
int						main (int argc, char* argv[])
#endif
{
	int						samplesPerFrame = 0;   /* Initialized just to prevent compiler warnings */
	int						nSamples;
	int						nSamples2;

	short					readBuffer[2304];
	int						x;

	char					input;
	Job						*psTemp;

	time_t					startTimeBatch, startTimeFile, currTime, oldTime = 0;
	float					seconds;
	double					batchSamplesTotal = 0.0, batchSamplesRead = 0.0;
	double					fileSamplesRead;

	CodecInitOut			*pCodecInfo;
	char					*pBuffer = NULL;   /* Initialized just to prevent compiler warnings */
	uint					encodedChunkSize;
	FILE					*fp = 0;			/* Initialized just to prevent compiler warnings */

	char					temp[256];


	argLink					*pArgLink, *pNextArg;


	int						bContinueNoGap;
	int						bConcatenate;
	int						nNoGapSamples;
	int						noGapOfs;

	encoder_flags_and_data flags_and_data;

#if SYSTEM == MAC_OS
	long					macTickStart;

	macTickStart = TickCount ();
#endif


	/*
		Setting default parameters. Might seem useless since most of them
		allready are predefined, but the MAC OS port uses main() as a subroutine
		which can be called multiple times.
	*/

	wantedBitrate	   = -1;   /* -1 = Unspecified. */
	wantedCRC		   = FALSE;
	wantedPrivate	   = FALSE;
	wantedCopyright    = FALSE;
	wantedOriginal	   = TRUE;
	wantedInputType    = STEREO;
	wantedChannelSwap  = FALSE;

	wantedDeleteSource = FALSE;
	wantedNoGap        = FALSE;
    wantedConcat       = FALSE;
	wantedSort         = FALSE;

	wantedQuit	       = DFLT_WANTED_QUIT;
	wantedQuiet        = FALSE;
	wantedSTDOUT       = FALSE;
	fPreparedSTDIN     = FALSE;

	pPrioString        = NULL;

	psJobQueue         = NULL;


#if SYSTEM == MAC_OS
	progType           = PROG_FILE_NORETURN;
#else
	progType           = PROG_BOTH_PERC_ETA;   /* Style of the progress bar */
#endif

	rawFrequency       = 44100;
	rawChannels        = 2;
	rawBits            = 16;
	rawSigned          = TRUE;
	rawByteorder       = BYTEORDER;

	textout            = stdout;
	outputDir[0]       = '\0';

	bContinueNoGap     = FALSE;
    bConcatenate       = FALSE;
	nNoGapSamples      = 0;
	noGapOfs           = 0;


	/* Fix ArgLink */

	pArgLink = argv2ArgLink (argc-1, argv+1);

	if (findStrInArgLink (pArgLink, "-NOCFG") == NULL)
		if (findConfigFile (argv[0], temp) == TRUE)
			addFileContentToArgLink (&pArgLink, temp);

#ifdef	WILDCARDS
	expandWildcards (&pArgLink);
#endif

	pNextArg = readGlobalSwitches (pArgLink);


	/* Check for STDOUT */

	if (findStrInArgLink (pArgLink, "stdout") != NULL)
	{
		prepStdout ();
		textout = stderr;
	}


	/* Print Text */

	if (!wantedQuiet)
	{
		fprintf (textout, "\n");
		fprintf (textout, "BladeEnc 0.94.2    (c) Tord Jansson            Homepage: http://bladeenc.mp3.no\n");
		fprintf (textout, "===============================================================================\n");
		fprintf (textout, "BladeEnc is free software, distributed under the Lesser General Public License.\n");
		fprintf (textout, "See the file COPYING, BladeEnc's homepage or www.fsf.org for more details.\n");
		fprintf (textout, "\n");
	}


	/* Initialise batch */

	while (pNextArg != NULL)
	{
		x = addCommandlineJob (&pNextArg);
		if (x == FALSE)
		{
			deleteArgLink (pArgLink);
			quit (4);
		}
	}

	deleteArgLink (pArgLink);


	/* Validate job settings */

	x = validateJobs (psJobQueue);
	if (x == FALSE)
		quit (2);


	/* Possibly sort the jobs alphabetically on input name */
	
	if (wantedSort)
		sortJobs (&psJobQueue);


	/* Make sure we don't have certain progress indicators if RAW file in batch */

	x = 0;
	for (psTemp = psJobQueue;  psTemp != NULL;  psTemp = psTemp->psNext)
	{
		if (psTemp->sInput.filetype == RAW)
			x++;
	}
	if (x != 0  &&  progType != PROG_OFF  &&  progType != PROG_FILE_RLEN)
		progType = PROG_FILE_RLEN;

	/* Set priority */

	if (setPriority (pPrioString) == FALSE)
	{
		fprintf (textout, "Error: '%s' is not a valid priority setting!\n", pPrioString);
		quit (3);
	};


	/* Procedure if no files found */

	if (psJobQueue == NULL)
	{
		printUsage ();                                                           /* No files on the commandline */
		quit (1);
	}


	/* Print number of files to encode */

	if (!wantedQuiet)
	{
		x = 0;
		for (psTemp = psJobQueue;  psTemp != NULL;  psTemp = psTemp->psNext)
			x++;
		fprintf (textout, "Files to encode: %d\n\n", x);
	}


	/* Calculate batchSamplesTotal */

	for (psTemp = psJobQueue;  psTemp != NULL;  psTemp = psTemp->psNext)
		if (psTemp->sInput.length == 0xFFFFFFFF)
		{
			batchSamplesTotal = 0xFFFFFFFF;
			break;
		}
		else
			batchSamplesTotal += psTemp->sInput.length;


	/* Encode */

#ifdef	PRECISE_TIMER
	timerCalibrate ();
#endif

	startTimeBatch = timerStart ();

	while (psJobQueue != NULL)
	{
		/* Print information */

		if (!wantedQuiet)
		{
			fprintf (textout, "Encoding:  %s\n", psJobQueue->sourceFilename);
			fprintf (textout, "Input:     %.1f kHz, %d bit, ", psJobQueue->sInput.freq/1000.f, psJobQueue->sInput.channelBits);
			if (psJobQueue->sInput.nChannels >= 2)
				fprintf (textout, "stereo.\n");
			else
				fprintf (textout, "mono.\n");
			fprintf (textout, "Output:    %d kBit, ", psJobQueue->sCodec.bitrate);
			if (psJobQueue->sCodec.mode == 0)
				fprintf (textout, "stereo.\n\n");
			else
				fprintf (textout, "mono.\n\n");

            if (bConcatenate)
                fprintf (textout, "Concatenates output to previous mp3-file\n");
			else if (bContinueNoGap)
				fprintf (textout, "Received %d samples from previous file due to -nogap\n", nNoGapSamples);
		}

		/* Init a new job */

		startTimeFile = timerStart ();
		fileSamplesRead = 0;

		if (!bContinueNoGap)
		{
			pCodecInfo = codecInit (&flags_and_data, &psJobQueue->sCodec);
			samplesPerFrame = pCodecInfo->nSamples;
			if (psJobQueue->sCodec.mode != 3)
				samplesPerFrame /= 2;
			pBuffer = (char *) malloc (pCodecInfo->bufferSize);
		}
		printf("samples per frame: %i\n", samplesPerFrame);

		if (strcmp (psJobQueue->outputFilename, "STDOUT") == 0)
		{
			fp = stdout;
		}
		else if (!bConcatenate)
		{
			fp = fopen (psJobQueue->outputFilename, "wb");
			if (fp == NULL)
			{
				/*  codecExit (); */
				closeInput (&psJobQueue->sInput);
				fprintf (textout, "ERROR: Couldn't create '%s'!\n", psJobQueue->outputFilename);
				quit (5);
			}
		}

		/* Encoding loop */
		while  ((nSamples = readSamples (&psJobQueue->sInput, samplesPerFrame - nNoGapSamples, readBuffer + noGapOfs)) > 0)
		{
			nSamples     += nNoGapSamples;
			nNoGapSamples = 0;
			noGapOfs      = 0;

			if (psJobQueue->bNoGap  &&  nSamples < samplesPerFrame)
				break;	

			/* Stuff needed for Petteri Kamppuri's Mac OS port */

#if SYSTEM == MAC_OS	
	 		if (macTickStart + gFriendliness < TickCount ())
			{
	        	macTickStart = TickCount ();
	
	       		Boolean					gotEvent;
	         	EventRecord				event;
	
	         	gotEvent = GetNextEvent (everyEvent, &event);
	         	if (gotEvent)
				{
	            	SIOUXHandleOneEvent (&event);
	           		DoEvent (&event);
	         	}
			}
#endif

			/* Encode a chunk and Write to stream */

			nSamples2 = nSamples;
			if (psJobQueue->sCodec.mode != 3)
				nSamples2 *= 2;

			encodedChunkSize = codecEncodeChunk (&flags_and_data, nSamples2, readBuffer, pBuffer);
			if (fwrite (pBuffer, 1, encodedChunkSize, fp) != encodedChunkSize)
			{
				fprintf (textout, "ERROR: Couldn't write '%s'! Disc probably full.\n", psJobQueue->outputFilename);
				quit (6);
			}


			/* Update progress indicator */

			batchSamplesRead += nSamples;
			fileSamplesRead  += nSamples;

			if (!wantedQuiet && progType != PROG_OFF)
			{
				currTime = time (NULL);
				if (currTime != oldTime)
				{
					oldTime = currTime;
					updateProgressIndicator (startTimeBatch, batchSamplesTotal, batchSamplesRead,
					                         startTimeFile, psJobQueue->sInput.length, fileSamplesRead, 
											 currTime, progType, psJobQueue);
				}
			}


			/* Handling pause through ESC (Windows & OS/2 only) */

			if (be_kbhit() != 0)
			{
				input = be_getch ();
				if (input == 27)
				{
					fprintf (textout, "\r                                                                             \r");
					fprintf (textout, "Quit, are you sure? (y/n)");
					fflush (textout);
					input = be_getch();
					if (input == 'y'  ||  input == 'Y')
					{
						encodedChunkSize = codecExit (&flags_and_data, pBuffer);
						if (encodedChunkSize != 0)
							if (fwrite (pBuffer, encodedChunkSize, 1, fp) != 1)
							{
								fprintf (textout, "ERROR: Couldn't write '%s'! Disc probably full.\n", psJobQueue->outputFilename);
								quit (6);
							}
						free (pBuffer);
						closeInput (&psJobQueue->sInput);
						if (fp != stdout)
							fclose (fp);
						return 0;
					}
					else
						fprintf (textout, "\r                                                                             \r");
				}

			}
		}


		/* File done */

		batchSamplesRead += nSamples;
		fileSamplesRead += nSamples;

		if (!wantedQuiet)
			updateProgressIndicator (startTimeBatch, batchSamplesTotal, batchSamplesRead,
			                         startTimeFile, psJobQueue->sInput.length, fileSamplesRead,
						 			 time (NULL), progType, psJobQueue);

		if (psJobQueue->bNoGap  &&  psJobQueue->psNext != NULL)
		{
			encodedChunkSize = codecFlush (&flags_and_data, pBuffer);
			bContinueNoGap   = TRUE;
			nNoGapSamples    = nSamples;
			noGapOfs         = nSamples;

			if (psJobQueue->sInput.outputType == STEREO  ||  psJobQueue->sInput.outputType == INVERSE_STEREO)
				noGapOfs       *= 2;
            if (psJobQueue->bConcat)
                bConcatenate    = TRUE;
		}
		else
		{
			encodedChunkSize = codecExit (&flags_and_data, pBuffer);
			bContinueNoGap   = FALSE;
            bConcatenate     = FALSE;
		}

		if (encodedChunkSize != 0)
			if (fwrite (pBuffer, encodedChunkSize, 1, fp) != 1)
			{
				fprintf (textout, "ERROR: Couldn't write '%s'! Disc probably full.\n", psJobQueue->outputFilename);
				quit (6);
			}

		if (!bConcatenate  &&  fp != stdout)
			fclose (fp);

		if (!bContinueNoGap  ||  psJobQueue->psNext == NULL)
			free (pBuffer);

		if (psJobQueue->bDeleteSource == TRUE)
			remove (psJobQueue->sourceFilename);

		seconds = timerStop (startTimeFile);
		x = (int) seconds;
		if (!wantedQuiet)
		{
			fprintf (textout, "\r                                                                             \r");
			fprintf (textout, "Completed. Encoding time: %02d:%02d:%02d (%.2fX)\n\n",
			         x/3600, (x/60)%60, x%60, ((float)fileSamplesRead) /
			         (psJobQueue->sInput.freq*seconds));
		}

		removeJobQueueEntry (psJobQueue);
	}


	/* Batch done */

	if (!wantedQuiet)
	{
		seconds = timerStop (startTimeBatch);
		fprintf (textout, "All operations completed. Total encoding time: %02d:%02d:%02d\n",
		         (int) seconds/3600, (int)(seconds/60)%60, (int) seconds%60);

#ifdef	WAIT_KEY
		if (!wantedQuit)
		{			
			fprintf (textout, "Press ENTER to exit...");
			fflush( textout );
			be_getch ();
			fprintf (textout, "\n");
		}
#endif
	}

	return 0;
}





/*____ quit() _________________________________________________________________*/

void					quit (int returnValue)
{
#ifdef	WAIT_KEY
	if (!wantedQuit)
	{
		fprintf (textout, "Press ENTER to exit...");
		fflush( textout );
		be_getch ();
		fprintf (textout, "\n");
	}
#endif
#if SYSTEM == MAC_OS
	throw returnValue;
#else
	exit (returnValue);
#endif
}





/*____ timerCalibrate() _______________________________________________________*/

void					timerCalibrate (void)
{
	time_t					x, y;
	int						i;

	x = y = time (NULL);

	while (y == x)
		y = time (NULL);

	for (i = 0;  y == time (NULL);  i++)
		{}   /* avoid compiler warning */

	printf ("Timer Calibration Value: %d\n", timerCalibrationValue = i);
}





/*____ timerStart() ___________________________________________________________*/

time_t					timerStart (void)
{
	time_t					x, y;

	x = y = time (NULL);

#ifdef	PRECISE_TIMER
	while (y == x)
		y = time (NULL);
#endif

	return y;
}





/*____ timerStop() ____________________________________________________________*/

float					timerStop (time_t startTime)
{
	float					seconds;
	time_t					stopTime;

	stopTime = time (NULL);
#ifdef	PRECISE_TIMER
	{
		int  i = 0;  while (stopTime == time (NULL))  i++;
	}
#endif

	seconds = (float) (stopTime - startTime);

#ifdef	PRECISE_TIMER
	seconds += (float)(timerCalibrationValue - i) / timerCalibrationValue;

	printf ("Exact seconds: %.5f\n", seconds);
#endif

	return seconds;
}





/*____ updateProgressIndicator() ______________________________________________*/

void					updateProgressIndicator
(
	time_t					startBatch,
	double					batchLen,
	double					batchDone,
	time_t					startFile,
	double					fileLen,
	double					fileDone, 
	time_t					currTime,
	int						progType,
	Job						*psJob
)
{
/*	time_t					currTime; */
	float					percentageFile, percentageBatch;
	int						fileEta, batchEta;

	char					temp[82];
	int						x, y, i;


	static	char			wheel[4] = { '|' , '/' , '-' , '\\' };
	static	int				wheelindex = 0;
	static	int				prevFileDone = -1;

/*	currTime = time (NULL); */

	switch (progType)
	{
		case PROG_BOTH_PERC_ETA:
			percentageFile = (float) (fileDone / fileLen * 100);
			if (percentageFile >= 100.f)
				percentageFile = (float) 99.9;

			fileEta = (int) ((float)(currTime - startFile) / fileDone * (fileLen - fileDone));

			percentageBatch = (float)(batchDone / batchLen * 100);
			batchEta = (int) ((float)(currTime - startBatch) / batchDone * (batchLen - batchDone));

			fprintf (textout, "Status:   %4.1f%% done, ETA %02d:%02d:%02d          BATCH: %4.1f%% done, ETA %02d:%02d:%02d\r",
					 percentageFile, fileEta/3600, (fileEta/60)%60, fileEta%60,
					 percentageBatch, batchEta/3600, (batchEta/60)%60, batchEta%60);
			fflush (textout);
			break;

		case PROG_FILE_GRAPH:

			strcpy (temp, "File progress: [..................................................]\r");
			x = (int) (fileDone * 50 / fileLen);
			memset (temp + 16, '*', x);
			if (x < 50)
			{
				temp[16+x] = wheel[wheelindex];
				wheelindex = (wheelindex + 1) % 4;
			}

			fprintf (textout, temp);
			fflush (textout);
			break;

		case PROG_BATCH_GRAPH:

			strcpy (temp, "Batch progress: [..................................................]\r");
			x = (int) (batchDone * 50 / batchLen);
			memset (temp + 16, '*', x);
			if (x < 50)
			{
				temp[16+x] = wheel[wheelindex];
				wheelindex = (wheelindex + 1) % 4;
			}

			fprintf (textout, temp);
			fflush (textout);
			break;

		case PROG_BOTH_GRAPH:

			strcpy (temp, "File: [.........................]   Batch: [.........................]\r");

			x = (int) (fileDone * 25 / fileLen);
			memset (temp + 7, '*', x);

			y = (int) (batchDone * 25 / batchLen);
			memset (temp + 44, '*', y);
			if (y < 25)
			{
				temp[7+x] = temp[44+y] = wheel[wheelindex];
				wheelindex = (wheelindex + 1) % 4;
			}

			fprintf (textout, temp);
			fflush (textout);
			break;

		case PROG_BOTH_SAMPLES:
			
			fprintf (textout, "Samples encoded:%10d / %d      BATCH:%10d / %d\r", 
					 (int) fileDone, (int) fileLen, (int) batchDone, (int) batchLen);
			break;

		case PROG_BOTH_ETA:
			fileEta = (int) (((float)(currTime - startFile)) / fileDone * (fileLen - fileDone));

			batchEta = (int) ((float)(currTime - startBatch) / batchDone * (batchLen - batchDone));

			fprintf (textout, "                >>> %02d:%02d:%02d <<<                >>> %02d:%02d:%02d <<<\r",
					 fileEta/3600, (fileEta/60)%60, fileEta%60,
					 batchEta/3600, (batchEta/60)%60, batchEta%60);
			fflush (textout);
			break;


		case PROG_FILE_RLEN:

			x = (int) (fileDone / psJob->sCodec.frequency);
			fprintf (textout, "Encoded runlength (current file): %02d:%02d:%02d\r",
			         x/3600, (x/60)%60, x%60);
			fflush (textout);
			break;

		case PROG_FILE_NORETURN:

			x = (int) (fileDone * 70 / fileLen);						
			if (prevFileDone > x  ||  prevFileDone == -1)
			{
				prevFileDone = 0;
				fprintf (textout, " |----------------------------------------------------------------------|\n |");
				fflush (textout);
			}
			for (i = prevFileDone;  i < x;  i++)
			{
				fprintf (textout, "#");
				fflush (textout);
			}

			if (x == 70)
				fprintf (textout, "|\n\n");
			prevFileDone = x;
			break;
	}
}





/*____ setOutputDir() _______________________________________________________*/

void					setOutputDir (char *pPath)
{
	int						i;

	strcpy (outputDir, pPath);
	i = strlen (outputDir) -1;
	if (outputDir[i] != DIRECTORY_SEPARATOR)
	{
		outputDir[i+1] = DIRECTORY_SEPARATOR;
		outputDir[i+2] = 0;
	}
}





/*____ readGlobalSwitches() ___________________________________________________*/

#define		option_is(s)			(!strcmp (arg+1, s))
#define		option_starts_with(s)	(strstr (arg+1, s) == arg+1)
#define		option_cont_with(s,n)	(strstr (arg+n, s) == arg+n)

argLink					*readGlobalSwitches (argLink *pArgLink)
{
	char					arg[256];
	int						x, y;

	for ( ;  pArgLink != NULL;  pArgLink = pArgLink->psNext)
	{
		strcpy (arg, pArgLink->pString);
		mystrupr (arg);
		if (arg[0] != '-')
			return	pArgLink;

		if (option_is("SORT"))
			wantedSort = TRUE;

		else if (option_is("MONO") || option_is("DM"))
			wantedInputType = DOWNMIX_MONO;
		else if (option_is("LEFTMONO") || option_is("LM"))
			wantedInputType = LEFT_CHANNEL_MONO;
		else if (option_is("RIGHTMONO") || option_is("RM"))
			wantedInputType = RIGHT_CHANNEL_MONO;
		else if (option_is("SWAP"))
			wantedInputType = INVERSE_STEREO;

		else if (option_is("CRC"))
			wantedCRC = TRUE;
		else if (option_is("PRIVATE") || option_is("P"))
			wantedPrivate = TRUE;
		else if (option_is("COPYRIGHT") || option_is("C"))
			wantedCopyright = TRUE;
		else if (option_is("ORIGINAL"))
			wantedOriginal = TRUE;
		else if (option_is("COPY"))
			wantedOriginal = FALSE;

		else if (option_is("DELETE") || option_is("DEL"))
			wantedDeleteSource = TRUE;

		else if (option_is("NOGAP"))
			wantedNoGap = TRUE;
		else if (option_is("CONCAT"))
        {
			wantedConcat = TRUE;
            wantedNoGap = TRUE;
        }
		else if (option_is("QUIT") || option_is("Q"))
			wantedQuit = TRUE;

		else if (option_is("QUIET"))
			wantedQuiet = TRUE;
		else if (option_is("NOCFG"))
			;   /* simply do nothing... */
		else if (option_starts_with ("PROGRESS="))
			progType = atoi (arg+10);
		else if (option_starts_with ("OUTDIR="))
			setOutputDir (pArgLink->pString + 8);
		else if (option_starts_with ("REFRESH="))
		{
			/* Legacy switch, not supported anymore... */
		}
#ifdef  PRIO
		else if (option_starts_with ("PRIO="))
		{
			strcpy (prioString, arg+6);
			pPrioString = prioString;
		}
#endif
		else if (option_is("RAWMONO"))
			rawChannels = 1;
		else if (option_is("RAWSTEREO"))
			rawChannels = 2;
		else if (option_is("RAWSIGNED"))
			rawSigned = TRUE;
		else if (option_is("RAWUNSIGNED"))
			rawSigned = FALSE;
		else if (option_starts_with ("RAWBITS="))
		{
			rawBits = atoi (arg+9);
			if (rawBits != 8  &&  rawBits != 16)
				rawBits = 16;
		}
		else if (option_starts_with ("RAWFREQ="))
		{
			rawFrequency = atoi (arg+9);
		}
		else if (option_starts_with ("RAWBYTEORDER="))
		{
			if (option_cont_with ("LITTLE", 14))
				rawByteorder = LITTLE_ENDIAN;
			if (option_cont_with ("BIG", 14))
				rawByteorder = BIG_ENDIAN;
			if (option_cont_with ("DEFAULT", 14))
				rawByteorder = BYTEORDER;
		}
		else if (option_starts_with ("RAWCHANNELS="))
		{
			rawChannels = atoi (arg+13);
			if (rawChannels != 1  &&  rawChannels != 2)
				rawChannels = 2;
		}
		else if (option_is("BR"))
		{
			pArgLink = pArgLink->psNext;
			if (pArgLink == NULL)
				return	NULL;
			wantedBitrate = atoi (pArgLink->pString);
			if (wantedBitrate > 1000)
				wantedBitrate /= 1000;
		}
		else
		{
			y = 0;
			for (x = 1;  arg[x] >= '0'  &&  arg[x] <= '9';  x++)
				y = y * 10 + (arg[x] - '0');
			if (arg[x] == 0)
			{
				wantedBitrate = y;
				if (wantedBitrate > 1000)
					wantedBitrate /= 1000;
			}
			else
				return pArgLink;
		}
	}

	return pArgLink;
}





/*____ readLocalSwitches() ___________________________________________________*/

argLink					*readLocalSwitches (argLink *pArgLink, Job *psJob)
{
	char					arg[256];
	int						x, y;

	for ( ; pArgLink != NULL;  pArgLink = pArgLink->psNext)
	{
		strcpy (arg, pArgLink->pString);
		mystrupr (arg);

		if (arg[0] != '-')
			return	pArgLink;

		if (option_is("MONO") || option_is("DM"))
		{
			psJob->sInput.outputType = DOWNMIX_MONO;
			psJob->sCodec.mode = 3;
		}
		else if (option_is("LEFTMONO") || option_is("LM"))
		{
			if (psJob->sInput.nChannels >= 2)
			{
				psJob->sInput.outputType = LEFT_CHANNEL_MONO;
				psJob->sCodec.mode = 3;
			}
		}
		else if (option_is("RIGHTMONO") || option_is("RM"))
		{
			if (psJob->sInput.nChannels >= 2)
			{
				psJob->sInput.outputType = RIGHT_CHANNEL_MONO;
				psJob->sCodec.mode = 3;
			}
		}
		else if (option_is("SWAP"))
		{
			if (psJob->sInput.nChannels >= 2)
			{
				psJob->sInput.outputType = INVERSE_STEREO;
				psJob->sCodec.mode = 0;
			}
		}

		else if (option_is("CRC"))
			psJob->sCodec.fCRC = TRUE;
		else if (option_is("PRIVATE") || option_is("P"))
			psJob->sCodec.fPrivate = TRUE;
		else if (option_is("COPYRIGHT") || option_is("C"))
			psJob->sCodec.fCopyright = TRUE;
		else if (option_is("ORIGINAL"))
			psJob->sCodec.fOriginal = TRUE;
		else if (option_is("COPY"))
			psJob->sCodec.fOriginal = FALSE;

		else if (option_is("DELETE") || option_is("DEL"))
			psJob->bDeleteSource = TRUE;

		else if (option_is("NOGAP"))
			psJob->bNoGap = TRUE;
		else if (option_is("CONCAT"))
        {
			psJob->bConcat = TRUE;
            psJob->bNoGap = TRUE;
        }

		else if (option_is("RAWMONO"))
		{
			if (psJob->sInput.filetype == RAW)
				psJob->sInput.nChannels = 1;
		}
		else if (option_is("RAWSTEREO"))
		{
			if (psJob->sInput.filetype == RAW)
				psJob->sInput.nChannels = 2;
		}
		else if (option_is("RAWSIGNED"))
		{
			if (psJob->sInput.filetype == RAW)
				psJob->sInput.fSign = TRUE;

		}
		else if (option_is("RAWUNSIGNED"))
		{
			if (psJob->sInput.filetype == RAW)
				psJob->sInput.fSign = FALSE;

		}
		else if (option_starts_with ("RAWBITS="))
		{
			if (psJob->sInput.filetype == RAW)
			{
				psJob->sInput.channelBits = atoi (arg+9);
				if (psJob->sInput.channelBits != 8 && psJob->sInput.channelBits != 16)
					psJob->sInput.channelBits = 16;
				psJob->sInput.sampleBits = psJob->sInput.channelBits * psJob->sInput.nChannels;
			}
		}
		else if (option_starts_with ("RAWFREQ="))
		{
			if (psJob->sInput.filetype == RAW)
				psJob->sInput.freq = atoi (arg+9);
		}
		else if (option_starts_with ("RAWBYTEORDER="))
		{
			if (psJob->sInput.filetype == RAW)
			{
				if (option_cont_with ("LITTLE", 14))
					psJob->sInput.byteorder = LITTLE_ENDIAN;
				if (option_cont_with ("BIG", 14))
					psJob->sInput.byteorder = BIG_ENDIAN;
				if (option_cont_with ("DEFAULT", 14))
					psJob->sInput.byteorder = BYTEORDER;
			}
		}
		else if (option_starts_with ("RAWCHANNELS="))
		{
			if (psJob->sInput.filetype == RAW)
			{
				psJob->sInput.nChannels = atoi (arg+13)-1;
				if (psJob->sInput.nChannels != 1 && psJob->sInput.nChannels != 2)
					psJob->sInput.nChannels = 2;
			}
		}

		else if (option_is("BR"))
		{
			pArgLink = pArgLink->psNext;
			if (pArgLink == NULL)
				return	pArgLink;
			psJob->sCodec.bitrate = atoi (pArgLink->pString);
			if (psJob->sCodec.bitrate > 1000)
			{
				psJob->sCodec.bitrate /= 1000;
				wantedQuit = TRUE;
			}
		}
		else if (option_is("HQ"))
		{
			wantedQuit = TRUE;   /* Dummy for l3enc support */
		}
		else
		{
			y = 0;
			for (x = 1 ; arg[x] >= '0' && arg[x] <= '9' ; x++)
				y = y * 10 + (arg[x] - '0');
			if (arg[x] == 0)
			{
				psJob->sCodec.bitrate = y;
				if (psJob->sCodec.bitrate > 1000)
					psJob->sCodec.bitrate /= 1000;
			}
			else
				return pArgLink;
		}
	}

	return pArgLink;
}





/*____ sortJobs() ___________________________________________________________*/

void					sortJobs (Job **ppsJob)
{
	Job						*psJob = *ppsJob;
	Job						*psJobSorted = NULL;
	Job						**ppsLink;
	Job						*psTemp;

	while (psJob != NULL)
	{
		for (ppsLink = &psJobSorted;  *ppsLink != NULL;  ppsLink = &(*ppsLink)->psNext)
			if (strcmp ((*ppsLink)->sourceFilename, psJob->sourceFilename) > 0)
				break;

		psTemp = psJob;
		psJob = psJob->psNext;
		psTemp->psNext = *ppsLink;
		*ppsLink = psTemp;
	}
	*ppsJob = psJobSorted;
}





/*____ validateJobs() _______________________________________________________*/

int						validateJobs (Job * psJob)
{
#if 0
	static	int				aValidBitrates[14] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
#else
	int						*aValidBitrates = &bitratex[1][1];
#endif

	int						i;
	int						fOk = TRUE;
	int						badBitrate = -1;

	while (psJob != NULL  &&  fOk)
	{
		fOk = FALSE;
		for (i = 0;  i < 14;  i++)
			if (psJob->sCodec.bitrate == aValidBitrates[i])
				fOk = TRUE;

		if (fOk == FALSE  &&  badBitrate == -1)
			badBitrate = psJob->sCodec.bitrate;

		psJob = psJob->psNext;
	}

	if (fOk)
		return TRUE;

	fprintf (textout, "ERROR: %d is not a valid bitrate!\n\n", badBitrate);

	fprintf (textout, "Valid bitrates are:\n\n");

	for (i = 0;  i < 13;  i++)
		fprintf (textout, "%d, ", aValidBitrates[i]);

	fprintf (textout, "and %d kBit.\n", aValidBitrates[i]);

	return FALSE;
}





/*____ printUsage() ___________________________________________________________*/

int     printUsage (void)
{
	fprintf (textout, "Usage: bladeenc [global switches] input1 [output1 [switches]] input2 ...\n");
	fprintf (textout, "\n");
	fprintf (textout, "General switches:\n");
	fprintf (textout, "  -[kbit], -br [kbit]  Set MP3 bitrate. Default is 128 (64 for mono output).\n");
	fprintf (textout, "  -crc                 Include checksum data in MP3 file.\n");
	fprintf (textout, "  -delete, -del        Delete sample after successful encoding.\n");
	fprintf (textout, "  -private, -p         Set the private-flag in the output file.\n");
	fprintf (textout, "  -copyright, -c       Set the copyright-flag in the output file.\n");
	fprintf (textout, "  -copy                Clears the original-flag in the output file.\n");
	fprintf (textout, "  -mono, -dm           Produce mono MP3 files by combining stereo channels.\n");
	fprintf (textout, "  -leftmono, -lm       Produce mono MP3 files from left stereo channel only.\n");
	fprintf (textout, "  -rightmono, -rm      Produce mono MP3 files from right stereo channel only.\n");
	fprintf (textout, "  -swap                Swap left and right stereo channels.\n");
	fprintf (textout, "  -rawfreq=[freq]      Specify frequency for RAW samples. Default is 44100.\n");
	fprintf (textout, "  -rawbits=[bits]      Specify bits per channel for RAW samples. Default is 16.\n");
	fprintf (textout, "  -rawmono             Specifies that RAW samples are in mono, not stereo.\n");
    fprintf (textout, "  -rawstereo           Specifies that RAW samples are in stereo (default).\n");
	fprintf (textout, "  -rawsigned           Specifies that RAW samples are signed (default).\n");
	fprintf (textout, "  -rawunsigned         Specifies that RAW samples are unsigned.\n");
#ifdef	PAUSE_25_LINES
	fprintf (textout, "Press ENTER to continue...");
	fflush (textout);
	be_getch();
	fprintf (textout, "\n");
#endif
	fprintf (textout, "  -rawbyteorder=[order]Specifies byteorder for RAW samples, LITTLE or BIG.\n");
	fprintf (textout, "  -rawchannels=[1/2]   Specifies number of channels for RAW samples. Does\n");
	fprintf (textout, "                       the same as -rawmono and -rawstereo respectively.\n");
	fprintf (textout, "  -nogap               Make sure there's no gap between this and next track.\n");
    fprintf (textout, "  -concat              Concatenate this and next track (also enables -nogap).\n");
	fprintf (textout, "\n");
	fprintf (textout, "Global only switches:\n");
	fprintf (textout, "  -quit, -q            Quit without waiting for keypress when finished.\n");
	fprintf (textout, "  -outdir=[dir]        Save MP3 files in specified directory.\n");
	fprintf (textout, "  -quiet               Disable screen output.\n");
	fprintf (textout, "  -nocfg               Don't take settings from the config-file.\n");
#ifdef  PRIO
	fprintf (textout, "  -prio=[prio]         Sets the task priority for BladeEnc. Valid settings are\n");
	fprintf (textout, "                       HIGHEST, HIGHER, NORMAL, LOWER, LOWEST(default) and IDLE\n");
#endif
	fprintf (textout, "  -progress=[0-8]      Which progress indicator to use. 0=Off, 1=Default.\n");
	fprintf (textout, "  -sort                Sort jobs on input name. Useful for -nogap on '*'.\n");
	fprintf (textout, "\n");
	fprintf (textout, "Input/output files can be replaced with STDIN and STDOUT respectively.\n");
#ifdef  DRAG_DROP
	fprintf (textout, "To make a normal 128kBit MP3, just drag-n-drop your WAV onto the BladeEnc icon.\n");
#endif
	fprintf (textout, "\n");

	return  TRUE;
}





/*____ addCommandlineJob() ____________________________________________________*/

int						addCommandlineJob (argLink **ppArgLink)
{
	char					temp[256];
	Job						*psOp, *psTemp;
	int						x;
	argLink					*pArgLink;

	pArgLink = *ppArgLink;


	psOp = (Job *) malloc (sizeof(Job));
	psOp->psNext = NULL;


	/* Open Input File              */

	strcpy (temp, pArgLink->pString);
	mystrupr (temp);
	if (strcmp (temp, "STDIN") == 0)
	{
	if (!fPreparedSTDIN)
	{
	prepStdin();
	fPreparedSTDIN = TRUE;
	}
	strcpy (psOp->sourceFilename, "Standard input stream");
	x = openInput (&psOp->sInput, NULL);
	}
	else
	{
	strcpy (psOp->sourceFilename, pArgLink->pString);
	x = openInput (&psOp->sInput, pArgLink->pString);
	}

	if (x != TRUE)
	{
		switch (psOp->sInput.errcode)
		{
		case	-1:
			fprintf (textout, "ERROR: '%s' is not a WAV or AIFF file!\n", psOp->sourceFilename);
			break;
		case	-2:
			fprintf (textout, "ERROR: Couldn't open '%s'!\n", psOp->sourceFilename);
			break;
		case	-3:
			fprintf (textout, "ERROR: Unexpected end of file '%s'!\n", psOp->sourceFilename);
			break;
		case	-5:
			fprintf (textout, "ERROR: Necessary chunk missing in '%s'!\n", psOp->sourceFilename);
			break;
		case	-6:
			fprintf (textout, "ERROR: Sample '%s' is of an unknown subtype!\n", psOp->sourceFilename);
			break;
		default:
			fprintf (textout, "ERROR: Unknown error while opening '%s'!\n", psOp->sourceFilename);

		}
		free (psOp);
		return  FALSE;
	}

	/* If RAW, set default values */

	if (psOp->sInput.filetype == RAW)
	{
		psOp->sInput.freq = rawFrequency;
		psOp->sInput.outputFreq = rawFrequency;
		psOp->sInput.nChannels = rawChannels;
		psOp->sInput.channelBits = rawBits;
		psOp->sInput.sampleBits = rawBits * rawChannels;
		psOp->sInput.fSign = rawSigned;
		psOp->sInput.byteorder = rawByteorder;
	}

	/*		*/

	if (psOp->sInput.nChannels == 1 && (wantedInputType == STEREO || wantedInputType == INVERSE_STEREO))
		psOp->sInput.outputType = DOWNMIX_MONO;
	else
		psOp->sInput.outputType = wantedInputType;


	/* Set sCodec.mode (MONO or STEREO) */

	if (psOp->sInput.outputType == DOWNMIX_MONO || psOp->sInput.outputType == LEFT_CHANNEL_MONO
	        || psOp->sInput.outputType == RIGHT_CHANNEL_MONO)
		psOp->sCodec.mode = 3;                                                   /* Force to mono... */
	else
	{
		psOp->sCodec.mode = 0;
	}

	/* Set frequency */

	if (psOp->sInput.freq != 44100 && psOp->sInput.freq != 48000
	        && psOp->sInput.freq != 32000)
	{
		fprintf (textout, "ERROR: Sample '%s' is not in 32, 44.1 or 48 kHz!\n", psOp->sourceFilename);
		closeInput (&(psOp->sInput));
		free (psOp);
		return  FALSE;
	}

	psOp->sCodec.frequency = psOp->sInput.freq;


	/* Set other parameters */

	psOp->sCodec.bitrate = wantedBitrate;

	psOp->sCodec.fPrivate = wantedPrivate;
	psOp->sCodec.fCRC = wantedCRC;
	psOp->sCodec.fCopyright = wantedCopyright;
	psOp->sCodec.fOriginal = wantedOriginal;
	psOp->bDeleteSource = wantedDeleteSource;
    psOp->bNoGap = wantedNoGap;
    psOp->bConcat = wantedConcat;

	/* Set unsupported parameters */

	psOp->sCodec.emphasis = 0;

	pArgLink = pArgLink->psNext;

	/* Check for output specification and set output name */

	psOp->outputFilename[0] = 0;
	if (pArgLink != NULL)
	{
		strcpy (temp, pArgLink->pString);
		mystrupr (temp);
		if (!strcmp (temp, "STDOUT"))
		{
			wantedSTDOUT = TRUE;
			strcpy (psOp->outputFilename, "STDOUT");
			pArgLink = pArgLink->psNext;
		}
		else if (strlen (temp) >= 4 && !strcmp (temp+strlen(temp)-4, ".MP3"))
		{
			strcpy (psOp->outputFilename, pArgLink->pString);
			pArgLink = pArgLink->psNext;
		}
	}

	/* Generate output name if not allready set */

	if (psOp->outputFilename[0] == 0)
	{
		if (outputDir[0] != 0)
		{
			strcpy (psOp->outputFilename, outputDir);

			strcpy (temp, psOp->sourceFilename);
			x = strlen (temp);
			while (temp[x] != '.' && x >=0 && temp[x] != DIRECTORY_SEPARATOR)
				x--;

			if (temp[x] == DIRECTORY_SEPARATOR)
				x = strlen(temp);

			if (x >= 0)
				strcpy (temp + x, ".mp3");
			else
			{
				x = strlen (temp);
				strcat (temp, ".mp3");
			}

			while (x >= 0 && temp[x] != '\\' && temp[x] != '/' && temp[x] != ':')
				x--;
			x++;

			strcat (psOp->outputFilename, temp + x);
		}
		else
		{
			strcpy (temp, psOp->sourceFilename);
			x = strlen (temp);
			while (temp[x] != '.' && x >=0 && temp[x] != DIRECTORY_SEPARATOR)
				x--;

			if (temp[x] == DIRECTORY_SEPARATOR)
				x = strlen(temp);

			if (x >= 0)
				strcpy (temp + x, ".mp3");
			else
				strcat (temp, ".mp3");

			strcpy (psOp->outputFilename, temp);
		}
	}


	/* Read local switches */

	pArgLink = readLocalSwitches (pArgLink, psOp);


	/* Set default bitrate if not allready defined */

	if (psOp->sCodec.bitrate == -1)
	{
		if (psOp->sCodec.mode == 3)
			psOp->sCodec.bitrate = 64;
		else
			psOp->sCodec.bitrate = 128;
	}


	/* Put this Job in the batch */

	if (psJobQueue == NULL)
		psJobQueue = psOp;
	else
	{
		psTemp = psJobQueue;
		while (psTemp->psNext != NULL)
			psTemp = psTemp->psNext;
		psTemp->psNext = psOp;
	}

	* ppArgLink = pArgLink;
	return  TRUE;
}





/*____ clearJobQueue() ________________________________________________________*/

int						clearJobQueue (void)
{
	while (psJobQueue != NULL)
		removeJobQueueEntry (psJobQueue);

	return TRUE;
}





/*____ removeQueueEntry() _____________________________________________________*/

int						removeJobQueueEntry (Job *psJob)
{
	Job						*psPrev;

	/* Unlink specified entry */

	if (psJob == psJobQueue)
		psJobQueue = psJob->psNext;
	else
	{
		psPrev = psJobQueue;
		while (psPrev->psNext != psJob  &&  psPrev->psNext != NULL)
			psPrev = psPrev->psNext;

		if (psPrev->psNext == NULL)
			return FALSE;

		psPrev->psNext = psJob->psNext;
	}

	/* Close open file, free the entry and return. */

	closeInput (&psJob->sInput);
	free (psJob);

	return TRUE;
}
#endif


