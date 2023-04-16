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

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	<assert.h>

#include	"arglink.h"
#include	"system.h"




/*  ========================================================================  */
/*              Predeclaration of local procedures                            */
/*  ========================================================================  */





static	argLink			*addStringToArgLink
(
	argLink					**wpPrev,
	const char				*pString,
	int						useCopyOfString
);





/*  ========================================================================  */
/*              local procedures                                              */
/*  ========================================================================  */





/*	------------    addStringToArgLink    ------------

	creates a linked argument list element from a string
	and adds it at position '*wpPrev' to the list.

	returns the pointer to the created argument,
	if successful, NULL otherwise.

	If 'useCopyOfString', a copy of the string will be
	created and linked to the list instead of the original.
*/
static	argLink			*addStringToArgLink
(
	argLink					**wpPrev,
	const char				*pString,
	int						useCopyOfString
)
{
	argLink					*pLink;

assert (wpPrev != NULL);   /* Did you forget to use the &-operator? */

	if (useCopyOfString)
	{
		pLink = (argLink  *) malloc (sizeof (argLink) + strlen (pString) + 1);
		pLink->pString = (char *) &pLink[1];
		strcpy (pLink->pString, pString);
	}
	else
	{
		pLink = (argLink *) malloc (sizeof (argLink));
		pLink->pString = (char *) pString;   /* cast to avoid compiler warning */
	}

	pLink->psNext = *wpPrev;
	*wpPrev = pLink;

	return pLink;
}






/*  ========================================================================  */
/*              glocal procedures                                             */
/*  ========================================================================  */





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
)
{
	return addStringToArgLink (wpPrev, pString, FALSE);
}



/*	------------    insertStringToArgLink    ------------

	acts similar to 'linkStringToArgLink()',
	but creates a copy of the string and links that to the list.
*/
argLink					*insertStringInArgLink
(
	argLink					**wpPrev,
	char					*pString
)
{
	return addStringToArgLink (wpPrev, pString, TRUE);
}



/*	------------    arg2ArgvLink    ------------

	creates a linked argument list from an array
	argv[] with argc string arguments.
*/
argLink					*argv2ArgLink
(
	int						argc,
	char					*argv[]
)
{
	argLink					*pFirst;
	int						i;

	pFirst = NULL;

	for (i = argc-1;  i >= 0;  i--)
		addStringToArgLink (&pFirst, argv[i], FALSE);

	return pFirst;
}



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
)
{
	char					buffer[1024];
	char					*start, *end, c;
	FILE					*fp;
	int						quote, nArg;


assert (wpPrev != NULL);   /* Did you forget the &-operator? */

	nArg = 0;

	fp = fopen (pFilename, "r");
	if (fp == NULL)
		return	-1;

	while (fgets (buffer, 1024, fp) != NULL)   /* take line by line */
	{
		start = buffer;
		do
		{
			while (*start != '\0'  &&  *start <= ' ')
				start++;   /* skip over characters <= ' ' */

			if (*start == '\"')   /* set quote mode, if argument starts with a double quote */
			{
				quote = TRUE;
				start++;   /* but the quote doesn't get part of the argument */
			}
			else
				quote = FALSE;

			end = start;
			while (*end != '\0')   /* the end of the string always terminates the argument */
			{
				if (quote)   /* in case of quote mode */
				{
					if (*end == '\"')   /* the argument gets terminated by another double quote */
						break;
				}
				else   /* otherwise */
				{
					if (*end <= ' ')   /* the argument gets terminated by a character <= ' ' */
						break;
					if (*end == '#')   /* or by the beginning of a commment starting with a '#' */
						break;
				}
				end++;
			}

			if( start != end )
			{
    			c = *end;
    			*end = '\0';   /* mark temporarily the end of the string */
    			addStringToArgLink (wpPrev, start, TRUE);   /* and add the argument to the list */
    		/*	printf( "Arg: %s\n", start ); */
    			*end = c;
    			nArg++;
    			if( * end == '\"' )
    				end++;
			}
			start = end + 1;   /* skip over the delimiter already read */
		} while (*end != '\0'  &&  *end != '#');
	}

	fclose (fp);

	return	nArg;
}



/*	------------    findStrInArgList    ------------

	searches for occurence of a string in the linked
	argument list and returns the pointer to the
	argument, if present, NULL otherwise.
*/
argLink					*findStrInArgLink
(
	argLink					*pLink,
	char					*pString
)
{
	char					S[256], T[256];

	strcpy (S, pString);
	mystrupr (S);

	while (pLink != NULL)
	{
		strcpy (T, pLink->pString);
		mystrupr (T);

		if (strcmp (S, T) == 0)
			return (argLink *) pLink;   /* cast to avoid compiler warning */

		pLink = pLink->psNext;
	}

	return NULL;
}



/*	------------    deleteArgLink    ------------

	deletes the entire linked argument list.
*/
void					deleteArgLink
(
	argLink					*pLink
)
{
	argLink					*pDelete;

	while (pLink != NULL)
	{
		pDelete = pLink;
		pLink = pDelete->psNext;
		free (pDelete);
	}
}





