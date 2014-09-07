/* ----------------------------------------------------------------------- */
/*  This file is part of rld.                                              */
/* ----------------------------------------------------------------------- */
/*  Copyright (c) 2000 stfsux <stfsux@tuxfamily.org>                       */
/*  This work is free. You can redistribute it and/or modify it under the  */
/*  terms of the Do What The Fuck You Want To Public License, Version 2,   */
/*  as published by Sam Hocevar. See the COPYING file for more details.    */
/* ----------------------------------------------------------------------- */

#ifndef _RLDLIST_H_
#define _RLDLIST_H_

typedef struct _lstr
{
  char **item;
  unsigned int nitems;
}lstr_t, *plstr_t;

plstr_t lstr_create (void);
void    lstr_add (plstr_t lstr, char *str);
void    lstr_destroy (plstr_t lstr);

#endif

