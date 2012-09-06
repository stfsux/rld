
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
  for (i = 0; i < argc; i++)
  {
    if (strlen(argv[i]) > 255)
      return 2;
  }

  /* create lists. */
  libpath = lstr_create ();
  libs    = lstr_create ();

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
        if (strncmp(optarg, "c", 1) == 0)
        {
          if (strncmp(optarg, "c=", 2) == 0)
          {
            if (strchr(optarg, '=') != NULL)
            {
              char *tmp = (char*)((unsigned long)strchr(optarg, '=')+(unsigned long)1);
              input = elf_load (tmp);
              if (input == NULL)
                goto _quit;
              lstr_add (libs, (char*)((unsigned long)strchr(optarg, '=')+(unsigned long)1));
              elf_close (input);
            }
          }
          else
          {
            fprintf (stderr, "%s: please specify the full path of your symbolic link to the libc (e.g.: -lc=/usr/local/libc.so.6).\n", __progname);
            goto _quit;
          }
        }
        else
        {
          if (rldfile_find (optarg, libpath, libs) == 0)
            goto _quit;
        }
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
      elf_close (input);
      goto _quit;
    }
    nsyms = platforms[target_platform]->elf_get_nsym (input);
    for (j = 1; j < nsyms; j++)
    {
      uint32_t hash = 0, sectype = 0, bind = 0;
      uint32_t type = 0, offset = 0;
      char *symname = NULL;
      sectype = platforms[target_platform]->elf_get_symsec (input, j);
      bind = platforms[target_platform]->elf_get_symbind (input, j);
      type = platforms[target_platform]->elf_get_symtype (input, j);
      if (sectype == SYM_SEC_ABS || type == SYM_TYPE_FILE)
        continue;
      if (type == SYM_TYPE_SEC)
      {
        symname = platforms[target_platform]->elf_get_secname (input, j);
        if ((strncmp (symname, ".text", 5) != 0)
            && (strncmp (symname, ".bss", 4) != 0)
            && (strncmp (symname, ".rodata", 7) != 0)
            && (strncmp (symname, ".data", 5) != 0))
          continue;
      }
      else
        symname = platforms[target_platform]->elf_get_symstr (input, j);
      hash = sym_hash (symname);
      offset = platforms[target_platform]->elf_get_symval (input, j);
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
      if (symtab_add_sym (symtab[i], symname, hash, sectype, bind, type, j, offset) == 0)
      {
        elf_close (input);
        goto _quit;
      }
      if (verbose)
        fprintf (stdout, "%s: symname: %-30s object: %-10s hash: %08X sectype: %08X bind: %08X type: %08X\n", __progname, symname, argv[optind+i], hash, sectype, bind, type);
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
          case 0x5C77F413:
          case 0xB144F4AD:
          case 0x183369B6:
          case 0xEBCAB480:
            symtab[i]->syms[j]->flags = (1<<2);
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

  /* create elf output file. */
  output = elf_create (filename);

  if (output == NULL) goto _quit;

  /* build headers. */
  vma_debug_ptr = platforms[target_platform]->elf_write_hdrs (output, libs);

  /* text section */
  for (i = 0; i < nobj; i++)
  {
    void *section = NULL;
    uint32_t size = 0;
    symtab[i]->code = lseek (output->fd, 0, SEEK_END);
    input = elf_load (argv[optind+i]);
    platforms[target_platform]->elf_chkfmt (input);
    section = platforms[target_platform]->elf_get_sec (input, ".text");
    if (section == NULL)
    {
      fprintf (stderr, "%s:%s: warning: there is no .text section in this file.\n", __progname, argv[optind+i]);
      elf_close (input);
      continue;
    }
    size = platforms[target_platform]->elf_get_secsz (input, ".text");
    write (output->fd, section, size);
    elf_close (input);
  }

  /* setup the jump table. */
  section_jmptab = lseek (output->fd, 0, SEEK_END);
  platforms[target_platform]->elf_build_jmptab (output, nexternal_symbols);

  /* data section. */
  for (i = 0; i < nobj; i++)
  {
    void *section = NULL;
    uint32_t size = 0;
    symtab[i]->data = lseek (output->fd, 0, SEEK_END);
    input = elf_load (argv[optind+i]);
    platforms[target_platform]->elf_chkfmt (input);
    section = platforms[target_platform]->elf_get_sec (input, ".data");
    if (section == NULL)
    {
      fprintf (stderr, "%s:%s: warning: there is no .data section in this file.\n", __progname, argv[optind+i]);
      elf_close (input);
      continue;
    }
    size = platforms[target_platform]->elf_get_secsz (input, ".data");
    write (output->fd, section, size);
    elf_close (input);
  }

  /* rodata section. */
  for (i = 0; i < nobj; i++)
  {
    void *section = NULL;
    uint32_t size = 0;
    symtab[i]->rodata = lseek (output->fd, 0, SEEK_END);
    input = elf_load (argv[optind+i]);
    platforms[target_platform]->elf_chkfmt (input);
    section = platforms[target_platform]->elf_get_sec (input, ".rodata");
    if (section == NULL)
    {
      fprintf (stderr, "%s:%s: warning: there is no .rodata section in this file.\n", __progname, argv[optind+i]);
      elf_close (input);
      continue;
    }
    size = platforms[target_platform]->elf_get_secsz (input, ".rodata");
    write (output->fd, section, size);
    elf_close (input);
  }

  /* hash section. */
  section_hash = lseek (output->fd, 0, SEEK_END);
  for (i = 0; i < nobj; i++)
  {
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->flags&0x2)
        write (output->fd, &symtab[i]->syms[j]->hash, sizeof(uint32_t));
    }
  }

  /* bss section. */
  for (i = 0; i < nobj; i++)
  {
    uint32_t size = 0;
    symtab[i]->bss = lseek (output->fd, 0, SEEK_END) + section_bss_sz;
    input = elf_load (argv[optind+i]);
    platforms[target_platform]->elf_chkfmt (input);
    size = platforms[target_platform]->elf_get_secsz (input, ".bss");
    section_bss_sz = section_bss_sz + size;
    elf_close (input);
  }

  section_addr = symtab[0]->bss + section_bss_sz;

  platforms[target_platform]->elf_update_jmptab (output, section_jmptab,
      section_addr, nexternal_symbols);

  /* relocation. */
  for (i = 0; i < nobj; i++)
  {
    platforms[target_platform]->elf_reloc (output, symtab, i, argv[optind+i],
        section_hash, section_addr, section_jmptab,
        nexternal_symbols, vma_debug_ptr);
  }

  /* find the entry point. */
  for (i = 0; i < nobj; i++)
  {
    for (j = 0; j < symtab[i]->nsyms; j++)
    {
      if (symtab[i]->syms[j]->hash == 0x5A8C726D
          && symtab[i]->syms[j]->sectype != SYM_SEC_UNDEF)
      {
        entry_offset = symtab[i]->code+symtab[i]->syms[j]->offset;
      }
    }
  }

  platforms[target_platform]->elf_update (output, entry_offset, section_bss_sz+nexternal_symbols*4);

  if (compress)
  {
    char cmd[1024];
    char stub[] = "a=~/I;tail -n+2 \\$0|lzcat>\\$a;chmod +x \\$a;\\$a;rm \\$a;exit";
    snprintf (cmd, sizeof(cmd), "lzma -z --best %s; echo \"%s\" > %s; cat %s.lzma >> %s;chmod +x %s",
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
