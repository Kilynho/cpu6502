# Instructions - Supported 6502 Instructions

## Best Practices

1. **Always update flags correctly** - This is critical for program correctness
2. **Test edge cases** - 0x00, 0xFF, boundary crossings
3. **Document cycle counts** - Include comments about timing
4. **Use helper functions** - `UpdateZeroAndNegativeFlags`, etc.
5. **Log memory access** - For debugging and tracing

## Example Usage with External Binary

```
./cpu_demo file ../examples/demo_program.bin
```

## Supported Instructions

- LDA (Immediate, Zero Page, Zero Page,X, Absolute, Absolute,X, Absolute,Y)
- LDX (Immediate)
- STA (Zero Page)
- JSR, RTS

Check the source code for implementation and logging details.

## Integration with I/O Devices

### Apple II I/O

The emulator supports Apple II I/O simulation through the `AppleIO` class:

- **Keyboard reading ($FD0C)**: Returns the next character from the input buffer, or 0 if there is no input.
- **Screen writing ($FDED)**: Sends the character to the screen buffer and prints it to the console.

### Example code

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "apple_io.hpp"

CPU cpu;
Mem mem;
auto appleIO = std::make_shared<AppleIO>();

cpu.Reset(mem);
cpu.registerIODevice(appleIO);

// Simulate keyboard input
appleIO->pushInput('A');

// Execute code that reads from $FD0C
// mem[0x8000] = 0xAD; mem[0x8001] = 0x0C; mem[0x8002] = 0xFD; // LDA $FD0C
// cpu.Execute(4, mem);
// The accumulator now contains 'A'

// Write to screen
// mem[0x8003] = 0x8D; mem[0x8004] = 0xED; mem[0x8005] = 0xFD; // STA $FDED
// cpu.Execute(4, mem);
// The screen now shows 'A'
```

### Extensibility

You can create your own I/O devices by implementing the `IODevice` interface and registering them with the CPU.
