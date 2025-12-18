# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]

## [2.0.0] - 2024-12-18

**Major Release**: This release represents a significant evolution of the CPU 6502 emulator from a basic instruction-level emulator to a comprehensive vintage computer system emulator with modern development tools and extensibility features.

### Added - Core Emulation Features
- **Complete MOS 6502 instruction set** (151 official instructions)
  - All addressing modes with accurate cycle timing
  - Proper handling of page boundary crossings
  - Emulation of documented 6502 quirks (e.g., JMP indirect bug)
- **Advanced debugging system** with breakpoints, watchpoints, and instruction tracing
  - Runtime CPU state inspection and modification
  - Memory access monitoring
  - Integration with CPU execution flow
- **Python scripting integration** via pybind11
  - Event-driven hook system for instruction execution
  - Direct access to CPU registers and memory from Python scripts
  - Script loading via command-line or interactive console
- **Comprehensive interrupt system**
  - IRQ (maskable) and NMI (non-maskable) support
  - Centralized `InterruptController` for device coordination
  - Proper vector handling and CPU state management
  - `InterruptSource` interface for device implementations

### Added - I/O and Peripheral Devices
- **Modular I/O device architecture** with `IODevice` base interface
  - Devices can intercept memory-mapped I/O accesses
  - Multiple devices can coexist with priority handling
- **Serial communication** (`TcpSerial`)
  - ACIA 6551-compatible registers at $FA00-$FA03
  - TCP/IP networking support (client and server modes)
  - Real-time bidirectional communication
  - Integration with modern tools (netcat, telnet, Python)
- **Programmable timers** (`BasicTimer`)
  - Configurable interval timers with IRQ generation
  - 16-bit counter with millisecond precision
  - Memory-mapped control registers
- **Text-mode video** (`TextScreen`)
  - 40×24 character display with video buffer at $FC00-$FFFB
  - Cursor control and auto-scroll
  - Control characters support (newline, tab, backspace)
  - Direct character port at $FFFF
- **Audio synthesis** (`BasicAudio`)
  - Square wave tone generator
  - Configurable frequency, duration, and volume
  - Memory-mapped registers at $FB00-$FB05
  - Non-blocking playback with SDL2
- **File I/O device** (`FileDevice`)
  - Load and save binary files from/to host filesystem
  - Both C++ API and memory-mapped register interface
  - Support for arbitrary memory regions
  - Ideal for program loading and data persistence
- **Apple II I/O emulation** (`AppleIO`)
  - Keyboard input at $FD0C
  - Screen output at $FDED
  - Compatible with vintage Apple II software patterns
- **Retro-style GUI** (`EmulatorGUI`)
  - Authentic 1980s terminal aesthetics (Apple II/C64/MSX-inspired)
  - 40×24 character display with 16×16 pixel fonts
  - 16-color palette matching vintage systems
  - Blinking cursor and full keyboard input
  - SDL2-based rendering with smooth performance

### Added - Development and Testing
- **Comprehensive test suite** (166 tests total)
  - Unit tests for all instruction families
  - Device behavior and integration tests
  - Interrupt handling validation
  - Scripting API tests
  - GUI component tests
- **Extensive documentation** in `docs/`
  - Architecture overview (`architecture.md`)
  - Instruction implementation guide (`instructions.md`)
  - Device-specific documentation for each peripheral
  - Debugger usage guide (`debugger.md`)
  - Scripting API reference (`scripting_api.md`)
  - GUI implementation guide (`emulator_gui.md`)
- **Example programs and demonstrations**
  - Demo programs for each device type
  - Musical scale generation (audio)
  - TCP echo server (serial)
  - Interactive terminal (GUI)
  - Interrupt-driven applications
- **Doxygen integration** for API documentation generation
- **GitHub Copilot agent configurations** for AI-assisted development
  - Specialized agents for C++, testing, and documentation

### Changed - Architecture and Organization
- **Complete source code reorganization**
  - Modular directory structure: `src/cpu/`, `src/devices/`, `src/gui/`, `src/debugger/`, `src/scripting/`, `src/interrupt/`
  - Clear separation of headers: `include/` (public API) vs `src/` (implementation)
  - Organized test structure in `tests/` with per-module test files
  - Example programs in `examples/` with dedicated demos
- **Enhanced CPU class** (`include/cpu.hpp`, `src/cpu/cpu.cpp`)
  - Debugger integration hooks
  - Interrupt controller support
  - I/O device registration and management
  - Improved instruction dispatch system
- **Improved memory system** with I/O device intercept capability
- **Modernized build system**
  - CMake 3.10+ with modular configuration
  - Reusable static library (`libcpu6502_lib.a`)
  - Separate targets for library, demo, tests, and examples
  - Optional dependencies (SDL2 for GUI, Python for scripting)
  - CTest integration for automated testing

### Changed - Documentation and Tooling
- **Updated README.md** with comprehensive sections
  - Feature overview and recent changes
  - Architecture description
  - Build and installation instructions
  - Usage examples for all major features
  - Device integration guides
- **Enhanced CONTRIBUTING.md**
  - Updated for new directory structure
  - Testing guidelines
  - Code style requirements
- **Improved CI/CD pipeline**
  - GitHub Actions workflow validation
  - Automatic dependency installation
  - Build verification across configurations
  - Test execution and reporting

### Technical
- **Dependencies**
  - CMake 3.10+ (required)
  - C++17 standard (required)
  - GoogleTest (included as submodule)
  - SDL2 2.0+ (optional, for GUI and audio)
  - Python 3.7+ (optional, for scripting)
  - pybind11 (included as submodule, for Python bindings)
- **Platform support**
  - Linux (tested on Ubuntu 20.04+)
  - macOS (tested on recent versions)
  - Windows (via WSL or native with adjustments)

### Breaking Changes
- **File structure reorganization**
  - Source files moved from root to `src/` subdirectories
  - Headers moved from `inc/` to `include/`
  - Demo program renamed from `main_6502` to `cpu_demo`
  - Legacy binary preserved in `examples/main_6502_legacy`
- **API changes**
  - Include paths changed: `#include "cpu.hpp"` → `#include "cpu.hpp"` (from include/)
  - Memory interface extended for I/O device integration
  - CPU class expanded with new methods for devices and interrupts
- **Build system changes**
  - CMake now generates a static library plus executables
  - New target names: `cpu_demo`, `cpu_tests`, device-specific demos
  - Old Makefile targets deprecated (use CMake targets or new Makefile wrapper)

### Migration Guide for Users

#### For existing code that uses the emulator library:

1. **Update include paths** in your source files:
   ```cpp
   // Old
   #include "../inc/cpu.hpp"
   
   // New
   #include "cpu.hpp"  // With -I flag pointing to include/
   ```

2. **Link against the new library**:
   ```cmake
   # In your CMakeLists.txt
   add_subdirectory(path/to/cpu6502)
   target_link_libraries(your_target cpu6502_lib)
   ```

3. **Update binary names** if you're calling the demo:
   ```bash
   # Old
   ./main_6502
   
   # New
   ./cpu_demo
   # Or with arguments
   ./cpu_demo file program.bin infinite
   ```

#### For developers extending the emulator:

1. **Place new device implementations** in `src/devices/` and headers in `include/devices/`
2. **Add tests** in `tests/test_<feature>.cpp`
3. **Register devices** using the new `IODevice` interface:
   ```cpp
   auto device = std::make_shared<YourDevice>();
   cpu.registerIODevice(device);
   ```
4. **Use the interrupt system** for device events:
   ```cpp
   InterruptController intCtrl;
   cpu.setInterruptController(&intCtrl);
   intCtrl.registerSource(yourInterruptSource);
   ```

#### For documentation contributors:

1. **Documentation files** are now in `docs/` (was scattered)
2. **Spanish documentation** has been translated to English (original Spanish versions preserved with `_es` suffix)
3. **Use Doxygen** for API documentation: `doxygen docs/Doxyfile`

### Notes
- Version 0.1.0 was the initial basic emulator
- Version 2.0.0 represents the first full-featured release
- All 166 tests passing on Linux
- No known critical issues

## [0.1.0] - 2024-12-08

### Added
- **Full set of 6502 instructions (151 official instructions)**
  - All load/store instructions (LDA, LDX, LDY, STA, STX, STY) with all addressing modes
  - Transfer instructions (TAX, TAY, TXA, TYA, TSX, TXS)
  - Stack operations (PHA, PHP, PLA, PLP)
  - Logical operations (AND, EOR, ORA, BIT)
  - Arithmetic operations (ADC, SBC)
  - Comparisons (CMP, CPX, CPY)
  - Increments and decrements (INC, INX, INY, DEC, DEX, DEY)
  - Shift and rotate operations (ASL, LSR, ROL, ROR)
  - Jumps and subroutines (JMP, JSR, RTS, RTI)
  - Conditional branches (BEQ, BNE, BPL, BMI, BCS, BCC, BVC, BVS)
  - Flag instructions (CLC, SEC, CLD, SED, CLI, SEI, CLV)
  - System instructions (BRK, RTI, NOP)
- **Modular addressing modes system**
  - Dedicated file `src/cpu/addressing.cpp` with all addressing modes
  - Support for Immediate, Zero Page, Zero Page X/Y, Absolute, Absolute X/Y, Indirect X/Y, Indirect
  - Correct handling of page boundary crossings
  - Emulation of the 6502 bug in JMP indirect
- **Optimized instruction handlers system**
  - Lookup table array[256] for fast instruction dispatch
  - Individual handlers per instruction in `src/cpu/instructions.cpp`
  - Helper functions for flag updates (UpdateZeroAndNegativeFlags, UpdateCarryFlag, UpdateOverflowFlag)
  - Unique initialization of the instruction table
- **Exhaustive test suite**
  - 68 unit tests in total (27 original + 41 new)
  - Dedicated tests for each instruction family
  - Coverage of edge cases, flags, and boundary conditions
  - Validation of arithmetic operations with carry and overflow
  - Branch tests with positive and negative offsets
  - File `tests/instruction_handlers_test.cpp` with tests for new instructions
- **Complete technical documentation**
  - `docs/instructions.md`: Exhaustive guide to implementing and testing instructions
  - `docs/architecture.md`: Detailed description of the emulator architecture
  - `docs/Doxyfile`: Doxygen configuration for documentation generation
  - Documentation section in README.md with instructions to generate docs
- **Complete modular architecture**
  - New directory structure: `src/`, `include/`, `tests/`, `examples/`, `docs/`
  - Clear separation between public API (`include/`) and implementation (`src/`)
  - Organized subdirectories: `src/cpu/`, `src/mem/`, `src/util/`, `src/main/`
- **Configurable logging system**
  - Logger with levels: NONE, ERROR, WARN, INFO, DEBUG
  - Runtime level control with `LogSetLevel()`
  - Helper functions: `LogError()`, `LogWarn()`, `LogInfo()`, `LogDebug()`
  - Macros for logging with streams: `LOG_ERROR()`, `LOG_WARN()`, etc.
  - Implementation in `include/util/logger.hpp` and `src/util/logger.cpp`
- **Multi-target build system with CMake**
  - Modular CMakeLists.txt (root, src/, tests/)
  - Generation of reusable static library (`libcpu6502_lib.a`)
  - Demo executable (`cpu_demo`)
  - Integrated test suite with CTest
  - Support for GoogleTest as a submodule and system
- **Improved Makefile**
  - CMake wrapper for backward compatibility
  - Useful targets: `all`, `test`, `runTests`, `demo`, `clean`, `rebuild`, `help`
  - Informative messages about generated artifacts
- Complete suite of unit tests with Google Test
  - Tests for all LDA variants (IM, ZP, ZPX, ABS, ABSX, ABSY)
  - Tests for LDX in immediate mode
  - Tests for STA with different values
  - Tests for JSR/RTS with stack validation
  - Edge case tests (zero, 0xFF, page boundaries)
  - Flag behavior tests (Z, N)
  - Tests for nested subroutine calls
  - Total of 27 unit tests (all passing)
- CI/CD with GitHub Actions (.github/workflows/ci.yml)
  - Automatic build on each push and PR
  - Automatic test execution
  - Support for CMake and Make
- CI badge in README.md showing build status
- Complete project documentation (README.md, CONTRIBUTING.md, CHANGELOG.md)
- LICENSE file with MIT license
- Security policy (SECURITY.md)
- GitHub issue templates (bugs and features)
- GitHub pull request template
- Badges in README for license, C++ version, and build status

### Changed
- **Complete reorganization of the source code**
  - `cpu.cpp` → `src/cpu/cpu.cpp` with `include/cpu.hpp` as public API
  - `mem.cpp` → `src/mem/mem.cpp` with `include/mem.hpp` as public API
  - `main_6502.cpp` → `src/main/cpu_demo.cpp` (demo program)
  - `test.cpp` → `tests/test_main.cpp`
  - Binary `main_6502` → `examples/main_6502_legacy` (reference)
- **Updated includes**
  - All source files now use headers from `include/`
  - Paths relative to the project root
- Improved README documentation with detailed sections
  - New project architecture section
  - Updated instructions for CMake and Make
  - Documentation of the logging system
  - Updated code examples
- Added complete installation and usage instructions
- Expanded testing section in README.md with detailed coverage description
- Updated CONTRIBUTING.md with information about:
  - New directory structure
  - Updated development commands
  - Guide to adding tests in the new structure
  - Information about tests and CI
- Updated .gitignore for new build artifacts

### Technical
- CMake 3.10+ required
- C++17 standard
- GoogleTest integrated as a submodule in `lib/googletest/`
- Reusable static library `libcpu6502_lib.a`
- Integrated CTest for test execution

## [0.1.0] - 2024-12-08

### Added
- Initial implementation of the 6502 CPU
- Memory system with support for ROM and RAM
- Basic 6502 instructions:
  - LDA (Load Accumulator): Immediate, Zero Page, Zero Page X, Absolute, Absolute X, Absolute Y
  - LDX (Load X Register): Immediate
  - STA (Store Accumulator): Immediate
  - JSR (Jump to Subroutine)
  - RTS (Return from Subroutine)
- Processor flag system (Zero, Negative, Carry, etc.)
- Automatic clock cycle calculation
- Execution tracing with detailed information
- Basic unit tests
- Build configuration with Make and CMake
- Support for multiple addressing modes
- CPU reset functionality
- Stack pointer system

### Technical Features
- Emulation of 64KB of addressable memory
- ROM at addresses 0x8000-0xFFFF
- RAM at addresses 0x0000-0x7FFF
- Stack at 0x0100-0x01FF
- Clock cycle precision
- Correct handling of page boundaries

## Types of Changes

- `Added` for new features
- `Changed` for changes in existing functionality
- `Deprecated` for features that will be removed
- `Removed` for deleted features
- `Fixed` for bug fixes
- `Security` for security vulnerabilities

## Future Roadmap

### [0.2.0] - Planned
- Complete support for arithmetic instructions (ADC, SBC)
- Logical instructions (AND, ORA, EOR)
- Increment/decrement instructions (INC, DEC, INX, INY, DEX, DEY)
- Comparison instructions (CMP, CPX, CPY)
- Complete flag system

### [0.3.0] - Planned
- Conditional branch instructions (BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC)
- Bit instructions (BIT)
- Shift and rotate instructions (ASL, LSR, ROL, ROR)

### [0.4.0] - Planned
- Stack instructions (PHA, PLA, PHP, PLP)
- Transfer instructions (TAX, TAY, TXA, TYA, TSX, TXS)
- Interrupt handling (IRQ, NMI)
- BRK instruction and break handling

### [1.0.0] - Goal
- Complete set of 6502 instructions
- Accurate timing emulation
- Complete documentation of all instructions
- Exhaustive test suite
- Example programs to run
- Support for loading binaries from files
- Basic interactive debugger

---

## Links

- [Project Repository](https://github.com/Kilynho/cpu6502)
- [Report a Bug](https://github.com/Kilynho/cpu6502/issues/new?template=bug_report.md)
- [Request Feature](https://github.com/Kilynho/cpu6502/issues/new?template=feature_request.md)
- [Contribution Guide](CONTRIBUTING.md)

---

**Note:** Dates and versions are tentative and may change according to development progress.
