#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include "rld.h"
#include "rldlist.h"
#include "rldsym.h"
#include "rldelf.h"

#define _X86_INTERP_OFFSET  (sizeof(ElfN_Ehdr)+3*sizeof(ElfN_Phdr))
#define _X86_JMPTAB_SIZE    (6)

#define elf_set_base(x) (vma_base+x)
#define elf_set_ehdr(member,data,size)\
{\
	lseek ( elf->fd, offsetof (ElfN_Ehdr, member) , SEEK_SET );\
	write ( elf->fd, data, size );\
	lseek ( elf->fd, 0, SEEK_END );\
}
#define elf_set_nphdr(n,member,data,size)\
{\
	lseek ( elf->fd, sizeof (ElfN_Ehdr)+n*sizeof(ElfN_Phdr)+offsetof (ElfN_Phdr, member) , SEEK_SET );\
	write ( elf->fd, data, size );\
	lseek ( elf->fd, 0, SEEK_END );\
}

typedef Elf32_Ehdr  ElfN_Ehdr;
typedef Elf32_Shdr  ElfN_Shdr;
typedef Elf32_Phdr  ElfN_Phdr;
typedef Elf32_Dyn   ElfN_Dyn;
typedef Elf32_Sym   ElfN_Sym;
typedef Elf32_Addr  ElfN_Addr;
typedef Elf32_Rel   ElfN_Rel;

uint32_t vma_base = 0x08048000;

/*-------------------------------------------------------------------*/
uint8_t
 _x86_elf_chkfmt (pelf_file_t elf)
{
  ElfN_Ehdr *ehdr = elf->mem;
  if (ehdr->e_machine != EM_386)
  {
    fprintf (stderr, "%s: file `%s' doesn't match with target architecture.\n", __progname, elf->filename);
    return 0;
  }
  if (ehdr->e_type == ET_REL)
    elf->flags |= ELF_FILE_OBJ;
  else if (ehdr->e_type == ET_DYN)
    elf->flags |= ELF_FILE_LIB;
  else
  {
    fprintf (stderr, "%s: unsupported file format `%s'.\n", __progname, elf->filename);
    return 0;
  }
  return 1;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_hdrsz (void)
{
  return sizeof(ElfN_Ehdr)+3*sizeof(ElfN_Phdr);
}

/*-------------------------------------------------------------------*/
ElfN_Shdr*
 _x86_elf_get_shdr (pelf_file_t elf, uint32_t type)
{
  ElfN_Ehdr *ehdr = elf->mem;
  ElfN_Shdr *shdr = NULL;
  shdr = (ElfN_Shdr*)((unsigned long)ehdr->e_shoff+(unsigned long)ehdr);
  unsigned int i;
  for (i = 0; i < ehdr->e_shnum; i++,shdr++)
  {
    if (i != ehdr->e_shstrndx && shdr->sh_type == type) break;
  }
  return shdr;
}

/*-------------------------------------------------------------------*/
ElfN_Dyn*
 _x86_elf_get_dyn (pelf_file_t elf, uint32_t d_tag)
{
  ElfN_Ehdr *ehdr = elf->mem;
  ElfN_Phdr *phdr = NULL;
  ElfN_Dyn *dyn = NULL;
  uint32_t i;
  phdr = (ElfN_Phdr*)(
    (unsigned long)ehdr+(unsigned long)ehdr->e_phoff
    );
  for (i = 0; i < ehdr->e_phnum; i++,phdr++)
  {
    if (phdr->p_type == PT_DYNAMIC)
      dyn = (ElfN_Dyn*)(
        (unsigned long)ehdr+(unsigned long)phdr->p_offset
        );
  }
  if (dyn == NULL)
    return NULL;
  while (dyn->d_tag != DT_NULL)
  {
    if (dyn->d_tag == (int32_t)d_tag)
      return dyn;
    dyn++;
  }
  return NULL;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_nsym (pelf_file_t elf)
{
  ElfN_Dyn *dyn = NULL;
  if (elf->flags&ELF_FILE_LIB)
  {
    if ((dyn=_x86_elf_get_dyn(elf, DT_HASH)) != NULL)
    {
      uint32_t *hashtable = (uint32_t*)((unsigned long)elf->mem+(unsigned long)dyn->d_un.d_ptr);
      return hashtable[1];
    }
    else if (dyn == NULL)
    {
      uint32_t nsym = 0;
      uint32_t *ptr = NULL, *buckets = NULL, *chains = NULL;
      uint32_t nbuckets, symidx, bitmsk, i;

      dyn = _x86_elf_get_dyn (elf, DT_GNU_HASH);
      if (dyn == NULL)
      {
        fprintf (
          stderr,
          "%s: cannot find dynamic .gnu.hash entry in `%s'.\n",
          __progname,
          elf->filename
          );
        return 0;
      }

      ptr = (uint32_t*)(
        (unsigned long)elf->mem+(unsigned long)dyn->d_un.d_ptr
        );
      nbuckets = (uint32_t)*ptr;
      symidx = (uint32_t)ptr[1];
      bitmsk = (uint32_t)ptr[2];
      buckets = &ptr[4+bitmsk];
      chains = &ptr[nbuckets+4+bitmsk];

      for ( i = 0; i < nbuckets ; i++ )
      {
        if ( buckets[i] != 0 )
        {
          uint32_t length = 1;
          uint32_t off;
          for ( off = buckets[i]-symidx; (chains[off]&1)==0; ++off )
            ++length;
          nsym += length;
        }
      }

      nsym += symidx;
      return nsym;

    }
  }
  else
  {
    ElfN_Shdr *symtab = _x86_elf_get_shdr (elf, SHT_SYMTAB);
    return symtab->sh_size/sizeof(ElfN_Sym);
  }
  return 0;
}

/*-------------------------------------------------------------------*/
char*
 _x86_elf_get_strsym (pelf_file_t elf, uint32_t id)
{
  ElfN_Sym *sym = NULL;
  if (elf->flags&ELF_FILE_OBJ)
  {
    ElfN_Shdr *shdr_symtab = _x86_elf_get_shdr (elf, SHT_SYMTAB);
    ElfN_Shdr *shdr_strtab = _x86_elf_get_shdr (elf, SHT_STRTAB);
    ElfN_Sym *symtab = NULL;
    char *strtab = NULL;
    symtab = (ElfN_Sym*)((unsigned long)elf->mem+(unsigned long)shdr_symtab->sh_offset);
    sym = &symtab[id];
    strtab = (char*)((unsigned long)elf->mem+(unsigned long)shdr_strtab->sh_offset);
    return &strtab[sym->st_name];
  }
  else if (elf->flags&ELF_FILE_LIB)
  {
    ElfN_Shdr *shdr_dynsym = _x86_elf_get_shdr (elf, SHT_DYNSYM);
    ElfN_Shdr *shdr_strtab = _x86_elf_get_shdr (elf, SHT_STRTAB);
    char *strtab = NULL;
    ElfN_Sym *dynsym = NULL;
    dynsym = (ElfN_Sym*)((unsigned long)elf->mem+(unsigned long)shdr_dynsym->sh_offset);
    sym = &dynsym[id];
    strtab = (char*)((unsigned long)elf->mem+(unsigned long)shdr_strtab->sh_offset);
    return &strtab[sym->st_name];
  }
  return NULL;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_shsz (pelf_file_t elf, char* secname)
{
  uint32_t i;
  ElfN_Ehdr *ehdr = elf->mem;
  ElfN_Shdr *shdr = (ElfN_Shdr*)((unsigned long)ehdr+(unsigned long)ehdr->e_shoff);
	char* strtab = (char*)((unsigned long)ehdr+(unsigned long)((ElfN_Shdr*)&shdr[ehdr->e_shstrndx])->sh_offset);  
  for (i = 0; i < ehdr->e_shnum; i++,shdr++)
  {
    if (strncmp (secname, &strtab[shdr->sh_name], strlen(secname)) == 0)
    {
      return shdr->sh_size;
    }
  }
  return 0;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_symtype (pelf_file_t elf, uint32_t id)
{
  ElfN_Sym *sym = NULL;
  if (elf->flags&ELF_FILE_OBJ)
  {
    ElfN_Shdr *shdr = _x86_elf_get_shdr (elf, SHT_SYMTAB);
    ElfN_Sym *symtab = NULL;
    symtab = (ElfN_Sym*)((unsigned long)elf->mem+(unsigned long)shdr->sh_offset);
    sym = &symtab[id];
    switch (ELF32_ST_TYPE(sym->st_info))
    {
      case STT_NOTYPE:
        return SYM_TYPE_NONE;

      case STT_OBJECT:
        return SYM_TYPE_OBJ;

      case STT_FUNC:
        return SYM_TYPE_FUNC;

      case STT_SECTION:
        return SYM_TYPE_SEC;

      case STT_FILE:
        return SYM_TYPE_FILE;
    }
  }
  return SYM_TYPE_NONE;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_symbind (pelf_file_t elf, uint32_t id)
{
  ElfN_Sym *sym = NULL;
  if (elf->flags&ELF_FILE_OBJ)
  {
    ElfN_Shdr *shdr = _x86_elf_get_shdr (elf, SHT_SYMTAB);
    ElfN_Sym *symtab = NULL;
    symtab = (ElfN_Sym*)((unsigned long)elf->mem+(unsigned long)shdr->sh_offset);
    sym = &symtab[id];
    switch (ELF32_ST_BIND(sym->st_info))
    {
      case STB_GLOBAL:
        return SYM_BIND_GLOBAL;

      case STB_LOCAL:
        return SYM_BIND_LOCAL;

      case STB_WEAK:
        return SYM_BIND_WEAK;
    }
  }
  return SYM_BIND_UNDEF;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_symsec (pelf_file_t elf, uint32_t id)
{
  ElfN_Sym *sym = NULL;
  char *strtab = NULL, *secname = NULL;
  ElfN_Ehdr *ehdr = elf->mem;
  ElfN_Shdr *shdr = (ElfN_Shdr*)((unsigned long)ehdr+(unsigned long)ehdr->e_shoff);
  if (elf->flags&ELF_FILE_OBJ)
  {
    ElfN_Shdr *shsymtab = _x86_elf_get_shdr (elf, SHT_SYMTAB);
    ElfN_Sym *symtab = NULL;
    symtab = (ElfN_Sym*)((unsigned long)ehdr+(unsigned long)shsymtab->sh_offset);
    strtab = (char*)((unsigned long)ehdr+(unsigned long)((ElfN_Shdr*)&shdr[ehdr->e_shstrndx])->sh_offset);
    sym = &symtab[id];
    switch (sym->st_shndx)
    {
      case SHN_ABS:
        return SYM_SEC_ABS;

      case SHN_UNDEF:
        return SYM_SEC_UNDEF;

      case SHN_COMMON:
        return SYM_SEC_BSS;

      default:
        secname = &strtab[((ElfN_Shdr*)&shdr[sym->st_shndx])->sh_name];
        if (strstr (secname, ".text") != NULL) return SYM_SEC_CODE;
        else if (strstr (secname, ".data") != NULL) return SYM_SEC_DATA;
        else if (strstr (secname, ".rodata") != NULL) return SYM_SEC_RODATA;
        else if (strstr (secname, ".bss") != NULL) return SYM_SEC_BSS;
        break;
    }
  }
  return SYM_SEC_UNDEF;
}

/*-------------------------------------------------------------------*/
uint16_t
 _x86_elf_get_symsecid (pelf_file_t elf, uint32_t id)
{
  ElfN_Sym *sym = NULL;
  char *strtab = NULL;
  ElfN_Ehdr *ehdr = elf->mem;
  ElfN_Shdr *shdr = (ElfN_Shdr*)((unsigned long)ehdr+(unsigned long)ehdr->e_shoff);
  if (elf->flags&ELF_FILE_OBJ)
  {
    ElfN_Shdr *shsymtab = _x86_elf_get_shdr (elf, SHT_SYMTAB);
    ElfN_Sym *symtab = NULL;
    symtab = (ElfN_Sym*)((unsigned long)ehdr+(unsigned long)shsymtab->sh_offset);
    strtab = (char*)((unsigned long)ehdr+(unsigned long)((ElfN_Shdr*)&shdr[ehdr->e_shstrndx])->sh_offset);
    sym = &symtab[id];
    if (sym->st_shndx == SHN_COMMON)
    {
      /*
      unsigned int i;
      for (i = 0; i < ehdr->e_shnum; i++, shdr++)
      {
        char* secname = &strtab[shdr->sh_name];
        if (strstr (secname, ".bss") != NULL)
          return i;
      }
      */
      return ~0;
    }
    return sym->st_shndx;
  }
  return 0;
}

/*-------------------------------------------------------------------*/
char*
 _x86_elf_get_secname (pelf_file_t elf, uint32_t id)
{
  ElfN_Sym *sym = NULL;
  char *strtab = NULL, *secname = NULL;
  ElfN_Ehdr *ehdr = elf->mem;
  ElfN_Shdr *shdr = (ElfN_Shdr*)((unsigned long)ehdr+(unsigned long)ehdr->e_shoff);
  if (elf->flags&ELF_FILE_OBJ)
  {
    ElfN_Shdr *shsymtab = _x86_elf_get_shdr (elf, SHT_SYMTAB);
    ElfN_Sym *symtab = NULL;
    symtab = (ElfN_Sym*)((unsigned long)ehdr+(unsigned long)shsymtab->sh_offset);
    strtab = (char*)((unsigned long)ehdr+(unsigned long)((ElfN_Shdr*)&shdr[ehdr->e_shstrndx])->sh_offset);
    sym = &symtab[id];
  }
  switch (sym->st_shndx)
  {
    case SHN_ABS:
      return NULL;

    case SHN_UNDEF:
      return NULL;

    case SHN_COMMON:
      return ".bss";

    default:
      secname = &strtab[((ElfN_Shdr*)&shdr[sym->st_shndx])->sh_name];
      break;
  }
  return secname;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_symsz (pelf_file_t elf, uint32_t id)
{
  ElfN_Sym *sym = NULL;
  if (elf->flags&ELF_FILE_OBJ)
  {
    ElfN_Shdr *shdr = _x86_elf_get_shdr (elf, SHT_SYMTAB);
    ElfN_Sym *symtab = NULL;
    symtab = (ElfN_Sym*)((unsigned long)elf->mem+(unsigned long)shdr->sh_offset);
    sym = &symtab[id];
  }
  else if (elf->flags&ELF_FILE_LIB)
  {
  }
  return sym->st_size;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_symval (elf, id)
  pelf_file_t elf;
  uint32_t id;
{
  ElfN_Sym *sym = NULL;
  ElfN_Ehdr *ehdr = elf->mem;
  ElfN_Shdr *shsymtab = _x86_elf_get_shdr (elf, SHT_SYMTAB);
  ElfN_Sym *symtab = NULL;
  symtab = (ElfN_Sym*)((unsigned long)ehdr+(unsigned long)shsymtab->sh_offset);
  sym = &symtab[id];
  return sym->st_value; 
}

/*-------------------------------------------------------------------*/
void*
 _x86_elf_get_sec (pelf_file_t elf, char* secname)
{
  ElfN_Ehdr * ehdr = elf->mem;
  ElfN_Shdr * shdr = (ElfN_Shdr*)((unsigned long)(ehdr->e_shoff)+(unsigned long)ehdr);
  char* shstrtab = (char*)((unsigned long)ehdr+(unsigned long)((ElfN_Shdr*)&shdr[ehdr->e_shstrndx])->sh_offset);
  uint32_t i;
  for ( i = 0; i < ehdr->e_shnum; i++,shdr++ )
  {
    if ( !strncmp ( secname, &shstrtab[shdr->sh_name], strlen(secname) ) )
      return (void*)((unsigned long)ehdr+(unsigned long)shdr->sh_offset);
  }
  return NULL;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_secsz (pelf_file_t elf, char* secname)
{
  ElfN_Ehdr * ehdr = elf->mem;
  ElfN_Shdr * shdr = (ElfN_Shdr*)((unsigned long)(ehdr->e_shoff)+(unsigned long)ehdr);
  /* IOCC-style and unsecure code FTW! */
  char* shstrtab = (char*)((unsigned long)ehdr+(unsigned long)((ElfN_Shdr*)&shdr[ehdr->e_shstrndx])->sh_offset);
  uint32_t i;
  for ( i = 0; i < ehdr->e_shnum; i++,shdr++ )
  {
    if ( !strncmp ( secname, &shstrtab[shdr->sh_name], strlen(secname) ) )
      return (uint32_t)(shdr->sh_size);
  }
  return 0;
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_get_secoff (pelf_file_t elf, char* secname)
{
  ElfN_Ehdr * ehdr = elf->mem;
  ElfN_Shdr * shdr = (ElfN_Shdr*)((unsigned long)(ehdr->e_shoff)+(unsigned long)ehdr);
  /* IOCC-style and unsecure code FTW! */
  char* shstrtab = (char*)((unsigned long)ehdr+(unsigned long)((ElfN_Shdr*)&shdr[ehdr->e_shstrndx])->sh_offset);
  uint32_t i;
  for ( i = 0; i < ehdr->e_shnum; i++,shdr++ )
  {
    if ( !strncmp ( secname, &shstrtab[shdr->sh_name], strlen(secname) ) )
      return (uint32_t)(shdr->sh_offset);
  }
  return 0;
}

/*-------------------------------------------------------------------*/
void
 _x86_elf_reloc (pelf_file_t elf, psymtab_t *symtab, uint8_t n,
     char *filename, uint32_t section_hash, uint32_t section_addr,
     uint32_t jmptab, uint32_t nimports, uint32_t vma_debug_ptr)
{
  pelf_file_t input = NULL;
  ElfN_Rel *rel = NULL;
  unsigned int i, j, k;
  uint32_t vma_section_hash = elf_set_base (section_hash);
  uint32_t vma_section_addr = elf_set_base (section_addr);
  ElfN_Ehdr *ehdr = NULL;
  ElfN_Shdr *shdr = NULL, *shdrtab = NULL;
  char *strsectab = NULL;
  char *secname = NULL;
  char *sectarget = NULL;
  uint32_t secsymid = 0;

  input = elf_load (filename);
  _x86_elf_chkfmt (input);

  ehdr = input->mem;
  shdr = shdrtab = (ElfN_Shdr*)((unsigned long)ehdr+(unsigned long)ehdr->e_shoff);
  strsectab = (char*)((unsigned long)ehdr+(unsigned long)((ElfN_Shdr*)&shdr[ehdr->e_shstrndx])->sh_offset);

  for (k = 0; k < ehdr->e_shnum; k++, shdr++)
  {
    if (shdr->sh_type == SHT_REL)
    {
      secname = &strsectab[shdr->sh_name];
      sectarget = strstr (secname, ".rel");
      
      if (sectarget == NULL)
        continue;
      else
        sectarget = (char*)((unsigned long)sectarget+4);

      for (i = 0; i < symtab[n]->nsyms; i++)
      {
        if (!strcmp(sectarget, symtab[n]->syms[i]->symname))
        {
          secsymid = i;
          break;
        }
      }

      rel = (ElfN_Rel*)((unsigned long)input->mem+(unsigned long)shdr->sh_offset);

      for (i = 0; i < shdr->sh_size/sizeof(ElfN_Rel); i++,rel++)
      {
        psym_t usym = NULL, sym = NULL;
        uint32_t src = 0, dst = 0, relocaddr = 0;
        uint32_t r_addend = 0;

        for (j = 0; j < symtab[n]->nsyms; j++)
        {
          if (symtab[n]->syms[j]->symrid == ELF32_R_SYM(rel->r_info))
          {
            usym = symtab[n]->syms[j];
            break;
          }
        }
        src = rel->r_offset+symtab[n]->syms[secsymid]->foffset;

        lseek (elf->fd, src, SEEK_SET);
        read (elf->fd, &r_addend, sizeof(uint32_t));
        lseek (elf->fd, src, SEEK_SET);

        
        if (r_addend == 0 || r_addend == (uint32_t)-4 ||
            usym->secid == 0xFFFF)
        {
          if ((usym->type != SYM_TYPE_SEC) && (usym->flags&(1<<0)))
          {
            if (usym->sectype == SYM_SEC_UNDEF)
            {
              sym = symtab[usym->fileid]->syms[usym->symid];
              r_addend += sym->offset;
            }
            else
            {
              r_addend += usym->offset;
            }
          }
        }

        printf ("relocating '%-20s' ", usym->symname);
        if (usym->flags&(1<<0))
        {
          /* symbol is located in other object file. */
          if (usym->sectype == SYM_SEC_UNDEF)
          {
            sym = symtab[usym->fileid]->syms[usym->symid];
            psym_t sec = symtab_get_symsec (symtab[usym->fileid], sym->secid);
            dst = sec->foffset;
          }
          /* symbol is located in the current object file. */
          else
          {
            psym_t sec = symtab_get_symsec (symtab[n], usym->secid);
            dst = sec->foffset;
          }
          
          switch (ELF32_R_TYPE(rel->r_info))
          {
            case R_386_32:
              relocaddr = elf_set_base(dst+r_addend);
              break;

            case R_386_PC32:
              relocaddr = dst+r_addend-src;
              break;
          }
          printf (" at offset %08X:%08X.\n", src, relocaddr);
          write (elf->fd, &relocaddr, sizeof(uint32_t));
        }
        else if (usym->flags&(1<<1))
        {
          switch (ELF32_R_TYPE(rel->r_info))
          {
            case R_386_PC32:
                relocaddr = jmptab + usym->hashid * _X86_JMPTAB_SIZE;
                relocaddr = relocaddr+r_addend - (src);
                break;
          }
          printf (" at offset %08X with location %08X.\n", src, relocaddr);
          write (elf->fd, &relocaddr, sizeof(uint32_t));
        }
        else if (usym->flags&(1<<2))
        {
          printf ("\n");
          /* __rld_debug_ptr */
          if (usym->hash == 0xB144F4AD)
            write (elf->fd, &vma_debug_ptr, sizeof(uint32_t));
          /* __rld_import_hash */
          else if (usym->hash == 0x5C77F413)
            write (elf->fd, &vma_section_hash, sizeof(uint32_t));
          /* __rld_import_addr */
          else if (usym->hash == 0x183369B6)
            write (elf->fd, &vma_section_addr, sizeof(uint32_t));
          /* __rld_num_imports */
          else if (usym->hash == 0xEBCAB480)
            write (elf->fd, &nimports, sizeof(uint32_t));
        }
      }
    }
  }
  elf_close (input);
}

/*-------------------------------------------------------------------*/
uint32_t
 _x86_elf_write_hdrs (pelf_file_t elf, plstr_t libs)
{
  uint32_t i, off = 1;
  const char interp[] = "/lib/ld-linux.so.2\0";
  uint32_t tmp = 0, debug = 0;
  ElfN_Ehdr ehdr = {
    .e_ident = {
      0x7F, 'E' , 'L' , 'F' , ELFCLASS32, ELFDATA2LSB,
      EV_CURRENT, ELFOSABI_NONE, 0, 0, 0, 0,
      0, 0, 0, 0
    } ,
    .e_type       = ET_EXEC ,
    .e_machine    = EM_386 ,
    .e_version    = EV_CURRENT ,
    .e_entry      = 0x00000000 ,
    .e_phoff      = sizeof ( ElfN_Ehdr ) ,
    .e_shoff      = 0x00000000 ,
    .e_flags      = 0x00000000 ,
    .e_ehsize     = sizeof ( ElfN_Ehdr ) ,
    .e_phentsize  = sizeof ( ElfN_Phdr ) ,
    .e_phnum      = 0x00000003 ,
    .e_shentsize  = 0x0000 ,
    .e_shnum      = 0x0000 ,
    .e_shstrndx   = 0x0000
  };
  ElfN_Phdr phdr[] = {
    {
      .p_type   = PT_INTERP ,
      .p_offset = _X86_INTERP_OFFSET ,
      .p_vaddr  = elf_set_base(_X86_INTERP_OFFSET) ,
      .p_paddr  = 0x00000000 ,
      .p_filesz = strlen(interp)+1 ,
      .p_memsz  = strlen(interp)+1 ,
      .p_flags  = PF_R ,
      .p_align  = 0x00000000
    } ,
    {
      .p_type   = PT_LOAD ,
      .p_offset = 0x00000000 ,
      .p_vaddr  = elf_set_base(0) ,
      .p_paddr  = 0x00000000 ,
      .p_filesz = 0x00000000 ,
      .p_memsz  = 0x00000000 ,
      .p_flags  = PF_R|PF_W|PF_X ,
      .p_align  = 0x00000100

    } ,
    {
      .p_type   = PT_DYNAMIC ,
      .p_offset = 0x00000000 ,
      .p_vaddr  = 0x00000000 ,
      .p_paddr  = 0x00000000 ,
      .p_filesz = 0x00000000 ,
      .p_memsz  = 0x00000000 ,
      .p_flags  = PF_R ,
      .p_align  = 0x00000004

    } ,
  };
  write (elf->fd, &ehdr, sizeof(ElfN_Ehdr));
  for (i = 0; i < 3; i++)
    write (elf->fd, &phdr[i], sizeof(ElfN_Phdr));
  write (elf->fd, interp, strlen(interp)+1);
  write (elf->fd, &tmp, sizeof(char));
  for (i = 0; i < libs->nitems; i++)
  {
    char *libname = NULL;
    if (strrchr (libs->item[i], '/') != NULL)
      libname = (char*)((unsigned long)strrchr (libs->item[i], '/')+1);
    else
      libname = libs->item[i];
    write (elf->fd, libname, strlen (libname)+1);
  }
  tmp = lseek (elf->fd, 0, SEEK_END);
  elf_set_nphdr (2, p_offset, &tmp, sizeof(uint32_t));
  tmp = elf_set_base (tmp);
  elf_set_nphdr (2, p_vaddr, &tmp, sizeof(uint32_t));
  tmp = libs->nitems*4*2+24;
  elf_set_nphdr (2, p_memsz, &tmp, sizeof(uint32_t));
  elf_set_nphdr (2, p_filesz, &tmp, sizeof(uint32_t));
  for (i = 0; i < libs->nitems; i++)
  {
    char *libname = NULL;
    if (strrchr (libs->item[i], '/') != NULL)
      libname = (char*)((unsigned long)strrchr (libs->item[i], '/')+1);
    else
      libname = libs->item[i];
    tmp = DT_NEEDED;
    write (elf->fd, &tmp, sizeof(uint32_t));
    write (elf->fd, &off, sizeof(uint32_t));
    off = off + strlen (libname) + 1;
  }
  tmp = DT_STRTAB;
  write (elf->fd, &tmp, sizeof(uint32_t));
  tmp = elf_set_base (_X86_INTERP_OFFSET+strlen(interp)+1);
  write (elf->fd, &tmp, sizeof(uint32_t));
  tmp = DT_SYMTAB;
  write (elf->fd, &tmp, sizeof(uint32_t));
  tmp = 0;
  write (elf->fd, &tmp, sizeof(uint32_t));
  tmp = DT_DEBUG;
  write (elf->fd, &tmp, sizeof(uint32_t));
  debug = lseek (elf->fd, 0, SEEK_END);
  tmp = 0;
  write (elf->fd, &tmp, sizeof(uint32_t));
  tmp = DT_NULL;
  write (elf->fd, &tmp, sizeof(uint32_t));
  tmp = lseek (elf->fd, 0, SEEK_END);
  elf_set_nphdr (1, p_filesz, &tmp, sizeof(uint32_t));
  elf_set_nphdr (1, p_memsz, &tmp, sizeof(uint32_t));
  return elf_set_base(debug);
}

/*-------------------------------------------------------------------*/
void 
 _x86_elf_build_jmptab (pelf_file_t elf, uint32_t nexternal_symbols)
{
  uint32_t i, zero = 0;
  char instr_jmpfar[] = { 0xFF, 0X25 };
  for (i = 0; i < nexternal_symbols; i++)
  {
    write (elf->fd, instr_jmpfar, 2*sizeof(char));
    write (elf->fd, &zero, sizeof(uint32_t));
  }
}

/*-------------------------------------------------------------------*/
void 
 _x86_elf_update_jmptab (pelf_file_t elf, uint32_t section_jmptab,
     uint32_t section_addr, uint32_t nexternal_symbols)
{
  uint32_t i, vma_addr = elf_set_base (section_addr);
  off_t savefp = 0;
  savefp = lseek (elf->fd, 0, SEEK_CUR);
  lseek (elf->fd, section_jmptab, SEEK_SET);
  for (i = 0; i < nexternal_symbols; i++)
  {
    lseek (elf->fd, 2, SEEK_CUR);
    write (elf->fd, &vma_addr, sizeof(uint32_t));
    vma_addr = vma_addr + sizeof(uint32_t);
  }
  lseek (elf->fd, savefp, SEEK_SET);
}

/*-------------------------------------------------------------------*/
void
 _x86_elf_update (pelf_file_t elf, uint32_t entrypoint, uint32_t bsssize)
{
  uint32_t filesz = 0;
  entrypoint = elf_set_base(entrypoint);
  elf_set_ehdr(e_entry, &entrypoint, sizeof(uint32_t));
  filesz = lseek(elf->fd, 0, SEEK_END);
  elf_set_nphdr(1, p_filesz, &filesz, sizeof(uint32_t));
  filesz = filesz + bsssize;
  elf_set_nphdr(1, p_memsz, &filesz, sizeof(uint32_t));
}


platform_t x86 = 
{
  .elf_get_hdrsz      = _x86_elf_get_hdrsz,
  .elf_chkfmt         = _x86_elf_chkfmt,
  .elf_get_nsym       = _x86_elf_get_nsym,
  .elf_get_symstr     = _x86_elf_get_strsym,
  .elf_get_symsec     = _x86_elf_get_symsec,
  .elf_get_symsecid   = _x86_elf_get_symsecid,
  .elf_get_symbind    = _x86_elf_get_symbind,
  .elf_get_symtype    = _x86_elf_get_symtype,
  .elf_get_symsz      = _x86_elf_get_symsz,
  .elf_get_symval     = _x86_elf_get_symval,
  .elf_reloc          = _x86_elf_reloc,
  .elf_get_sec        = _x86_elf_get_sec,
  .elf_get_secsz      = _x86_elf_get_secsz,
  .elf_get_secoff     = _x86_elf_get_secoff,
  .elf_get_secname    = _x86_elf_get_secname,
  .elf_get_shsz       = _x86_elf_get_shsz,
  .elf_write_hdrs     = _x86_elf_write_hdrs,
  .elf_build_jmptab   = _x86_elf_build_jmptab,
  .elf_update_jmptab  = _x86_elf_update_jmptab,
  .elf_update         = _x86_elf_update,
};


