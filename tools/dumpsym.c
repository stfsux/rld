#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>

#define USAGE "dumpsym FILE...\n"

void*
 elf_find_dyn (void* mem, unsigned int dt_type)
{
  Elf32_Phdr* ptr = NULL;
  unsigned int i;
  Elf32_Ehdr* ehdr = mem;
  ptr = (Elf32_Phdr*)((unsigned long)mem+ehdr->e_phoff);
  for (i = 0; i < ehdr->e_phnum; i++, ptr++)
  {
    if (ptr->p_type == PT_DYNAMIC)
    {
      printf ("[+] Dynamic data found...\n");
      break;
    }
  }
  Elf32_Dyn* dyn = (Elf32_Dyn*)((unsigned long)mem+ptr->p_offset);
  while (dyn->d_tag != DT_NULL)
  {
    if (dyn->d_tag == dt_type)
    {
      void* p = (unsigned int*)((unsigned long)mem+dyn->d_un.d_ptr);
      return p;
    }
    dyn++;
  }
  return NULL;
}

unsigned int
 sym_hash (str)
    char *str;
{
  unsigned int hash=0;
  char c;
  while((c=*str)!=0)
  {
    hash = ((c-hash+(hash<<6)) + (hash<<16));
    str++;
  }
  return hash;
}

int
 main (int argc, char* const argv[])
{
  int elf = 0;
  off_t sz = 0;
  void* mem = NULL;
  Elf32_Ehdr* ehdr = NULL;
  Elf32_Phdr *ptr = NULL;
  unsigned int i;
  unsigned int nsyms = 0;
  if (argc < 2)
  {
    fprintf (stderr, USAGE);
    return 0;
  }
  elf = open (argv[1], O_RDONLY);
  if (elf == -1)
  {
    fprintf (stderr, "%s: cannot read file `%s'.\n",
        argv[0], argv[1]);
    return 0;
  }
  printf ("[+] File `%s' opened...\n", argv[1]);
  sz = lseek (elf, 0, SEEK_END);
  lseek (elf, 0, SEEK_SET);
  mem = mmap ( NULL, sz, PROT_READ, MAP_PRIVATE, elf, 0);
  if (mem == MAP_FAILED)
  {
    fprintf (stderr, "%s: cannot map file `%s'.\n",
        argv[0], argv[1]);
    close (elf);
    return 0;
  }
  printf ("[+] File `%s' mapped in memory...\n", argv[1]);
  ehdr = (Elf32_Ehdr*)mem;
  unsigned int* dt_hash = elf_find_dyn (mem, DT_HASH);
  nsyms = *(dt_hash+1);
  printf ("[+] File `%s' containing %u symbols (%u)...\n", argv[1], nsyms, *(dt_hash));
  char* dt_strtab = elf_find_dyn (mem, DT_STRTAB);
  Elf32_Sym* symtab = elf_find_dyn (mem, DT_SYMTAB);
  printf ("+------------------------------------------+------------+------------+\n");
  for (i = 0; i < nsyms; i++, symtab++)
  {
    printf ("| %40s | 0x%08X | 0x%08X |\n", &dt_strtab[symtab->st_name], symtab->st_value,
        sym_hash (&dt_strtab[symtab->st_name]));
    printf ("+------------------------------------------+------------+------------+\n");
  }
_quit:
  munmap (mem, sz);
  close (elf);
  return 0;
}

