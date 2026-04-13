/*
 * flux-isa.h — FLUX Unified ISA: 247 opcodes
 * Complete instruction set definition for the FLUX bytecode architecture.
 */

#ifndef FLUX_ISA_H
#define FLUX_ISA_H

#include <stdint.h>

/* Operand encoding modes */
#define OP_NONE    0   /* No operands */
#define OP_R       1   /* Single register: 1 byte */
#define OP_RR      2   /* Two registers: 2 bytes (rd, rs) */
#define OP_RRR     3   /* Three registers: 3 bytes */
#define OP_RI16    4   /* Register + u16 immediate: 3 bytes */
#define OP_I16     5   /* Single u16 immediate: 2 bytes */
#define OP_I8      6   /* Single u8 immediate: 1 byte */
#define OP_RI8     7   /* Register + u8 immediate: 2 bytes */
#define OP_RRI8    8   /* Two registers + u8 immediate: 3 bytes */
#define OP_RRRI8   9   /* Three registers + u8 immediate: 4 bytes */
#define OP_I32    10   /* Single u32 immediate: 4 bytes */
#define OP_RI32   11   /* Register + u32 immediate: 5 bytes */
#define OP_COND   12   /* Condition code + u16: 3 bytes */

/* Number of opcodes */
#define FLUX_NUM_OPCODES 247

/* Maximum number of registers */
#define FLUX_MAX_REGS 256

/* Opcode categories */
typedef enum {
    CAT_CONTROL    = 0x00, /* 0x00–0x1F: Control flow */
    CAT_ARITH      = 0x20, /* 0x20–0x3F: Arithmetic & Logic */
    CAT_MEMORY     = 0x40, /* 0x40–0x5F: Memory operations */
    CAT_STACK      = 0x60, /* 0x60–0x6F: Stack operations */
    CAT_COMPARE    = 0x70, /* 0x70–0x7F: Compare & branch */
    CAT_CONFIDENCE = 0x80, /* 0x80–0x8F: Confidence ops */
    CAT_TRUST      = 0x90, /* 0x90–0x9F: Trust ops */
    CAT_A2A        = 0xA0, /* 0xA0–0xAF: Agent-to-agent */
    CAT_INSTINCT   = 0xB0, /* 0xB0–0xBF: Instinct ops */
    CAT_ENERGY     = 0xC0, /* 0xC0–0xCF: Energy/ATP */
    CAT_SYSTEM     = 0xD0, /* 0xD0–0xDF: System/debug */
    CAT_CRYPTO     = 0xE0, /* 0xE0–0xEF: Crypto/hash */
    CAT_VECTOR     = 0xF0, /* 0xF0–0xF6: SIMD/vector */
} FluxCategory;

typedef struct {
    const char *name;
    uint8_t code;
    uint8_t mode;
    uint8_t category;
} FluxOpInfo;

/* Complete 247-opcode table */
static const FluxOpInfo FLUX_OPCODES[FLUX_NUM_OPCODES] = {
    /* ── Control Flow (0x00–0x1F) ──────────────────────────────── */
    {"NOP",         0x00, OP_NONE,  CAT_CONTROL},
    {"MOV",         0x01, OP_RR,    CAT_CONTROL},
    {"LOAD",        0x02, OP_RR,    CAT_CONTROL},
    {"STORE",       0x03, OP_RR,    CAT_CONTROL},
    {"PUSH",        0x04, OP_I16,   CAT_CONTROL},
    {"POP",         0x05, OP_R,     CAT_CONTROL},
    {"JMP",         0x06, OP_I16,   CAT_CONTROL},
    {"JZ",          0x07, OP_RI16,  CAT_CONTROL},
    {"JNZ",         0x08, OP_RI16,  CAT_CONTROL},
    {"CALL",        0x09, OP_I16,   CAT_CONTROL},
    {"RET",         0x0A, OP_NONE,  CAT_CONTROL},
    {"HALT",        0x0B, OP_NONE,  CAT_CONTROL},
    {"BRK",         0x0C, OP_NONE,  CAT_CONTROL},
    {"SWI",         0x0D, OP_I8,    CAT_CONTROL},
    {"IRET",        0x0E, OP_NONE,  CAT_CONTROL},
    {"LOOP",        0x0F, OP_RI16,  CAT_CONTROL},
    {"LOOPE",       0x10, OP_RI16,  CAT_CONTROL},
    {"LOOPNE",      0x11, OP_RI16,  CAT_CONTROL},
    {"JC",          0x12, OP_RI16,  CAT_CONTROL},
    {"JNC",         0x13, OP_RI16,  CAT_CONTROL},
    {"JA",          0x14, OP_RI16,  CAT_CONTROL},
    {"JAE",         0x15, OP_RI16,  CAT_CONTROL},
    {"JB",          0x16, OP_RI16,  CAT_CONTROL},
    {"JBE",         0x17, OP_RI16,  CAT_CONTROL},
    {"JG",          0x18, OP_RI16,  CAT_CONTROL},
    {"JGE",         0x19, OP_RI16,  CAT_CONTROL},
    {"JL",          0x1A, OP_RI16,  CAT_CONTROL},
    {"JLE",         0x1B, OP_RI16,  CAT_CONTROL},
    {"JO",          0x1C, OP_RI16,  CAT_CONTROL},
    {"JNO",         0x1D, OP_RI16,  CAT_CONTROL},
    {"JS",          0x1E, OP_RI16,  CAT_CONTROL},
    {"JNS",         0x1F, OP_RI16,  CAT_CONTROL},

    /* ── Arithmetic & Logic (0x20–0x3F) ────────────────────────── */
    {"CADD",        0x20, OP_RRR,   CAT_ARITH},
    {"CSUB",        0x21, OP_RRR,   CAT_ARITH},
    {"CMUL",        0x22, OP_RRR,   CAT_ARITH},
    {"CDIV",        0x23, OP_RRR,   CAT_ARITH},
    {"CMOD",        0x24, OP_RRR,   CAT_ARITH},
    {"CAND",        0x25, OP_RRR,   CAT_ARITH},
    {"COR",         0x26, OP_RRR,   CAT_ARITH},
    {"CXOR",        0x27, OP_RRR,   CAT_ARITH},
    {"CSHL",        0x28, OP_RRR,   CAT_ARITH},
    {"CSHR",        0x29, OP_RRR,   CAT_ARITH},
    {"CSAR",        0x2A, OP_RRR,   CAT_ARITH},
    {"CNOT",        0x2B, OP_RR,    CAT_ARITH},
    {"CNEG",        0x2C, OP_RR,    CAT_ARITH},
    {"CABS",        0x2D, OP_RR,    CAT_ARITH},
    {"CMIN",        0x2E, OP_RRR,   CAT_ARITH},
    {"CMAX",        0x2F, OP_RRR,   CAT_ARITH},
    {"ADDI",        0x30, OP_RI16,  CAT_ARITH},
    {"SUBI",        0x31, OP_RI16,  CAT_ARITH},
    {"MULI",        0x32, OP_RI16,  CAT_ARITH},
    {"DIVI",        0x33, OP_RI16,  CAT_ARITH},
    {"ANDI",        0x34, OP_RI16,  CAT_ARITH},
    {"ORI",         0x35, OP_RI16,  CAT_ARITH},
    {"XORI",        0x36, OP_RI16,  CAT_ARITH},
    {"SHLI",        0x37, OP_RI16,  CAT_ARITH},
    {"SHRI",        0x38, OP_RI16,  CAT_ARITH},
    {"INCR",        0x39, OP_R,     CAT_ARITH},
    {"DECR",        0x3A, OP_R,     CAT_ARITH},
    {"CLZ",         0x3B, OP_RR,    CAT_ARITH},
    {"CTZ",         0x3C, OP_RR,    CAT_ARITH},
    {"POPCNT",      0x3D, OP_RR,    CAT_ARITH},
    {"BREV",        0x3E, OP_RR,    CAT_ARITH},
    {"BSWAP",       0x3F, OP_RR,    CAT_ARITH},

    /* ── Memory (0x40–0x5F) ────────────────────────────────────── */
    {"LOAD8",       0x40, OP_RI16,  CAT_MEMORY},
    {"LOAD16",      0x41, OP_RI16,  CAT_MEMORY},
    {"LOAD32",      0x42, OP_RI16,  CAT_MEMORY},
    {"STORE8",      0x43, OP_RI16,  CAT_MEMORY},
    {"STORE16",     0x44, OP_RI16,  CAT_MEMORY},
    {"STORE32",     0x45, OP_RI16,  CAT_MEMORY},
    {"LEA",         0x46, OP_RI16,  CAT_MEMORY},
    {"CMPXCHG",     0x47, OP_RRR,   CAT_MEMORY},
    {"XCHG",        0x48, OP_RR,    CAT_MEMORY},
    {"PUSH_MEM",    0x49, OP_RI16,  CAT_MEMORY},
    {"POP_MEM",     0x4A, OP_RI16,  CAT_MEMORY},
    {"LOADR",       0x4B, OP_RRR,   CAT_MEMORY},
    {"STORER",      0x4C, OP_RRR,   CAT_MEMORY},
    {"MEMCPY",      0x4D, OP_RI32,  CAT_MEMORY},
    {"MEMSET",      0x4E, OP_RI32,  CAT_MEMORY},
    {"MEMCMP",      0x4F, OP_RI32,  CAT_MEMORY},
    {"ALLOCA",      0x50, OP_RI16,  CAT_MEMORY},
    {"FREEA",       0x51, OP_R,     CAT_MEMORY},
    {"LOAD_IND",    0x52, OP_RR,    CAT_MEMORY},
    {"STORE_IND",   0x53, OP_RR,    CAT_MEMORY},
    {"LOAD_OFF",    0x54, OP_RRI8,  CAT_MEMORY},
    {"STORE_OFF",   0x55, OP_RRI8,  CAT_MEMORY},
    {"MOVZX",       0x56, OP_RR,    CAT_MEMORY},
    {"MOVSX",       0x57, OP_RR,    CAT_MEMORY},
    {"SWAP",        0x58, OP_RR,    CAT_MEMORY},
    {"PUSH_ALL",    0x59, OP_NONE,  CAT_MEMORY},
    {"POP_ALL",     0x5A, OP_NONE,  CAT_MEMORY},
    {"LOAD_IMM",    0x5B, OP_RI32,  CAT_MEMORY},
    {"STORE_IMM",   0x5C, OP_I32,   CAT_MEMORY},
    {"FENCE",       0x5D, OP_NONE,  CAT_MEMORY},
    {"TLB_INV",     0x5E, OP_NONE,  CAT_MEMORY},
    {"CACHE_FLUSH", 0x5F, OP_NONE,  CAT_MEMORY},

    /* ── Stack (0x60–0x6F) ─────────────────────────────────────── */
    {"PUSH_R",      0x60, OP_R,     CAT_STACK},
    {"POP_R",       0x61, OP_R,     CAT_STACK},
    {"DUP",         0x62, OP_NONE,  CAT_STACK},
    {"SWAP2",       0x63, OP_NONE,  CAT_STACK},
    {"ROT",         0x64, OP_NONE,  CAT_STACK},
    {"OVER",        0x65, OP_NONE,  CAT_STACK},
    {"PICK",        0x66, OP_I8,    CAT_STACK},
    {"ROLL",        0x67, OP_I8,    CAT_STACK},
    {"DEPTH",       0x68, OP_R,     CAT_STACK},
    {"DROP",        0x69, OP_NONE,  CAT_STACK},
    {"NIP",         0x6A, OP_NONE,  CAT_STACK},
    {"TUCK",        0x6B, OP_NONE,  CAT_STACK},
    {"PUSH_IMM8",   0x6C, OP_I8,    CAT_STACK},
    {"PUSH_IMM16",  0x6D, OP_I16,   CAT_STACK},
    {"PUSH_IMM32",  0x6E, OP_I32,   CAT_STACK},
    {"PEEK",        0x6F, OP_R,     CAT_STACK},

    /* ── Compare & Branch (0x70–0x7F) ──────────────────────────── */
    {"CMP_EQ",      0x70, OP_RRR,   CAT_COMPARE},
    {"CMP_NE",      0x71, OP_RRR,   CAT_COMPARE},
    {"CMP_LT",      0x72, OP_RRR,   CAT_COMPARE},
    {"CMP_GT",      0x73, OP_RRR,   CAT_COMPARE},
    {"CMP_LE",      0x74, OP_RRR,   CAT_COMPARE},
    {"CMP_GE",      0x75, OP_RRR,   CAT_COMPARE},
    {"CMP_ULT",     0x76, OP_RRR,   CAT_COMPARE},
    {"CMP_UGT",     0x77, OP_RRR,   CAT_COMPARE},
    {"TEST",        0x78, OP_RR,    CAT_COMPARE},
    {"CMP_I16",     0x79, OP_RI16,  CAT_COMPARE},
    {"RANGE_CHK",   0x7A, OP_RRI8,  CAT_COMPARE},
    {"SET_FLAG",    0x7B, OP_RI8,   CAT_COMPARE},
    {"GET_FLAG",    0x7C, OP_RI8,   CAT_COMPARE},
    {"CMP_TEST",    0x7D, OP_RR,    CAT_COMPARE},
    {"BOUND",       0x7E, OP_RI16,  CAT_COMPARE},
    {"CMPX",        0x7F, OP_RRR,   CAT_COMPARE},

    /* ── Confidence (0x80–0x8F) ────────────────────────────────── */
    {"CONF_FUSE",     0x80, OP_RR,    CAT_CONFIDENCE},
    {"CONF_CHAIN",    0x81, OP_R,     CAT_CONFIDENCE},
    {"CONF_SET",      0x82, OP_RI16,  CAT_CONFIDENCE},
    {"CONF_THRESH",   0x83, OP_RI16,  CAT_CONFIDENCE},
    {"CONF_DECAY",    0x84, OP_RR,    CAT_CONFIDENCE},
    {"CONF_BOOST",    0x85, OP_RR,    CAT_CONFIDENCE},
    {"CONF_MERGE",    0x86, OP_RRR,   CAT_CONFIDENCE},
    {"CONF_SPLIT",    0x87, OP_RR,    CAT_CONFIDENCE},
    {"CONF_QUERY",    0x88, OP_R,     CAT_CONFIDENCE},
    {"CONF_RESET",    0x89, OP_R,     CAT_CONFIDENCE},
    {"CONF_MIN",      0x8A, OP_RI16,  CAT_CONFIDENCE},
    {"CONF_MAX",      0x8B, OP_RI16,  CAT_CONFIDENCE},
    {"CONF_AVG",      0x8C, OP_RRR,   CAT_CONFIDENCE},
    {"CONF_WEIGHT",   0x8D, OP_RRI8,  CAT_CONFIDENCE},
    {"CONF_NORMALIZE",0x8E, OP_RR,    CAT_CONFIDENCE},
    {"CONF_SNAPSHOT", 0x8F, OP_R,     CAT_CONFIDENCE},

    /* ── Trust (0x90–0x9F) ─────────────────────────────────────── */
    {"TRUST_CHECK",    0x90, OP_RR,    CAT_TRUST},
    {"TRUST_UPDATE",   0x91, OP_RRR,   CAT_TRUST},
    {"TRUST_QUERY",    0x92, OP_R,     CAT_TRUST},
    {"TRUST_SET",      0x93, OP_RI16,  CAT_TRUST},
    {"TRUST_DECAY",    0x94, OP_RR,    CAT_TRUST},
    {"TRUST_BOOST",    0x95, OP_RR,    CAT_TRUST},
    {"TRUST_TRANSFER", 0x96, OP_RRR,   CAT_TRUST},
    {"TRUST_VERIFY",   0x97, OP_RR,    CAT_TRUST},
    {"TRUST_REVOKE",   0x98, OP_R,     CAT_TRUST},
    {"TRUST_LIST",     0x99, OP_R,     CAT_TRUST},
    {"TRUST_SCORE",    0x9A, OP_RR,    CAT_TRUST},
    {"TRUST_THRESHOLD",0x9B, OP_RI16,  CAT_TRUST},
    {"TRUST_AUDIT",    0x9C, OP_R,     CAT_TRUST},
    {"TRUST_RESTORE",  0x9D, OP_R,     CAT_TRUST},
    {"TRUST_HISTORY",  0x9E, OP_RI16,  CAT_TRUST},
    {"TRUST_PENALTY",  0x9F, OP_RRI8,  CAT_TRUST},

    /* ── Agent-to-Agent (0xA0–0xAF) ────────────────────────────── */
    {"TELL",        0xA0, OP_RR,    CAT_A2A},
    {"ASK",         0xA1, OP_RR,    CAT_A2A},
    {"DELEGATE",    0xA2, OP_RR,    CAT_A2A},
    {"BROADCAST",   0xA3, OP_I16,   CAT_A2A},
    {"REDUCE",      0xA4, OP_RR,    CAT_A2A},
    {"GOSSIP",      0xA5, OP_RR,    CAT_A2A},
    {"PROPOSE",     0xA6, OP_RRR,   CAT_A2A},
    {"VOTE",        0xA7, OP_RR,    CAT_A2A},
    {"CONSENSUS",   0xA8, OP_R,     CAT_A2A},
    {"NEGOTIATE",   0xA9, OP_RRR,   CAT_A2A},
    {"MEDIATE",     0xAA, OP_RR,    CAT_A2A},
    {"PUBLISH",     0xAB, OP_R,     CAT_A2A},
    {"SUBSCRIBE",   0xAC, OP_RI16,  CAT_A2A},
    {"UNSUBSCRIBE", 0xAD, OP_RI16,  CAT_A2A},
    {"RELAY",       0xAE, OP_RRR,   CAT_A2A},
    {"ROUTE",       0xAF, OP_RI16,  CAT_A2A},

    /* ── Instinct (0xB0–0xBF) ──────────────────────────────────── */
    {"INSTINCT_ACT",  0xB0, OP_R,     CAT_INSTINCT},
    {"INSTINCT_QRY",  0xB1, OP_R,     CAT_INSTINCT},
    {"INSTINCT_LEARN",0xB2, OP_RRR,   CAT_INSTINCT},
    {"INSTINCT_FORGET",0xB3,OP_R,     CAT_INSTINCT},
    {"INSTINCT_PRUNE",0xB4, OP_R,     CAT_INSTINCT},
    {"INSTINCT_BLOOM",0xB5, OP_R,     CAT_INSTINCT},
    {"INSTINCT_MERGE",0xB6, OP_RRR,   CAT_INSTINCT},
    {"INSTINCT_FORK", 0xB7, OP_RR,    CAT_INSTINCT},
    {"INSTINCT_EVOLVE",0xB8,OP_R,     CAT_INSTINCT},
    {"INSTINCT_SCORE", 0xB9,OP_RR,    CAT_INSTINCT},
    {"INSTINCT_SET",   0xBA,OP_RI8,   CAT_INSTINCT},
    {"INSTINCT_GET",   0xBB,OP_RR,    CAT_INSTINCT},
    {"INSTINCT_RESET", 0xBC,OP_R,     CAT_INSTINCT},
    {"INSTINCT_EXPORT",0xBD,OP_R,     CAT_INSTINCT},
    {"INSTINCT_IMPORT",0xBE,OP_R,     CAT_INSTINCT},
    {"INSTINCT_PRIME", 0xBF,OP_RR,    CAT_INSTINCT},

    /* ── Energy/ATP (0xC0–0xCF) ────────────────────────────────── */
    {"ATP_GEN",     0xC0, OP_R,     CAT_ENERGY},
    {"ATP_USE",     0xC1, OP_R,     CAT_ENERGY},
    {"ATP_QRY",     0xC2, OP_NONE,  CAT_ENERGY},
    {"ATP_XFER",    0xC3, OP_RRR,   CAT_ENERGY},
    {"APOPTOSIS",   0xC4, OP_NONE,  CAT_ENERGY},
    {"ENERGY_POOL", 0xC5, OP_R,     CAT_ENERGY},
    {"ENERGY_DRAIN",0xC6, OP_RR,    CAT_ENERGY},
    {"ENERGY_SHARE",0xC7, OP_RRR,   CAT_ENERGY},
    {"ENERGY_RESERVE",0xC8,OP_RI16, CAT_ENERGY},
    {"ENERGY_RELEASE",0xC9,OP_R,    CAT_ENERGY},
    {"ENERGY_MIN",  0xCA, OP_RI16,  CAT_ENERGY},
    {"ENERGY_MAX",  0xCB, OP_RI16,  CAT_ENERGY},
    {"ENERGY_SYNC", 0xCC, OP_RR,    CAT_ENERGY},
    {"ENERGY_QUERY",0xCD, OP_R,     CAT_ENERGY},
    {"ENERGY_MONITOR",0xCE,OP_R,    CAT_ENERGY},
    {"ENERGY_ALARM",0xCF, OP_RI16,  CAT_ENERGY},

    /* ── System/Debug (0xD0–0xDF) ──────────────────────────────── */
    {"DBG_PRINT",   0xD0, OP_R,     CAT_SYSTEM},
    {"BARRIER",     0xD1, OP_NONE,  CAT_SYSTEM},
    {"RESOURCE",    0xD2, OP_RR,    CAT_SYSTEM},
    {"TIMER_START", 0xD3, OP_R,     CAT_SYSTEM},
    {"TIMER_STOP",  0xD4, OP_R,     CAT_SYSTEM},
    {"TIMER_READ",  0xD5, OP_R,     CAT_SYSTEM},
    {"LOG",         0xD6, OP_RI8,   CAT_SYSTEM},
    {"TRACE",       0xD7, OP_NONE,  CAT_SYSTEM},
    {"ASSERT",      0xD8, OP_RR,    CAT_SYSTEM},
    {"PANIC",       0xD9, OP_I16,   CAT_SYSTEM},
    {"YIELD",       0xDA, OP_NONE,  CAT_SYSTEM},
    {"SLEEP",       0xDB, OP_I16,   CAT_SYSTEM},
    {"WAKE",        0xDC, OP_R,     CAT_SYSTEM},
    {"SPAWN",       0xDD, OP_I16,   CAT_SYSTEM},
    {"JOIN",        0xDE, OP_R,     CAT_SYSTEM},
    {"KILL",        0xDF, OP_R,     CAT_SYSTEM},

    /* ── Crypto/Hash (0xE0–0xEF) ───────────────────────────────── */
    {"HASH_MD5",    0xE0, OP_RR,    CAT_CRYPTO},
    {"HASH_SHA256", 0xE1, OP_RR,    CAT_CRYPTO},
    {"HASH_SHA512", 0xE2, OP_RR,    CAT_CRYPTO},
    {"HMAC_GEN",    0xE3, OP_RRR,   CAT_CRYPTO},
    {"HMAC_VERIFY", 0xE4, OP_RRR,   CAT_CRYPTO},
    {"AES_ENC",     0xE5, OP_RRR,   CAT_CRYPTO},
    {"AES_DEC",     0xE6, OP_RRR,   CAT_CRYPTO},
    {"SIG_GEN",     0xE7, OP_RR,    CAT_CRYPTO},
    {"SIG_VERIFY",  0xE8, OP_RRR,   CAT_CRYPTO},
    {"RNG_SEED",    0xE9, OP_R,     CAT_CRYPTO},
    {"RNG_NEXT",    0xEA, OP_R,     CAT_CRYPTO},
    {"CRC32",       0xEB, OP_RR,    CAT_CRYPTO},
    {"BASE64_ENC",  0xEC, OP_RR,    CAT_CRYPTO},
    {"BASE64_DEC",  0xED, OP_RR,    CAT_CRYPTO},
    {"KEY_GEN",     0xEE, OP_R,     CAT_CRYPTO},
    {"KEY_DERIVE",  0xEF, OP_RRR,   CAT_CRYPTO},

    /* ── Vector/SIMD (0xF0–0xF6) ──────────────────────────────── */
    {"VADD",        0xF0, OP_RRR,   CAT_VECTOR},
    {"VSUB",        0xF1, OP_RRR,   CAT_VECTOR},
    {"VMUL",        0xF2, OP_RRR,   CAT_VECTOR},
    {"VDOT",        0xF3, OP_RRR,   CAT_VECTOR},
    {"VLOAD",       0xF4, OP_RI16,  CAT_VECTOR},
    {"VSTORE",      0xF5, OP_RI16,  CAT_VECTOR},
    {"VMAP",        0xF6, OP_RRR,   CAT_VECTOR},
};

/* Category names */
static const char *FLUX_CATEGORY_NAMES[] = {
    "Control", "Arithmetic", "Memory", "Stack",
    "Compare", "Confidence", "Trust", "A2A",
    "Instinct", "Energy", "System", "Crypto", "Vector",
};

#endif /* FLUX_ISA_H */
