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

	2000-12-05  Andre Piotrowski

	-	reformatted
*/

#ifndef		__BLADESYS__
#define		__BLADESYS__





#define		MAX_NAMELEN				256





extern	int				setPriority (char *pPrioString);

extern	void			prepStdin (void);
extern	void			prepStdout (void);

extern	int				be_kbhit (void);
extern	int				be_getch (void);

#ifdef		WILDCARDS
int						expandWildcards (argLink **ppArgLink);
#endif

int						findConfigFile (char *pExepath, char *wpPath);





#endif		/* __BLADESYS__ */
