# cpu6502

## Recent Changes (December 2025)

- `cpu_demo` now allows loading external binaries using:
  ```
  ./cpu_demo file ../examples/demo_program.bin
  ```
- If no binary is specified, it runs a classic test program in memory.
- Detailed logging of memory accesses has been added to `cpu_log.txt`.
- Command line arguments:
  - `file <path>`: loads an external binary at 0x8000.
  - `infinite`: runs infinite cycles.
- Improvements to inline documentation and code comments.

### Apple II I/O Integration

- The CPU now supports modular I/O devices via the `IODevice` interface.
- `AppleIO` is included to simulate the keyboard ($FD0C) and screen ($FDED) of the Apple II.
- Registering I/O devices:
  ```cpp
  auto appleIO = std::make_shared<AppleIO>();
  cpu.registerIODevice(appleIO);
  ```
- I/O devices intercept memory accesses before standard read/write.
- Ideal for extending the emulator with peripherals, timers, graphics, etc.

### File Storage Support (FileDevice)

- New `FileDevice` that allows loading and saving binaries to/from host files.
- Two modes of operation:
  - **Direct C++ API**: `loadBinary()` and `saveBinary()` methods
  - **Memory-mapped registers**: Control from 6502 code at `$FE00-$FE4F`
- Perfect for development, testing, and data persistence.
- Example usage:
  ```cpp
  auto fileDevice = std::make_shared<FileDevice>(&mem);
  cpu.registerIODevice(fileDevice);
  
  // Load program from file
  fileDevice->loadBinary("program.bin", 0x8000);
  
  // Save memory data
  fileDevice->saveBinary("data.bin", 0x0200, 256);
  ```
- See [docs/file_device.md](docs/file_device.md) for complete documentation and [examples/file_device_demo.cpp](examples/file_device_demo.cpp) for examples.

### Interrupt System Support (IRQ/NMI)

- Centralized interrupt management system via `InterruptController`.
- Full support for IRQ (maskable) and NMI (non-maskable).
- Devices can implement the `InterruptSource` interface to trigger interrupts.
- Automatic handling of interrupt vectors, stack, and CPU flags.
- Example usage:
  ```cpp
  InterruptController intCtrl;
  cpu.setInterruptController(&intCtrl);
  
  auto timer = std::make_shared<BasicTimer>();
  intCtrl.registerSource(timer);
  
  // Timer triggers IRQ periodically
  timer->setLimit(1000);
  timer->write(0xFC08, 0x03);  // Enable | IRQ Enable
  
  // In the main loop
  cpu.checkAndHandleInterrupts(mem);
  ```
- See [docs/interrupt_system.md](docs/interrupt_system.md) for complete documentation and [examples/interrupt_demo.cpp](examples/interrupt_demo.cpp) for examples.
### Text Screen Graphics Support

- New `TextScreen` device simulating a 40x24 character text screen.
- Main features:
  - **Video buffer mapped in memory**: `$FC00-$FFFB` (960 bytes)
  - **Character port**: `$FFFF` - write characters directly
  - **Cursor control**: `$FFFC` (column) and `$FFFD` (row)
  - **Control register**: `$FFFE` (auto-scroll, clear screen, etc.)
  - **Support for control characters**: `\n`, `\r`, `\t`, `\b`
  - **Automatic auto-scroll** when the screen is full
- Example usage:
  ```cpp
  auto textScreen = std::make_shared<TextScreen>();
  cpu.registerIODevice(textScreen);
  
  // Write from 6502 code via character port
  mem[0x8000] = 0xA9;  // LDA #'H'
  mem[0x8001] = 'H';
  mem[0x8002] = 0x8D;  // STA $FFFF
  mem[0x8003] = 0xFF;
  mem[0x8004] = 0xFF;
  
  // Or use the C++ API directly
  textScreen->writeCharAtCursor('H');
  textScreen->writeCharAtCursor('i');
  std::cout << textScreen->getBuffer();
  ```
- See [docs/video_device.md](docs/video_device.md) for complete documentation and [examples/text_screen_demo.cpp](examples/text_screen_demo.cpp) for examples.

### Audio Support: Tone Generator (BasicAudio)

- New `BasicAudio` device simulating a simple tone generator for audio playback.
- Main features:
  - **Square wave generation** with frequencies from 20 Hz to 20 kHz
  - **Memory-mapped registers**: `$FB00-$FB05` for full control from 6502
  - **Programmable frequency, duration, and volume**
  - **Real-time playback** using SDL2 Audio
  - **Ideal for retro music and sound effects** like classic computer chips
- Example usage:
  ```cpp
  auto audio = std::make_shared<BasicAudio>();
  audio->initialize();
  cpu.registerIODevice(audio);
  
  // Play A (440 Hz) for 500 ms
  audio->playTone(440, 500, 200);
  
  // Or from 6502 code:
  // LDA #$B8 : STA $FB00  ; Low frequency
  // LDA #$01 : STA $FB01  ; High frequency
  // LDA #$F4 : STA $FB02  ; Low duration (500 ms)
  // LDA #$01 : STA $FB03  ; High duration
  // LDA #$C8 : STA $FB04  ; Volume (200)
  // LDA #$01 : STA $FB05  ; Play
  ```
- Run the demo (listen to the musical scale!):
  ```bash
  cd build
  ./audio_demo
  ```
- See [docs/audio_device.md](docs/audio_device.md) for complete documentation and [examples/audio_demo.cpp](examples/audio_demo.cpp) for examples.

### Serial Communication over TCP (TcpSerial)

- New `TcpSerial` device simulating an ACIA 6551 serial port using TCP/IP
- Main features:
  - **ACIA 6551 compatible**: Standard registers `$FA00-$FA03` for classic 6502
  - **TCP extensions**: Network configuration via additional registers
  - **Client mode**: Connect to remote TCP servers
  - **Server mode**: Listen for incoming connections on a port
  - **Bidirectional communication** in real-time
  - **Integration with modern tools**: netcat, telnet, Python, etc.
- Example usage:
  ```cpp
  auto tcpSerial = std::make_shared<TcpSerial>();
  tcpSerial->initialize();
  cpu.registerIODevice(tcpSerial);
  
  // Listen for connections on port 12345
  tcpSerial->listen(12345);
  
  // Send/receive data
  while (tcpSerial->dataAvailable()) {
      uint8_t byte = tcpSerial->receiveByte();
      tcpSerial->transmitByte(byte);  // Echo
  }
  
  // Or from 6502 code:
  // LDA #$39 : STA $FA04  ; Low port (12345)
  // LDA #$30 : STA $FA05  ; High port
  // LDA #$02 : STA $FA06  ; Activate listen mode
  // ; Loop: LDA $FA01 : AND #$01 : BEQ Loop
  // LDA $FA00             ; Read data
  // STA $FA00             ; Send (echo)
  ```
- Run the demo (connect with `nc localhost 12345`!):
  ```bash
  cd build
  ./tcp_serial_demo
  ```
- See [docs/serial_device.md](docs/serial_device.md) for complete documentation and [examples/tcp_serial_demo.cpp](examples/tcp_serial_demo.cpp) for examples.

### Retro GUI Interface (EmulatorGUI)

- **New retro-style GUI from the 80s** inspired by Apple II, Commodore 64, and MSX
- Main features:
  - **40x24 character screen** with authentic vintage look
  - **16 color palette** like Apple II/Commodore 64
  - **Blinking block cursor** like classic terminals
  - **Rendered with SDL2** for smooth performance
  - **Full keyboard input** with special characters
  - **Seamless integration with TextScreen**
- Example usage:
  ```cpp
  #include "emulator_gui.hpp"
  #include "text_screen.hpp"
  
  // Create GUI with 16x16 pixel characters
  EmulatorGUI gui("6502 Retro Terminal", 16, 16);
  gui.initialize();
  
  // Connect with TextScreen
  auto textScreen = std::make_shared<TextScreen>();
  gui.attachTextScreen(textScreen);
  
  // Main loop
  while (gui.isInitialized()) {
      if (gui.hasKey()) {
          char key = gui.getLastKey();
          textScreen->writeCharAtCursor(key);
      }
      gui.update();
      SDL_Delay(16);
  }
  ```
- Run the demo:
  ```bash
  cd build
  ./gui_demo
  ```
- See [docs/emulator_gui.md](docs/emulator_gui.md) for complete documentation.

Check the files in [docs/](docs/) for architecture details and supported instructions.

### Advanced Debugging

- A debugger has been added with breakpoints, watchpoints, tracing, and CPU state inspection.
- See [docs/debugger.md](docs/debugger.md) for the complete usage guide.
