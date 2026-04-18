/*
 * xref.h — Cross-reference analysis for FLUX disassembler
 */

#ifndef XREF_H
#define XREF_H

#include <stdint.h>
#include "symbol_table.h"

#define XREF_MAX_ENTRIES 2048

typedef enum {
    XREF_CALL,
    XREF_JUMP,
    XREF_BRANCH_COND,
    XREF_DATA_REF,
    XREF_INDIRECT,
} XrefType;

typedef struct {
    uint32_t from_addr;   /* Address of the instruction making the reference */
    uint32_t to_addr;     /* Target address */
    XrefType type;
    char from_sym[128];   /* Symbol at from_addr (if any) */
    char to_sym[128];     /* Symbol at to_addr (if any) */
} XrefEntry;

typedef struct {
    XrefEntry entries[XREF_MAX_ENTRIES];
    uint32_t count;
} XrefTable;

void xref_init(XrefTable *xt);
void xref_add(XrefTable *xt, uint32_t from, uint32_t to, XrefType type,
              const SymbolTable *st);
const XrefEntry *xref_find_calls_to(const XrefTable *xt, uint32_t addr);
const XrefEntry *xref_find_jumps_to(const XrefTable *xt, uint32_t addr);
int xref_count_refs_to(const XrefTable *xt, uint32_t addr);
void xref_sort(XrefTable *xt);
void xref_print(const XrefTable *xt);
void xref_print_callers(const XrefTable *xt, const SymbolTable *st);
void xref_print_callees(const XrefTable *xt, const SymbolTable *st);
void xref_print_summary(const XrefTable *xt);

#endif /* XREF_H */
