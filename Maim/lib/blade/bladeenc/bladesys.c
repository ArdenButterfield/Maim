/*
			(c) Copyright 1998-2000 - Tord Jansson
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

	2000-12-15  Andre Piotrowski

	-	reformatted, slightly optimized

	2001-01-19	Tord Jansson

	-	Added Andrea Vallinotto's Amiga WarpOS changes.
*/

/******************************************************************************
							>>> BLADESYS <<<

The intention of this file is to keep all system specific things away from the
rest of the code.

******************************************************************************/



#include		<stdio.h>
#include		<stdlib.h>
#include		<string.h>
#include		<time.h>
#include		"system.h"

#if SYSTEM != AMIGA_WOS	
#	include		<fcntl.h>		/* Needed by Windows (and maybe others) but doesn't exist on Amiga WarpOS */
#endif


#ifdef  MSWIN
#	include		<windows.h>
#	include		<conio.h>
#	include		<io.h>
#endif


#ifdef OS2
#	define		INCL_DOSFILEMGR
#	define		INCL_DOSERRORS
#	define		INCL_DOSPROCESS
#	define		INCL_KBD
#	include		<os2.h>
#	include		<conio.h>
#endif

#if SYSTEM == AMIGA_WOS /* Using VBCC! */

#	ifdef __VBCC__
#		pragma amiga-align
#	endif

#	include <proto/dos.h> /* Unbelivable: it works! :-) */
#	include <clib/powerpc_protos.h>
#	include <proto/exec.h> /* I'll ask for a FindTask(NULL) */

#	ifdef __VBCC__
#	pragma default-align
#	endif

#	ifndef	PRIO_MAX
#		define	PRIO_MAX	20
#		define	PRIO_MIN	-20
#	endif
	static const char verstring[]="$VER: BladeEnc 1.3 (03.03.01) by Tord Jansson (BladeEnc 0.93.11), Amiga port by Andrea Vallinotto";
#endif /* AMIGA_WOS */

#ifdef UNIX_SYSTEM
#	include		<pwd.h>
#	include		<sys/time.h>
#	include		<sys/resource.h>
#	include		<unistd.h>
#	include		<sys/types.h>

#	ifndef PRIO_MAX
#		define		PRIO_MAX	 20
#		define		PRIO_MIN	-20
#	endif
#endif

#if	SYSTEM == MAC_OS
#	include		"EventHandling.h"
#endif


#include	"codec.h"
#include	"samplein.h"
#include	"arglink.h"
#include	"bladesys.h"




/******************************************************************************

				>>> SYSTEM FOR PRIORITY HANDLING <<<


******************************************************************************/






#ifdef OS2

typedef		struct
			{
				ULONG					ulClass;
				LONG					loDelta;
			}						OS2PRIORITIES;

OS2PRIORITIES			OS2PrioTab[] =
			{
			    {PRTYC_IDLETIME    ,   0},
			    {PRTYC_IDLETIME    ,  31},
			    {PRTYC_REGULAR     , -31},
			    {PRTYC_REGULAR     ,   0},
			    {PRTYC_REGULAR     ,  31},
			    {PRTYC_TIMECRITICAL,   0}
			};

#endif





int						setPriority
(
	char					*pPrioString
)
{

#ifdef  MSWIN
	HANDLE					hThread;

	hThread = GetCurrentThread();

	     if (        pPrioString == NULL         )  SetThreadPriority (hThread, THREAD_PRIORITY_LOWEST);   /* Set default priority if NULL! */
	else if (strcmp (pPrioString, "HIGHEST") == 0)  SetThreadPriority (hThread, THREAD_PRIORITY_HIGHEST);
	else if (strcmp (pPrioString, "HIGHER" ) == 0)  SetThreadPriority (hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	else if (strcmp (pPrioString, "NORMAL" ) == 0)  SetThreadPriority (hThread, THREAD_PRIORITY_NORMAL);
	else if (strcmp (pPrioString, "LOWER"  ) == 0)  SetThreadPriority (hThread, THREAD_PRIORITY_BELOW_NORMAL);
	else if (strcmp (pPrioString, "LOWEST" ) == 0)  SetThreadPriority (hThread, THREAD_PRIORITY_LOWEST);
	else if (strcmp (pPrioString, "IDLE"   ) == 0)  SetThreadPriority (hThread, THREAD_PRIORITY_IDLE);
	else return FALSE;
#endif


#ifdef OS2
	APIRET					rc;
	int						prio;

	     if (        pPrioString == NULL         )  prio = 1;   /* Set default priority if NULL! */
	else if (strcmp (pPrioString, "HIGHEST") == 0)  prio = 5;
	else if (strcmp (pPrioString, "HIGHER" ) == 0)  prio = 4;
	else if (strcmp (pPrioString, "NORMAL" ) == 0)  prio = 3;
	else if (strcmp (pPrioString, "LOWER"  ) == 0)  prio = 2;
	else if (strcmp (pPrioString, "LOWEST" ) == 0)  prio = 1;
	else if (strcmp (pPrioString, "IDLE"   ) == 0)  prio = 0;
	else return FALSE;

	if ((rc = DosSetPriority (PRTYS_PROCESS, OS2PrioTab[prio].ulClass, OS2PrioTab[prio].loDelta, 0L)) != NO_ERROR)
	{
		printf (" DosSetPriority error : rc = %lu\n", rc);
		exit (1);
	}
#endif


#ifdef	UNIX_SYSTEM
	pid_t					my_pid;
	int						prio;

	     if (        pPrioString == NULL         )  return TRUE;   	/* Keep priority executed with if NULL! */
	else if (strcmp (pPrioString, "HIGHEST") == 0)  prio =      PRIO_MIN;
	else if (strcmp (pPrioString, "HIGHER" ) == 0)  prio = (2 * PRIO_MIN +     PRIO_MAX) / 3;
	else if (strcmp (pPrioString, "NORMAL" ) == 0)  prio = (    PRIO_MIN +     PRIO_MAX) / 2;
	else if (strcmp (pPrioString, "LOWER"  ) == 0)  prio = (    PRIO_MIN + 2 * PRIO_MAX) / 3;
	else if (strcmp (pPrioString, "LOWEST" ) == 0)  prio = (    PRIO_MIN + 3 * PRIO_MAX) / 4;
	else if (strcmp (pPrioString, "IDLE"   ) == 0)  prio =                     PRIO_MAX;
	else
	{
		prio = atoi (pPrioString);
		if (prio < PRIO_MIN)  prio = PRIO_MIN;
		if (prio > PRIO_MAX)  prio = PRIO_MAX;
	}
	my_pid = getpid();
	setpriority (PRIO_PROCESS, my_pid, prio);
#endif

#if SYSTEM == AMIGA_WOS /* WarpOS. */

	LONG			prio=0;

	if( pPrioString == NULL ) return TRUE;   	/* Keep priority executed with if NULL! */
	else if( strcmp( pPrioString, "HIGHEST" ) == 0 )
		prio = 20;
	else if( strcmp( pPrioString, "HIGHER" ) == 0 )
		prio = 5;
	else if( strcmp( pPrioString, "NORMAL" ) == 0 )
		prio = 0;
	else if( strcmp( pPrioString, "LOWER" ) == 0 )
		prio = -1;
	else if( strcmp( pPrioString, "LOWEST" ) == 0 )
		prio = -5;
	else if( strcmp( pPrioString, "IDLE" ) == 0 )
		prio = -20;
	else sscanf(pPrioString, "%ld", &prio);

	SetTaskPriPPC(FindTaskPPC(NULL), prio);

#endif

	/* Include Prioritysettings for other systems here... */

	return TRUE;
}





/* Some systems use text-input/output by default instead of binary.:- (*/

void prepStdin( void )
{
#ifdef MSWIN
#	ifdef __BORLANDC__
		setmode(_fileno(stdin), O_BINARY );
#	else
		_setmode(_fileno(stdin), _O_BINARY );
#	endif
#endif
}
void prepStdout( void )
{
#ifdef MSWIN
#	ifdef __BORLANDC__
		setmode(_fileno(stdout), O_BINARY );
#	else
		_setmode(_fileno(stdout), _O_BINARY );
#	endif
#endif
}




/******************************************************************************

											>>> ROUTINES FOR KEYHANDLING <<<

******************************************************************************/



int						be_kbhit (void)
{

#ifdef MSWIN
	return kbhit();
#endif

#ifdef OS2
	KBDKEYINFOk;

	return KbdPeek(&k, NULLHANDLE) == 0  &&  (k.fbStatus & KBDTRF_FINAL_CHAR_IN);
#endif

#if !defined(MSWIN) && !defined(OS2)
	return 0;
#endif

}



int						be_getch (void)
{

#if defined(MSWIN) || defined(OS2)
	return getch();

#elif SYSTEM == AMIGA_WOS
		return getchar();
#else
	return 0;
#endif

}





/******************************************************************************

		>>> EXPAND WILDCARDS FOR SYSTEMS THAT DON'T DO IT AUTOMATICALLY <<<

******************************************************************************/

#ifdef WILDCARDS



static	int				findFirstMatch (char *pFileName, char *wpName);
static	int				findNextMatch (char *pFileName, char *wpName);




/*____ expandWildcards() ____________________________________________________*/

int						expandWildcards (argLink **ppArgLink)
{
	argLink					*pFirstNewLink = NULL;
	argLink					**wppPrev = &pFirstNewLink;

	argLink					*pArgLink = *ppArgLink;
	argLink					*pTmp;

	int						nNew = 0;
	int						x;
	char					temp[MAX_NAMELEN];

	while (pArgLink != NULL)
	{
#ifdef	MSWIN
		if (strchr (pArgLink->pString, '*') != NULL || strchr (pArgLink->pString, '?') != NULL || strchr (pArgLink->pString, '~') != NULL)
#else
		if (strchr (pArgLink->pString, '*') != NULL || strchr (pArgLink->pString, '?') != NULL)
#endif
		{
			x = findFirstMatch (pArgLink->pString, temp);
			while (x)
			{
				pTmp = insertStringInArgLink (wppPrev, temp);
				wppPrev = &pTmp->psNext;
				nNew++;

				x = findNextMatch (pArgLink->pString, temp);
			}
			pTmp = pArgLink;
			pArgLink = pArgLink->psNext;
			free (pTmp);
		}
		else
		{
			*wppPrev = pArgLink;
			wppPrev = &pArgLink->psNext;
			pArgLink = pArgLink->psNext;
		}		
	}

	*wppPrev = NULL;
	*ppArgLink = pFirstNewLink;

	return nNew;
}



#ifdef MSWIN

static	struct			_finddata_t sFind;
static	long			hFind;


/*____ findFirstMatch() - MSWIN version ______________________________________*/

static	int				findFirstMatch (char *pFileName, char *wpName)
{
	int						x;

	hFind = _findfirst (pFileName, &sFind);
	if (hFind == -1)
		return FALSE;

	if ((sFind.attrib & _A_SUBDIR) != 0)
		return findNextMatch (pFileName, wpName);


	strcpy (wpName, pFileName);
	for (x = strlen(wpName)-1;  wpName[x] != '\\' && wpName[x] !='/' && wpName[x] != ':' && x > 0;  x--)
	{}   /* avoid compiler warning */

	if (x != 0)
		x++;
	strcpy (wpName + x, sFind.name);

	return TRUE;
}


/*____ findNextMatch() - MSWIN version _______________________________________*/

static	int				findNextMatch (char *pFileName, char *wpName)
{
	int						x;

	while (1)
	{
		if (_findnext (hFind, &sFind) != 0)
			return FALSE;

		if ((sFind.attrib & _A_SUBDIR) == 0)
		{
			strcpy (wpName, pFileName);
			for (x = strlen(wpName)-1;  wpName[x] != '\\' && wpName[x] !='/' && wpName[x] != ':' && x > 0;  x--)
			{}   /* avoid compiler warning */

			if (x != 0)
				x++;
			strcpy (wpName + x, sFind.name);

			return TRUE;
		}
	}
}

#endif  /* MSWIN */



#ifdef OS2

static	HDIR			hdirFindHandle = HDIR_SYSTEM;
static	FILEFINDBUF3	FindBuffer     = {0};
static	ULONG			ulResultBufLen = sizeof(FILEFINDBUF3);
static	ULONG			ulFindCount    = 1;


/*____ findFirstMatch() - OS/2 version _______________________________________*/

static	int				findFirstMatch (char *pFileName, char *wpName)
{
	int						x;
	APIRET					rc = NO_ERROR;

	ulFindCount = 1;
	rc = DosFindFirst (pFileName, &hdirFindHandle, FILE_NORMAL, &FindBuffer, ulResultBufLen, &ulFindCount, FIL_STANDARD);

	if (rc != NO_ERROR)
		return FALSE;

	if ((FindBuffer.attrFile & FILE_DIRECTORY) != 0)
		return findNextMatch (pFileName, wpName);


	strcpy (wpName, pFileName);
	for (x = strlen(wpName)-1;  wpName[x] != '\\' && wpName[x] !='/' && wpName[x] != ':' && x > 0;  x--)
	{}   /* avoid compiler warning */

	if (x != 0)
		x++;
	strcpy (wpName + x, FindBuffer.achName);

	return TRUE;
}


/*____ findNextMatch() - OS/2 version ________________________________________*/

static	int				findNextMatch (char *pFileName, char *wpName)
{
	int						x;
	APIRET					rc = NO_ERROR;

	while (1)
	{
		ulFindCount = 1;

		if ((rc = DosFindNext(hdirFindHandle, &FindBuffer, ulResultBufLen, &ulFindCount)) != NO_ERROR)
		{
			if ((rc = DosFindClose(hdirFindHandle)) != NO_ERROR)
				printf ("DosFindClose error: return code = %lu\n", rc);

			return FALSE;
		}

		if ((FindBuffer.attrFile & FILE_DIRECTORY) == 0)
		{
			strcpy (wpName, pFileName);
			for (x = strlen(wpName)-1;  wpName[x] != '\\' && wpName[x] !='/' && wpName[x] != ':' && x > 0;  x--)
			{}   /* avoid compiler warning */

			if (x != 0)
				x++;
			strcpy (wpName + x, FindBuffer.achName);

			return TRUE;
		}
	}
}

#endif /* OS/2 */

#if SYSTEM == AMIGA_WOS /* WarpOS. */

#define PARSEBUFLEN (MAX_NAMELEN<<1)+2
static char parsebuf[PARSEBUFLEN];
static char pathpart[MAX_NAMELEN];
static char filepart[MAX_NAMELEN];
static struct FileInfoBlock * fib;
struct List filelist;
struct Node * currentnode;

static int findFirstMatch( char * pFileName, char * wpName )
{
static BOOL isfirst=TRUE;
BPTR dirlock;
struct Node * newnode;
/* char * onefname; */
char * point;
unsigned len;

/* printf("Called findFirstMatch: %s %s\n", pFileName, wpName); */

if(isfirst)
	{
	isfirst=FALSE;

	NewListPPC(&filelist);

	fib = AllocDosObject(DOS_FIB, NULL);

	strcpy(filepart, FilePart(pFileName));

	if((point=PathPart(pFileName)) == pFileName) /* Current directory */
		{
		dirlock=DupLock(((struct Process *)FindTask(NULL))->pr_CurrentDir);
		*pathpart=0;
		}
	else
		{
		len =(unsigned) (point-pFileName);
		strncpy(pathpart, pFileName, len);
		pathpart[len]=0;

		dirlock=Lock(pathpart, ACCESS_READ);
		}

	if(dirlock)
		{
		if(Examine(dirlock, fib))
			{
			while(ExNext(dirlock, fib))
				{
				if(fib->fib_DirEntryType == ST_FILE || fib->fib_DirEntryType == ST_LINKFILE)
					{
					/* printf("%s\n", fib->fib_FileName); */

					newnode=malloc(sizeof(struct Node));

					newnode->ln_Type = NT_USER;
					newnode->ln_Name=malloc(strlen(fib->fib_FileName)+1);
					strcpy(newnode->ln_Name, fib->fib_FileName);

					AddTailPPC(&filelist, newnode);

					}
				}
			}

		UnLock(dirlock);

		FreeDosObject(DOS_FIB, fib);
		}
	else	return FALSE;
	}

if(ParsePatternNoCase(filepart, parsebuf, PARSEBUFLEN))
	{
	for(currentnode = filelist.lh_Head; currentnode->ln_Succ; currentnode = currentnode->ln_Succ)
		{
		/* printf("Match %s %s\n", parsebuf, currentnode->ln_Name); */
		if(MatchPatternNoCase(parsebuf, currentnode->ln_Name))
			{
			/* printf("Trovato %s\n", currentnode->ln_Name); */
			strcpy(wpName, pathpart);
			AddPart(wpName, currentnode->ln_Name, MAX_NAMELEN);
			currentnode = currentnode->ln_Succ;
			return TRUE;
			}
		}
/*	return FALSE; */
	}
return FALSE;
}

static int findNextMatch( char * pFileName, char * wpName )
{
while(currentnode->ln_Succ)
	{
	/* printf("Trying with %s\n", currentnode->ln_Name); */
	if(MatchPatternNoCase(parsebuf, currentnode->ln_Name))
		{
		/* printf("Trovato %s\n", currentnode->ln_Name); */
		strcpy(wpName, pathpart);
		AddPart(wpName, currentnode->ln_Name, MAX_NAMELEN);
		currentnode = currentnode->ln_Succ;
		return TRUE;
		}
	currentnode = currentnode->ln_Succ;
	}
return FALSE;
}

#endif /* AMIGA_WOS */


#endif /* WILDCARDS */





/******************************************************************************

				>>> Routine for finding config file <<<

******************************************************************************/

/*____ findConfigFile() _____________________________________________________*/

int						findConfigFile (char *pExepath, char *wpPath)
{
	char					*pTmp;
	FILE					*fp;


#if			defined(MSWIN) || defined(OS2) || (SYSTEM == BEOS_X86) || (SYSTEM == BEOS_PPC) || (SYSTEM == MAC_OS)

#if			defined(MSWIN) || defined(OS2)
#	define		CONFIG_FILE		"bladeenc.cfg"
#elif		(SYSTEM == BEOS_X86) || (SYSTEM == BEOS_PPC)
#	define		CONFIG_FILE		"bladeenc.cfg"
#elif		(SYSTEM == MAC_OS)
#	define		CONFIG_FILE		MAC_BLADEENC_PREFSFILE
#endif

	strcpy (wpPath, pExepath);
	pTmp = wpPath + strlen (wpPath) - 1;

	while (pTmp >= wpPath  &&  *pTmp != DIRECTORY_SEPARATOR)
		pTmp--;

	/* now we have  pTmp == wpPath-1  or  *pTmp == DIRECTORY_SEPARATOR !!! */
	strcpy (pTmp+1, CONFIG_FILE);

	fp = fopen (wpPath, "r");
	if (fp != NULL)
	{
		fclose (fp);
		return TRUE;
	}
#endif


#ifdef	UNIX_SYSTEM
	pTmp = getenv ("HOME");
	if (pTmp != NULL)
	{
		strcpy (wpPath, pTmp);
		if (wpPath[strlen(wpPath)-1] != '/')
			strcat (wpPath, "/");

		strcat (wpPath, ".bladeencrc");

		fp = fopen (wpPath, "r");
		if (fp != NULL)
		{
			fclose (fp);
			return	TRUE;
		}
	}
#endif

#if SYSTEM == AMIGA_WOS /* WarpOS. */
	BPTR lockme;

	strcpy(wpPath, "PROGDIR:BladeEnc.cfg");
	lockme=Lock(wpPath, ACCESS_READ);

	if(lockme)
		{
		UnLock(lockme);
		return	TRUE;
		}
	else	{
		strcpy(wpPath, "ENV:BladeEnc.cfg");

		lockme=Lock(wpPath, ACCESS_READ);

		if(lockme)
			{
			UnLock(lockme);
			return TRUE;
			}
		}

#endif /* AMIGA_WOS */

	return FALSE;
}

