# WOZMON - Apple 1 Monitor Integration

## Quick Start

```bash
cd /home/kilynho/src/cpu6502
mkdir -p build && cd build
cmake ..
make -j$(nproc)
./wozmon_demo
```

## Interactive Commands

Once running, you can use these commands:

```
? - Show this help
X8000 - Examine (view) memory at address 0x8000
.8000 - Block examine - show 128 bytes starting at 0x8000
R - Run program at current address
Q - Quit
```

## Example Session

```
> X0200
0200: 00 00 00 00 00 00 00 00

> .0000
0000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
0010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
...

> Q
Exiting WOZMON emulator
```

## Hardware Emulation

### PIA (Peripheral Interface Adapter)

The Apple 1 used a 6821 PIA chip for I/O:

| Address | Register | Purpose |
|---------|----------|---------|
| $D010 | KBD | Keyboard input (bit 7 = ready) |
| $D011 | KBDCR | Keyboard control |
| $D012 | DSP | Display output (bit 7 = ready) |
| $D013 | DSPCR | Display control |

### Memory Layout

```
0x0000-0x00FF - Zero page
0x0100-0x01FF - Stack  
0x0200-0x027F - Input buffer
0x0280-0xCFFF - User programs
0xD000-0xD0FF - PIA I/O
0xFF00-0xFFFF - WOZMON ROM
```

## Files

- `include/devices/pia.hpp` - PIA device header
- `src/devices/pia.cpp` - PIA device implementation
- `examples/wozmon_demo.cpp` - Interactive WOZMON demo
- `docs/wozmon_integration.md` - Complete documentation
- `src/wozmon/wozmon.asm` - Original WOZMON source code

## More Information

See `docs/wozmon_integration.md` for:
- Detailed command usage
- Technical architecture
- Historical context
- Future enhancements

## Historical Note

Steve Wozniak wrote the original WOZMON in 1976 as the primary user interface for the Apple 1. This emulation faithfully recreates that early interactive debugging experience.
