/* Undefined symbols manipulation */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include "rld.h"
#include "rldlist.h"
#include "rldsym.h"
#include "rldelf.h"

/*------------------------------------------------------------------*/
psymtab_t *
 symtab_create (uint8_t nobj)
{
  psymtab_t *symtab = 0;
  uint8_t i, j;
  symtab = calloc (nobj, sizeof(psymtab_t));
  if (symtab == NULL)
  {
    fprintf (
          stderr,
          "%s: cannot allocate `%lu' byte(s).\n",
          __progname,
          (unsigned long)nobj*sizeof(psymtab_t)
        );
    return NULL;
  }
  for (i = 0; i < nobj; i++)
  {
    symtab[i] = calloc (1, sizeof(symtab_t));
    if (symtab[i] == NULL)
    {
      for (j = 0; j < i; j++)
        free (symtab[i]);
      free (symtab);
      fprintf (
            stderr,
            "%s: cannot allocate `%lu' byte(s).\n",
            __progname,
            (unsigned long)nobj*sizeof(symtab_t)
          );
    }
    
  }
  return symtab;
}

/*------------------------------------------------------------------*/
uint8_t
 symtab_add_sym (psymtab_t symtab, char* symname, uint32_t hash,
     uint32_t sectype, uint32_t bind, uint32_t type, uint32_t rid, 
     uint32_t offset)
{
  symtab->syms = realloc (
                    symtab->syms,
                    (symtab->nsyms+1)*sizeof(psym_t)
                  );
  if (symtab->syms == NULL)
    return 0;
  symtab->syms[symtab->nsyms] = calloc (1, sizeof(sym_t));
  if (symtab->syms[symtab->nsyms] == NULL)
    return 0;
  symtab->syms[symtab->nsyms]->symname = calloc (
                                            strlen(symname)+1,
                                            sizeof(char)
                                          );
  if (symtab->syms[symtab->nsyms]->symname == NULL)
    return 0;
  strncpy (
      symtab->syms[symtab->nsyms]->symname,
      symname,
      strlen (symname)
    );
  symtab->syms[symtab->nsyms]->symname[strlen (symname)] = 0;
  symtab->syms[symtab->nsyms]->hash = hash;
  symtab->syms[symtab->nsyms]->sectype = sectype;
  symtab->syms[symtab->nsyms]->bind = bind;
  symtab->syms[symtab->nsyms]->type = type;
  symtab->syms[symtab->nsyms]->symrid = rid;
  symtab->syms[symtab->nsyms]->offset = offset;
  if (symtab->syms[symtab->nsyms]->sectype != SYM_SEC_UNDEF)
    symtab->syms[symtab->nsyms]->flags = 1;
  symtab->nsyms++;
  return 1;
}

/*------------------------------------------------------------------*/
uint32_t
 sym_hash (char* str)
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

/*------------------------------------------------------------------*/
uint8_t
 symtab_check_usym (psymtab_t *symtab, uint8_t nobj, uint8_t n,
     uint32_t hash)
{
  unsigned int i, j;
  for (i = 0; i < nobj; i++)
  {
    if (i == n)
      continue;

    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->hash == hash
          && symtab[i]->syms[j]->sectype == SYM_SEC_UNDEF)
        return 1;
    }
  }
  return 0;
}

/*------------------------------------------------------------------*/
uint32_t
 symtab_get_usym_hashid (psymtab_t *symtab, uint8_t nobj, uint8_t n,
     uint32_t hash)
{
  unsigned int i, j;
  uint32_t hashid = 0;
  for (i = 0; i < nobj; i++)
  {
    if (i == n)
      continue;

    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->hash == hash
          && symtab[i]->syms[j]->sectype == SYM_SEC_UNDEF)
      {
        hashid = symtab[i]->syms[j]->hashid;
        break;
      }
    }
  }
  return hashid;
}

/*------------------------------------------------------------------*/
void
 symtab_destroy (psymtab_t *symtab, uint8_t nobj)
{
  uint8_t i;
  uint32_t j;
  if (symtab != NULL)
  {
    for (i = 0; i < nobj; i++)
    {
      if (symtab[i] != NULL)
      {
        for (j = 0; j < symtab[i]->nsyms; j++)
        {
          if (symtab[i]->syms != NULL)
          {
            if (symtab[i]->syms[j] != NULL)
            {
              if (symtab[i]->syms[j]->symname != NULL)
                free (symtab[i]->syms[j]->symname);
              free (symtab[i]->syms[j]);
            }
          }
        }
        free (symtab[i]->syms);
        free (symtab[i]);
      }
    }
    free (symtab);
  }
}

