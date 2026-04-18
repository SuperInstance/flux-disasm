/*
 * flux-disasm.c — FLUX Enhanced Disassembler
 * Turns silicon back into words. 247-opcode unified ISA, symbol tables,
 * relocations, multiple output formats, cross-reference analysis.
 *
 * Usage:
 *   flux-disasm [options] [file]
 *   echo "04 2A 00 05 00 0B" | flux-disasm [options]
 *
 * Options:
 *   -f intel|att|hex|json   Output format (default: intel)
 *   -s symfile              Load symbol table
 *   -r relocfile            Load relocation table
 *   -a                      Show addresses (default: on)
 *   -x                      Enable cross-reference analysis
 *   -c                      Show category annotations
 *   -o outfile              Write output to file
 *   --stdin-bin             Read raw binary from stdin (not hex)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "flux-isa.h"
#include "symbol_table.h"
#include "relocation.h"
#include "output.h"
#include "xref.h"

/* Opcode lookup — returns pointer into FLUX_OPCODES or NULL */
static const FluxOpInfo *find_op(uint8_t code) {
    for (int i = 0; i < FLUX_NUM_OPCODES; i++) {
        if (FLUX_OPCODES[i].code == code) return &FLUX_OPCODES[i];
    }
    return NULL;
}

/* Operand byte count for each mode */
static int operand_size(uint8_t mode) {
    switch (mode) {
    case OP_NONE:  return 0;
    case OP_R:     return 1;
    case OP_RR:    return 2;
    case OP_RRR:   return 3;
    case OP_RI16:  return 3;
    case OP_I16:   return 2;
    case OP_I8:    return 1;
    case OP_RI8:   return 2;
    case OP_RRI8:  return 3;
    case OP_RRRI8: return 4;
    case OP_I32:   return 4;
    case OP_RI32:  return 5;
    case OP_COND:  return 3;
    default:       return 0;
    }
}

/* Build xrefs from first pass — detect call/jump targets */
typedef struct {
    uint32_t ip;
    uint16_t target;
    uint8_t code;
} XrefPending;

static int collect_xrefs(const uint8_t *code, uint32_t len,
                         XrefPending *pending, int max_pending) {
    int count = 0;
    uint32_t ip = 0;
    while (ip < len && count < max_pending) {
        uint8_t opc = code[ip];
        const FluxOpInfo *op = find_op(opc);
        if (!op) { ip++; continue; }

        uint32_t start = ip;
        ip++; /* skip opcode byte */

        int osz = operand_size(op->mode);
        if (ip + osz > len) break; /* truncated */
        const uint8_t *operands = &code[ip];
        ip += osz;

        /* Collect jump/call targets */
        if (op->mode == OP_I16) {
            uint16_t target = operands[0] | (operands[1] << 8);
            if (opc == 0x09 /* CALL */) {
                pending[count++] = (XrefPending){start, target, XREF_CALL};
            } else if (opc == 0x06 /* JMP */) {
                pending[count++] = (XrefPending){start, target, XREF_JUMP};
            } else if (opc == 0xA3 /* BROADCAST */) {
                pending[count++] = (XrefPending){start, target, XREF_DATA_REF};
            }
        } else if (op->mode == OP_RI16) {
            uint16_t target = operands[1] | (operands[2] << 8);
            /* Conditional branches */
            if (opc >= 0x07 && opc <= 0x1F) {
                pending[count++] = (XrefPending){start, target, XREF_BRANCH_COND};
            }
        }
    }
    return count;
}

/* Main disassembly pass */
static void disasm(const uint8_t *code, uint32_t len,
                   const SymbolTable *st, const RelocTable *rt,
                   OutputConfig *cfg, int do_xref) {
    XrefTable xrefs;
    if (do_xref) {
        xref_init(&xrefs);

        /* First pass: collect cross-references */
        XrefPending pending[4096];
        int npend = collect_xrefs(code, len, pending, 4096);
        for (int i = 0; i < npend; i++) {
            xref_add(&xrefs, pending[i].ip, pending[i].target,
                     (XrefType)pending[i].code, st);
        }
        xref_sort(&xrefs);
        xref_print_summary(&xrefs);
    }

    output_header(cfg, len);

    uint32_t ip = 0;
    int instr_count = 0;
    while (ip < len) {
        uint32_t start = ip;
        uint8_t opcode = code[ip++];
        const FluxOpInfo *op = find_op(opcode);

        if (!op) {
            output_unknown(cfg, start, opcode);
            continue;
        }

        int osz = operand_size(op->mode);
        int available = (ip + osz <= len) ? osz : (len - ip);
        const uint8_t *operands = &code[ip];
        ip += available; /* advance even if truncated */

        output_instruction(cfg, start, op, operands, available, st, rt);
        instr_count++;
    }

    output_footer(cfg);

    if (do_xref) {
        xref_print_callers(&xrefs, st);
        xref_print_callees(&xrefs, st);
    }

    (void)instr_count;
}

/* Buffer disassembly — for library use */
void flux_disasm_buffer(const uint8_t *code, uint32_t len,
                        const SymbolTable *st, const RelocTable *rt,
                        OutputFormat fmt, FILE *out) {
    OutputConfig cfg;
    output_init(&cfg, fmt);
    cfg.out = out ? out : stdout;
    disasm(code, len, st, rt, &cfg, 0);
}

/* Print usage */
static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s [options] [file]\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -f intel|att|hex|json   Output format (default: intel)\n");
    fprintf(stderr, "  -s symfile              Load symbol table\n");
    fprintf(stderr, "  -r relocfile            Load relocation table\n");
    fprintf(stderr, "  -x                      Enable cross-reference analysis\n");
    fprintf(stderr, "  -c                      Show category annotations\n");
    fprintf(stderr, "  -o outfile              Write output to file\n");
    fprintf(stderr, "  --stdin-bin             Read raw binary from stdin\n");
    fprintf(stderr, "  --help                  Show this help\n");
}

/* Parse command line */
#ifndef FLUX_DISASM_MAIN
int main(int argc, char **argv) {
    const char *sym_path = NULL;
    const char *reloc_path = NULL;
    const char *out_path = NULL;
    OutputFormat fmt = FMT_INTEL;
    int do_xref = 0;
    int show_cats = 0;
    int stdin_binary = 0;
    const char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            i++;
            if (strcmp(argv[i], "intel") == 0) fmt = FMT_INTEL;
            else if (strcmp(argv[i], "att") == 0) fmt = FMT_ATT;
            else if (strcmp(argv[i], "hex") == 0) fmt = FMT_RAW_HEX;
            else if (strcmp(argv[i], "json") == 0) fmt = FMT_JSON;
            else { fprintf(stderr, "Unknown format: %s\n", argv[i]); return 1; }
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            sym_path = argv[++i];
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            reloc_path = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            out_path = argv[++i];
        } else if (strcmp(argv[i], "-x") == 0) {
            do_xref = 1;
        } else if (strcmp(argv[i], "-c") == 0) {
            show_cats = 1;
        } else if (strcmp(argv[i], "--stdin-bin") == 0) {
            stdin_binary = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        }
    }

    /* Load optional tables */
    SymbolTable st;
    sym_init(&st);
    if (sym_path) sym_load(&st, sym_path);

    RelocTable rt;
    reloc_init(&rt);
    if (reloc_path) reloc_load(&rt, reloc_path);

    /* Configure output */
    OutputConfig cfg;
    output_init(&cfg, fmt);
    cfg.show_categories = show_cats;
    cfg.show_xrefs = do_xref;

    if (out_path) {
        cfg.out = fopen(out_path, "w");
        if (!cfg.out) { fprintf(stderr, "Cannot open output: %s\n", out_path); return 1; }
    }

    /* Read bytecode */
    uint8_t buf[1048576]; /* 1MB max */
    size_t len = 0;

    if (input_file) {
        FILE *f = fopen(input_file, "rb");
        if (!f) { fprintf(stderr, "flux-disasm: cannot open %s\n", input_file); return 1; }
        len = fread(buf, 1, sizeof(buf), f);
        fclose(f);
    } else if (stdin_binary) {
        /* Read raw binary from stdin */
        len = fread(buf, 1, sizeof(buf), stdin);
    } else {
        /* Read hex from stdin */
        int c;
        while ((c = getchar()) != EOF && len < sizeof(buf)) {
            int h = -1;
            if (c >= '0' && c <= '9') h = c - '0';
            else if (c >= 'a' && c <= 'f') h = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') h = c - 'A' + 10;
            else if (c == ' ' || c == '\n' || c == '\t') continue;
            if (h >= 0) {
                static int nibble = -1;
                if (nibble < 0) { nibble = h; }
                else { buf[len++] = (nibble << 4) | h; nibble = -1; }
            }
        }
    }

    /* Disassemble */
    disasm(buf, (uint32_t)len, &st, &rt, &cfg, do_xref);

    if (out_path && cfg.out != stdout) fclose(cfg.out);
    return 0;
}
#endif /* FLUX_DISASM_MAIN */
