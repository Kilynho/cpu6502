# 6502 Instructions Implementation Guide

This guide provides comprehensive information about implementing and testing 6502 instructions in this emulator.

## Table of Contents
- [Architecture Overview](#architecture-overview)
- [Instruction Categories](#instruction-categories)
- [Adding New Instructions](#adding-new-instructions)
- [Testing Instructions](#testing-instructions)
- [Instruction Reference](#instruction-reference)

## Architecture Overview

The emulator uses a modular architecture with separate components:

- **CPU Core** (`src/cpu/cpu.cpp`): Main CPU execution loop
- **Addressing Modes** (`src/cpu/addressing.cpp`): Handles all addressing mode calculations
- **Instruction Handlers** (`src/cpu/instructions.cpp`): Individual instruction implementations
- **Instruction Table**: Array[256] lookup table for fast opcode dispatch

## Instruction Categories

### Load/Store Instructions
Load and store data between registers and memory.

**Instructions:** LDA, LDX, LDY, STA, STX, STY

**Addressing Modes Supported:**
- Immediate: `LDA #$42`
- Zero Page: `LDA $42`
- Zero Page,X: `LDA $42,X`
- Zero Page,Y: `LDX $42,Y`
- Absolute: `LDA $4400`
- Absolute,X: `LDA $4400,X`
- Absolute,Y: `LDA $4400,Y`
- Indirect,X: `LDA ($42,X)`
- Indirect,Y: `LDA ($42),Y`

**Flags Affected:** Z, N

### Transfer Instructions
Transfer data between registers.

**Instructions:** TAX, TAY, TXA, TYA, TSX, TXS

**Flags Affected:** Z, N (except TXS)

### Stack Instructions
Push and pull data from the stack.

**Instructions:** PHA, PHP, PLA, PLP

**Flags Affected:** 
- PLA: Z, N
- PLP: All flags

### Logical Instructions
Perform bitwise logical operations.

**Instructions:** AND, EOR, ORA, BIT

**Addressing Modes:** Immediate, Zero Page, Zero Page,X, Absolute, Absolute,X, Absolute,Y, Indirect,X, Indirect,Y

**Flags Affected:**
- AND, EOR, ORA: Z, N
- BIT: Z, N, V

### Arithmetic Instructions
Perform arithmetic operations with carry.

**Instructions:** ADC, SBC

**Addressing Modes:** All standard modes

**Flags Affected:** Z, N, C, V

## Best Practices

1. **Always update flags correctly** - This is critical for program correctness
2. **Test edge cases** - 0x00, 0xFF, boundary crossings
3. **Document cycle counts** - Include comments about timing
4. **Use helper functions** - `UpdateZeroAndNegativeFlags`, etc.
5. **Log memory access** - For debugging and tracing
