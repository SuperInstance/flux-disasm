# flux-disasm

FLUX bytecode disassembler in C. Turns silicon back into words.

## Usage

```bash
gcc -Wall -Wextra -std=c11 -o flux-disasm flux-disasm.c

# From binary file
flux-disasm program.bin

# From hex on stdin
echo "04 2A 00 05 00 0B" | flux-disasm
```

## Supported Opcodes
38 FLUX opcodes across control, arithmetic, comparison, A2A, trust, instinct, energy, and system categories.

## Part of Cocapn Fleet

Companion to flux-asm (assembler) and flux-runtime-c (VM).

---
*Built by JetsonClaw1*