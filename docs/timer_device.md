# TimerDevice - Timer Device Documentation

## Overview

`TimerDevice` is an interface that defines the basic operations for timer devices in the 6502 emulator. The `BasicTimer` implementation provides a programmable timer that counts CPU cycles and can generate periodic IRQ interrupts, similar to the timer chips of classic 8-bit computers (VIA 6522, CIA 6526, PIA 6821, etc.).

## Features

- **32-bit Counter**: Ability to count up to 4,294,967,295 cycles
- **Configurable Limit**: Defines when to generate IRQ or stop the counter
- **IRQ Generation**: Periodic interrupts when the counter reaches the limit
- **Auto-reload Mode**: Automatic restart of the counter for periodic timers
- **Memory-mapped Registers**: Direct access from 6502 code
- **C++ API**: Programmatic control from emulator code
- **Thread-safe**: Safe operations for multi-threaded use

## Architecture

### TimerDevice Interface

```cpp
class TimerDevice : public IODevice {
public:
    virtual bool initialize() = 0;
    virtual uint32_t getCounter() const = 0;
    virtual void setCounter(uint32_t value) = 0;
    virtual void reset() = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual bool hasIRQ() const = 0;
    virtual void clearIRQ() = 0;
    virtual void tick(uint32_t cycles) = 0;
    virtual void cleanup() = 0;
};
```

### BasicTimer Implementation

`BasicTimer` implements the `TimerDevice` interface with a 32-bit counter that increments with each executed CPU cycle.

## Memory Map

The timer registers are mapped to the following addresses:

| Address | Name | Description |
|-----------|--------|-------------|
| `$FC00` | COUNTER_LOW | Low byte of the counter (bits 0-7) |
| `$FC01` | COUNTER_MID1 | Middle byte 1 of the counter (bits 8-15) |
| `$FC02` | COUNTER_MID2 | Middle byte 2 of the counter (bits 16-23) |
| `$FC03` | COUNTER_HIGH | High byte of the counter (bits 24-31) |
| `$FC04` | LIMIT_LOW | Low byte of the limit (bits 0-7) |
| `$FC05` | LIMIT_MID1 | Middle byte 1 of the limit (bits 8-15) |
| `$FC06` | LIMIT_MID2 | Middle byte 2 of the limit (bits 16-23) |
| `$FC07` | LIMIT_HIGH | High byte of the limit (bits 24-31) |
| `$FC08` | CONTROL | Control and configuration |
| `$FC09` | STATUS | Status (read-only) |

### Control Register ($FC08)

| Bit | Name | Description |
|-----|--------|-------------|
| 0 | ENABLE | 1=Timer enabled, 0=Disabled |
| 1 | IRQ_ENABLE | 1=Generate IRQ on limit reached, 0=Do not generate |
| 2 | IRQ_FLAG | Write: 1=Clear pending IRQ |
| 3 | RESET | Write: 1=Reset counter to 0 |
| 4 | AUTO_RELOAD | 1=Auto-restart, 0=Stop at limit |
| 5-7 | - | Reserved |

### Status Register ($FC09)

| Bit | Name | Description |
|-----|--------|-------------|
| 0 | ENABLED | 1=Timer enabled, 0=Disabled |
| 1 | IRQ_PENDING | 1=IRQ pending, 0=No IRQ |
| 2 | LIMIT_REACHED | 1=Counter reached the limit, 0=Not reached |
| 3-7 | - | Reserved |

## Usage from 6502 Code

### Example 1: Read Elapsed Time

```assembly
; Read the current counter (32 bits)
LDA $FC00       ; Low byte
STA TIME        
LDA $FC01       ; Middle byte 1
STA TIME+1      
LDA $FC02       ; Middle byte 2
STA TIME+2      
LDA $FC03       ; High byte
STA TIME+3      

; Now TIME contains the elapsed cycles
```

### Example 2: Configure Periodic IRQ

```assembly
; Set limit: 1,000,000 cycles (0x000F4240)
LDA #$40        ; Low byte
STA $FC04       
LDA #$42        ; Middle byte 1
STA $FC05       
LDA #$0F        ; Middle byte 2
STA $FC06       
LDA #$00        ; High byte
STA $FC07       

; Enable timer with IRQ and auto-reload
LDA #$13        ; Enable | IRQ Enable | Auto-reload
STA $FC08       

; The timer will now generate an IRQ every 1,000,000 cycles
```

### Example 3: Interrupt Handler

```assembly
; Set IRQ vector
; On 6502, the IRQ vector is at $FFFE/$FFFF
    .org $8000
    
INIT:
    ; Set limit for IRQ every 100,000 cycles
    LDA #$A0        ; 100000 & 0xFF
    STA $FC04
    LDA #$86        ; (100000 >> 8) & 0xFF
    STA $FC05
    LDA #$01        ; (100000 >> 16) & 0xFF
    STA $FC06
    LDA #$00        ; 100000 >> 24
    STA $FC07
    
    ; Enable timer with IRQ and auto-reload
    LDA #$13
    STA $FC08
    
    ; Enable interrupts
    CLI             ; Clear Interrupt Disable
    
LOOP:
    ; Main program
    JMP LOOP

; IRQ Handler
IRQ_HANDLER:
    PHA             ; Save registers
    TXA
    PHA
    TYA
    PHA
    
    ; Clear timer IRQ
    LDA #$04        ; IRQ_FLAG bit
    STA $FC08
    
    ; Increment tick counter
    INC TICK_COUNT
    
    ; Do something every tick...
    ; (your code here)
    
    PLA             ; Restore registers
    TAY
    PLA
    TAX
    PLA
    RTI             ; Return from Interrupt

TICK_COUNT:
    .byte 0

    ; Set IRQ vector
    .org $FFFE
    .word IRQ_HANDLER
```

### Example 4: Timed Wait (Delay)

```assembly
; Delay of approximately 10,000 cycles
DELAY_10K:
    ; Restart and configure timer
    LDA #$08        ; Reset bit
    STA $FC08
    
    ; Set limit to 10,000 (0x00002710)
    LDA #$10        ; Low byte
    STA $FC04
    LDA #$27        ; Middle byte 1
    STA $FC05
    LDA #$00        ; Middle byte 2
    STA $FC06
    LDA #$00        ; High byte
    STA $FC07
    
    ; Enable timer without IRQ or auto-reload
    LDA #$01        ; Only Enable
    STA $FC08
    
WAIT_LOOP:
    ; Read status
    LDA $FC09
    AND #$04        ; Check LIMIT_REACHED
    BEQ WAIT_LOOP   ; If not reached, keep waiting
    
    RTS
```

### Example 5: Measure Execution Duration

```assembly
; Measure how many cycles it takes to execute a routine
MEASURE_ROUTINE:
    ; Restart timer
    LDA #$08
    STA $FC08
    
    ; Enable timer
    LDA #$01
    STA $FC08
    
    ; Execute routine to measure
    JSR MY_ROUTINE
    
    ; Disable timer
    LDA #$00
    STA $FC08
    
    ; Read counter
    LDA $FC00
    STA CYCLES
    LDA $FC01
    STA CYCLES+1
    LDA $FC02
    STA CYCLES+2
    LDA $FC03
    STA CYCLES+3
    
    RTS

MY_ROUTINE:
    ; Routine to measure
    NOP
    NOP
    NOP
    RTS

CYCLES:
    .word 0, 0      ; 32 bits to store the result
```

## Usage from C++

### Initialization

```cpp
#include "basic_timer.hpp"

// Create timer device
auto timer = std::make_shared<BasicTimer>();

// Initialize
if (!timer->initialize()) {
    std::cerr << "Error: Could not initialize timer" << std::endl;
    return 1;
}

// Register with CPU
cpu.registerIODevice(timer);
```

### Configure Periodic Timer

```cpp
// Set limit to 1,000,000 cycles
timer->setLimit(1000000);

// Enable timer with IRQ and auto-reload
timer->setEnabled(true);
timer->write(0xFC08, 0x13);  // Enable | IRQ Enable | Auto-reload

// In the main loop
while (running) {
    // Execute CPU
    cpu.Execute(1000, mem);
    
    // Update timer
    timer->tick(1000);
    
    // Check for IRQ
    if (timer->hasIRQ()) {
        // Handle IRQ
        handleTimerInterrupt();
        
        // Clear IRQ
        timer->clearIRQ();
    }
}
```

### Read Elapsed Time

```cpp
// Get current counter value
uint32_t cycles = timer->getCounter();
std::cout << "Elapsed cycles: " << cycles << std::endl;
```

### Restart Timer

```cpp
// Reset counter to zero
timer->reset();

// Or using the control register
timer->write(0xFC08, 0x08);  // RESET bit
```

### Cleanup

```cpp
// Before exiting the program
timer->cleanup();
cpu.unregisterIODevice(timer);
```

## Integration with the Interrupt System

The `BasicTimer` can generate IRQ interrupts that the CPU must handle. Here is a complete example:

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "basic_timer.hpp"

int main() {
    Mem mem;
    CPU cpu;
    
    // Initialize memory
    mem.Initialize();
    cpu.Reset(mem);
    
    // Create and configure timer
    auto timer = std::make_shared<BasicTimer>();
    timer->initialize();
    cpu.registerIODevice(timer);
    
    // Set limit to 50,000 cycles (approximately 50ms at 1 MHz)
    timer->setLimit(50000);
    
    // Enable with IRQ and auto-reload
    timer->write(0xFC08, 0x13);
    
    // Load program that handles IRQ
    // ... (load 6502 code with IRQ handler)
    
    // Main loop
    bool running = true;
    while (running) {
        // Execute 1000 CPU cycles
        cpu.Execute(1000, mem);
        
        // Update timer
        timer->tick(1000);
        
        // Check IRQ
        if (timer->hasIRQ() && !cpu.I) {  // I = Interrupt Disable flag
            // Generate IRQ on the CPU
            // 1. Save PC and flags on stack
            cpu.PushPCToStack(cycles, mem);
            cpu.WriteByte(cycles, cpu.SPToAddress(), 
                         (cpu.N << 7) | (cpu.V << 6) | (cpu.B << 4) | 
                         (cpu.D << 3) | (cpu.I << 2) | (cpu.Z << 1) | cpu.C,
                         mem);
            cpu.SP--;
            
            // 2. Set I flag
            cpu.I = 1;
            
            // 3. Jump to IRQ vector
            cpu.PC = mem.ReadWord(Mem::IRQ_VECTOR);
            
            // Clear timer IRQ
            timer->clearIRQ();
        }
    }
    
    timer->cleanup();
    return 0;
}
```

## Use Cases

### 1. Real-Time Clock (RTC)

```cpp
// Configure timer to increment every second at 1 MHz
timer->setLimit(1000000);  // 1 million cycles = 1 second at 1 MHz
timer->write(0xFC08, 0x13); // Auto-reload with IRQ

// In the IRQ handler, increment seconds
uint32_t seconds = 0;
if (timer->hasIRQ()) {
    seconds++;
    timer->clearIRQ();
}
```

### 2. Frame Timing for Games

```cpp
// 60 FPS = 16.67ms per frame
// At 1 MHz, 16.67ms = ~16,670 cycles
timer->setLimit(16670);
timer->write(0xFC08, 0x13);

// In the game loop
if (timer->hasIRQ()) {
    updateGame();
    renderFrame();
    timer->clearIRQ();
}
```

### 3. Code Profiling

```cpp
// Measure performance of a function
timer->reset();
timer->setEnabled(true);

// Execute code to measure
functionToProfile();

// Read cycles
uint32_t cycles = timer->getCounter();
std::cout << "Function took " << cycles << " cycles" << std::endl;
```

### 4. Periodic Event Generation

```cpp
// Update sensors every 100,000 cycles
timer->setLimit(100000);
timer->write(0xFC08, 0x13);

if (timer->hasIRQ()) {
    readSensors();
    updateDisplay();
    timer->clearIRQ();
}
```

## Limitations

- **Precision**: The timer counts CPU cycles, not real time
- **Resolution**: One CPU cycle (at 1 MHz = 1 microsecond)
- **Maximum range**: 4,294,967,295 cycles (~71.6 minutes at 1 MHz)
- **Manual IRQ**: The emulator must call `tick()` and handle IRQ manually

## Performance Considerations

- The timer uses atomic operations for thread-safety
- Read/write operations are O(1)
- The `tick()` method is very efficient and can be called frequently
- The mutex only blocks during critical operations

## Timing Examples

At a CPU speed of 1 MHz (1,000,000 cycles/second):

| Limit | Approximate Time |
|--------|-------------------|
| 1,000 | 1 ms |
| 10,000 | 10 ms |
| 16,667 | ~16.67 ms (60 FPS) |
| 20,000 | 20 ms (50 Hz) |
| 50,000 | 50 ms |
| 100,000 | 100 ms |
| 1,000,000 | 1 second |
| 60,000,000 | 1 minute |

## Testing

The timer includes 14 exhaustive tests that verify:

1. Correct initialization
2. 32-bit register read/write
3. Enable/disable functionality
4. Cycle counting
5. IRQ generation
6. IRQ clearing
7. Auto-reload mode
8. Stop at limit
9. Counter reset
10. CPU reading
11. Periodic IRQ
12. Control bits
13. Behavior with IRQ disabled

Run the tests with:

```bash
cd build
./runTests --gtest_filter="BasicTimerTest.*"
```

## Integration with Other Devices

The timer can be coordinated with other devices:

```cpp
// Synchronize audio with timer
if (timer->getCounter() % 44100 == 0) {  // Every second at 44.1 kHz
    audio->playTone(440, 100, 128);
}

// Update screen at regular intervals
if (timer->hasIRQ()) {
    textScreen->refresh();
    timer->clearIRQ();
}
```

## API Reference

### BasicTimer::initialize()
Initializes the timer and all its registers.
- **Returns**: `true` if initialized successfully

### BasicTimer::tick(cycles)
Increments the counter and checks limits.
- **cycles**: Number of cycles to add to the counter

### BasicTimer::getCounter()
Gets the current value of the counter.
- **Returns**: Counter value (0-4294967295)

### BasicTimer::setCounter(value)
Sets the value of the counter.
- **value**: New counter value

### BasicTimer::getLimit()
Gets the configured limit.
- **Returns**: Limit value

### BasicTimer::setLimit(value)
Sets the limit for IRQ.
- **value**: New limit value

### BasicTimer::reset()
Resets the counter to 0 and clears IRQ.

### BasicTimer::isEnabled()
Checks if the timer is enabled.
- **Returns**: `true` if enabled

### BasicTimer::setEnabled(enabled)
Enables or disables the timer.
- **enabled**: `true` to enable, `false` to disable

### BasicTimer::hasIRQ()
Checks if there is a pending IRQ.
- **Returns**: `true` if there is a pending and enabled IRQ

### BasicTimer::clearIRQ()
Clears the IRQ flag.

### BasicTimer::isIRQEnabled()
Checks if IRQ generation is enabled.
- **Returns**: `true` if IRQ is enabled

### BasicTimer::isAutoReload()
Checks if auto-reload mode is enabled.
- **Returns**: `true` if auto-reload is enabled

### BasicTimer::cleanup()
Frees resources and disables the timer.

## See Also

- [Audio Device Documentation](audio_device.md)
- [Serial Device Documentation](serial_device.md)
- [Architecture Documentation](architecture.md)
- [File Device Documentation](file_device.md)
