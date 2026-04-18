# flux-disasm

**FLUX Disassembler — Turns silicon back into words**

A C11 disassembler that converts FLUX bytecode (binary or hex) into human-readable assembly listings.

## What It Does

flux-disasm reads FLUX bytecode produced by flux-asm and displays it as labeled, indented assembly text. Supports both binary files and hex input from stdin.

## Quick Start

```bash
# Clone
git clone https://github.com/Lucineer/flux-disasm.git
cd flux-disasm

# Compile
gcc -Wall -Wextra -std=c11 -Werror -O2 -o flux-disasm src/flux-disasm.c

# Disassemble a binary file
./flux-disasm program.bin

# Disassemble hex from stdin
echo "04 2A 00 05 00 0B" | ./flux-disasm

# Roundtrip with assembler
./flux-asm test.asm | ./flux-disasm
```

### Cross-Compile for ARM64

```bash
aarch64-linux-gnu-gcc -Wall -Wextra -std=c11 -Werror -O2 -o flux-disasm src/flux-disasm.c
```

## Output Format

```
; flux-disasm: 29 bytes
  0000: PUSH         0x2A
  0003: POP          r0
  0005: PUSH         0x3A
  0008: POP          r1
  000A: CADD         r2 r0 r1
  000E: MOV          r3 r2
  0011: CONF_SET     r3 0x5F
  0015: TELL         r0 r1
  0018: ATP_GEN      r4
  001A: ATP_USE      r4
  001C: HALT
```

## Operand Encoding Modes

The disassembler understands all FLUX operand formats:

| Mode | Description | Size | Example |
|------|-------------|------|---------|
| OP_NONE | No operands | 0 bytes | `HALT` |
| OP_R | Single register | 1 byte | `POP r0` |
| OP_RR | Two registers | 2 bytes | `MOV r3 r2` |
| OP_RR_RI | Three registers | 3 bytes | `CADD r2 r0 r1` |
| OP_R_RI16 | Register + u16 immediate | 3 bytes | `JZ r3 0x5F` |
| OP_I16 | Single u16 immediate | 2 bytes | `PUSH 0x2A` |

## Supported Opcodes (38)

| Category | Opcodes |
|----------|---------|
| Control | NOP, MOV, LOAD, STORE, PUSH, POP, JMP, JZ, JNZ, CALL, RET, HALT |
| Arithmetic | CADD, CSUB, CMUL, CDIV |
| Confidence | CONF_FUSE, CONF_CHAIN, CONF_SET, CONF_THRESH |
| Compare | CMP_EQ, CMP_NE, CMP_LT, CMP_GT |
| A2A | TELL, ASK, DELEGATE, BROADCAST, REDUCE |
| Trust | TRUST_CHECK, TRUST_UPDATE, TRUST_QUERY |
| Instinct | INSTINCT_ACT, INSTINCT_QRY |
| Energy | ATP_GEN, ATP_USE, ATP_QRY, ATP_XFER, APOPTOSIS |
| System | DBG_PRINT, BARRIER, RESOURCE |

## Input Modes

### Binary File
```bash
./flux-disasm program.bin
```
Reads raw bytes from a file.

### Hex from Stdin
```bash
echo "04 2A 00 0B" | ./flux-disasm
```
Pairs of hex digits (space, newline, or tab separated).

### Binary from Pipe
```bash
./flux-asm test.asm | xxd -r -p | ./flux-disasm
```

## Roundtrip Verification

```bash
# Assemble → disassemble → should match original
echo 'PUSH 42
POP r0
HALT' | flux-asm | flux-disasm
# Output: PUSH 0x2A / POP r0 / HALT ✓
```

Verified with flux-asm: 29-byte program, all opcodes roundtrip correctly.

## Companion Tools

| Tool | Repo | Description |
|------|------|-------------|
| [flux-runtime-c](https://github.com/Lucineer/flux-runtime-c) | C11 | Execute FLUX bytecode |
| [flux-asm](https://github.com/Lucineer/flux-asm) | C11 | Assemble text to bytecode |

## Role in the FLUX Fleet

`flux-disasm` is the lightweight C disassembler in the fleet toolchain — perfect for embedded environments and cross-compilation where Python runtimes aren't available. It pairs with the Python-based [`flux-decompiler`](https://github.com/SuperInstance/flux-decompiler) which adds jump resolution, control flow annotation, and labeled output.

**Fleet toolchain flow:**
```
flux-asm → [bytecode] → flux-disasm (C, lightweight)
                         ↘ flux-decompiler (Python, annotated)
                         ↘ flux-timeline (tracing)
                         ↘ flux-profiler (profiling)
                         ↘ flux-debugger (debugging)
```

### Related Fleet Repos

- [`flux-decompiler`](https://github.com/SuperInstance/flux-decompiler) — Python decompiler with annotations
- [`flux-signatures`](https://github.com/SuperInstance/flux-signatures) — Pattern detection in bytecode
- [`flux-timeline`](https://github.com/SuperInstance/flux-timeline) — Execution tracing
- [`flux-profiler`](https://github.com/SuperInstance/flux-profiler) — Performance profiling
- [`flux-debugger`](https://github.com/SuperInstance/flux-debugger) — Step debugger

## License

MIT

---

*Built by JetsonClaw1 — verified on Jetson Super Orin Nano 8GB, ARM64*
*Part of the [SuperInstance](https://github.com/SuperInstance) FLUX fleet.*
