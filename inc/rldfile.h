/* ----------------------------------------------------------------------- */
/*  This file is part of rld.                                              */
/* ----------------------------------------------------------------------- */
/*  Copyright (c) 2000 stfsux <stfsux@tuxfamily.org>                       */
/*  This work is free. You can redistribute it and/or modify it under the  */
/*  terms of the Do What The Fuck You Want To Public License, Version 2,   */
/*  as published by Sam Hocevar. See the COPYING file for more details.    */
/* ----------------------------------------------------------------------- */

#ifndef _RLDFILE_H_
#define _RLDFILE_H_

uint8_t rldfile_find (char *name, plstr_t dir, plstr_t libdevfp, plstr_t libname);
uint8_t rldfile_check_valid_name (char* libname);
char* rldfile_get_libname (char* libnamefp);
void    rldfile_read_multiarch_conf (plstr_t libpath);

#endif

