

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

