/*
 * symbol_table.c — Symbol table implementation for FLUX disassembler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "symbol_table.h"

void sym_init(SymbolTable *st) {
    memset(st, 0, sizeof(*st));
}

void sym_add(SymbolTable *st, const char *name, uint32_t addr, SymType type) {
    if (st->count >= SYM_MAX_ENTRIES) return;
    SymEntry *e = &st->entries[st->count++];
    strncpy(e->name, name, SYM_MAX_NAME - 1);
    e->name[SYM_MAX_NAME - 1] = '\0';
    e->address = addr;
    e->type = type;
    e->size = 0;
}

void sym_add_auto(SymbolTable *st, uint32_t addr, SymType type) {
    static int lbl_counter = 0;
    char name[SYM_MAX_NAME];
    switch (type) {
    case SYM_FUNCTION:
        snprintf(name, SYM_MAX_NAME, "fn_%04X", addr);
        break;
    case SYM_LABEL:
        snprintf(name, SYM_MAX_NAME, "L_%04X", addr);
        break;
    default:
        snprintf(name, SYM_MAX_NAME, "data_%04X_%d", addr, lbl_counter++);
        break;
    }
    sym_add(st, name, addr, type);
}

const SymEntry *sym_lookup(const SymbolTable *st, uint32_t addr) {
    for (uint32_t i = 0; i < st->count; i++) {
        if (st->entries[i].address == addr)
            return &st->entries[i];
    }
    return NULL;
}

const SymEntry *sym_lookup_name(const SymbolTable *st, const char *name) {
    if (!name) return NULL;
    for (uint32_t i = 0; i < st->count; i++) {
        if (strcmp(st->entries[i].name, name) == 0)
            return &st->entries[i];
    }
    return NULL;
}

static int sym_cmp(const void *a, const void *b) {
    return (int)((const SymEntry *)a)->address - (int)((const SymEntry *)b)->address;
}

void sym_sort(SymbolTable *st) {
    qsort(st->entries, st->count, sizeof(SymEntry), sym_cmp);
}

void sym_print(const SymbolTable *st) {
    printf("; Symbol Table (%u entries):\n", st->count);
    const char *type_names[] = {"LABEL", "FUNC", "DATA", "EXT"};
    for (uint32_t i = 0; i < st->count; i++) {
        const SymEntry *e = &st->entries[i];
        printf(";   %06X  %-8s  %s\n", e->address, type_names[e->type], e->name);
    }
}

int sym_save(const SymbolTable *st, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "; FLUX Symbol Table\n");
    const char *type_names[] = {"LABEL", "FUNC", "DATA", "EXT"};
    for (uint32_t i = 0; i < st->count; i++) {
        const SymEntry *e = &st->entries[i];
        fprintf(f, "%06X  %-8s  %s\n", e->address, type_names[e->type], e->name);
    }
    fclose(f);
    return 0;
}

int sym_load(SymbolTable *st, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    sym_init(st);
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == ';' || line[0] == '\n') continue;
        uint32_t addr;
        char type_str[16], name[SYM_MAX_NAME];
        if (sscanf(line, "%X %s %s", &addr, type_str, name) == 3) {
            SymType type = SYM_LABEL;
            if (strcmp(type_str, "FUNC") == 0) type = SYM_FUNCTION;
            else if (strcmp(type_str, "DATA") == 0) type = SYM_DATA;
            else if (strcmp(type_str, "EXT") == 0) type = SYM_EXTERNAL;
            sym_add(st, name, addr, type);
        }
    }
    fclose(f);
    return 0;
}

int sym_load_elf(SymbolTable *st, const uint8_t *data, uint32_t len) {
    (void)data; (void)len;
    /* Stub: ELF symbol loading would parse ELF headers here */
    return -1; /* Not implemented for raw FLUX bytecode */
}
