/*
 * Tests for flux-disasm — Enhanced test suite
 *
 * Compile: gcc -Wall -Wextra -std=c11 -g -o test_disasm \
 *          tests/test_disasm.c src/flux-disasm.c src/symbol_table.c \
 *          src/relocation.c src/output.c src/xref.c -Iinclude
 * Run:     ./test_disasm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* Guard main in flux-disasm.c */
#define FLUX_DISASM_MAIN
/* Include source directly for static function access */
#include "../src/flux-disasm.c"
#include "../src/symbol_table.c"
#include "../src/relocation.c"
#include "../src/output.c"
#include "../src/xref.c"

/* ---- Test harness ---- */

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define RUN_TEST(fn) do { \
    tests_run++; \
    printf("  %-55s ", #fn); \
    fflush(stdout); \
    fn(); \
    tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("FAIL: %s != %s (got %d, expected %d)\n", #a, #b, (int)(a), (int)(b)); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_NE(a, b) do { \
    if ((a) == (b)) { \
        printf("FAIL: %s == %s\n", #a, #b); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        printf("FAIL: \"%s\" != \"%s\"\n", (a), (b)); \
        tests_failed++; return; \
    } \
} while(0)

#define ASSERT_NULL(p) ASSERT_EQ((p), NULL)
#define ASSERT_NOT_NULL(p) ASSERT_NE((p), NULL)

/* ================================================================
 * ISA Tests
 * ================================================================ */

/* Test total opcode count is 247 */
static void test_opcode_count(void) {
    ASSERT_EQ(FLUX_NUM_OPCODES, 247);
}

/* Test first and last opcodes */
static void test_opcode_boundaries(void) {
    ASSERT_NOT_NULL(find_op(0x00));
    ASSERT_STR_EQ(find_op(0x00)->name, "NOP");
    ASSERT_NOT_NULL(find_op(0xF6));
    ASSERT_STR_EQ(find_op(0xF6)->name, "VMAP");
}

/* Test all original 38 opcodes are preserved at new addresses */
static void test_original_opcodes_preserved(void) {
    /* Map from old code -> new code for original opcodes */
    struct { uint8_t old; uint8_t newc; const char *name; } map[] = {
        {0x00, 0x00, "NOP"}, {0x01, 0x01, "MOV"}, {0x02, 0x02, "LOAD"}, {0x03, 0x03, "STORE"},
        {0x04, 0x04, "PUSH"}, {0x05, 0x05, "POP"}, {0x06, 0x06, "JMP"},
        {0x07, 0x07, "JZ"}, {0x08, 0x08, "JNZ"}, {0x09, 0x09, "CALL"},
        {0x0A, 0x0A, "RET"}, {0x0B, 0x0B, "HALT"},
        {0x10, 0x20, "CADD"}, {0x11, 0x21, "CSUB"}, {0x12, 0x22, "CMUL"}, {0x13, 0x23, "CDIV"},
        {0x14, 0x80, "CONF_FUSE"}, {0x15, 0x81, "CONF_CHAIN"},
        {0x16, 0x82, "CONF_SET"}, {0x17, 0x83, "CONF_THRESH"},
        {0x20, 0x70, "CMP_EQ"}, {0x21, 0x71, "CMP_NE"}, {0x22, 0x72, "CMP_LT"}, {0x23, 0x73, "CMP_GT"},
        {0x38, 0xA0, "TELL"}, {0x39, 0xA1, "ASK"}, {0x3A, 0xA2, "DELEGATE"},
        {0x3B, 0xA3, "BROADCAST"}, {0x3C, 0xA4, "REDUCE"},
        {0x50, 0x90, "TRUST_CHECK"}, {0x51, 0x91, "TRUST_UPDATE"}, {0x52, 0x92, "TRUST_QUERY"},
        {0x68, 0xB0, "INSTINCT_ACT"}, {0x69, 0xB1, "INSTINCT_QRY"},
        {0x70, 0xC0, "ATP_GEN"}, {0x71, 0xC1, "ATP_USE"},
        {0x72, 0xC2, "ATP_QRY"}, {0x73, 0xC3, "ATP_XFER"}, {0x74, 0xC4, "APOPTOSIS"},
        {0x78, 0xD0, "DBG_PRINT"}, {0x79, 0xD1, "BARRIER"}, {0x7A, 0xD2, "RESOURCE"},
    };
    for (int i = 0; i < (int)(sizeof(map)/sizeof(map[0])); i++) {
        const FluxOpInfo *op = find_op(map[i].newc);
        ASSERT_NOT_NULL(op);
        if (op) ASSERT_STR_EQ(op->name, map[i].name);
    }
}

/* Test all 13 categories are represented */
static void test_categories(void) {
    uint8_t cats_present[256] = {0};
    int unique = 0;
    for (int i = 0; i < FLUX_NUM_OPCODES; i++) {
        uint8_t cat = FLUX_OPCODES[i].category;
        if (!cats_present[cat]) { cats_present[cat] = 1; unique++; }
    }
    /* Should have 13 categories: 0x00,0x20,0x40,0x60,0x70,0x80,0x90,0xA0,0xB0,0xC0,0xD0,0xE0,0xF0 */
    ASSERT_EQ(unique, 13);
    ASSERT_EQ(cats_present[0x00], 1);
    ASSERT_EQ(cats_present[0x20], 1);
    ASSERT_EQ(cats_present[0x40], 1);
    ASSERT_EQ(cats_present[0x60], 1);
    ASSERT_EQ(cats_present[0x70], 1);
    ASSERT_EQ(cats_present[0x80], 1);
    ASSERT_EQ(cats_present[0x90], 1);
    ASSERT_EQ(cats_present[0xA0], 1);
    ASSERT_EQ(cats_present[0xB0], 1);
    ASSERT_EQ(cats_present[0xC0], 1);
    ASSERT_EQ(cats_present[0xD0], 1);
    ASSERT_EQ(cats_present[0xE0], 1);
    ASSERT_EQ(cats_present[0xF0], 1);
}

/* Test operand_size for all modes */
static void test_operand_sizes(void) {
    ASSERT_EQ(operand_size(OP_NONE),  0);
    ASSERT_EQ(operand_size(OP_R),     1);
    ASSERT_EQ(operand_size(OP_RR),    2);
    ASSERT_EQ(operand_size(OP_RRR),   3);
    ASSERT_EQ(operand_size(OP_RI16),  3);
    ASSERT_EQ(operand_size(OP_I16),   2);
    ASSERT_EQ(operand_size(OP_I8),    1);
    ASSERT_EQ(operand_size(OP_RI8),   2);
    ASSERT_EQ(operand_size(OP_RRI8),  3);
    ASSERT_EQ(operand_size(OP_RRRI8), 4);
    ASSERT_EQ(operand_size(OP_I32),   4);
    ASSERT_EQ(operand_size(OP_RI32),  5);
    ASSERT_EQ(operand_size(OP_COND),  3);
}

/* Test all arithmetic opcodes */
static void test_arithmetic_opcodes(void) {
    const char *arith_names[] = {
        "CADD","CSUB","CMUL","CDIV","CMOD","CAND","COR","CXOR",
        "CSHL","CSHR","CSAR","CNOT","CNEG","CABS","CMIN","CMAX",
        "ADDI","SUBI","MULI","DIVI","ANDI","ORI","XORI","SHLI",
        "SHRI","INCR","DECR","CLZ","CTZ","POPCNT","BREV","BSWAP"
    };
    for (int i = 0; i < 32; i++) {
        const FluxOpInfo *op = find_op(0x20 + i);
        ASSERT_NOT_NULL(op);
        if (op) ASSERT_STR_EQ(op->name, arith_names[i]);
    }
}

/* Test new memory opcodes */
static void test_memory_opcodes(void) {
    ASSERT_NOT_NULL(find_op(0x40)); ASSERT_STR_EQ(find_op(0x40)->name, "LOAD8");
    ASSERT_NOT_NULL(find_op(0x41)); ASSERT_STR_EQ(find_op(0x41)->name, "LOAD16");
    ASSERT_NOT_NULL(find_op(0x42)); ASSERT_STR_EQ(find_op(0x42)->name, "LOAD32");
    ASSERT_NOT_NULL(find_op(0x43)); ASSERT_STR_EQ(find_op(0x43)->name, "STORE8");
    ASSERT_NOT_NULL(find_op(0x44)); ASSERT_STR_EQ(find_op(0x44)->name, "STORE16");
    ASSERT_NOT_NULL(find_op(0x45)); ASSERT_STR_EQ(find_op(0x45)->name, "STORE32");
    ASSERT_NOT_NULL(find_op(0x46)); ASSERT_STR_EQ(find_op(0x46)->name, "LEA");
    ASSERT_NOT_NULL(find_op(0x47)); ASSERT_STR_EQ(find_op(0x47)->name, "CMPXCHG");
    ASSERT_NOT_NULL(find_op(0x48)); ASSERT_STR_EQ(find_op(0x48)->name, "XCHG");
}

/* Test stack opcodes */
static void test_stack_opcodes(void) {
    const char *stack_names[] = {
        "PUSH_R","POP_R","DUP","SWAP2","ROT","OVER","PICK","ROLL",
        "DEPTH","DROP","NIP","TUCK","PUSH_IMM8","PUSH_IMM16","PUSH_IMM32","PEEK"
    };
    for (int i = 0; i < 16; i++) {
        const FluxOpInfo *op = find_op(0x60 + i);
        ASSERT_NOT_NULL(op);
        if (op) ASSERT_STR_EQ(op->name, stack_names[i]);
    }
}

/* Test crypto opcodes */
static void test_crypto_opcodes(void) {
    ASSERT_NOT_NULL(find_op(0xE0)); ASSERT_STR_EQ(find_op(0xE0)->name, "HASH_MD5");
    ASSERT_NOT_NULL(find_op(0xE1)); ASSERT_STR_EQ(find_op(0xE1)->name, "HASH_SHA256");
    ASSERT_NOT_NULL(find_op(0xE5)); ASSERT_STR_EQ(find_op(0xE5)->name, "AES_ENC");
    ASSERT_NOT_NULL(find_op(0xE6)); ASSERT_STR_EQ(find_op(0xE6)->name, "AES_DEC");
    ASSERT_NOT_NULL(find_op(0xE9)); ASSERT_STR_EQ(find_op(0xE9)->name, "RNG_SEED");
    ASSERT_NOT_NULL(find_op(0xEB)); ASSERT_STR_EQ(find_op(0xEB)->name, "CRC32");
}

/* Test vector/SIMD opcodes */
static void test_vector_opcodes(void) {
    ASSERT_NOT_NULL(find_op(0xF0)); ASSERT_STR_EQ(find_op(0xF0)->name, "VADD");
    ASSERT_NOT_NULL(find_op(0xF1)); ASSERT_STR_EQ(find_op(0xF1)->name, "VSUB");
    ASSERT_NOT_NULL(find_op(0xF2)); ASSERT_STR_EQ(find_op(0xF2)->name, "VMUL");
    ASSERT_NOT_NULL(find_op(0xF3)); ASSERT_STR_EQ(find_op(0xF3)->name, "VDOT");
    ASSERT_NOT_NULL(find_op(0xF4)); ASSERT_STR_EQ(find_op(0xF4)->name, "VLOAD");
    ASSERT_NOT_NULL(find_op(0xF5)); ASSERT_STR_EQ(find_op(0xF5)->name, "VSTORE");
    ASSERT_NOT_NULL(find_op(0xF6)); ASSERT_STR_EQ(find_op(0xF6)->name, "VMAP");
}

/* Test no duplicate opcodes */
static void test_no_duplicates(void) {
    for (int i = 0; i < FLUX_NUM_OPCODES; i++) {
        for (int j = i + 1; j < FLUX_NUM_OPCODES; j++) {
            ASSERT_EQ(FLUX_OPCODES[i].code, FLUX_OPCODES[i].code); /* sanity */
            /* If same code, fail */
            if (FLUX_OPCODES[i].code == FLUX_OPCODES[j].code) {
                printf("FAIL: duplicate opcode 0x%02X (%s and %s)\n",
                       FLUX_OPCODES[i].code, FLUX_OPCODES[i].name, FLUX_OPCODES[j].name);
                tests_failed++; return;
            }
        }
    }
}

/* Test unknown opcodes */
static void test_unknown_opcodes(void) {
    /* 0xF7 through 0xFF are not in the 247-opcode table */
    for (int i = 0xF7; i <= 0xFF; i++) {
        ASSERT_NULL(find_op((uint8_t)i));
    }
}

/* ================================================================
 * Symbol Table Tests
 * ================================================================ */

static void test_sym_add_lookup(void) {
    SymbolTable st;
    sym_init(&st);
    sym_add(&st, "main", 0x0000, SYM_FUNCTION);
    sym_add(&st, "helper", 0x0020, SYM_FUNCTION);
    sym_add(&st, "data_buf", 0x0100, SYM_DATA);

    ASSERT_EQ(st.count, 3u);
    ASSERT_NOT_NULL(sym_lookup(&st, 0x0000));
    ASSERT_STR_EQ(sym_lookup(&st, 0x0000)->name, "main");
    ASSERT_NOT_NULL(sym_lookup(&st, 0x0020));
    ASSERT_STR_EQ(sym_lookup(&st, 0x0020)->name, "helper");
    ASSERT_NULL(sym_lookup(&st, 0x0040));
}

static void test_sym_lookup_name(void) {
    SymbolTable st;
    sym_init(&st);
    sym_add(&st, "test_func", 0x100, SYM_FUNCTION);
    ASSERT_NOT_NULL(sym_lookup_name(&st, "test_func"));
    ASSERT_NULL(sym_lookup_name(&st, "nonexistent"));
}

static void test_sym_auto_generate(void) {
    SymbolTable st;
    sym_init(&st);
    sym_add_auto(&st, 0x0000, SYM_FUNCTION);
    sym_add_auto(&st, 0x0010, SYM_LABEL);
    ASSERT_EQ(st.count, 2u);
    ASSERT_STR_EQ(st.entries[0].name, "fn_0000");
    ASSERT_STR_EQ(st.entries[1].name, "L_0010");
}

static void test_sym_sort(void) {
    SymbolTable st;
    sym_init(&st);
    sym_add(&st, "c_func", 0x0030, SYM_FUNCTION);
    sym_add(&st, "a_func", 0x0010, SYM_FUNCTION);
    sym_add(&st, "b_func", 0x0020, SYM_FUNCTION);
    sym_sort(&st);
    ASSERT_EQ(st.entries[0].address, 0x0010u);
    ASSERT_EQ(st.entries[1].address, 0x0020u);
    ASSERT_EQ(st.entries[2].address, 0x0030u);
}

static void test_sym_save_load(void) {
    SymbolTable st;
    sym_init(&st);
    sym_add(&st, "main", 0x0000, SYM_FUNCTION);
    sym_add(&st, "data1", 0x0100, SYM_DATA);

    const char *path = "/tmp/test_flux_sym.txt";
    ASSERT_EQ(sym_save(&st, path), 0);

    SymbolTable st2;
    ASSERT_EQ(sym_load(&st2, path), 0);
    ASSERT_EQ(st2.count, 2u);
    ASSERT_STR_EQ(st2.entries[0].name, "main");
    ASSERT_EQ(st2.entries[0].address, 0x0000u);
    remove(path);
}

/* ================================================================
 * Relocation Tests
 * ================================================================ */

static void test_reloc_add_lookup(void) {
    RelocTable rt;
    reloc_init(&rt);
    reloc_add(&rt, 0x0006, RELOC_ABS16, "target_func", 0);
    reloc_add(&rt, 0x000A, RELOC_REL16, "data_buf", 4);

    ASSERT_EQ(rt.count, 2u);
    ASSERT_NOT_NULL(reloc_at(&rt, 0x0006));
    ASSERT_STR_EQ(reloc_at(&rt, 0x0006)->symbol, "target_func");
    ASSERT_NULL(reloc_at(&rt, 0x00FF));
}

static void test_reloc_resolve(void) {
    RelocTable rt;
    reloc_init(&rt);
    reloc_add(&rt, 0x0006, RELOC_SYM16, "main", 0);
    reloc_add(&rt, 0x0010, RELOC_SYM16, "helper", 0);

    int resolved = reloc_resolve(&rt, "main", 0x0100);
    ASSERT_EQ(resolved, 1);
    ASSERT_EQ(rt.entries[0].resolved, 0x0100u);
    ASSERT_EQ(rt.entries[0].is_resolved, 1);
    ASSERT_EQ(rt.entries[1].is_resolved, 0);
}

static void test_reloc_apply(void) {
    RelocTable rt;
    reloc_init(&rt);
    reloc_add(&rt, 0x0001, RELOC_ABS16, "addr", 0);
    reloc_resolve(&rt, "addr", 0x1234);

    uint8_t code[16] = {0};
    int applied = reloc_apply(&rt, code, 16);
    ASSERT_EQ(applied, 1);
    ASSERT_EQ(code[1], 0x34);
    ASSERT_EQ(code[2], 0x12);
}

static void test_reloc_save_load(void) {
    RelocTable rt;
    reloc_init(&rt);
    reloc_add(&rt, 0x0006, RELOC_ABS16, "func", 0);
    reloc_add(&rt, 0x0010, RELOC_REL16, "data", 8);

    const char *path = "/tmp/test_flux_reloc.txt";
    ASSERT_EQ(reloc_save(&rt, path), 0);

    RelocTable rt2;
    ASSERT_EQ(reloc_load(&rt2, path), 0);
    ASSERT_EQ(rt2.count, 2u);
    ASSERT_STR_EQ(rt2.entries[0].symbol, "func");
    remove(path);
}

/* ================================================================
 * Output Format Tests
 * ================================================================ */

/* Redirect stdout to buffer for testing output */
static char test_buf[65536];
static size_t test_buf_pos = 0;

static void capture_start(void) {
    memset(test_buf, 0, sizeof(test_buf));
    test_buf_pos = 0;
}

static void test_output_init(OutputConfig *cfg, OutputFormat fmt) {
    output_init(cfg, fmt);
    cfg->out = tmpfile();
}

static void capture_end(FILE *f) {
    if (!f) { test_buf[0] = '\0'; return; }
    long sz = ftell(f);
    if (sz > 0) { rewind(f); sz = fread(test_buf, 1, sizeof(test_buf)-1, f); }
    test_buf[sz] = '\0';
    fclose(f);
}

static void test_output_intel_basic(void) {
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_INTEL);

    uint8_t code[] = {0x00, 0x01, 0x03, 0x02}; /* NOP; MOV r3, r2 */
    const FluxOpInfo *op1 = find_op(0x00);
    const FluxOpInfo *op2 = find_op(0x01);
    ASSERT_NOT_NULL(op1);
    ASSERT_NOT_NULL(op2);

    output_header(&cfg, 4);
    output_instruction(&cfg, 0, op1, NULL, 0, NULL, NULL);
    output_instruction(&cfg, 1, op2, &code[2], 2, NULL, NULL);
    output_footer(&cfg);
    capture_end(cfg.out);

    ASSERT_NOT_NULL(strstr(test_buf, "NOP"));
    ASSERT_NOT_NULL(strstr(test_buf, "MOV"));
}

static void test_output_att_basic(void) {
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_ATT);

    uint8_t code[] = {0x01, 0x03, 0x02}; /* MOV r3, r2 */
    const FluxOpInfo *op = find_op(0x01);
    ASSERT_NOT_NULL(op);

    output_instruction(&cfg, 0, op, &code[1], 2, NULL, NULL);
    capture_end(cfg.out);

    /* AT&T should have %r prefix and lowercase */
    ASSERT_NOT_NULL(strstr(test_buf, "%r"));
}

static void test_output_raw_hex(void) {
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_RAW_HEX);

    uint8_t code[] = {0x00, 0x01, 0x03, 0x02};
    const FluxOpInfo *op = find_op(0x00);
    ASSERT_NOT_NULL(op);

    output_instruction(&cfg, 0, op, NULL, 0, NULL, NULL);
    capture_end(cfg.out);

    ASSERT_NOT_NULL(strstr(test_buf, "00"));
}

static void test_output_json_basic(void) {
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_JSON);

    uint8_t code[] = {0x00};
    const FluxOpInfo *op = find_op(0x00);
    ASSERT_NOT_NULL(op);

    output_header(&cfg, 1);
    output_instruction(&cfg, 0, op, NULL, 0, NULL, NULL);
    output_footer(&cfg);
    capture_end(cfg.out);

    ASSERT_NOT_NULL(strstr(test_buf, "\"mnemonic\""));
    ASSERT_NOT_NULL(strstr(test_buf, "\"NOP\""));
    ASSERT_NOT_NULL(strstr(test_buf, "\"category\""));
}

static void test_output_with_symbols(void) {
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_INTEL);

    SymbolTable st;
    sym_init(&st);
    sym_add(&st, "my_func", 0x0000, SYM_FUNCTION);

    uint8_t code[] = {0x00};
    const FluxOpInfo *op = find_op(0x00);
    ASSERT_NOT_NULL(op);

    output_instruction(&cfg, 0, op, NULL, 0, &st, NULL);
    capture_end(cfg.out);

    ASSERT_NOT_NULL(strstr(test_buf, "my_func"));
}

static void test_output_unknown_opcode(void) {
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_INTEL);

    output_unknown(&cfg, 0x0005, 0xFF);
    capture_end(cfg.out);

    ASSERT_NOT_NULL(strstr(test_buf, "DB"));
}

/* ================================================================
 * Disassembly Tests
 * ================================================================ */

static void test_disasm_simple_nop_halt(void) {
    uint8_t code[] = {0x00, 0x0B};
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_INTEL);
    disasm(code, 2, NULL, NULL, &cfg, 0);
    capture_end(cfg.out);
    ASSERT_NOT_NULL(strstr(test_buf, "NOP"));
    ASSERT_NOT_NULL(strstr(test_buf, "HALT"));
}

static void test_disasm_call_jmp(void) {
    uint8_t code[] = {
        0x09, 0x20, 0x00,  /* CALL 0x0020 */
        0x06, 0x10, 0x00,  /* JMP 0x0010 */
        0x0B,              /* HALT */
    };
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_INTEL);
    disasm(code, sizeof(code), NULL, NULL, &cfg, 0);
    capture_end(cfg.out);
    ASSERT_NOT_NULL(strstr(test_buf, "CALL"));
    ASSERT_NOT_NULL(strstr(test_buf, "JMP"));
    ASSERT_NOT_NULL(strstr(test_buf, "HALT"));
}

static void test_disasm_with_relocations(void) {
    uint8_t code[] = {0x09, 0x20, 0x00, 0x0B};
    RelocTable rt;
    reloc_init(&rt);
    reloc_add(&rt, 0x0000, RELOC_SYM16, "target", 0);
    reloc_resolve(&rt, "target", 0x0200);

    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_INTEL);
    disasm(code, sizeof(code), NULL, &rt, &cfg, 0);
    capture_end(cfg.out);
    ASSERT_NOT_NULL(strstr(test_buf, "reloc"));
}

static void test_disasm_empty(void) {
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_INTEL);
    disasm(NULL, 0, NULL, NULL, &cfg, 0);
    capture_end(cfg.out);
    /* Should not crash */
}

static void test_disasm_truncated(void) {
    uint8_t code[] = {0x01}; /* MOV needs 3 bytes total */
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_INTEL);
    disasm(code, 1, NULL, NULL, &cfg, 0);
    capture_end(cfg.out);
    /* Should handle gracefully */
}

static void test_disasm_all_none_ops(void) {
    uint8_t none_ops[] = {
        0x00, /* NOP */
        0x0A, /* RET */
        0x0B, /* HALT */
        0x0C, /* BRK */
        0x0E, /* IRET */
        0x5D, /* FENCE */
        0x5E, /* TLB_INV */
        0x5F, /* CACHE_FLUSH */
        0x62, /* DUP */
        0x63, /* SWAP2 */
        0x64, /* ROT */
        0x65, /* OVER */
        0x69, /* DROP */
        0x6A, /* NIP */
        0xC2, /* ATP_QRY */
        0xC4, /* APOPTOSIS */
        0xD1, /* BARRIER */
        0xD7, /* TRACE */
        0xDA, /* YIELD */
        0x59, /* PUSH_ALL */
        0x5A, /* POP_ALL */
    };
    for (size_t i = 0; i < sizeof(none_ops); i++) {
        const FluxOpInfo *op = find_op(none_ops[i]);
        ASSERT_NOT_NULL(op);
        if (op) ASSERT_EQ(op->mode, OP_NONE);
    }
}

/* ================================================================
 * Cross-Reference Tests
 * ================================================================ */

static void test_xref_basic(void) {
    XrefTable xt;
    xref_init(&xt);
    xref_add(&xt, 0x0000, 0x0020, XREF_CALL, NULL);
    xref_add(&xt, 0x0010, 0x0020, XREF_CALL, NULL);
    xref_add(&xt, 0x0005, 0x0010, XREF_JUMP, NULL);

    ASSERT_EQ(xt.count, 3u);
    ASSERT_EQ(xref_count_refs_to(&xt, 0x0020), 2);
    ASSERT_EQ(xref_count_refs_to(&xt, 0x0010), 1);
    ASSERT_EQ(xref_count_refs_to(&xt, 0x0050), 0);
}

static void test_xref_with_symbols(void) {
    XrefTable xt;
    xref_init(&xt);
    SymbolTable st;
    sym_init(&st);
    sym_add(&st, "main", 0x0000, SYM_FUNCTION);
    sym_add(&st, "helper", 0x0020, SYM_FUNCTION);

    xref_add(&xt, 0x0000, 0x0020, XREF_CALL, &st);
    ASSERT_STR_EQ(xt.entries[0].from_sym, "main");
    ASSERT_STR_EQ(xt.entries[0].to_sym, "helper");
}

static void test_xref_sort(void) {
    XrefTable xt;
    xref_init(&xt);
    xref_add(&xt, 0x0030, 0x0010, XREF_JUMP, NULL);
    xref_add(&xt, 0x0000, 0x0020, XREF_CALL, NULL);
    xref_add(&xt, 0x0010, 0x0010, XREF_JUMP, NULL);
    xref_sort(&xt);
    /* Should be sorted by to_addr then from_addr */
    ASSERT_EQ(xt.entries[0].to_addr, 0x0010u);
    ASSERT_EQ(xt.entries[0].from_addr, 0x0010u);
}

static void test_xref_find_calls_to(void) {
    XrefTable xt;
    xref_init(&xt);
    xref_add(&xt, 0x0000, 0x0050, XREF_CALL, NULL);
    xref_add(&xt, 0x0010, 0x0050, XREF_CALL, NULL);
    xref_add(&xt, 0x0020, 0x0050, XREF_JUMP, NULL);

    const XrefEntry *calls = xref_find_calls_to(&xt, 0x0050);
    ASSERT_NOT_NULL(calls);
}

/* ================================================================
 * Disassembly with xref enabled
 * ================================================================ */

static void test_disasm_with_xref(void) {
    uint8_t code[] = {
        0x09, 0x0A, 0x00,  /* CALL 0x000A */
        0x0B,              /* HALT */
        /* 0x000A: */
        0x00,              /* NOP */
        0x0A,              /* RET */
    };
    OutputConfig cfg;
    capture_start();
    test_output_init(&cfg, FMT_INTEL);
    /* xref output goes to stdout via printf, so just verify no crash */
    disasm(code, sizeof(code), NULL, NULL, &cfg, 1);
    capture_end(cfg.out);
    /* Verify the disasm output contains CALL and HALT */
    ASSERT_NOT_NULL(strstr(test_buf, "CALL"));
    ASSERT_NOT_NULL(strstr(test_buf, "HALT"));
}

/* ================================================================
 * Buffer API test
 * ================================================================ */

static void test_buffer_api(void) {
    uint8_t code[] = {0x00, 0x0B};
    capture_start();
    FILE *f = tmpfile();
    flux_disasm_buffer(code, 2, NULL, NULL, FMT_INTEL, f);
    long sz = ftell(f); if (sz > 0) { rewind(f); sz = fread(test_buf, 1, sizeof(test_buf)-1, f); }
    test_buf[sz] = '\0';
    fclose(f);
    ASSERT_NOT_NULL(strstr(test_buf, "NOP"));
    ASSERT_NOT_NULL(strstr(test_buf, "HALT"));
}

/* ================================================================
 * New instruction mode tests
 * ================================================================ */

static void test_i8_mode(void) {
    const FluxOpInfo *op = find_op(0x0D); /* SWI */
    ASSERT_NOT_NULL(op);
    if (op) ASSERT_EQ(op->mode, OP_I8);
}

static void test_i32_mode(void) {
    const FluxOpInfo *op = find_op(0x5B); /* LOAD_IMM */
    ASSERT_NOT_NULL(op);
    if (op) ASSERT_EQ(op->mode, OP_RI32);
}

static void test_cond_mode(void) {
    const FluxOpInfo *op = find_op(0x12); /* JC */
    ASSERT_NOT_NULL(op);
    if (op) ASSERT_EQ(op->mode, OP_RI16);
}

static void test_confidence_category(void) {
    for (int i = 0x80; i <= 0x8F; i++) {
        const FluxOpInfo *op = find_op((uint8_t)i);
        ASSERT_NOT_NULL(op);
        if (op) ASSERT_EQ(op->category, CAT_CONFIDENCE);
    }
}

static void test_instinct_category(void) {
    for (int i = 0xB0; i <= 0xBF; i++) {
        const FluxOpInfo *op = find_op((uint8_t)i);
        ASSERT_NOT_NULL(op);
        if (op) ASSERT_EQ(op->category, CAT_INSTINCT);
    }
}

static void test_energy_category(void) {
    for (int i = 0xC0; i <= 0xCF; i++) {
        const FluxOpInfo *op = find_op((uint8_t)i);
        ASSERT_NOT_NULL(op);
        if (op) ASSERT_EQ(op->category, CAT_ENERGY);
    }
}

/* ================================================================
 * Collect xrefs pass test
 * ================================================================ */

static void test_collect_xrefs(void) {
    uint8_t code[] = {
        0x09, 0x10, 0x00,  /* CALL 0x0010 */
        0x06, 0x10, 0x00,  /* JMP 0x0010 */
        0x07, 0x00, 0x10, 0x00,  /* JZ r0, 0x0010 */
        0x0B,
    };
    XrefPending pending[64];
    int n = collect_xrefs(code, sizeof(code), pending, 64);
    ASSERT_EQ(n, 3); /* CALL + JMP + JZ */
}

/* ================================================================
 * Main
 * ================================================================ */

int main(void) {
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║  flux-disasm Enhanced Test Suite (247 opcodes)   ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    printf("[ISA — Opcode Table]\n");
    RUN_TEST(test_opcode_count);
    RUN_TEST(test_opcode_boundaries);
    RUN_TEST(test_original_opcodes_preserved);
    RUN_TEST(test_categories);
    RUN_TEST(test_operand_sizes);
    RUN_TEST(test_arithmetic_opcodes);
    RUN_TEST(test_memory_opcodes);
    RUN_TEST(test_stack_opcodes);
    RUN_TEST(test_crypto_opcodes);
    RUN_TEST(test_vector_opcodes);
    RUN_TEST(test_no_duplicates);
    RUN_TEST(test_unknown_opcodes);
    RUN_TEST(test_i8_mode);
    RUN_TEST(test_i32_mode);
    RUN_TEST(test_cond_mode);
    RUN_TEST(test_confidence_category);
    RUN_TEST(test_instinct_category);
    RUN_TEST(test_energy_category);

    printf("\n[Symbol Table]\n");
    RUN_TEST(test_sym_add_lookup);
    RUN_TEST(test_sym_lookup_name);
    RUN_TEST(test_sym_auto_generate);
    RUN_TEST(test_sym_sort);
    RUN_TEST(test_sym_save_load);

    printf("\n[Relocation]\n");
    RUN_TEST(test_reloc_add_lookup);
    RUN_TEST(test_reloc_resolve);
    RUN_TEST(test_reloc_apply);
    RUN_TEST(test_reloc_save_load);

    printf("\n[Output Formats]\n");
    RUN_TEST(test_output_intel_basic);
    RUN_TEST(test_output_att_basic);
    RUN_TEST(test_output_raw_hex);
    RUN_TEST(test_output_json_basic);
    RUN_TEST(test_output_with_symbols);
    RUN_TEST(test_output_unknown_opcode);

    printf("\n[Disassembly]\n");
    RUN_TEST(test_disasm_simple_nop_halt);
    RUN_TEST(test_disasm_call_jmp);
    RUN_TEST(test_disasm_with_relocations);
    RUN_TEST(test_disasm_empty);
    RUN_TEST(test_disasm_truncated);
    RUN_TEST(test_disasm_all_none_ops);
    RUN_TEST(test_disasm_with_xref);
    RUN_TEST(test_buffer_api);

    printf("\n[Cross-Reference Analysis]\n");
    RUN_TEST(test_xref_basic);
    RUN_TEST(test_xref_with_symbols);
    RUN_TEST(test_xref_sort);
    RUN_TEST(test_xref_find_calls_to);
    RUN_TEST(test_collect_xrefs);

    printf("\n══════════════════════════════════════════════════\n");
    printf("  Results: %d/%d passed, %d failed\n",
           tests_passed, tests_run, tests_failed);
    printf("══════════════════════════════════════════════════\n");

    return (tests_failed == 0 && tests_passed == tests_run) ? 0 : 1;
}
