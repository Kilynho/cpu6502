# CPU 6502 Emulator Architecture

## Overview

This document describes the architecture of the CPU 6502 emulator after the modular refactoring.

## Project Structure

```
cpu6502/
├── include/                    # Public API headers
│   ├── cpu.hpp                # CPU interface
│   ├── cpu_addressing.hpp     # Addressing modes interface
│   ├── cpu_instructions.hpp   # Instruction handlers interface
│   ├── mem.hpp                # Memory interface
│   └── util/
│       └── logger.hpp         # Logging system
├── src/                       # Implementation files
│   ├── cpu/
│   │   ├── cpu.cpp           # CPU implementation
│   │   ├── addressing.cpp    # Addressing modes implementation
│   │   └── instructions.cpp  # Instruction handlers implementation
│   ├── mem/
│   │   └── mem.cpp           # Memory implementation
│   ├── util/
│   │   └── logger.cpp        # Logger implementation
│   └── main/
│       └── cpu_demo.cpp      # Demo program
├── tests/                     # Test suite
│   ├── test_main.cpp         # Original tests
│   └── instruction_handlers_test.cpp  # New instruction tests
├── docs/                      # Documentation
│   ├── instructions.md       # Instruction implementation guide
│   └── architecture.md       # This file
└── lib/
    └── googletest/           # Google Test framework
```

## Core Components

### CPU (`cpu.hpp` / `cpu.cpp`)
The main CPU class that emulates the 6502 processor.

**Key Features:**
- 8-bit registers: A, X, Y, SP
- 16-bit program counter: PC
- Status flags: N, V, B, D, I, Z, C
- Execute loop that fetches and executes instructions
- Memory access logging
- Stack operations

**Public Interface:**
```cpp
class CPU {
public:
    void Reset(Mem& memory);
    void Execute(u32 Cycles, Mem& memory);
    // ... registers and methods
};
```

### Memory (`mem.hpp` / `mem.cpp`)
64KB addressable memory space.

**Memory Map:**
- `0x0000-0x00FF`: Zero Page (fast access)
- `0x0100-0x01FF`: Stack
- `0x0200-0x7FFF`: RAM
- `0x8000-0xFFFF`: ROM
- `0xFFFC-0xFFFD`: Reset Vector
- `0xFFFE-0xFFFF`: IRQ/BRK Vector

### Addressing Modes (`cpu_addressing.hpp` / `addressing.cpp`)
Handles all 6502 addressing mode calculations.

**Supported Modes:**
- Immediate: Operand is in the next byte
- Zero Page: Address in zero page (0x00-0xFF)
- Zero Page,X/Y: Zero page address + X/Y register
- Absolute: Full 16-bit address
- Absolute,X/Y: 16-bit address + X/Y register
- Indirect,X: Indexed indirect (pre-indexed)
- Indirect,Y: Indirect indexed (post-indexed)
- Indirect: Used for JMP instruction

**Key Functions:**
```cpp
namespace Addressing {
    Word Immediate(CPU& cpu, u32& cycles, Mem& memory);
    Word ZeroPage(CPU& cpu, u32& cycles, Mem& memory);
    Word Absolute(CPU& cpu, u32& cycles, Mem& memory);
    // ...
}
```

### Instruction Handlers (`cpu_instructions.hpp` / `instructions.cpp`)
Individual instruction implementations organized by category.

**Architecture:**
- **Instruction Table**: Static array[256] of function pointers
- **Handler Functions**: One function per instruction operation
- **Opcode Mapping**: Each opcode maps to handler + addressing mode

**Example:**
```cpp
// Instruction implementation
void LDA(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.A = memory[address];
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

// Opcode mapping
instructionTable[0xA9] = [](CPU& cpu, u32& cycles, Mem& memory) {
    LDA(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
};
```

### Logger (`util/logger.hpp` / `logger.cpp`)
Configurable logging system.

**Log Levels:**
- NONE: No logging
- ERROR: Errors only
- WARN: Warnings and errors
- INFO: Informational messages
- DEBUG: Detailed debugging info

## Design Patterns

### Separation of Concerns
Each component has a single, well-defined responsibility:
- CPU: Execution and state management
- Memory: Data storage
- Addressing: Address calculation
- Instructions: Operation implementation

### Function Pointers / Lambdas
The instruction table uses lambdas to combine handlers with addressing modes:
```cpp
instructionTable[opcode] = [](CPU& cpu, u32& cycles, Mem& memory) {
    Handler(cpu, cycles, memory, AddressingMode(cpu, cycles, memory));
};
```

### Header/Implementation Split
Public API in `include/`, implementation in `src/`.

Benefits:
- Clean separation between interface and implementation
- Easier to maintain and test
- Library can be reused in other projects

## Execution Flow

1. **Reset**: CPU initializes, loads reset vector from 0xFFFC-0xFFFD
2. **Fetch**: Read opcode from memory at PC
3. **Decode**: Look up handler in instruction table
4. **Execute**: Call handler which:
   - Calls addressing mode function to get operand address
   - Performs the operation
   - Updates CPU state and flags
   - Consumes cycles
5. **Repeat**: Continue until cycles exhausted

## Cycle Counting

Each operation consumes CPU cycles:
- **Fetch**: 1 cycle per byte
- **Addressing**: 1-4 cycles depending on mode
- **Operation**: 1-3 cycles depending on instruction
- **Page Crossing**: +1 cycle if page boundary crossed

The `cycles` parameter is passed by reference and decremented as operations consume time.

## Flag Updates

Status flags are updated according to 6502 specifications:

**Helper Functions:**
```cpp
void UpdateZeroAndNegativeFlags(CPU& cpu, Byte value);
void UpdateCarryFlag(CPU& cpu, bool carry);
void UpdateOverflowFlag(CPU& cpu, bool overflow);
```

**Flag Conditions:**
- **Z** (Zero): Result is 0x00
- **N** (Negative): Bit 7 of result is set
- **C** (Carry): Varies by instruction (overflow, borrow, bit shifted out)
- **V** (Overflow): Signed overflow occurred

## Testing Strategy

### Unit Tests
Each component is tested independently:
- **Instruction Handlers**: Test individual instruction logic
- **Addressing Modes**: Test address calculations
- **Integration**: Test complete instruction execution

### Test Structure
```cpp
TEST_F(InstructionHandlersTest, TestName) {
    // Setup: Initialize CPU and memory
    // Execute: Call instruction handler
    // Verify: Check results and flags
}
```

### Coverage
- Basic operation
- Flag behavior
- Edge cases (0x00, 0xFF, wraparound)
- All addressing modes
- Page boundary crossings

## Build System

### CMake Structure
```
CMakeLists.txt (root)
├── src/CMakeLists.txt (library + demo)
└── tests/CMakeLists.txt (test suite)
```

**Targets:**
- `cpu6502_lib`: Static library
- `cpu_demo`: Demo executable
- `runTests`: Test executable

### Building
```bash
mkdir build && cd build
cmake ..
make
```

### Testing
```bash
make test  # Run with CTest
./runTests # Run directly
```

## Future Enhancements

### Potential Improvements
1. **Decimal Mode**: Full BCD arithmetic support
2. **Timing Accuracy**: Precise cycle-by-cycle emulation
3. **Interrupts**: NMI and IRQ handling
4. **Debugger**: Step-through debugging interface
5. **Disassembler**: Convert memory to assembly
6. **Performance**: JIT compilation for speed

### Extensibility
The modular design allows easy extension:
- Add new instructions by implementing handler and adding to table
- Add new addressing modes in addressing.cpp
- Add custom peripherals by extending Memory class
- Add debugging features without changing core

## References

- [6502 Architecture](http://www.6502.org/)
- [Instruction Set Reference](http://www.6502.org/tutorials/6502opcodes.html)
- [Visual 6502](http://www.visual6502.org/)
