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
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "rld.h"
#include "rldlist.h"
#include "rldsym.h"
#include "rldelf.h"

/*------------------------------------------------------------------*/
pelf_file_t
 elf_load (char* filename)
{
  pelf_file_t elf = NULL;
  elf = calloc (1, sizeof(elf_file_t));
  if (elf == NULL)
    return NULL;
  elf->fd = open (filename, O_RDONLY);
  if (elf->fd == -1)
  {
    fprintf (
        stderr,
        "%s: no such file `%s'.\n",
        __progname,
        filename
        );
    free (elf);
    return NULL;
  }
  elf->filename = calloc (strlen(filename)+1, sizeof(char));
  if (elf->filename == NULL)
  {
    fprintf (
        stderr,
        "%s: cannot allocate %lu bytes.\n",
        __progname,
        (unsigned long)strlen(filename)*sizeof(char)
        );
    close (elf->fd);
    free (elf);
    return NULL;
  }
  strncpy (elf->filename, filename, strlen(filename));
  elf->filename[strlen(filename)] = 0;
  elf->size = lseek (elf->fd, 0, SEEK_END);
  lseek (elf->fd, 0, SEEK_SET);
  elf->mem = mmap (
                0,
                elf->size,
                PROT_READ,
                MAP_PRIVATE,
                elf->fd,
                0
              );
  if (elf->mem == MAP_FAILED)
  {
    fprintf (
        stderr,
        "%s: cannot map file `%s'.\n",
        __progname,
        filename
        );
    close (elf->fd);
    free (elf);
    return NULL;
  }
  return elf;
}

/*------------------------------------------------------------------*/
pelf_file_t
 elf_create (char* filename)
{
  pelf_file_t elf = NULL;
  elf = calloc (1, sizeof(elf_file_t));
  if (elf == NULL)
  {
    fprintf (
        stderr,
        "%s: cannot allocate %lu bytes.\n",
        __progname,
        (unsigned long)sizeof(elf_file_t)
        );
    return NULL;
  }
  elf->fd = open (filename,
          O_CREAT|O_RDWR|O_TRUNC,
          S_IRWXU|S_IRWXG|S_IRWXO);
  if (elf->fd == -1)
  {
    /* Check errno. */
    fprintf (
        stderr,
        "%s: cannot create file `%s'.\n",
        __progname,
        filename
        );
    free (elf);
    return NULL;
  }
  return elf;
}

/*------------------------------------------------------------------*/
void
 elf_close (pelf_file_t elf)
{
  if (elf != NULL)
  {
    if (elf->mem != MAP_FAILED || elf->size != 0)
      munmap (elf->mem, elf->size);
    if (elf->filename != NULL)
      free (elf->filename);
    if (elf->fd != -1)
      close (elf->fd);
    free (elf);
  }
}

