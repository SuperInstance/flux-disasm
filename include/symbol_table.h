/*
 * symbol_table.h — Symbol table for label resolution in FLUX disassembler
 */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdint.h>

#define SYM_MAX_NAME    128
#define SYM_MAX_ENTRIES 4096

typedef enum {
    SYM_LABEL,
    SYM_FUNCTION,
    SYM_DATA,
    SYM_EXTERNAL,
} SymType;

typedef struct {
    char name[SYM_MAX_NAME];
    uint32_t address;
    SymType type;
    uint32_t size;
} SymEntry;

typedef struct {
    SymEntry entries[SYM_MAX_ENTRIES];
    uint32_t count;
} SymbolTable;

void sym_init(SymbolTable *st);
void sym_add(SymbolTable *st, const char *name, uint32_t addr, SymType type);
void sym_add_auto(SymbolTable *st, uint32_t addr, SymType type);
const SymEntry *sym_lookup(const SymbolTable *st, uint32_t addr);
const SymEntry *sym_lookup_name(const SymbolTable *st, const char *name);
void sym_sort(SymbolTable *st);
void sym_print(const SymbolTable *st);
int sym_load_elf(SymbolTable *st, const uint8_t *data, uint32_t len);
int sym_save(const SymbolTable *st, const char *path);
int sym_load(SymbolTable *st, const char *path);

#endif /* SYMBOL_TABLE_H */
