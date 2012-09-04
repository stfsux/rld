
#ifndef _RLDLIST_H_
#define _RLDLIST_H_

typedef struct _lstr
{
  char **item;
  unsigned int nitems;
}lstr_t, *plstr_t;

plstr_t lstr_create (void);
void    lstr_add (plstr_t lstr, char *str);
void    lstr_destroy (plstr_t lstr);

#endif

