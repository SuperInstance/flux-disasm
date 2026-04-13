/*
 * relocation.h — Relocation support for FLUX disassembler
 */

#ifndef RELOCATION_H
#define RELOCATION_H

#include <stdint.h>

#define RELOC_MAX_ENTRIES 1024

typedef enum {
    RELOC_ABS16,
    RELOC_ABS32,
    RELOC_REL16,
    RELOC_REL32,
    RELOC_SYM16,
    RELOC_SYM32,
} RelocType;

typedef struct {
    uint32_t offset;     /* Offset in bytecode where relocation applies */
    RelocType type;
    char symbol[128];    /* Symbol name (if symbolic) */
    int32_t addend;      /* Addend */
    uint32_t resolved;   /* Resolved address (0 if unresolved) */
    int is_resolved;
} RelocEntry;

typedef struct {
    RelocEntry entries[RELOC_MAX_ENTRIES];
    uint32_t count;
} RelocTable;

void reloc_init(RelocTable *rt);
void reloc_add(RelocTable *rt, uint32_t offset, RelocType type,
               const char *symbol, int32_t addend);
const RelocEntry *reloc_at(const RelocTable *rt, uint32_t offset);
int reloc_resolve(RelocTable *rt, const char *sym_name, uint32_t addr);
int reloc_apply(RelocTable *rt, uint8_t *code, uint32_t code_len);
void reloc_print(const RelocTable *rt);
int reloc_save(const RelocTable *rt, const char *path);
int reloc_load(RelocTable *rt, const char *path);

#endif /* RELOCATION_H */
