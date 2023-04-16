/*
	Function: strupr
	Input:    char *     - String to convert
	Output:   char *     - Converted string
	Converts the string to uppercase.
	Added by Trevor Phillips, 25/6/98, for Solaris port.



	------------    Changes    ------------

	2000-12-11  Andre Piotrowski

	-	include "arglink.h" containing the prototype
		(to avoid the compiler warning)
*/
#include	"arglink.h"





char					*mystrupr (char *s)
{
	unsigned int			i;

	for (i = 0; s[i] != '\0';  i++)
		if (s[i] >= 'a'  &&  s[i] <= 'z')
			s[i] += 'A'-'a';

	return s;
}
