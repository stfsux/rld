
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include "rld.h"
#include "rldlist.h"
#include "rldsym.h"
#include "rldelf.h"
#include "rldfile.h"

extern platform_t x86;

pplatform_t platforms[] = {
  &x86, NULL
};

/*------------------------------------------------------------------*/
int
 main (int argc, char *argv[])
{
  /* input/output elf file handle. */
  pelf_file_t input = NULL, output = NULL;

  psymtab_t *symtab = NULL;

  /* list of library search path. */
  plstr_t libpath = NULL;
  /* list of full path libraries. */
  plstr_t libs = NULL;
  
  unsigned int i, j, k, n;

  /* internal symbols flag                                 */
  /* bit-n is set when the internal symbol has been found. */
  /* [bit 0] __rld_debug_ptr                               */
  /* [bit 1] __rld_import_hash                             */
  /* [bit 2] __rld_import_addr                             */
  /* [bit 3] __rld_num_imports                             */
  /* [bit 4] Not used                                      */
  /* [bit 5] Not used                                      */
  /* [bit 6] Not used                                      */
  /* [bit 7] Not used                                      */
  unsigned char internal_symbols = 0;
  
  unsigned long section_hash = 0;
  unsigned long section_addr = 0;
  unsigned long section_jmptab = 0;
  unsigned long section_bss_sz = 0;

  uint32_t nexternal_symbols = 0;
  unsigned long vma_debug_ptr = 0;
  unsigned long entry_offset = 0;

  /* verbose mode. */
  int verbose = 0;

  /* compress the output elf. */
  int compress = 0;

  unsigned int target_platform = __RLD_DEFAULT_PLATFORM;

  /* number of object file. */
  uint8_t nobj = 0;

  /* output file name. */
  char *filename = NULL;
  char *default_filename = "a.out";

  /* Current option. */
  int opt = 0;
  /* Options */
  struct option optlist[] =
  {
    { "help",         0, NULL,      'h' },
    { "output",       1, NULL,      'o' },
    { "architecture", 1, NULL,      'a' },
    { "library-path", 1, NULL,      'L' },
    { "library",      1, NULL,      'l' },
    { "verbose",      0, &verbose,   1  },
    { "compress",     0, &compress,  1  },
    { "version",      0, NULL,      'v' },
  };

  /* argc/argv limitations. */
  if ((unsigned int)argc > 255)
    return 1;
  for (i = 0; i < (unsigned int)argc; i++)
  {
    if (strlen(argv[i]) > 255)
      return 2;
  }

  /* create lists. */
  libpath = lstr_create ();
  libs    = lstr_create ();
  rldfile_read_multiarch_conf (libpath);

  /* parse cmd. */
  while ((opt=getopt_long (argc, argv, "hva:L:l:o:", optlist, NULL)) != -1)
  {
    switch (opt)
    {
      case 'h':
        fprintf (stdout, __RLD_USAGE);
        goto _quit;

      case 'v':
        fprintf (stdout, __RLD_VERSION);
        goto _quit;

      case 'a':
        fprintf (stderr, "%s: x86 architecture supported only.\n", __progname);
        goto _quit;

      case 'L':
        lstr_add (libpath, optarg);
        break;

      case 'l':
        if (rldfile_find (optarg, libpath, libs) == 0)
          goto _quit;
        break;

      case 'o':
        filename = optarg;
        break;
    }
  }

  if ((unsigned int)optind >= (unsigned int)argc)
  {
    fprintf (stderr, "%s: no input file(s).\n", __progname);
    goto _quit;
  }

  if (filename == NULL)
    filename = default_filename;

  if (verbose)
  {
    for (i = 0; i < libpath->nitems; i++)
      fprintf (stdout, "%s: library path `%s'.\n", __progname, libpath->item[i]);
  }

  nobj = (argc-optind)&0xFF;

  symtab = symtab_create (nobj);

  if (symtab == NULL) goto _quit;

  /* store symbols. */
  for (i = 0; i < nobj; i++)
  {
    uint32_t nsyms = 0;
    input = elf_load (argv[optind+i]);
    if (input == NULL) goto _quit;
    if (platforms[target_platform]->elf_chkfmt (input) == 0)
    {
      fprintf (stderr, "%s: object file '%s' doesn't match with platform target.\n", __progname, argv[optind+i]);
      elf_close (input);
      goto _quit;
    }
    nsyms = platforms[target_platform]->elf_get_nsym (input);
    for (j = 1; j < nsyms; j++)
    {
      uint32_t hash = 0, sectype = 0, bind = 0;
      uint32_t type = 0, offset = 0, sz = 0;
      uint16_t secid = 0;
      char *symname = NULL;
      sectype = platforms[target_platform]->elf_get_symsec (input, j);
      bind = platforms[target_platform]->elf_get_symbind (input, j);
      type = platforms[target_platform]->elf_get_symtype (input, j);
      secid = platforms[target_platform]->elf_get_symsecid (input, j);
      if (sectype == SYM_SEC_ABS || type == SYM_TYPE_FILE)
        continue;
      if (type == SYM_TYPE_SEC)
      {
        symname = platforms[target_platform]->elf_get_secname (input, j);
        if ((strstr (symname, ".text") == NULL)
            && (strstr (symname, ".bss") == NULL)
            && (strstr (symname, ".rodata") == NULL)
            && (strstr (symname, ".data") == NULL))
          continue;
        sz = platforms[target_platform]->elf_get_secsz (input, symname);
        offset = platforms[target_platform]->elf_get_secoff (input, symname);
      }
      else
      {
        symname = platforms[target_platform]->elf_get_symstr (input, j);
        offset = platforms[target_platform]->elf_get_symval (input, j);
        sz = platforms[target_platform]->elf_get_symsz (input, j);
      }
      hash = sym_hash (symname);
      if (type != SYM_TYPE_SEC)
      {
        /* check for multiple definition. */
        if (sectype != SYM_SEC_UNDEF)
        {
          for (n = 0; n < i; n++)
          {
            for (k = 0; k < symtab[n]->nsyms; k++)
            {
              if (hash == symtab[n]->syms[k]->hash)
              {
                if (bind == SYM_BIND_GLOBAL
                    && symtab[n]->syms[k]->bind == SYM_BIND_GLOBAL
                    && symtab[n]->syms[k]->sectype != SYM_SEC_UNDEF)
                {
                  fprintf (stderr, "%s:%s: multiple definition of `%s'.\n", 
                      __progname, argv[optind+i], platforms[target_platform]->elf_get_symstr (input, j));
                  elf_close (input);
                  goto _quit;
                }
              }
            }
          }
        }
      }
      if (symtab_add_sym (symtab[i], symname, hash, sectype, bind, type, j, offset, sz, secid) == 0)
      {
        elf_close (input);
        goto _quit;
      }
      if (verbose)
      {
        fprintf (stdout, "%s: symname: %-30s object: %-10s hash: %08X sectype: %08X bind: %08X type: %08X offset: %08X size: %08X secid: %04X", __progname, symname, argv[optind+i], hash, sectype, bind, type, offset, sz, secid);
        if (secid != 0 && secid != (uint16_t)~0)
          fprintf (stdout, " %s", symtab_get_secname (symtab[i], secid));
        fprintf (stdout, "\n");
      }
    }
    elf_close (input);
  }

  /* set up the symtab. */
  for (i = 0; i < nobj; i++)
  {
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->sectype == SYM_SEC_UNDEF)
      {
        switch (symtab[i]->syms[j]->hash)
        {
          
          case 0xB144F4AD:
            symtab[i]->syms[j]->flags = (1<<2);
            _rld_set_bit(internal_symbols, 0);
            continue;

          case 0x5C77F413:
            symtab[i]->syms[j]->flags = (1<<2);
            _rld_set_bit(internal_symbols, 1);
            continue;

          case 0x183369B6:
            symtab[i]->syms[j]->flags = (1<<2);
            _rld_set_bit(internal_symbols, 2);
            continue;

          case 0xEBCAB480:
            symtab[i]->syms[j]->flags = (1<<2);
            _rld_set_bit(internal_symbols, 3);
            continue;
        }
        for (n = 0; ((n < nobj)&&(symtab[i]->syms[j]->flags == 0)); n++)
        {
          for (k = 0; k < symtab[n]->nsyms; k++)
          {
            if ((symtab[i]->syms[j]->hash == symtab[n]->syms[k]->hash) &&
                (symtab[n]->syms[k]->sectype != SYM_SEC_UNDEF))
            {
              symtab[i]->syms[j]->flags = (1<<0);
              symtab[i]->syms[j]->fileid = n;
              symtab[i]->syms[j]->symid = k;
              break;
            }
          }
        }
        for (n = 0; ((n < libs->nitems) && (symtab[i]->syms[j]->flags == 0)); n++)
        {
          uint32_t nsyms = 0, hash = 0/*, sectype = 0*/;
          input = elf_load (libs->item[n]);
          if (input == NULL) goto _quit;
          if (platforms[target_platform]->elf_chkfmt (input) == 0)
          {
            elf_close (input);
            goto _quit;
          }
          nsyms = platforms[target_platform]->elf_get_nsym (input);
          for (k = 0; k < nsyms; k++)
          {
            hash = sym_hash (platforms[target_platform]->elf_get_symstr (input, k));
            if (hash == symtab[i]->syms[j]->hash
                /* && sectype != SYM_SEC_UNDEF */)
            {
              symtab[i]->syms[j]->flags = (1<<1);
              symtab[i]->syms[j]->fileid = n;
              symtab[i]->syms[j]->symid = k;
              if (symtab_check_usym (symtab, nobj, i, hash))
                symtab[i]->syms[j]->hashid = symtab_get_usym_hashid (symtab, nobj, i, hash);
              else
              {
                symtab[i]->syms[j]->hashid = nexternal_symbols;
                nexternal_symbols++;
              }
              break;
            }
          }
          elf_close (input);
        }
      }
    }
  }
  /* check for undefined symbols. */
  for (i = 0; i < nobj; i++)
  {
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->flags == 0)
      {
        fprintf (stderr, "%s: undefined reference to `%s'.\n", __progname, symtab[i]->syms[j]->symname);
        goto _quit;
      }
    }
  }
  for (i = 0; i < 4; i++)
  {
    if (_rld_bit_is_clr(internal_symbols,i))
    {
      char *sym[] = {
        "__rld_debug_ptr", "__rld_import_hash",
        "__rld_import_addr", "__rld_num_imports"
      };
      fprintf (stderr, "%s: undefined reference to `%s'.\n", __progname,
          sym[i]); 
    }
  }

  /* create elf output file. */
  output = elf_create (filename);

  if (output == NULL) goto _quit;

  /* build headers. */
  vma_debug_ptr = platforms[target_platform]->elf_write_hdrs (output, libs);

  /* text section */
  for (i = 0; i < nobj; i++)
  {
    void *section = NULL;
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->type == SYM_TYPE_SEC &&
          symtab[i]->syms[j]->sectype == SYM_SEC_CODE &&
          symtab[i]->syms[j]->size != 0)
      {
        symtab[i]->syms[j]->foffset = lseek (output->fd, 0, SEEK_END);
        if (verbose)
          fprintf (stdout, "%s: merging %08X bytes from section '%s' at output file offset %08X.\n", __progname,symtab[i]->syms[j]->size, symtab[i]->syms[j]->symname, (uint32_t)symtab[i]->syms[j]->foffset);
        input = elf_load (argv[optind+i]);
        section = (void*)((unsigned long)input->mem + symtab[i]->syms[j]->offset);
        write (output->fd, section, symtab[i]->syms[j]->size);
        elf_close (input);
      }
    }
  }

  /* setup the jump table. */
  section_jmptab = lseek (output->fd, 0, SEEK_END);
  if (verbose)
    fprintf (stdout, "%s: jumptable start at file offset %08X:%u.\n", __progname, (uint32_t)section_jmptab, (uint32_t)section_jmptab);
  platforms[target_platform]->elf_build_jmptab (output, nexternal_symbols);

  /* data section. */
  for (i = 0; i < nobj; i++)
  {
    void *section = NULL;
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->type == SYM_TYPE_SEC &&
          symtab[i]->syms[j]->sectype == SYM_SEC_DATA &&
          symtab[i]->syms[j]->size != 0)
      {
        symtab[i]->syms[j]->foffset = lseek (output->fd, 0, SEEK_END);
        if (verbose)
          fprintf (stdout, "%s: merging %08X bytes from section '%s' at output file offset %08X.\n", __progname, symtab[i]->syms[j]->size, symtab[i]->syms[j]->symname, (uint32_t)symtab[i]->syms[j]->foffset);
        input = elf_load (argv[optind+i]);
        section = (void*)((unsigned long)input->mem + symtab[i]->syms[j]->offset);
        write (output->fd, section, symtab[i]->syms[j]->size);
        elf_close (input);
      }
    }
  }

  /* rodata section. */
  for (i = 0; i < nobj; i++)
  {
    void *section = NULL;
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->type == SYM_TYPE_SEC &&
          symtab[i]->syms[j]->sectype == SYM_SEC_RODATA &&
          symtab[i]->syms[j]->size != 0)
      {
        symtab[i]->syms[j]->foffset = lseek (output->fd, 0, SEEK_END);
        if (verbose)
          fprintf (stdout, "%s: merging section '%s' at output file offset %08X.\n", __progname, symtab[i]->syms[j]->symname, (uint32_t)symtab[i]->syms[j]->foffset);
        input = elf_load (argv[optind+i]);
        section = (void*)((unsigned long)input->mem + symtab[i]->syms[j]->offset);
        write (output->fd, section, symtab[i]->syms[j]->size);
        elf_close (input);
      }
    }
  }

  /* hash section. */
  section_hash = lseek (output->fd, 0, SEEK_END);
  if (verbose)
    fprintf (stdout, "%s: hash section start at file offset %08X:%u.\n", __progname, (uint32_t)section_hash, (uint32_t)section_hash);
  for (i = 0; i < nobj; i++)
  {
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->flags&0x2)
        write (output->fd, &symtab[i]->syms[j]->hash, sizeof(uint32_t));
    }
  }

  /* bss section. */
  uint32_t section_bss = lseek (output->fd, 0, SEEK_END);
  for (i = 0; i < nobj; i++)
  {
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if ((symtab[i]->syms[j]->sectype == SYM_SEC_BSS &&
          symtab[i]->syms[j]->type == SYM_TYPE_SEC) ||
          (symtab[i]->syms[j]->sectype == SYM_SEC_BSS &&
            symtab[i]->syms[j]->secid == (uint16_t)~0)) 
      {
        symtab[i]->syms[j]->foffset = section_bss + symtab[i]->syms[j]->size;
        section_bss_sz = section_bss_sz + symtab[i]->syms[j]->size;
      }
    }
  }

  section_addr = section_bss + section_bss_sz;
  if (verbose)
    fprintf (stdout, "%s: .bss section start at file offset %08X end at %08X\n", __progname, (uint32_t)section_bss, (uint32_t)section_bss+(uint32_t)section_bss_sz);

  platforms[target_platform]->elf_update_jmptab (output, section_jmptab,
      section_addr, nexternal_symbols);

  /* relocation. */
  for (i = 0; i < nobj; i++)
  {
    platforms[target_platform]->elf_reloc (output, symtab, i, argv[optind+i],
        section_hash, section_addr, section_jmptab,
        nexternal_symbols, vma_debug_ptr, verbose);
  }

  /* find the entry point. */
  for (i = 0; i < nobj; i++)
  {
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->hash == 0x5A8C726D
          && symtab[i]->syms[j]->sectype != SYM_SEC_UNDEF)
      {
        psym_t sec = symtab_get_symsec (symtab[i], symtab[i]->syms[j]->secid);
        entry_offset = sec->foffset+symtab[i]->syms[j]->offset;
      }
    }
  }

  platforms[target_platform]->elf_update (output, entry_offset, section_bss_sz+nexternal_symbols*4);

  if (compress)
  {
    /* TODO: use execv instead of system. */
    char cmd[1024];
    char stub[] = "#!/bin/sh\na=~/I;tail -n+3 $0|lzcat>$a;chmod +x $a;$a;rm $a;exit";
    snprintf (cmd, sizeof(cmd), "lzma -z --best %s; echo \'%s\' > %s; cat %s.lzma >> %s;chmod +x %s",
        filename, stub, filename, filename, filename, filename);
    system (cmd);
    snprintf (cmd, sizeof(cmd), "%s.lzma", filename);
    remove (cmd);
  }

  /* End of main(). */
_quit:
  symtab_destroy (symtab, nobj);
  lstr_destroy (libpath);
  lstr_destroy (libs);
  elf_close (output);
  return 0;
}
