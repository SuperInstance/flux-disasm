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

typedef struct { const char *name; uint8_t code; int operands; } OpInfo;

static const OpInfo OPCODES[] = {
    {"NOP",       0x00, 0}, {"MOV",    0x01, 2}, {"LOAD",   0x02, 2}, {"STORE",  0x03, 2},
    {"PUSH",      0x04, 1}, {"POP",     0x05, 1}, {"JMP",    0x06, 1}, {"JZ",     0x07, 2},
    {"JNZ",       0x08, 2}, {"CALL",    0x09, 1}, {"RET",    0x0A, 0}, {"HALT",   0x0B, 0},
    {"CADD",      0x10, 3}, {"CSUB",    0x11, 3}, {"CMUL",   0x12, 3}, {"CDIV",   0x13, 3},
    {"CONF_FUSE", 0x14, 2}, {"CONF_CHAIN",0x15,1}, {"CONF_SET", 0x16, 2}, {"CONF_THRESH",0x17,2},
    {"CMP_EQ",    0x20, 3}, {"CMP_NE",  0x21, 3}, {"CMP_LT",  0x22, 3}, {"CMP_GT",  0x23, 3},
    {"TELL",      0x38, 2}, {"ASK",     0x39, 2}, {"DELEGATE",0x3A, 2}, {"BROADCAST",0x3B,1},
    {"REDUCE",    0x3C, 2},
    {"TRUST_CHECK",0x50,2}, {"TRUST_UPDATE",0x51,3}, {"TRUST_QUERY",0x52,1},
    {"INSTINCT_ACT",0x68,1},{"INSTINCT_QRY",0x69,1},
    {"ATP_GEN",   0x70, 1}, {"ATP_USE", 0x71, 1}, {"ATP_QRY", 0x72, 0}, {"ATP_XFER",0x73, 3},
    {"APOPTOSIS", 0x74, 0},
    {"DBG_PRINT", 0x78, 1}, {"BARRIER", 0x79, 0}, {"RESOURCE",0x7A, 2},
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

        switch (op->operands) {
        case 0: break;
        case 1:
            if (ip < len) printf(" r%d", code[ip++]);
            break;
        case 2:
            if (ip + 1 < len) printf(" r%d r%d", code[ip], code[ip+1]);
            ip += 2;
            break;
        case 3:
            if (ip + 2 < len) printf(" r%d r%d r%d", code[ip], code[ip+1], code[ip+2]);
            ip += 3;
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
