/* ----------------------------------------------------------------------- */
/*  This file is part of rld.                                              */
/* ----------------------------------------------------------------------- */
/*  Copyright (c) 2000 stfsux <stfsux@tuxfamily.org>                       */
/*  This work is free. You can redistribute it and/or modify it under the  */
/*  terms of the Do What The Fuck You Want To Public License, Version 2,   */
/*  as published by Sam Hocevar. See the COPYING file for more details.    */
/* ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

int a __attribute__((section(".test.rodata"))) = 1337;
char b[] __attribute__ ((aligned (8))) = "hello world!";
char *c __attribute__ ((aligned (8))) = "pouet pouet!";
char tmp[10];

void main (void)
{
  tmp[3] = 0x55;
  printf ("a = %d\n", a);
  printf ("b = %s\n", b);
  printf ("c = %s\n", c);
  printf ("tmp = %02X\n", tmp[3]);
}

