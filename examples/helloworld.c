/* ----------------------------------------------------------------------- */
/*  This file is part of rld.                                              */
/* ----------------------------------------------------------------------- */
/*  Copyright (c) 2000 stfsux <stfsux@tuxfamily.org>                       */
/*  This work is free. You can redistribute it and/or modify it under the  */
/*  terms of the Do What The Fuck You Want To Public License, Version 2,   */
/*  as published by Sam Hocevar. See the COPYING file for more details.    */
/* ----------------------------------------------------------------------- */

#include <stdio.h>
char *lol = "pouet pouet!";
void main(void) __attribute__((section(".text")));
void main(void)
{ puts (lol); }
