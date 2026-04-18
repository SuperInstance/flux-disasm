/*
 * output.h — Multiple output format support for FLUX disassembler
 */

#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdint.h>
#include "flux-isa.h"
#include "symbol_table.h"
#include "relocation.h"

typedef enum {
    FMT_INTEL,      /* Default: MNEMONIC operands */
    FMT_ATT,        /* AT&T: MNEMONIC src, dst with %prefix */
    FMT_RAW_HEX,    /* Just raw hex dump */
    FMT_JSON,       /* JSON structured output */
} OutputFormat;

typedef struct {
    OutputFormat format;
    int show_addresses;
    int show_hex_bytes;
    int show_categories;
    int show_xrefs;
    int indent_level;
    FILE *out;
} OutputConfig;

void output_init(OutputConfig *cfg, OutputFormat fmt);
void output_header(OutputConfig *cfg, uint32_t code_len);
void output_instruction(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                        const uint8_t *operands, int operand_bytes,
                        const SymbolTable *st, const RelocTable *rt);
void output_unknown(OutputConfig *cfg, uint32_t ip, uint8_t byte);
void output_footer(OutputConfig *cfg);
void output_xref_summary(OutputConfig *cfg, const SymbolTable *st);

/* Format-specific helpers */
void format_intel(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                  const uint8_t *operands, int operand_bytes,
                  const SymbolTable *st, const RelocTable *rt);
void format_att(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                const uint8_t *operands, int operand_bytes,
                const SymbolTable *st, const RelocTable *rt);
void format_raw_hex(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                    const uint8_t *operands, int operand_bytes);
void format_json(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                 const uint8_t *operands, int operand_bytes,
                 const SymbolTable *st, const RelocTable *rt, uint32_t total_len);

#endif /* OUTPUT_H */
