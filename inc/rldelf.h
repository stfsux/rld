
#ifndef __RLDELF_H_
#define __RLDELF_H_

#define ELF_FILE_OBJ    (1<<0)
#define ELF_FILE_LIB    (1<<1)

/*-------------------------------------------------------------------*/
typedef struct _elf_file
{
  char *filename;
  int fd;
  void *mem;
  off_t size;
  uint8_t flags;
}elf_file_t, *pelf_file_t;

/*-------------------------------------------------------------------*/
typedef struct _platform
{
  uint8_t (*elf_chkfmt)(pelf_file_t elf);
  uint32_t (*elf_get_hdrsz)(void);
  uint32_t (*elf_get_nsym)(pelf_file_t elf);
  char* (*elf_get_symstr)(pelf_file_t elf, uint32_t id);
  uint32_t (*elf_get_symsec)(pelf_file_t elf, uint32_t id);
  uint32_t (*elf_get_symbind)(pelf_file_t elf, uint32_t id);
  uint32_t (*elf_get_symtype)(pelf_file_t elf, uint32_t id);
  uint32_t (*elf_get_symsz)(pelf_file_t elf, uint32_t id);
  uint32_t (*elf_get_symval)(pelf_file_t elf, uint32_t id);
  void (*elf_reloc)(pelf_file_t elf, psymtab_t *symtab, uint8_t n,
      char *filename, uint32_t section_hash, uint32_t section_addr,
      uint32_t jmptab, uint32_t nimports, uint32_t vma_debug_ptr);
  void* (*elf_get_sec)(pelf_file_t elf, char *secname);
  uint32_t (*elf_get_secsz)(pelf_file_t elf, char *secname);
  char* (*elf_get_secname)(pelf_file_t elf, uint32_t id);
  uint32_t (*elf_get_shsz)(pelf_file_t elf, char* secname);
  uint32_t (*elf_write_hdrs)(pelf_file_t elf, plstr_t libs);
  void (*elf_build_jmptab)(pelf_file_t elf, uint32_t nexternal_symbols);
  void (*elf_update_jmptab)(pelf_file_t elf, uint32_t section_jmptab,
      uint32_t section_addr, uint32_t nexternal_symbols);
  void (*elf_update)(pelf_file_t elf, uint32_t entrypoint, uint32_t bsssize);
}platform_t, *pplatform_t;

pelf_file_t  elf_load (char *filename);
pelf_file_t  elf_create (char *filename);
void         elf_close (pelf_file_t elf);

#endif

