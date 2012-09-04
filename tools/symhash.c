#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t
 sym_hash (str)
    char *str;
{
  uint32_t hash=0;
  char c;
  while((c=*str)!=0)
  {
    hash = ((c-hash+(hash<<6)) + (hash<<16));
    str++;
  }
  return hash;
}

int main (int argc, char **argv)
{
  if (argc < 2)
    return 0;
  fprintf (stdout, "symbol: %s hash: %08X\n", argv[1], sym_hash(argv[1]));
  return 0;
}

