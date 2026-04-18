/*
 * output.c — Multiple output format implementation for FLUX disassembler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "output.h"
#include "symbol_table.h"
#include "relocation.h"

void output_init(OutputConfig *cfg, OutputFormat fmt) {
    cfg->format = fmt;
    cfg->show_addresses = 1;
    cfg->show_hex_bytes = 1;
    cfg->show_categories = 0;
    cfg->show_xrefs = 0;
    cfg->indent_level = 1;
    cfg->out = stdout;
}

void output_header(OutputConfig *cfg, uint32_t code_len) {
    switch (cfg->format) {
    case FMT_INTEL:
    case FMT_ATT:
        fprintf(cfg->out, "; flux-disasm: %u bytes\n", code_len);
        break;
    case FMT_RAW_HEX:
        fprintf(cfg->out, "; flux-disasm RAW HEX: %u bytes\n", code_len);
        break;
    case FMT_JSON:
        fprintf(cfg->out, "{\n  \"bytecode_size\": %u,\n  \"instructions\": [\n", code_len);
        break;
    }
}

void output_footer(OutputConfig *cfg) {
    if (cfg->format == FMT_JSON) {
        fprintf(cfg->out, "  ]\n}\n");
    }
}

void output_unknown(OutputConfig *cfg, uint32_t ip, uint8_t byte) {
    switch (cfg->format) {
    case FMT_INTEL:
        fprintf(cfg->out, "%*s%04X: DB 0x%02X\n", cfg->indent_level * 2, "", ip, byte);
        break;
    case FMT_ATT:
        fprintf(cfg->out, "%*s%04X: .byte 0x%02X\n", cfg->indent_level * 2, "", ip, byte);
        break;
    case FMT_RAW_HEX:
        fprintf(cfg->out, "%*s%04X: %02X\n", cfg->indent_level * 2, "", ip, byte);
        break;
    case FMT_JSON:
        fprintf(cfg->out, "    {\"addr\": %u, \"type\": \"db\", \"value\": %u}",
                ip, byte);
        break;
    }
}

static void print_indent(OutputConfig *cfg) {
    for (int i = 0; i < cfg->indent_level * 2; i++)
        fputc(' ', cfg->out);
}

static const char *reg_name_intel(uint8_t r) {
    static char buf[16];
    snprintf(buf, sizeof(buf), "r%u", r);
    return buf;
}

static const char *reg_name_att(uint8_t r) {
    static char buf[16];
    snprintf(buf, sizeof(buf), "%%r%u", r);
    return buf;
}

static const char *resolve_label(const SymbolTable *st, uint32_t addr) {
    if (!st) return NULL;
    const SymEntry *e = sym_lookup(st, addr);
    return e ? e->name : NULL;
}

void format_intel(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                  const uint8_t *operands, int operand_bytes,
                  const SymbolTable *st, const RelocTable *rt) {
    print_indent(cfg);
    if (cfg->show_addresses)
        fprintf(cfg->out, "%04X: ", ip);

    fprintf(cfg->out, "%-14s", op->name);

    switch (op->mode) {
    case OP_NONE:
        break;
    case OP_R:
        if (operand_bytes >= 1)
            fprintf(cfg->out, "%s", reg_name_intel(operands[0]));
        break;
    case OP_RR:
        if (operand_bytes >= 2)
            fprintf(cfg->out, "%s, %s", reg_name_intel(operands[0]), reg_name_intel(operands[1]));
        break;
    case OP_RRR:
        if (operand_bytes >= 3)
            fprintf(cfg->out, "%s, %s, %s",
                    reg_name_intel(operands[0]), reg_name_intel(operands[1]),
                    reg_name_intel(operands[2]));
        break;
    case OP_RI16: {
        if (operand_bytes >= 3) {
            uint16_t imm = operands[1] | (operands[2] << 8);
            const char *lbl = resolve_label(st, imm);
            fprintf(cfg->out, "%s, %s", reg_name_intel(operands[0]),
                    lbl ? lbl : "");
            if (!lbl) fprintf(cfg->out, "0x%X", imm);
        }
        break;
    }
    case OP_I16: {
        if (operand_bytes >= 2) {
            uint16_t imm = operands[0] | (operands[1] << 8);
            const char *lbl = resolve_label(st, imm);
            if (lbl) fprintf(cfg->out, "%s", lbl);
            else fprintf(cfg->out, "0x%X", imm);
        }
        break;
    }
    case OP_I8:
        if (operand_bytes >= 1)
            fprintf(cfg->out, "0x%X", operands[0]);
        break;
    case OP_RI8:
        if (operand_bytes >= 2)
            fprintf(cfg->out, "%s, 0x%X", reg_name_intel(operands[0]), operands[1]);
        break;
    case OP_RRI8:
        if (operand_bytes >= 3)
            fprintf(cfg->out, "%s, %s, 0x%X",
                    reg_name_intel(operands[0]), reg_name_intel(operands[1]), operands[2]);
        break;
    case OP_RRRI8:
        if (operand_bytes >= 4)
            fprintf(cfg->out, "%s, %s, %s, 0x%X",
                    reg_name_intel(operands[0]), reg_name_intel(operands[1]),
                    reg_name_intel(operands[2]), operands[3]);
        break;
    case OP_I32:
        if (operand_bytes >= 4) {
            uint32_t imm = operands[0] | (operands[1] << 8) | (operands[2] << 16) | (operands[3] << 24);
            fprintf(cfg->out, "0x%X", imm);
        }
        break;
    case OP_RI32:
        if (operand_bytes >= 5) {
            uint32_t imm = operands[1] | (operands[2] << 8) | (operands[3] << 16) | (operands[4] << 24);
            fprintf(cfg->out, "%s, 0x%X", reg_name_intel(operands[0]), imm);
        }
        break;
    case OP_COND:
        if (operand_bytes >= 3) {
            uint16_t imm = operands[1] | (operands[2] << 8);
            const char *lbl = resolve_label(st, imm);
            fprintf(cfg->out, "cc%u, %s", operands[0], lbl ? lbl : "");
            if (!lbl) fprintf(cfg->out, "0x%X", imm);
        }
        break;
    }

    /* Show relocation info */
    if (rt) {
        const RelocEntry *re = reloc_at(rt, ip);
        if (re) {
            fprintf(cfg->out, "  ; reloc: %s+0x%X", re->symbol, re->addend);
            if (re->is_resolved) fprintf(cfg->out, " -> 0x%X", re->resolved);
        }
    }

    fprintf(cfg->out, "\n");
}

void format_att(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                const uint8_t *operands, int operand_bytes,
                const SymbolTable *st, const RelocTable *rt) {
    (void)rt;
    print_indent(cfg);
    if (cfg->show_addresses)
        fprintf(cfg->out, "%04X: ", ip);

    /* AT&T uses lowercase mnemonic with suffix */
    const char *suffix = "";
    switch (op->mode) {
    case OP_I16: case OP_RI16: suffix = "w"; break;
    case OP_I32: case OP_RI32: suffix = "l"; break;
    case OP_I8:  case OP_RI8:  suffix = "b"; break;
    default: break;
    }
    char mnemonic[32];
    snprintf(mnemonic, sizeof(mnemonic), "%s%s", op->name, suffix);
    /* Convert to lowercase for AT&T */
    for (char *p = mnemonic; *p; p++) *p = (*p >= 'A' && *p <= 'Z') ? *p + 32 : *p;

    fprintf(cfg->out, "%-14s", mnemonic);

    switch (op->mode) {
    case OP_NONE:
        break;
    case OP_R:
        if (operand_bytes >= 1)
            fprintf(cfg->out, "%s", reg_name_att(operands[0]));
        break;
    case OP_RR:
        if (operand_bytes >= 2)
            fprintf(cfg->out, "%s, %s", reg_name_att(operands[1]), reg_name_att(operands[0]));
        break;
    case OP_RRR:
        if (operand_bytes >= 3)
            fprintf(cfg->out, "%s, %s, %s",
                    reg_name_att(operands[2]), reg_name_att(operands[1]),
                    reg_name_att(operands[0]));
        break;
    case OP_RI16: {
        if (operand_bytes >= 3) {
            uint16_t imm = operands[1] | (operands[2] << 8);
            const char *lbl = resolve_label(st, imm);
            fprintf(cfg->out, "$%s, %s", lbl ? lbl : "", reg_name_att(operands[0]));
            if (!lbl) fprintf(cfg->out, "0x%X", imm);
        }
        break;
    }
    case OP_I16: {
        if (operand_bytes >= 2) {
            uint16_t imm = operands[0] | (operands[1] << 8);
            const char *lbl = resolve_label(st, imm);
            fprintf(cfg->out, "$%s", lbl ? lbl : "");
            if (!lbl) fprintf(cfg->out, "0x%X", imm);
        }
        break;
    }
    case OP_I8:
        if (operand_bytes >= 1) fprintf(cfg->out, "$0x%X", operands[0]);
        break;
    case OP_RI8:
        if (operand_bytes >= 2)
            fprintf(cfg->out, "$0x%X, %s", operands[0], reg_name_att(operands[1]));
        break;
    case OP_RRI8:
        if (operand_bytes >= 3)
            fprintf(cfg->out, "$0x%X, %s, %s", operands[2],
                    reg_name_att(operands[1]), reg_name_att(operands[0]));
        break;
    case OP_RRRI8:
        if (operand_bytes >= 4)
            fprintf(cfg->out, "$0x%X, %s, %s, %s", operands[3],
                    reg_name_att(operands[2]), reg_name_att(operands[1]),
                    reg_name_att(operands[0]));
        break;
    case OP_I32:
        if (operand_bytes >= 4) {
            uint32_t imm = operands[0] | (operands[1] << 8) | (operands[2] << 16) | (operands[3] << 24);
            fprintf(cfg->out, "$0x%X", imm);
        }
        break;
    case OP_RI32:
        if (operand_bytes >= 5) {
            uint32_t imm = operands[1] | (operands[2] << 8) | (operands[3] << 16) | (operands[4] << 24);
            fprintf(cfg->out, "$0x%X, %s", imm, reg_name_att(operands[0]));
        }
        break;
    case OP_COND:
        if (operand_bytes >= 3) {
            uint16_t imm = operands[1] | (operands[2] << 8);
            fprintf(cfg->out, "$0x%X, %%cc%u", imm, operands[0]);
        }
        break;
    }
    fprintf(cfg->out, "\n");
}

void format_raw_hex(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                    const uint8_t *operands, int operand_bytes) {
    print_indent(cfg);
    fprintf(cfg->out, "%04X: %02X", ip, op->code);
    for (int i = 0; i < operand_bytes; i++)
        fprintf(cfg->out, " %02X", operands[i]);
    fprintf(cfg->out, "\n");
}

static const char *cat_name(uint8_t category) {
    switch (category) {
    case 0x00: return "Control";
    case 0x20: return "Arithmetic";
    case 0x40: return "Memory";
    case 0x60: return "Stack";
    case 0x70: return "Compare";
    case 0x80: return "Confidence";
    case 0x90: return "Trust";
    case 0xA0: return "A2A";
    case 0xB0: return "Instinct";
    case 0xC0: return "Energy";
    case 0xD0: return "System";
    case 0xE0: return "Crypto";
    case 0xF0: return "Vector";
    default:   return "Unknown";
    }
}

void format_json(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                 const uint8_t *operands, int operand_bytes,
                 const SymbolTable *st, const RelocTable *rt, uint32_t total_len) {
    (void)total_len;
    (void)rt;
    print_indent(cfg);
    fprintf(cfg->out, "    {\"addr\": %u, \"opcode\": \"0x%02X\", \"mnemonic\": \"%s\", \"category\": \"%s\"",
            ip, op->code, op->name, cat_name(op->category));

    const char *lbl = resolve_label(st, ip);
    if (lbl)
        fprintf(cfg->out, ", \"label\": \"%s\"", lbl);

    if (operand_bytes > 0) {
        fprintf(cfg->out, ", \"operands\": [");
        for (int i = 0; i < operand_bytes; i++) {
            if (i > 0) fprintf(cfg->out, ", ");
            fprintf(cfg->out, "%u", operands[i]);
        }
        fprintf(cfg->out, "]");
    }
    fprintf(cfg->out, "}");
}

void output_instruction(OutputConfig *cfg, uint32_t ip, const FluxOpInfo *op,
                        const uint8_t *operands, int operand_bytes,
                        const SymbolTable *st, const RelocTable *rt) {
    /* Check for symbol at this address (label) */
    if (st && cfg->format != FMT_RAW_HEX && cfg->format != FMT_JSON) {
        const SymEntry *se = sym_lookup(st, ip);
        if (se) {
            if (se->type == SYM_FUNCTION)
                fprintf(cfg->out, "\n%04X: <%s>:\n", ip, se->name);
            else
                fprintf(cfg->out, "%04X: %s:\n", ip, se->name);
        }
    }

    switch (cfg->format) {
    case FMT_INTEL:
        format_intel(cfg, ip, op, operands, operand_bytes, st, rt);
        break;
    case FMT_ATT:
        format_att(cfg, ip, op, operands, operand_bytes, st, rt);
        break;
    case FMT_RAW_HEX:
        format_raw_hex(cfg, ip, op, operands, operand_bytes);
        break;
    case FMT_JSON:
        format_json(cfg, ip, op, operands, operand_bytes, st, rt, 0);
        fprintf(cfg->out, ",\n");
        break;
    }
}

void output_xref_summary(OutputConfig *cfg, const SymbolTable *st) {
    (void)st;
    if (cfg->format != FMT_JSON)
        fprintf(cfg->out, "; End of disassembly\n");
}
