
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "rld.h"
#include "rldlist.h"

uint8_t
 rldfile_find (char* libname, plstr_t dir, plstr_t path)
{
  unsigned int i;
  DIR *dp = NULL;
  struct dirent *entry = NULL;
  char filename[256], tmp[256];
  snprintf (filename, sizeof(filename), "lib%s", libname);
  for (i = 0; i < dir->nitems; i++)
  {
    dp = opendir (dir->item[i]);
    if (dp == NULL)
      continue;
    while ((entry=readdir(dp)) != NULL)
    {
      if (entry->d_type == DT_LNK)
      {
        if (strncmp (entry->d_name, filename, strlen(filename)) == 0)
        {
          if (strlen(entry->d_name) > strlen(filename))
          {
            char c = entry->d_name[strlen(filename)];
            if (c == '.' || c == '-')
            {
              if (strstr (entry->d_name, ".so.") != NULL)
              {
                if ((strlen(filename)+strlen(dir->item[i])) < 255)
                {
                  if (dir->item[i][strlen(dir->item[i])-1] == '/')
                    snprintf (tmp, sizeof(tmp), "%s%s", dir->item[i], entry->d_name);
                  else
                    snprintf (tmp, sizeof(tmp), "%s/%s", dir->item[i], entry->d_name);
                  lstr_add (path, tmp);
                  closedir (dp);
                  return 1;
                }
              }
            }
          }
        }
      }
    }
    closedir (dp);
  }
  fprintf (stderr, "%s: cannot find `%s'.\n", __progname, filename);
  return 0;
}

