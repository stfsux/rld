/* ----------------------------------------------------------------------- */
/*  This file is part of rld.                                              */
/* ----------------------------------------------------------------------- */
/*  Copyright (c) 2000 stfsux <stfsux@tuxfamily.org>                       */
/*  This work is free. You can redistribute it and/or modify it under the  */
/*  terms of the Do What The Fuck You Want To Public License, Version 2,   */
/*  as published by Sam Hocevar. See the COPYING file for more details.    */
/* ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <libgen.h>
#include <ctype.h>
#include "rld.h"
#include "rldlist.h"
#include "rldfile.h"

/*------------------------------------------------------------------*/
uint8_t
 rldfile_find (char* name, plstr_t dir, plstr_t libdevfp,
     plstr_t libname)
{
  unsigned int i;
  DIR *dp = NULL;
  struct dirent *entry = NULL;
  char filename[256+6];
  char tmp[256+6];
  char *lname = NULL;

  memset (tmp, 0, sizeof(tmp));
  memset (filename, 0, sizeof(tmp));

  snprintf (filename, sizeof(filename), "lib%s.so", name);
  for (i = 0; i < dir->nitems; i++)
  {
    dp = opendir (dir->item[i]);
    if (dp == NULL)
      continue;
    while ((entry=readdir(dp)) != NULL)
    {
      if (entry->d_type == DT_LNK)
      {
        if (strcmp (name, "c") == 0)
        {
          if (strncmp (entry->d_name, "libc.so.", 8) == 0)
          {
            if (rldfile_check_valid_name (entry->d_name))
            {
              snprintf (tmp, sizeof(tmp), "%s/%s",
                        dir->item[i], entry->d_name);
              lstr_add (libname, entry->d_name);
              lstr_add (libdevfp, tmp);
              closedir (dp);
              return 1;
            }
          }
        }
        else if (strcmp (filename, entry->d_name)== 0)
        {
          snprintf (tmp, sizeof(tmp), "%s/%s",
                      dir->item[i], entry->d_name);
          lname = rldfile_get_libname (tmp);
          if (lname == NULL)
            lstr_add (libname, entry->d_name);
          else
            lstr_add (libname, lname);
          lstr_add (libdevfp, tmp);
          closedir (dp);
          return 1;
        }
      }
    }
    closedir (dp);
  }
  fprintf (stderr, "%s: cannot find `%s'.\n", __progname, filename);
  return 0;
}

/*------------------------------------------------------------------*/
uint8_t
 rldfile_check_valid_name (char* libname)
{
  char* ptr = NULL;

  ptr = strstr (libname, ".so.");
  if (ptr == NULL) return 0;

  ptr = (char*)((uintptr_t)ptr+(uintptr_t)4);
  while (*ptr)
  {
    if (!isdigit(*ptr)) return 0;
    ptr++;
  }
  return 1;
}

/*------------------------------------------------------------------*/
char*
 rldfile_get_libname (char* libnamefp)
{
  char* libname = NULL;
  char* rpath = NULL;
  size_t i = 0;
  uint8_t cnt = 0;

  rpath = realpath (libnamefp, NULL);
  if (rpath == NULL)
    return NULL;

  libname = basename (rpath);
  if (libname == NULL)
    return NULL;

  for (i = strlen(libname); i != 0; i--)
  {
    if (libname[i] == '.') cnt++;
    libname[i] = 0;
    if (cnt == 2) break;
  }

  free (rpath);
  return libname;
}

/*------------------------------------------------------------------*/
uint8_t
 rldfile_check_libname_conv (char *libname)
{
  uint8_t cnt = 0;
  char* ptr = NULL;

  ptr = strstr (libname, ".so");
  if (ptr == NULL)
    return 0;
  while (*ptr)
  {
    if (*ptr == '.') cnt++;
    ptr++;
  }
  if (cnt == 2) return 1;
  return 0;
}

/*------------------------------------------------------------------*/
void
 rldfile_read_multiarch_conf (plstr_t libpath)
{
  FILE* conf = NULL;
  DIR* dp = NULL;
  struct dirent *entry = NULL;
  char filename[256], tmp[256];
  char *arch[] = {
    "i386", "i486", "i686"
  };
  unsigned int i;

  dp = opendir ("/etc/ld.so.conf.d/");
  if (dp == NULL)
    return;

  while ((entry=readdir(dp)) != NULL)
  {
    if (entry->d_type == DT_REG)
    {
      for (i = 0; i < 3; i++)
      {
        if (strstr (entry->d_name, arch[i]) != NULL)
        {
          snprintf (filename, sizeof(filename)-1,
              "/etc/ld.so.conf.d/%s", entry->d_name);
          conf = fopen (filename, "r");
          if (conf == NULL)
            continue;
          while (fgets (tmp, sizeof(tmp), conf) != NULL)
          {
            if (strchr (tmp, '#') != NULL)
              continue;
            else if (strstr (tmp, "include") != NULL)
              continue;
            if (strlen (tmp) > 1)
            {
              tmp[strlen(tmp)-1] = 0;
              lstr_add (libpath, tmp);
            }
          }
          fclose (conf);
        }
      }
    }
  }
  closedir (dp);
}

