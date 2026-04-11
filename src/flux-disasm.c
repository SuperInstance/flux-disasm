/*
 * flux-disasm — FLUX bytecode disassembler
 * Turns silicon back into words.
 *
 * Usage: echo -ne '\x04\x2a\x00\x05\x00\x0b' | flux-disasm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Operand encoding modes */
#define OP_NONE   0   /* No operands */
#define OP_R      1   /* Single register: 1 byte */
#define OP_RR     2   /* Two registers: 2 bytes (rd, rs) */
#define OP_RR_RI  3   /* Three registers: 3 bytes */
#define OP_R_RI16 4   /* Register + u16 immediate: 3 bytes (for JZ, JNZ, CONF_SET etc) */
#define OP_I16    5   /* Single u16 immediate: 2 bytes (for JMP, PUSH, CALL, BROADCAST) */

typedef struct { const char *name; uint8_t code; int mode; } OpInfo;

static const OpInfo OPCODES[] = {
    {"NOP",       0x00, OP_NONE}, {"MOV",    0x01, OP_RR}, {"LOAD",   0x02, OP_RR}, {"STORE",  0x03, OP_RR},
    {"PUSH",      0x04, OP_I16}, {"POP",     0x05, OP_R}, {"JMP",    0x06, OP_I16}, {"JZ",     0x07, OP_R_RI16},
    {"JNZ",       0x08, OP_R_RI16}, {"CALL",    0x09, OP_I16}, {"RET",    0x0A, OP_NONE}, {"HALT",   0x0B, OP_NONE},
    {"CADD",      0x10, OP_RR_RI}, {"CSUB",    0x11, OP_RR_RI}, {"CMUL",   0x12, OP_RR_RI}, {"CDIV",   0x13, OP_RR_RI},
    {"CONF_FUSE", 0x14, OP_RR}, {"CONF_CHAIN",0x15,OP_R}, {"CONF_SET", 0x16, OP_R_RI16}, {"CONF_THRESH",0x17,OP_R_RI16},
    {"CMP_EQ",    0x20, OP_RR_RI}, {"CMP_NE",  0x21, OP_RR_RI}, {"CMP_LT",  0x22, OP_RR_RI}, {"CMP_GT",  0x23, OP_RR_RI},
    {"TELL",      0x38, OP_RR}, {"ASK",     0x39, OP_RR}, {"DELEGATE",0x3A, OP_RR}, {"BROADCAST",0x3B,OP_I16},
    {"REDUCE",    0x3C, OP_RR},
    {"TRUST_CHECK",0x50,OP_RR}, {"TRUST_UPDATE",0x51,OP_RR_RI}, {"TRUST_QUERY",0x52,OP_R},
    {"INSTINCT_ACT",0x68,OP_R},{"INSTINCT_QRY",0x69,OP_R},
    {"ATP_GEN",   0x70, OP_R}, {"ATP_USE", 0x71, OP_R}, {"ATP_QRY", 0x72, OP_NONE}, {"ATP_XFER",0x73, OP_RR_RI},
    {"APOPTOSIS", 0x74, OP_NONE},
    {"DBG_PRINT", 0x78, OP_R}, {"BARRIER", 0x79, OP_NONE}, {"RESOURCE",0x7A, OP_RR},
};
#define N_OPS (sizeof(OPCODES) / sizeof(OPCODES[0]))

static const OpInfo *find_op(uint8_t code) {
    for (size_t i = 0; i < N_OPS; i++)
        if (OPCODES[i].code == code) return &OPCODES[i];
    return NULL;
}

static void disasm(const uint8_t *code, uint32_t len) {
    uint32_t ip = 0;
    while (ip < len) {
        uint32_t start = ip;
        uint8_t opcode = code[ip++];
        const OpInfo *op = find_op(opcode);
        if (!op) {
            printf("  %04X: DB 0x%02X\n", start, opcode);
            continue;
        }
        printf("  %04X: %-12s", start, op->name);

        switch (op->mode) {
        case OP_NONE: break;
        case OP_R:
            if (ip < len) printf(" r%d", code[ip++]);
            break;
        case OP_RR:
            if (ip + 1 < len) printf(" r%d r%d", code[ip], code[ip+1]);
            ip += 2;
            break;
        case OP_RR_RI:
            if (ip + 2 < len) printf(" r%d r%d r%d", code[ip], code[ip+1], code[ip+2]);
            ip += 3;
            break;
        case OP_R_RI16:
            if (ip + 2 < len) {
                uint16_t imm = code[ip+1] | (code[ip+2] << 8);
                printf(" r%d 0x%X", code[ip], imm);
                ip += 3;
            }
            break;
        case OP_I16:
            if (ip + 1 < len) {
                uint16_t imm = code[ip] | (code[ip+1] << 8);
                printf(" 0x%X", imm);
                ip += 2;
            }
            break;
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *f = fopen(argv[1], "rb");
        if (!f) { fprintf(stderr, "flux-disasm: cannot open %s\n", argv[1]); return 1; }
        uint8_t buf[65536];
        size_t n = fread(buf, 1, sizeof(buf), f);
        fclose(f);
        printf("; flux-disasm: %zu bytes\n", n);
        disasm(buf, (uint32_t)n);
        return 0;
    }

    /* Read hex from stdin */
    uint8_t buf[65536];
    size_t len = 0;
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
    printf("; flux-disasm: %zu bytes\n", len);
    disasm(buf, (uint32_t)len);
    return 0;
}
