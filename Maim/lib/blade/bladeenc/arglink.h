/*
			(c) Copyright 1999-2000 - Tord Jansson
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

	2000-11-27  Andre Piotrowski

	-	redesigned, commented

	-	feature: addFileContentToArgLink()
		Now, (filename) arguments surrounded by double quotes may include spaces!
*/

#ifndef		__ARGLINK__
#define		__ARGLINK__





extern	char			*mystrupr (char *);





typedef		struct argLinkDef
			{
				struct argLinkDef		*psNext;
				char					*pString;
			}						argLink;





/*	------------    linkStringToArgLink    ------------

	creates a linked argument list element from a string
	and adds it at position '*wpPrev' to the list.

	returns the pointer to the created argument,
	if successful, NULL otherwise.
*/
argLink					*linkStringToArgLink
(
	argLink					**wpPrev,
	char					*pString
);


/*	------------    insertStringToArgLink    ------------

	acts similar to 'linkStringToArgLink()',
	but creates a copy of the string and links that to the list.
*/
argLink					*insertStringInArgLink
(
	argLink					**wpPrev,
	char					*pString
);


/*	------------    arg2ArgvLink    ------------

	creates a linked argument list from an array
	argv[] with argc string arguments.
*/
argLink					*argv2ArgLink
(
	int						argc,
	char					*argv[]
);


/*	------------    addFileContentToArgLink    ------------

	reads arguments from a file given by its filename
	and adds them at position '*wpPrev' to the list.

	returns the number of arguments added, if successful,
	-1 in case that the file couldn't been opened.

	Now, (filename) arguments surrounded by double quotes may include spaces!
*/
int						addFileContentToArgLink
(
	argLink					**wpPrev,
	char					*pFilename
);


/*	------------    findStrInArgList    ------------

	searches for occurence of a string in the linked
	argument list and returns the pointer to the
	argument, if present, NULL otherwise.
*/
argLink					*findStrInArgLink
(
	argLink					*pLink,
	char					*pString
);


/*	------------    deleteArgLink    ------------

	deletes the entire linked argument list.
*/
void					deleteArgLink
(
	argLink					*pLink
);





#endif		/* __ARGLINK__ */
