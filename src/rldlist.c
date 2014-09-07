/* ----------------------------------------------------------------------- */
/*  This file is part of rld.                                              */
/* ----------------------------------------------------------------------- */
/*  Copyright (c) 2000 stfsux <stfsux@tuxfamily.org>                       */
/*  This work is free. You can redistribute it and/or modify it under the  */
/*  terms of the Do What The Fuck You Want To Public License, Version 2,   */
/*  as published by Sam Hocevar. See the COPYING file for more details.    */
/* ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rld.h"
#include "rldlist.h"

/*------------------------------------------------------------------*/
plstr_t 
 lstr_create ( void )
{
	plstr_t lstr = NULL;
	lstr = calloc (1, sizeof(lstr_t));
	if (lstr == NULL)
		return NULL;
	lstr->nitems = 0;
	lstr->item = calloc (1, sizeof(char*));
	return lstr;
}

/*------------------------------------------------------------------*/
void
 lstr_add (plstr_t lstr, char* str)
{
	void *ptr = NULL;
	ptr = realloc (lstr->item, sizeof(char*)*(lstr->nitems+1));
	if (ptr == NULL)
  {
    fprintf (
          stderr,
          "%s: cannot re-allocate %lu bytes.\n",
          __progname,
          (unsigned long)(lstr->nitems+1)*sizeof(char*)
        );
		return;
  }
	lstr->item = ptr;
	lstr->item[lstr->nitems] = calloc (strlen(str)+1, sizeof(char));
	if (lstr->item[lstr->nitems] == NULL)
  {
    fprintf (
          stderr,
          "%s: cannot allocate %lu bytes.\n",
          __progname,
          (unsigned long)(strlen(str)+1)*sizeof(char)
        );
		return;
  }
	strncpy (lstr->item[lstr->nitems], str, strlen(str));
	lstr->item[lstr->nitems][strlen(str)] = 0;
	lstr->nitems++;
}

/*------------------------------------------------------------------*/
void
 lstr_destroy (plstr_t lstr)
{
	unsigned int i;
	if (lstr != NULL)
	{
		if (lstr->item != NULL)
		{
			for (i = 0; i < lstr->nitems; i++)
			{
				if (lstr->item[i] != NULL)
					free (lstr->item[i]);
			}
			free (lstr->item);
		}
		free (lstr);
	}
}

