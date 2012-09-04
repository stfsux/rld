
#ifndef _RLDSYM_H_
#define _RLDSYM_H_

#define SYM_SEC_UNDEF   (1<<0)
#define SYM_SEC_CODE    (1<<1)
#define SYM_SEC_DATA    (1<<2)
#define SYM_SEC_RODATA  (1<<3)
#define SYM_SEC_BSS     (1<<4)
#define SYM_SEC_ABS     (1<<5)

#define SYM_BIND_UNDEF  (1<<0)
#define SYM_BIND_LOCAL  (1<<1)
#define SYM_BIND_GLOBAL (1<<2)
#define SYM_BIND_WEAK   (1<<3)

#define SYM_TYPE_NONE (1<<0)
#define SYM_TYPE_OBJ  (1<<1)
#define SYM_TYPE_FUNC (1<<2)
#define SYM_TYPE_SEC  (1<<3)
#define SYM_TYPE_FILE (1<<4)

typedef struct _sym_t
{
  char *symname;
  uint32_t hash;
  uint32_t sectype;
  uint32_t bind;
  uint32_t type;
  uint32_t offset;
  uint32_t flags;
  uint32_t fileid;
  uint32_t symid;
  uint32_t symrid;
  uint32_t hashid;
}sym_t, *psym_t;

typedef struct _symtab
{
  psym_t *syms;
  uint32_t nsyms;
  uint32_t code;
  uint32_t data;
  uint32_t rodata;
  uint32_t bss;
}symtab_t, *psymtab_t;

psymtab_t *symtab_create (uint8_t nobj);
uint8_t symtab_add_sym (psymtab_t symtab, char *symname, uint32_t hash, uint32_t sectype, uint32_t bind, uint32_t type, uint32_t rid, uint32_t offset);
uint8_t symtab_check_usym (psymtab_t *symtab, uint8_t nobj, uint8_t n, uint32_t hash);
uint32_t symtab_get_usym_hashid (psymtab_t *symtab, uint8_t nobj, uint8_t n, uint32_t hash);
void symtab_destroy (psymtab_t *symtab, uint8_t nobj);
uint32_t sym_hash (char *str);

#endif

