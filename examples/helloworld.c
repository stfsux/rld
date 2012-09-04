#include <stdio.h>
char *lol = "pouet pouet!";
void main(void) __attribute__((section(".text")));
void main(void)
{ puts (lol); }
