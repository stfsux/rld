
#ifndef _RLDFILE_H_
#define _RLDFILE_H_

uint8_t rldfile_find (char *libname, plstr_t dir, plstr_t path);
void    rldfile_read_multiarch_conf (plstr_t libpath);

#endif

