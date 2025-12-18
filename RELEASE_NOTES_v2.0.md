# Release Notes - CPU 6502 Emulator v2.0.0

**Release Date**: December 18, 2024  
**Release Type**: Major Version Release

---

## 🎉 Overview

Version 2.0.0 represents a transformative release for the CPU 6502 Emulator project. What started as a basic instruction-level emulator has evolved into a comprehensive vintage computer system emulator with modern development tools, extensive peripheral support, and powerful extensibility features.

This release includes **59 commits**, **11,000+ lines of new code**, **166 passing tests**, and represents months of development effort to create a production-ready emulation platform.

---

## ✨ Highlights

### 🔧 Complete 6502 Instruction Set
All 151 official MOS 6502 instructions are now implemented with accurate cycle timing and proper emulation of hardware quirks. The emulator handles all addressing modes and edge cases that made the 6502 legendary.

### 🐛 Advanced Debugging System
A powerful integrated debugger with breakpoints, watchpoints, and instruction tracing makes development and testing easier than ever. Inspect and modify CPU state in real-time while programs run.

### 🐍 Python Scripting Integration
Extend the emulator with Python scripts. Hook into instruction execution, access registers and memory, and automate testing or create custom tools—all with the power of Python.

### 🎮 Rich Peripheral Support
From serial communication over TCP/IP to audio synthesis, timers, text displays, and file I/O, version 2.0 provides a complete vintage computer environment with modern connectivity.

### 🖥️ Retro Terminal GUI
Experience authentic 1980s computing with a beautiful SDL2-powered terminal interface featuring period-accurate colors, fonts, and that classic blinking cursor.

### 📚 Comprehensive Documentation
Every feature is fully documented with usage examples, API references, and integration guides. Over 20 markdown files cover architecture, devices, and development practices.

---

## 🆕 What's New

### Core Emulation

#### Complete Instruction Set Implementation
- ✅ All 151 official 6502 instructions
- ✅ All addressing modes (Immediate, Zero Page, Absolute, Indexed, Indirect)
- ✅ Accurate cycle counting including page boundary penalties
- ✅ Emulation of the JMP indirect addressing bug
- ✅ Proper flag handling for all operations

#### Advanced Debugging
- 🔴 **Breakpoints**: Pause execution at specific addresses
- 👁️ **Watchpoints**: Monitor memory read/write operations
- 📜 **Instruction tracing**: Record execution history
- 🔍 **State inspection**: Examine registers, flags, and memory
- ✏️ **Runtime modification**: Change values during execution

Example:
```cpp
Debugger dbg;
dbg.attach(&cpu, &mem);
cpu.setDebugger(&dbg);

dbg.addBreakpoint(0x8003);      // Break at address
dbg.addWatchpoint(0x0042);      // Watch memory location

cpu.Execute(1000, mem);

if (dbg.hitBreakpoint()) {
    auto state = dbg.inspectCPU();
    std::cout << "PC: " << state.pc << ", A: " << state.a << std::endl;
}
```

#### Python Scripting
- 🐍 Full Python 3 bindings via pybind11
- 🔗 Hook system for instruction execution
- 📊 Direct access to CPU state and memory
- 🚀 Event-driven script execution
- 💡 Perfect for automated testing and development tools

Example Python script:
```python
import cpu6502

def on_instruction(pc, opcode):
    print(f"Executing at {hex(pc)}: opcode {hex(opcode)}")
    
cpu = cpu6502.CPU()
cpu.on_execute(on_instruction)
cpu.execute(100)
```

### Interrupt System

#### IRQ and NMI Support
- ⚡ **IRQ**: Maskable interrupts for timers and I/O
- 🚨 **NMI**: Non-maskable interrupts for critical events
- 🎛️ **InterruptController**: Centralized interrupt management
- 🔌 **InterruptSource**: Interface for interrupt-generating devices
- ✅ Accurate vector handling and CPU state preservation

Example:
```cpp
InterruptController intCtrl;
cpu.setInterruptController(&intCtrl);

auto timer = std::make_shared<BasicTimer>();
intCtrl.registerSource(timer);

timer->setLimit(1000);  // Interrupt every 1000ms
timer->enableIRQ();

// In main loop
cpu.checkAndHandleInterrupts(mem);
```

### Peripheral Devices

#### Serial Communication (TcpSerial)
- 📡 ACIA 6551-compatible serial port
- 🌐 TCP/IP networking (client and server modes)
- 💬 Real-time bidirectional communication
- 🔧 Memory-mapped registers at $FA00-$FA03
- 🔌 Connect with netcat, telnet, or custom clients

Example usage:
```cpp
auto serial = std::make_shared<TcpSerial>();
serial->initialize();
serial->listen(12345);  // Listen on port 12345

// Connect with: nc localhost 12345
```

#### Programmable Timer (BasicTimer)
- ⏱️ Configurable interval timer
- 🔔 IRQ generation on expiry
- 📏 16-bit counter with millisecond precision
- 🎚️ Memory-mapped control at $FC00-$FC09

#### Text Display (TextScreen)
- 📺 40×24 character text mode
- 🎨 Video buffer at $FC00-$FFFB (960 bytes)
- ✏️ Direct character port at $FFFF
- 🖱️ Cursor control (position, auto-scroll)
- ⌨️ Control character support (\n, \t, \b, \r)

#### Audio Synthesis (BasicAudio)
- 🎵 Square wave tone generator
- 🎛️ Configurable frequency (0-65535 Hz)
- ⏲️ Duration control (milliseconds)
- 🔊 Volume adjustment (0-255)
- 🔇 Non-blocking playback
- 🎹 Perfect for sound effects and music

Example musical scale:
```cpp
auto audio = std::make_shared<BasicAudio>();
cpu.registerIODevice(audio);

// Play A4 (440Hz) for 500ms at medium volume
audio->playTone(440, 500, 128);
```

#### File I/O (FileDevice)
- 💾 Load binaries from host filesystem
- 📝 Save memory regions to files
- 🎯 Both C++ API and memory-mapped registers
- 🔄 Ideal for program loading and data persistence
- 📂 Access at $FE00-$FE4F

#### Apple II Compatibility (AppleIO)
- 🍎 Keyboard input at $FD0C
- 📟 Screen output at $FDED
- 🕹️ Compatible with vintage Apple II software

### Graphics and User Interface

#### Retro Terminal GUI (EmulatorGUI)
- 🎨 Authentic 1980s terminal aesthetic
- 📐 40×24 character display
- 🖼️ 16×16 pixel character cells
- 🌈 16-color palette (Apple II/C64/MSX-inspired)
- ⬛ Blinking block cursor
- ⌨️ Full keyboard input
- 🚀 SDL2-based rendering
- 🔗 Seamless TextScreen integration

Example:
```cpp
EmulatorGUI gui("6502 Retro Terminal", 16, 16);
gui.initialize();

auto textScreen = std::make_shared<TextScreen>();
gui.attachTextScreen(textScreen);

while (gui.isInitialized()) {
    if (gui.hasKey()) {
        textScreen->writeCharAtCursor(gui.getLastKey());
    }
    gui.update();
}
```

---

## 🏗️ Architecture Changes

### Directory Reorganization

**Before (v0.1.0)**:
```
cpu6502/
├── cpu.cpp
├── mem.cpp
├── main_6502.cpp
├── test.cpp
└── inc/
```

**After (v2.0.0)**:
```
cpu6502/
├── src/
│   ├── cpu/          # CPU implementation
│   ├── mem/          # Memory system
│   ├── devices/      # I/O devices
│   ├── gui/          # Graphics
│   ├── debugger/     # Debug tools
│   ├── scripting/    # Python bindings
│   ├── interrupt/    # Interrupt controller
│   ├── main/         # Demo programs
│   └── util/         # Utilities
├── include/
│   ├── cpu.hpp       # Public API
│   ├── devices/      # Device interfaces
│   ├── gui/          # GUI classes
│   └── util/         # Utility headers
├── tests/            # Unit tests
├── examples/         # Demo programs
├── docs/             # Documentation
└── lib/              # Dependencies
```

### Benefits of New Structure
- ✅ **Clear separation** between API and implementation
- ✅ **Modular organization** by functional area
- ✅ **Easier navigation** for developers
- ✅ **Better testability** with isolated components
- ✅ **Scalable architecture** for future growth

---

## 🧪 Testing

### Test Coverage
- **166 total tests** (all passing ✅)
- **27 original tests** (v0.1.0)
- **139 new tests** (v2.0.0)

### Test Categories
- ✅ CPU instructions (all addressing modes)
- ✅ Device functionality (I/O, timers, audio, video)
- ✅ Interrupt handling (IRQ, NMI)
- ✅ Debugger operations
- ✅ Python scripting API
- ✅ GUI components
- ✅ Edge cases and boundary conditions

### Running Tests
```bash
cd build
make test          # Quick test run
make runTests      # Detailed output
ctest --verbose    # Direct CTest invocation
```

---

## 📖 Documentation

### New Documentation Files
All documentation has been translated from Spanish to English:

- 📘 **architecture.md** - System architecture overview
- 📘 **instructions.md** - Instruction implementation guide
- 📘 **debugger.md** - Debugger usage guide
- 📘 **scripting_api.md** - Python scripting reference
- 📘 **interrupt_system.md** - Interrupt system documentation
- 📘 **serial_device.md** - Serial/TCP communication
- 📘 **timer_device.md** - Timer device reference
- 📘 **audio_device.md** - Audio synthesis guide
- 📘 **video_device.md** - Text display documentation
- 📘 **file_device.md** - File I/O reference
- 📘 **emulator_gui.md** - GUI implementation guide
- 📘 **peripheral_devices.md** - Device overview

### Example Programs
Each device includes a complete demo program:
- `examples/tcp_serial_demo.cpp`
- `examples/interrupt_demo.cpp`
- `examples/audio_demo.cpp`
- `examples/text_screen_demo.cpp`
- `examples/file_device_demo.cpp`
- `examples/gui_demo.cpp`
- `examples/apple_io_demo.cpp`

---

## 🔄 Breaking Changes

### File Locations
- ❌ **Old**: Source files in project root
- ✅ **New**: Sources in `src/` subdirectories
- ❌ **Old**: Headers in `inc/`
- ✅ **New**: Headers in `include/`

### Include Paths
```cpp
// Old (v0.1.0)
#include "../inc/cpu.hpp"

// New (v2.0.0)
#include "cpu.hpp"  // With -I pointing to include/
```

### Binary Names
- ❌ **Old**: `main_6502`
- ✅ **New**: `cpu_demo`
- 📝 Legacy binary preserved at `examples/main_6502_legacy`

### Build Targets
CMake now generates multiple targets:
- `cpu6502_lib` - Static library
- `cpu_demo` - Main demo program
- `cpu_tests` - Test suite
- Device-specific demos: `tcp_serial_demo`, `gui_demo`, etc.

### API Changes
The CPU class has new methods for v2.0:
```cpp
// New in v2.0
void setDebugger(Debugger* debugger);
void setInterruptController(InterruptController* ctrl);
void registerIODevice(std::shared_ptr<IODevice> device);
void checkAndHandleInterrupts(Mem& memory);
```

---

## 📦 Migration Guide

### For Library Users

1. **Update your CMakeLists.txt**:
```cmake
# Add cpu6502 as a subdirectory
add_subdirectory(path/to/cpu6502)

# Link your target
target_link_libraries(your_app cpu6502_lib)

# Add include directories
target_include_directories(your_app PRIVATE 
    path/to/cpu6502/include
)
```

2. **Update include statements**:
```cpp
#include "cpu.hpp"      // Not "../inc/cpu.hpp"
#include "mem.hpp"      // Not "../inc/mem.hpp"
```

3. **Update any hardcoded binary paths**:
```bash
# Old
./main_6502

# New
./build/cpu_demo
```

### For Device Developers

1. **Create device headers** in `include/devices/`
2. **Implement devices** in `src/devices/`
3. **Inherit from IODevice**:
```cpp
class MyDevice : public IODevice {
public:
    bool handles(uint16_t address) override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
};
```

4. **Register with CPU**:
```cpp
auto device = std::make_shared<MyDevice>();
cpu.registerIODevice(device);
```

5. **Add tests** in `tests/test_my_device.cpp`

### For Documentation Contributors

1. **Place new docs** in `docs/` directory
2. **Use English** as primary language
3. **Follow existing format** (see `docs/serial_device.md` as template)
4. **Include code examples** for all features
5. **Generate API docs**: `doxygen docs/Doxyfile`

---

## 🛠️ Building and Installation

### Requirements
- **CMake 3.10+** (required)
- **C++17 compiler** (GCC 7+, Clang 5+, MSVC 2017+)
- **GoogleTest** (included as submodule)
- **SDL2 2.0+** (optional, for GUI and audio)
- **Python 3.7+** (optional, for scripting)
- **pybind11** (included as submodule)

### Quick Start
```bash
# Clone with submodules
git clone --recursive https://github.com/Kilynho/cpu6502.git
cd cpu6502

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run tests
make test

# Run demo
./cpu_demo
```

### Building with Optional Features
```bash
# With SDL2 (GUI and Audio)
cmake -DBUILD_GUI=ON -DBUILD_AUDIO=ON ..

# With Python scripting
cmake -DBUILD_PYTHON_BINDINGS=ON ..

# Full build with all features
cmake -DBUILD_GUI=ON -DBUILD_AUDIO=ON -DBUILD_PYTHON_BINDINGS=ON ..
make -j$(nproc)
```

---

## 🐛 Known Issues

None critical. The emulator has been extensively tested with 166 passing unit tests.

Minor notes:
- SDL2 window close detection could be improved
- Some device registers could benefit from additional validation
- Python bindings could expose more utility functions

Report issues at: https://github.com/Kilynho/cpu6502/issues

---

## 🚀 Future Roadmap

While v2.0.0 is feature-complete for a vintage computer emulator, we have exciting plans:

### Planned for v2.1.0
- Bitmap graphics modes (320×200, 256 colors)
- Joystick/gamepad input device
- VIA 6522 emulation (timers, shift register, I/O)
- Save states and replay functionality
- Performance profiler

### Planned for v2.2.0
- Disk I/O (floppy disk emulation)
- Cassette tape storage simulation
- Sound synthesis (additional waveforms)
- Network multiplayer support
- Web-based remote terminal

### Long-term Goals
- Full Apple II system emulation
- Commodore 64 compatibility mode
- Atari 2600 TIA emulation
- Debugger GUI with disassembler
- Built-in assembler/disassembler

---

## 🙏 Acknowledgments

This release was made possible by:
- The MOS 6502 community for decades of documentation
- GoogleTest team for excellent testing infrastructure
- pybind11 developers for Python integration
- SDL2 team for cross-platform graphics
- GitHub Copilot for development assistance
- All contributors and testers

---

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## 📞 Support and Community

- **Repository**: https://github.com/Kilynho/cpu6502
- **Issues**: https://github.com/Kilynho/cpu6502/issues
- **Discussions**: https://github.com/Kilynho/cpu6502/discussions
- **Contributing**: See [CONTRIBUTING.md](CONTRIBUTING.md)
- **Security**: See [SECURITY.md](SECURITY.md)

---

**Thank you for using CPU 6502 Emulator v2.0.0!**

We hope this release brings you joy in exploring vintage computing and building amazing retro projects. Happy coding! 🎮✨
