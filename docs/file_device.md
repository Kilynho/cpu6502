# FileDevice: File Storage Integration

## Introduction

The FileDevice module provides file storage integration for the CPU 6502 emulator, allowing programs to read and write files as if they were using a real disk or tape device. This enables persistent data storage and interoperability with modern file systems.

The `FileDevice` system allows the 6502 emulator to load and save binaries from/to host system files. This functionality is essential for:

- Loading programs from external binary files
- Saving memory data to files
- Persisting memory state
- Facilitating development and testing of 6502 programs

## Architecture

### Class Hierarchy

```
IODevice (base interface)
    └── StorageDevice (storage interface)
            └── FileDevice (file-based implementation)
```

### StorageDevice Interface

Base interface that defines storage operations:

```cpp
class StorageDevice : public IODevice {
public:
    // Load a binary from file to memory
    virtual bool loadBinary(const std::string& filename, 
                           uint16_t startAddress) = 0;
    
    // Save a block of memory to file
    virtual bool saveBinary(const std::string& filename, 
                           uint16_t startAddress, 
                           uint16_t length) = 0;
    
    // Check if a file exists
    virtual bool fileExists(const std::string& filename) const = 0;
};
```

### FileDevice Implementation

`FileDevice` implements `StorageDevice` and provides two modes of operation:

1. **Direct API**: C++ methods for programmatic control
2. **Mapped Registers**: Control from 6502 code via memory addresses

## Memory-Mapped Registers

FileDevice maps the following registers in the `0xFE00-0xFE4F` range:

| Address | Name | Type | Description |
|-----------|--------|------|-------------|
| `0xFE00` | CONTROL | W/R | Operation control register |
| `0xFE01` | START_ADDR_LO | W/R | Low byte of start address |
| `0xFE02` | START_ADDR_HI | W/R | High byte of start address |
| `0xFE03` | LENGTH_LO | W/R | Low byte of length |
| `0xFE04` | LENGTH_HI | W/R | High byte of length |
| `0xFE05` | STATUS | W/R | Status of the last operation |
| `0xFE10-0xFE4F` | FILENAME | W/R | Filename buffer (64 bytes) |

### Control Register (0xFE00)

Possible values:

| Value | Operation | Description |
|-------|-----------|-------------|
| 0 | NONE | No operation |
| 1 | LOAD | Load file to memory |
| 2 | SAVE | Save memory to file |

**Note**: The operation is automatically executed when writing a value other than 0 to this register.

### Status Register (0xFE05)

Return values:

| Value | Status | Description |
|-------|--------|-------------|
| 0 | SUCCESS | Successful operation |
| 1 | ERROR | Error in the operation |

## Usage from C++

### Initialization

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "file_device.hpp"

Mem mem;
CPU cpu;
auto fileDevice = std::make_shared<FileDevice>(&mem);

cpu.Reset(mem);
cpu.registerIODevice(fileDevice);
```

### Load a Binary

```cpp
// Load programa.bin at address 0x8000
if (fileDevice->loadBinary("programa.bin", 0x8000)) {
    std::cout << "Program loaded successfully\n";
    cpu.PC = 0x8000;  // Jump to program
    cpu.Execute(1000, mem);
} else {
    std::cerr << "Error loading program\n";
}
```

### Save Memory Data

```cpp
// Save 256 bytes from 0x0200 to datos.bin
if (fileDevice->saveBinary("datos.bin", 0x0200, 256)) {
    std::cout << "Data saved successfully\n";
} else {
    std::cerr << "Error saving data\n";
}
```

## Usage from 6502 Code

### Example: Load a File

```assembly
; Set up filename
LDA #'p'
STA $FE10
LDA #'r'
STA $FE11
LDA #'o'
STA $FE12
LDA #'g'
STA $FE13
LDA #'.'
STA $FE14
LDA #'b'
STA $FE15
LDA #'i'
STA $FE16
LDA #'n'
STA $FE17
LDA #0        ; Null terminator
STA $FE18

; Set up start address = 0x8000
LDA #$00
STA $FE01     ; Low byte
LDA #$80
STA $FE02     ; High byte

; Execute LOAD operation
LDA #1
STA $FE00

; Check status
LDA $FE05
BEQ success   ; If status = 0, success
; Error handling...
success:
JMP $8000     ; Jump to loaded program
```

### Example: Save Data

```assembly
; Set up filename in $FE10-$FE4F
; (similar to previous example)

; Set up start address = 0x0200
LDA #$00
STA $FE01
LDA #$02
STA $FE02

; Set up length = 100 bytes
LDA #100
STA $FE03
LDA #0
STA $FE04

; Execute SAVE operation
LDA #2
STA $FE00

; Check status
LDA $FE05
BEQ success
; Error handling...
success:
; Continue...
```

## Helper Routines

### Writing Filename

```assembly
; Write a null-terminated string to the filename buffer
; Input: String address in $00-$01 (zero page)
write_filename:
    LDY #0
.loop:
    LDA ($00),Y
    STA $FE10,Y
    BEQ .done
    INY
    CPY #64      ; Max 64 characters
    BNE .loop
.done:
    RTS
```

### Status Check

```assembly
; Check the status of the last operation
; Returns: Z=1 if success, Z=0 if error
check_status:
    LDA $FE05
    RTS
```

## Integration with Apple BASIC

From Apple BASIC, you can use the PEEK/POKE addresses to control FileDevice:

```basic
10 REM Load file "PROG.BIN" to $8000
20 REM Write filename
30 POKE 65040,80: REM 'P'
40 POKE 65041,82: REM 'R'
50 POKE 65042,79: REM 'O'
60 POKE 65043,71: REM 'G'
70 POKE 65044,46: REM '.'
80 POKE 65045,66: REM 'B'
90 POKE 65046,73: REM 'I'
100 POKE 65047,78: REM 'N'
110 POKE 65048,0: REM null
120 REM Set address
130 POKE 65025,0: REM low byte
140 POKE 65026,128: REM high byte ($80)
150 REM Execute LOAD
160 POKE 65024,1
170 REM Check status
180 S = PEEK(65029)
190 IF S = 0 THEN PRINT "LOAD OK"
200 IF S <> 0 THEN PRINT "ERROR"
```

## Integration with WOZMON

WOZMON (Wozniak Monitor) can be extended to include file commands:

### LOAD Command (L)

```
L8000<prog.bin
```

Loads `prog.bin` at address `$8000`.

### SAVE Command (S)

```
S8000.81FF>datos.bin
```

Saves from `$8000` to `$81FF` to `datos.bin`.

**Note**: These extensions require modifying the WOZMON code to use the FileDevice registers.

## Complete Examples

### Full Demo

See `examples/file_device_demo.cpp` for a complete example demonstrating:

1. Loading and running a program from file
2. Saving memory data
3. Using mapped registers
4. Integrity checking

### Running the Demo

```bash
cd build
./file_device_demo
```

## Limitations and Considerations

### Current Limitations

1. **File Size**: Files cannot exceed 64KB (limit of 6502 address space)
2. **Filename Length**: Maximum 64 characters (null-terminated)
3. **Paths**: Supports relative and absolute paths of the host system
4. **Concurrency**: No protection against concurrent accesses

### Security Considerations

1. **Path Validation**: FileDevice does not validate paths, relies on the operating system
2. **Permissions**: Respects host file system permissions
3. **Overwrite**: SAVE will overwrite existing files without confirmation

### Best Practices

1. Always check the status after an operation
2. Use absolute paths to avoid ambiguities
3. Validate sizes before loading/saving
4. Keep filenames short and compatible

## Use Cases

### 6502 Software Development

```cpp
// Compile 6502 code -> programa.bin
// Load and test in emulator
fileDevice->loadBinary("programa.bin", 0x8000);
cpu.PC = 0x8000;
cpu.Execute(10000, mem);
```

### Data Persistence

```cpp
// Save game state
fileDevice->saveBinary("savegame.dat", 0x2000, 0x1000);

// Load game state
fileDevice->loadBinary("savegame.dat", 0x2000);
```

### Debugging

```cpp
// Save memory dump for analysis
fileDevice->saveBinary("memdump.bin", 0x0000, 0x10000);
```

## Testing

### Running Tests

```bash
make test
# Or directly:
./build/runTests --gtest_filter=FileDevice*
```

### Test Coverage

The tests cover:

- Device creation and setup
- File loading and saving
- Mapped registers
- Error handling
- Data integrity
- CPU integration

See `tests/test_file_device.cpp` for full details.

## References

- **Source Code**:
  - `include/storage_device.hpp` - StorageDevice interface
  - `include/devices/file_device.hpp` - FileDevice header
  - `src/devices/file_device.cpp` - FileDevice implementation
  - `tests/test_file_device.cpp` - Test suite
  - `examples/file_device_demo.cpp` - Complete example

- **Related Documentation**:
  - `docs/architecture.md` - IODevice system architecture
  - `README.md` - General project information

## Future Extensions

### Possible Improvements

1. **Data Buffering**: Larger transfers with intermediate buffering
2. **Asynchronous Operations**: Background loading/saving
3. **Compression**: Support for compressed files
4. **Special Formats**: Support for .PRG, .D64 formats, etc.
5. **Virtual File System**: Emulate a complete file system
6. **Callbacks**: Notifications when operations complete

### Extended Compatibility

1. **Cassette Emulation**: Simulate loading from tape
2. **Disk Emulation**: Emulate disk drives
3. **Networking**: File transfer over network
