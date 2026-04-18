/*
 * xref.c — Cross-reference analysis for FLUX disassembler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "xref.h"

void xref_init(XrefTable *xt) {
    memset(xt, 0, sizeof(*xt));
}

void xref_add(XrefTable *xt, uint32_t from, uint32_t to, XrefType type,
              const SymbolTable *st) {
    if (xt->count >= XREF_MAX_ENTRIES) return;
    XrefEntry *e = &xt->entries[xt->count++];
    e->from_addr = from;
    e->to_addr = to;
    e->type = type;

    /* Resolve symbol names */
    e->from_sym[0] = '\0';
    e->to_sym[0] = '\0';
    if (st) {
        const SymEntry *se;
        se = sym_lookup(st, from);
        if (se) strncpy(e->from_sym, se->name, sizeof(e->from_sym) - 1);
        se = sym_lookup(st, to);
        if (se) strncpy(e->to_sym, se->name, sizeof(e->to_sym) - 1);
    }
}

const XrefEntry *xref_find_calls_to(const XrefTable *xt, uint32_t addr) {
    static XrefEntry buf[256];
    static uint32_t buf_count = 0;
    buf_count = 0;
    for (uint32_t i = 0; i < xt->count; i++) {
        if (xt->entries[i].to_addr == addr && xt->entries[i].type == XREF_CALL) {
            if (buf_count < 256)
                buf[buf_count++] = xt->entries[i];
        }
    }
    return (buf_count > 0) ? buf : NULL;
}

const XrefEntry *xref_find_jumps_to(const XrefTable *xt, uint32_t addr) {
    static XrefEntry buf[256];
    static uint32_t buf_count = 0;
    buf_count = 0;
    for (uint32_t i = 0; i < xt->count; i++) {
        if (xt->entries[i].to_addr == addr &&
            (xt->entries[i].type == XREF_JUMP || xt->entries[i].type == XREF_BRANCH_COND)) {
            if (buf_count < 256)
                buf[buf_count++] = xt->entries[i];
        }
    }
    return (buf_count > 0) ? buf : NULL;
}

int xref_count_refs_to(const XrefTable *xt, uint32_t addr) {
    int count = 0;
    for (uint32_t i = 0; i < xt->count; i++) {
        if (xt->entries[i].to_addr == addr) count++;
    }
    return count;
}

static int xref_cmp(const void *a, const void *b) {
    const XrefEntry *ea = (const XrefEntry *)a;
    const XrefEntry *eb = (const XrefEntry *)b;
    if (ea->to_addr != eb->to_addr)
        return (ea->to_addr > eb->to_addr) ? 1 : -1;
    return (ea->from_addr > eb->from_addr) ? 1 : -1;
}

void xref_sort(XrefTable *xt) {
    qsort(xt->entries, xt->count, sizeof(XrefEntry), xref_cmp);
}

void xref_print(const XrefTable *xt) {
    const char *type_names[] = {"CALL", "JMP", "BRANCH", "DATA", "INDR"};
    printf("; Cross-Reference Table (%u entries):\n", xt->count);
    for (uint32_t i = 0; i < xt->count; i++) {
        const XrefEntry *e = &xt->entries[i];
        const char *from = (e->from_sym[0]) ? e->from_sym : "";
        const char *to = (e->to_sym[0]) ? e->to_sym : "";
        printf(";   %06X (%s%-12s) --%s-->  %06X (%s%s)\n",
               e->from_addr, from, from[0] ? "+" : "",
               type_names[e->type],
               e->to_addr, to, to[0] ? "+" : "");
    }
}

void xref_print_callers(const XrefTable *xt, const SymbolTable *st) {
    printf("; Callers:\n");
    if (!st) return;
    for (uint32_t i = 0; i < st->count; i++) {
        const SymEntry *se = &st->entries[i];
        if (se->type != SYM_FUNCTION) continue;
        int refs = xref_count_refs_to(xt, se->address);
        if (refs > 0) {
            printf(";   %s (%06X) called by %d site(s)\n", se->name, se->address, refs);
        }
    }
}

void xref_print_callees(const XrefTable *xt, const SymbolTable *st) {
    printf("; Callees from each function:\n");
    if (!st) return;
    for (uint32_t i = 0; i < st->count; i++) {
        const SymEntry *se = &st->entries[i];
        if (se->type != SYM_FUNCTION) continue;
        printf(";   %s (%06X) calls:\n", se->name, se->address);
        for (uint32_t j = 0; j < xt->count; j++) {
            if (xt->entries[j].from_addr == se->address) {
                const char *target = xt->entries[j].to_sym[0] ? xt->entries[j].to_sym : "";
                printf(";     -> %06X (%s)\n", xt->entries[j].to_addr, target);
            }
        }
    }
}

void xref_print_summary(const XrefTable *xt) {
    uint32_t calls = 0, jumps = 0, branches = 0, data_refs = 0, indirect = 0;
    for (uint32_t i = 0; i < xt->count; i++) {
        switch (xt->entries[i].type) {
        case XREF_CALL: calls++; break;
        case XREF_JUMP: jumps++; break;
        case XREF_BRANCH_COND: branches++; break;
        case XREF_DATA_REF: data_refs++; break;
        case XREF_INDIRECT: indirect++; break;
        }
    }
    printf("; XRef Summary: %u calls, %u jumps, %u branches, %u data refs, %u indirect\n",
           calls, jumps, branches, data_refs, indirect);
}
