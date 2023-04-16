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

	Use TABSIZE 4!!!



	------------    Changes    ------------

	2000-12-11  Andre Piotrowski

	-	added autodetection for Mac OS

	2001-01-19	Tord Jansson

	-	Added Andrea Vallinotto's Amiga WarpOS changes.
	-	Added some more defines and cleaned up.

*/

#ifndef		__SYSTEM__
#define		__SYSTEM__



/*==== THE SYSTEMS WE KNOW OF AND CAN AUTODETECT ==========================*/

#define		UNIX_AUTOCONF			1   /* All well behaving UNIX systems where you run configure/make */
									    /* Includes Linux, *BSD, Solaris, IRIX, UnixWare, SCO Open Server, AIX, HPUX */

#define		WIN32_INTEL				2   /* Windows 95/98/NT/2k etc on i386 compatible machines */

#define		BEOS					3   /* BeOS, version 4.5+ on i386 and PPC */

#define		MSDOS_DJGPP				4   /* MS-DOS protected mode using DJGPP */

#define		MAC_OS					5   /* To compile the Mac version you also need Petteri Kamppuri's wrapper */



/*==== SYSTEMS WE SUPPORT BUT DON'T KNOW HOW TO AUTODETECT YET ==============*/
/*====       THESE NEEDS TO BE SET EXPLICITLY BELOW!!!         ==============*/


#define		WIN32_ALPHA				6   /* Windows NT on DEC/Compaq Alpha architectures */
            
#define		IBM_OS2					7

#define		ATARI_TOS				8

#define		AMIGA_WOS				9 	/* Amiga WarpOS */


/*= UNCOMMENT LINE AND SET SYSTEM HERE IF YOUR SYSTEM CAN'T BE AUTODETECTED! */
/*===========================================================================*/
/*#define		SYSTEM		MAC_OS*/

/*==== SYSTEM AUTODETECTION PROCEDURE ========================*/

#ifndef	SYSTEM
#	ifdef	WIN32
#		define		SYSTEM		WIN32_INTEL		
#	endif

#	ifdef  __BEOS__
#		define		SYSTEM		BEOS
#	endif

#	ifdef __DJGPP__
#       define  	SYSTEM      MSDOS_DJGPP
#	endif

#	if	defined(__dest_os) && (__dest_os == __mac_os)   /* defined in "ansi_prefix.mac.h" */
#		define		SYSTEM		MAC_OS
#	endif

#	ifndef	SYSTEM
#		define		SYSTEM		UNIX_AUTOCONF	/* Take UNIX for granted... */
#	endif

#endif



/*==== Low level defines ======================================================*/
 
/*
  LIST OF DEFINES
  ===============

  BYTEORDER	[byteorder]     Should either be set to BIG_ENDIAN or LITTLE_ENDIAN
							 depending on the processor.

  DRAG_DROP                 Set if Drag-n-Drop operations are supported. If
							 defined, the hint for drag and drop is displayed
							 in the help text.

  PRIO                      Set if priority can be set with the -PRIO switch
							 (UNIX_SYSTEM enables this automatically).

  MSWIN                     Set this for MSWin systems. Includes "windows.h" etc.

  WILDCARDS                 Set this if the program has to expand wildcards
							 itself on your system.

  DIRECTORY_SEPARATOR		Should either be '\\' or '/'.
							 (UNIX_SYSTEM automatically gives you '/').

  WAIT_KEY					Set this on systems where we as default want to
							 wait for a keypress before quiting.

  UNIX_SYSTEM				Set this for UNIX-systems that are posix and
							 (at least to some extent) bsd compliant to enable
							 stuff like config-file support, priority settings etc.

  PAUSE_25_LINES			Wait for keypress after 25 lines of output when
							 listing important information.
*/


/*  Most systems allready have these two defines, but some doesn't 
    so we have to put them here, before they are used. */

#ifndef BIG_ENDIAN
#	define			BIG_ENDIAN				4321
#endif

#ifndef LITTLE_ENDIAN
#	define			LITTLE_ENDIAN			1234
#endif


/*____ UNIX with Autoconf support _____________________________________________*/

#if		SYSTEM == UNIX_AUTOCONF
#	ifdef BYTE_ORDER
#		define			BYTEORDER				BYTE_ORDER
#	else
#		include <config.h>
#		if WORDS_BIGENDIAN
#			define 		BYTEORDER 				BIG_ENDIAN
#		else
#			define 		BYTEORDER 				LITTLE_ENDIAN
#		endif
#	endif
#	define			UNIX_SYSTEM
#endif

/*_____ Windows 95/98/NT Intel defines ________________________________________*/

#if     SYSTEM == WIN32_INTEL

#		define			BYTEORDER				LITTLE_ENDIAN
#		define			DRAG_DROP
#		define			PRIO
#		define			MSWIN
#		define			WILDCARDS
#		define			DIRECTORY_SEPARATOR		'\\'
#		define			WAIT_KEY
#		define			PAUSE_25_LINES
#endif

/*_____ Windows NT DEC Alpha defines __________________________________________*/

#if SYSTEM == WIN32_ALPHA

#		define			BYTEORDER				LITTLE_ENDIAN
#		define			DRAG_DROP
#		define			PRIO
#		define			MSWIN
#		define			WILDCARDS
#		define			DIRECTORY_SEPARATOR		'\\'
#		define			WAIT_KEY
#		define			PAUSE_25_LINES
#endif


/*____ OS/2 _________________________________________________________________*/

#if SYSTEM == IBM_OS2

#		define			BYTEORDER				LITTLE_ENDIAN
#		define			PRIO
#		define			WILDCARDS
#		define			DIRECTORY_SEPARATOR		'\\'
#		define			OS2
#		define			WAIT_KEY
#		define			PAUSE_25_LINES
#endif


/*____ Atari TOS ____________________________________________________________*/

#if SYSTEM == ATARI_TOS

#		define			BYTEORDER				BIG_ENDIAN
#		define			DIRECTORY_SEPARATOR		'\\'
#		define			TOS
#		define			PAUSE_25_LINES
#endif

/*____ MSDOS_DJGPP __________________________________________________________*/

#if SYSTEM == MSDOS_DJGPP

#		define          BYTEORDER				LITTLE_ENDIAN
#		define			DIRECTORY_SEPARATOR		'\\'
#endif

/*____ MAC_OS ________________________________________________________________*/

#if SYSTEM == MAC_OS

#		define			BYTEORDER				BIG_ENDIAN
#		define			DIRECTORY_SEPARATOR		':'
#endif

/*____ BeOS PowerPC & x86 ___________________________________________________*/

#if SYSTEM == BEOS

#	ifdef __i386__
#		define 			BYTEORDER				LITTLE_ENDIAN
#	else
#		define 			BYTEORDER				BIG_ENDIAN
#	endif
#		define 			DIRECTORY_SEPARATOR		'/'
#endif

/*____ Amiga PowerPC WarpOS / VBCC or Storm-GCC_______________________________*/

#if SYSTEM == AMIGA_WOS
#	define				BYTEORDER              BIG_ENDIAN
#	define				DIRECTORY_SEPARATOR     '/'
#	define				SECOND_DIRECTORY_SEPARATOR     ':' 	/* Special case for volumes */
#	define				PRIO 								/* Yes, it can be done */
#	define				WILDCARDS 							/* Uses dos/ParsePatternNoCase() and friends.*/
#	define				PROTO_ARGS 							/* Undocumented, it's useful for GCC-Storm */
#	ifdef __VBCC__
#		pragma amiga-align
#	endif
#	include <dos/dos.h>
#	ifdef __VBCC__
#		pragma default-align
#	endif

#endif



/*____ Some defines automatically gives other...*/

#ifdef UNIX_SYSTEM
#	define			PRIO
#	define			DIRECTORY_SEPARATOR		'/'
#endif


/*==== COMPILER SPECIFIC DEFINES ============================================ */
/*
	Unfortunately the inline command differs slightly between compilers (for
	example GCC calls it 'inline' while VC++ calls it '_inline'), so we have
	to use our own define for it to keep everything compiler independent.

	Inline just speeds things up, so if we don't recognice the architecture we
	just define an empty INLINE and take a minor performance hit.

*/

#ifdef	__GNUC__											/* GNU C and family */
#		define			INLINE					inline
#endif

#ifdef	__TURBOC__											/* Borland Turbo C, Turbo C++, and Borland C++ */
#		define			INLINE					__inline
#endif

#ifdef	_MSC												/* Visual C++ */
#		define			INLINE					_inline		
#endif

#ifdef __STORM
#		define 			INLINE 					__inline	/* Storm C4 */
#endif

#ifdef __VBCC__
#		define			INLINE 							/* VBCC */
#endif

#ifndef	INLINE
#		define			INLINE								/* Empty construct default, minor performance hit. */
#endif


/*____ To make sure that certain necessary defines are set... */

#ifndef FALSE
#	define         FALSE           0
#endif

#ifndef TRUE
#	define         TRUE            1
#endif

/*==== Other Global Definitions, placed here for convenience ==================*/

/*#define			PRECISE_TIMER*/		/* Gives more accurate speed calculations, */
										/* just for debug purposes. Disable in release version! */

typedef		unsigned	char 	uchar;

#if (defined(UNIX_SYSTEM) && !defined(SYS_TYPES_H) && !defined(_SYS_TYPES_H)) || (!defined UNIX_SYSTEM  && !defined(__GNUC__))
		typedef		unsigned short	ushort;
		typedef		unsigned int	uint;
#endif


#endif		/* __SYSTEM__ */
