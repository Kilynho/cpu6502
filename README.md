# cpu6502

# cpu6502

**Emulador 65C02/6502 avanzado con integración de WOZMON y BASIC**

---

## Tabla de Contenidos

- [Descripción General](#descripción-general)
- [Características Principales](#características-principales)
- [Novedades Destacadas](#novedades-destacadas)
- [Instalación](#instalación)
- [Ejemplos y Demos](#ejemplos-y-demos)
  - [WOZMON (Apple 1 Monitor)](#wozmon-apple-1-monitor)
  - [BASIC (Microsoft BASIC)](#basic-microsoft-basic)
- [Uso Básico](#uso-básico)
- [Sistema de Interrupciones](#sistema-de-interrupciones)
- [Depurador y Scripting](#depurador-y-scripting)
- [Contribuir](#contribuir)
- [Documentación Avanzada](#documentación-avanzada)
- [Licencia](#licencia)

---

## Descripción General

cpu6502 es un emulador moderno y modular del microprocesador 65C02/6502, orientado a la precisión, extensibilidad y facilidad de integración con sistemas retro y modernos. Incluye soporte para dispositivos, scripting en Python, depuración avanzada y ejemplos completos de integración con monitores y lenguajes clásicos.

---

## Características Principales

- Emulación precisa de 65C02 y 6502 clásico
- Sistema de interrupciones centralizado (IRQ/NMI)
- Dispositivos integrados: PIA, ACIA, VIA, Timer, Audio, Serial, etc.
- Depurador avanzado con breakpoints, watchpoints y traza de instrucciones
- Scripting API en Python
- Ejemplos completos: integración con WOZMON (Apple 1 Monitor) y Microsoft BASIC
- Cobertura de tests exhaustiva (más de 200 tests automáticos)
- Documentación modular y ejemplos listos para usar

---

## Novedades Destacadas

### Integración de WOZMON (Apple 1 Monitor)

- Demo y documentación completa en [docs/wozmon_integration.md](docs/wozmon_integration.md)
- Ejecutable: `wozmon_demo`
- Permite interactuar con el monitor original Apple 1, examinar memoria, ejecutar código y probar la emulación en un entorno clásico.

### Integración de Microsoft BASIC

- Soporte para cargar y ejecutar binarios de BASIC (ver carpeta `msbasic/`)
- Ejemplo de traza y ejecución en [examples/basic_trace_demo.cpp](examples/basic_trace_demo.cpp)
- Permite validar la compatibilidad y estabilidad de la emulación con uno de los intérpretes más populares de la época.

---

## Instalación

```bash
git clone https://github.com/Kilynho/cpu6502.git
cd cpu6502
mkdir build && cd build
cmake ..
make -j$(nproc)
```

---

## Ejemplos y Demos

### WOZMON (Apple 1 Monitor)

- Ejecuta el demo:
  ```bash
  ./wozmon_demo
  ```
- Consulta la guía de uso y comandos en [docs/wozmon_integration.md](docs/wozmon_integration.md).

### BASIC (Microsoft BASIC)

- Ejecuta el demo de traza:
  ```bash
  ./basic_trace_demo
  ```
- Carga binarios de BASIC desde la carpeta `msbasic/` y explora la integración con el emulador.

---

## Uso Básico

```cpp
#include "cpu.hpp"
#include "mem.hpp"

Mem mem;
CPU cpu;
cpu.Reset(mem);
// ...cargar programa...
cpu.Execute(1000, mem);
```

Consulta [docs/interrupt_system.md](docs/interrupt_system.md) y [docs/debugger.md](docs/debugger.md) para detalles avanzados.

---

## Sistema de Interrupciones

- Controlador centralizado (`InterruptController`)
- Soporte completo para IRQ y NMI
- Ejemplo de uso:
  ```cpp
  InterruptController intCtrl;
  cpu.setInterruptController(&intCtrl);
  // ...registrar dispositivos...
  cpu.checkAndHandleInterrupts(mem);
  ```
- Más información en [docs/interrupt_system.md](docs/interrupt_system.md) y [examples/interrupt_demo.cpp](examples/interrupt_demo.cpp).

---

## Depurador y Scripting

- Breakpoints, watchpoints, inspección de estado y traza
- API de scripting en Python: [docs/scripting_api.md](docs/scripting_api.md)
- Ejemplo de integración:
  ```cpp
  Debugger dbg;
  dbg.attach(&cpu, &mem);
  cpu.setDebugger(&dbg);
  dbg.addBreakpoint(0x8003);
  cpu.Execute(100, mem);
  ```
- Más detalles en [docs/debugger.md](docs/debugger.md)

---

## Contribuir

- Lee [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md) para guías de estilo y flujo de trabajo.
- Tests automáticos: `make test` o `ctest`.

---

## Documentación Avanzada

- [docs/wozmon_integration.md](docs/wozmon_integration.md): Integración Apple 1 Monitor
- [docs/interrupt_system.md](docs/interrupt_system.md): Sistema de interrupciones
- [docs/debugger.md](docs/debugger.md): Depurador avanzado
- [docs/scripting_api.md](docs/scripting_api.md): Scripting en Python
- [RELEASE_NOTES_v2.0.md](RELEASE_NOTES_v2.0.md): Notas de la versión

---

## Licencia

MIT License. Consulta el archivo LICENSE para más detalles.

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
  #include "gui/emulator_gui.hpp"
  #include "devices/text_screen.hpp"
  
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
