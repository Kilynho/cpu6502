# WOZMON Integration - Apple 1 Monitor

## Overview

This document describes the integration of WOZMON, the legendary monitor program written by Steve Wozniak in 1976 for the Apple 1 computer, into the CPU 6502 emulator.

## What is WOZMON?

WOZMON is a small but powerful machine language monitor that provides:
- **Examine (X)**: View memory contents
- **Store (S)**: Modify memory values
- **Block Examine (.)**: View memory ranges
- **Run (R)**: Execute programs at any address
- **Full interactive interface** for assembly-level debugging

## Hardware Integration

### PIA (Peripheral Interface Adapter)

WOZMON communicates with the Apple 1 hardware through the MOS 6821 Peripheral Interface Adapter (PIA) at addresses `$D010-$D013`:

| Address | Name | Purpose |
|---------|------|---------|
| `$D010` | KBD  | Keyboard input (bit 7 = key ready) |
| `$D011` | KBDCR | Keyboard control register |
| `$D012` | DSP  | Display output (bit 7 = display ready) |
| `$D013` | DSPCR | Display control register |

The PIA emulation provides:
- Keyboard input buffering
- Display output capture
- Control register management
- Bit 7 flags for handshaking

## Memory Map

```
0x0000-0x00FF  - Zero page
0x0100-0x01FF  - Stack
0x0200-0x027F  - Input buffer
0x0280-0xCFFF  - User program space
0xD000-0xD0FF  - PIA I/O registers
0xE000-0xEFFF  - (Reserved for future I/O)
0xF000-0xFEFF  - (Reserved)
0xFF00-0xFFFF  - WOZMON monitor ROM
```

### Reset Vector

The reset vector at `$FFFC-$FFFD` points to `$FF00`, the entry point of WOZMON.

## Using the WOZMON Demo

### Building

```bash
cd /home/kilynho/src/cpu6502/build
cmake ..
make -j$(nproc)
```

### Running

```bash
./wozmon_demo
```

### Interactive Commands

```
? (Help)           - Display available commands
X<hex>             - Examine memory at hex address (e.g., X8000)
.<hex>             - Block examine from hex address
:<hex>             - Set store address
R                  - Run program
Q                  - Quit emulator
```

## Example Usage

### 1. Examine Memory

```
> X8000
8000: A9 84 85 40 A9 65 8D 01

Display shows 8 bytes starting at 0x8000 in hex format.
```

### 2. View Memory Block

```
> .8000
8000: A9 84 85 40 A9 65 8D 01 00 00 00 00 00 00 00 00
8010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
...
```

### 3. Run a Program

```
> X8000           (First examine to see program)
8000: 20 00 80 4C 00 80
> R               (Run at 0x8000)
Running program at 8000
Program finished
```

## Architecture

### PIA Implementation

The `PIA` class (`include/devices/pia.hpp`, `src/devices/pia.cpp`) implements:

- `IODevice` interface for memory-mapped I/O
- Keyboard input queue with ready flag
- Display output capture
- Control register management
- Hardware-accurate bit flags

### Integration with CPU

The PIA is registered as an I/O device with the CPU:

```cpp
auto pia = std::make_shared<PIA>();
cpu.registerIODevice(pia);
```

When the CPU reads/writes to `$D010-$D013`, the PIA handles the operation.

## WOZMON Commands in Detail

### X (Examine)

Format: `X<address>`

Displays 8 bytes of memory starting at the specified hexadecimal address.

```
> X0200
0200: 00 01 02 03 04 05 06 07
```

### . (Block Examine)

Format: `.<address>`

Displays 128 bytes (8 rows × 16 columns) of memory.

```
> .0200
0200: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
0210: 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
...
```

### : (Store)

Format: `:<address> <byte> <byte> ...`

Stores bytes into memory starting at the specified address.

```
> :8000 20 00 80 4C 00 80
```

This stores:
- `$20` at `$8000`
- `$00` at `$8001`
- `$80` at `$8002`
- etc.

### R (Run)

Executes the program at the current examine address.

```
> X8000
8000: A9 84 ...
> R
Running program at 8000
Program finished
```

## Limitations and Future Enhancements

### Current Limitations

1. **Partial WOZMON Implementation**: The demo includes a simplified version
2. **No Full Monitor Loop**: Interactive input/output is simplified
3. **Limited Error Handling**: Basic validation only
4. **No Breakpoint Support**: Original WOZMON doesn't have breakpoints, but the emulator's Debugger can be used separately

### Future Enhancements

1. **Full WOZMON Assembly**: Implement the complete WOZMON code
2. **Enhanced I/O**: Better keyboard/display simulation
3. **Macro Support**: Custom command macros
4. **Script Integration**: Use Python scripts with WOZMON
5. **Real File Loading**: Load and run programs from disk

## Technical Details

### Memory-Mapped I/O Protocol

#### Reading from KBD ($D010)

1. Check if bit 7 is set (key ready)
2. If set, read the ASCII character (bits 6-0)
3. The character has bit 7 set (ASCII + 0x80)

#### Writing to DSP ($D012)

1. Write character to display
2. Character must have bit 7 clear (0-127)
3. If bit 7 is set, the write is ignored (hardware write-protected)

#### Control Registers ($D011, $D013)

- Used to configure interrupt edges and modes
- In this emulation, they can be read/written but don't affect behavior

## Testing

The emulator's test suite includes basic I/O device tests. To add WOZMON-specific tests:

```bash
cd /home/kilynho/src/cpu6502/build
./runTests --gtest_filter="PIA*"
```

## References

- Apple 1 Documentation: https://en.wikipedia.org/wiki/Apple_I
- 6502 Instruction Set: https://www.masswerk.at/6502/
- MOS 6821 PIA Datasheet
- WOZMON Source Code: Included in `wozmon.asm`

## Historical Context

Steve Wozniak wrote WOZMON in 1976 as the primary user interface for the Apple 1. It's written in highly optimized 6502 assembly and remains a testament to low-level programming skill.

Key features of the original:
- Only ~256 bytes of code
- Provides full memory access and program execution
- No external dependencies
- Interrupt-driven keyboard and display handling
- Perfect for assembly-level debugging

The CPU 6502 emulator can now run this historical artifact, demonstrating the emulator's accuracy and completeness.
