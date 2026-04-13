/*
 * relocation.c — Relocation support for FLUX disassembler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "relocation.h"

void reloc_init(RelocTable *rt) {
    memset(rt, 0, sizeof(*rt));
}

void reloc_add(RelocTable *rt, uint32_t offset, RelocType type,
               const char *symbol, int32_t addend) {
    if (rt->count >= RELOC_MAX_ENTRIES) return;
    RelocEntry *e = &rt->entries[rt->count++];
    e->offset = offset;
    e->type = type;
    if (symbol) {
        strncpy(e->symbol, symbol, sizeof(e->symbol) - 1);
        e->symbol[sizeof(e->symbol) - 1] = '\0';
    } else {
        e->symbol[0] = '\0';
    }
    e->addend = addend;
    e->resolved = 0;
    e->is_resolved = 0;
}

const RelocEntry *reloc_at(const RelocTable *rt, uint32_t offset) {
    for (uint32_t i = 0; i < rt->count; i++) {
        if (rt->entries[i].offset == offset)
            return &rt->entries[i];
    }
    return NULL;
}

int reloc_resolve(RelocTable *rt, const char *sym_name, uint32_t addr) {
    int count = 0;
    for (uint32_t i = 0; i < rt->count; i++) {
        if (strcmp(rt->entries[i].symbol, sym_name) == 0) {
            rt->entries[i].resolved = addr + rt->entries[i].addend;
            rt->entries[i].is_resolved = 1;
            count++;
        }
    }
    return count;
}

int reloc_apply(RelocTable *rt, uint8_t *code, uint32_t code_len) {
    int applied = 0;
    for (uint32_t i = 0; i < rt->count; i++) {
        RelocEntry *e = &rt->entries[i];
        if (!e->is_resolved) continue;

        switch (e->type) {
        case RELOC_ABS16:
            if (e->offset + 1 < code_len) {
                code[e->offset]     = e->resolved & 0xFF;
                code[e->offset + 1] = (e->resolved >> 8) & 0xFF;
                applied++;
            }
            break;
        case RELOC_ABS32:
            if (e->offset + 3 < code_len) {
                code[e->offset]     = e->resolved & 0xFF;
                code[e->offset + 1] = (e->resolved >> 8) & 0xFF;
                code[e->offset + 2] = (e->resolved >> 16) & 0xFF;
                code[e->offset + 3] = (e->resolved >> 24) & 0xFF;
                applied++;
            }
            break;
        case RELOC_REL16:
            if (e->offset + 1 < code_len) {
                int16_t rel = (int16_t)(e->resolved - e->offset);
                code[e->offset]     = rel & 0xFF;
                code[e->offset + 1] = (rel >> 8) & 0xFF;
                applied++;
            }
            break;
        case RELOC_REL32:
            if (e->offset + 3 < code_len) {
                int32_t rel = (int32_t)(e->resolved - e->offset);
                code[e->offset]     = rel & 0xFF;
                code[e->offset + 1] = (rel >> 8) & 0xFF;
                code[e->offset + 2] = (rel >> 16) & 0xFF;
                code[e->offset + 3] = (rel >> 24) & 0xFF;
                applied++;
            }
            break;
        case RELOC_SYM16:
        case RELOC_SYM32:
            /* Symbol-based relocations already resolved to address */
            if (e->type == RELOC_SYM16 && e->offset + 1 < code_len) {
                code[e->offset]     = e->resolved & 0xFF;
                code[e->offset + 1] = (e->resolved >> 8) & 0xFF;
                applied++;
            } else if (e->type == RELOC_SYM32 && e->offset + 3 < code_len) {
                code[e->offset]     = e->resolved & 0xFF;
                code[e->offset + 1] = (e->resolved >> 8) & 0xFF;
                code[e->offset + 2] = (e->resolved >> 16) & 0xFF;
                code[e->offset + 3] = (e->resolved >> 24) & 0xFF;
                applied++;
            }
            break;
        }
    }
    return applied;
}

void reloc_print(const RelocTable *rt) {
    const char *type_names[] = {
        "ABS16", "ABS32", "REL16", "REL32", "SYM16", "SYM32"
    };
    printf("; Relocation Table (%u entries):\n", rt->count);
    for (uint32_t i = 0; i < rt->count; i++) {
        const RelocEntry *e = &rt->entries[i];
        printf(";   %06X  %-6s  %-16s  addend=%+d  resolved=%s (0x%X)\n",
               e->offset, type_names[e->type], e->symbol, e->addend,
               e->is_resolved ? "yes" : "no", e->resolved);
    }
}

int reloc_save(const RelocTable *rt, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "; FLUX Relocation Table\n");
    const char *type_names[] = {
        "ABS16", "ABS32", "REL16", "REL32", "SYM16", "SYM32"
    };
    for (uint32_t i = 0; i < rt->count; i++) {
        const RelocEntry *e = &rt->entries[i];
        fprintf(f, "%06X  %-6s  %-16s  %+d\n",
                e->offset, type_names[e->type], e->symbol, e->addend);
    }
    fclose(f);
    return 0;
}

int reloc_load(RelocTable *rt, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    reloc_init(rt);
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == ';' || line[0] == '\n') continue;
        uint32_t offset;
        char type_str[16], symbol[128];
        int32_t addend = 0;
        if (sscanf(line, "%X %s %s %d", &offset, type_str, symbol, &addend) >= 3) {
            RelocType type = RELOC_ABS16;
            if (strcmp(type_str, "ABS32") == 0) type = RELOC_ABS32;
            else if (strcmp(type_str, "REL16") == 0) type = RELOC_REL16;
            else if (strcmp(type_str, "REL32") == 0) type = RELOC_REL32;
            else if (strcmp(type_str, "SYM16") == 0) type = RELOC_SYM16;
            else if (strcmp(type_str, "SYM32") == 0) type = RELOC_SYM32;
            reloc_add(rt, offset, type, symbol, addend);
        }
    }
    fclose(f);
    return 0;
}
