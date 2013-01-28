

#include <stdio.h>
#include <stdlib.h>

int a __attribute__((section(".test.rodata"))) = 1337;
char b[] __attribute__ ((aligned (8))) = "hello world!";
char *c __attribute__ ((aligned (8))) = "pouet pouet!";

void main (void)
{
  printf ("a = %d\n", a);
  printf ("b = %s\n", b);
  printf ("c = %s\n", c);
}

