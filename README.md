# cpu6502

## Cambios recientes (diciembre 2025)

- Ahora `cpu_demo` permite cargar binarios externos usando:
  ```
  ./cpu_demo file ../examples/demo_program.bin
  ```
- Si no se especifica un binario, ejecuta un programa de prueba clásico en memoria.
- Se añadió logging detallado de accesos a memoria en `cpu_log.txt`.
- Argumentos de línea de comandos:
  - `file <ruta>`: carga un binario externo en 0x8000.
  - `infinite`: ejecuta ciclos infinitos.
- Mejoras en la documentación inline y comentarios del código.

### Integración de E/S Apple II

- La CPU ahora soporta dispositivos de E/S modulares mediante la interfaz `IODevice`.
- Se incluye `AppleIO` para simular el teclado ($FD0C) y pantalla ($FDED) de Apple II.
- Registrar dispositivos IO:
  ```cpp
  auto appleIO = std::make_shared<AppleIO>();
  cpu.registerIODevice(appleIO);
  ```
- Los dispositivos IO interceptan accesos a memoria antes de la lectura/escritura estándar.
- Ideal para extender el emulador con periféricos, timers, gráficos, etc.

### Soporte para Almacenamiento de Archivos (FileDevice)

- Nuevo dispositivo `FileDevice` que permite cargar y guardar binarios desde/hacia archivos del host.
- Dos modos de operación:
  - **API directa en C++**: Métodos `loadBinary()` y `saveBinary()`
  - **Registros mapeados en memoria**: Control desde código 6502 en direcciones `$FE00-$FE4F`
- Perfecto para desarrollo, pruebas y persistencia de datos.
- Ejemplo de uso:
  ```cpp
  auto fileDevice = std::make_shared<FileDevice>(&mem);
  cpu.registerIODevice(fileDevice);
  
  // Cargar programa desde archivo
  fileDevice->loadBinary("programa.bin", 0x8000);
  
  // Guardar datos de memoria
  fileDevice->saveBinary("datos.bin", 0x0200, 256);
  ```
- Ver `docs/file_device.md` para documentación completa y `examples/file_device_demo.cpp` para ejemplos.

### Soporte Gráfico: Pantalla de Texto (TextScreen)

### Sistema de Interrupciones (IRQ/NMI)

- Sistema centralizado de gestión de interrupciones mediante `InterruptController`.
- Soporte completo para IRQ (enmascarable) y NMI (no enmascarable).
- Los dispositivos pueden implementar la interfaz `InterruptSource` para disparar interrupciones.
- Manejo automático de vectores de interrupción, pila y flags de la CPU.
- Ejemplo de uso:
  ```cpp
  InterruptController intCtrl;
  cpu.setInterruptController(&intCtrl);
  
  auto timer = std::make_shared<BasicTimer>();
  intCtrl.registerSource(timer);
  
  // El timer dispara IRQ periódicamente
  timer->setLimit(1000);
  timer->write(0xFC08, 0x03);  // Enable | IRQ Enable
  
  // En el loop principal
  cpu.checkAndHandleInterrupts(mem);
  ```
- Ver `docs/interrupt_system.md` para documentación completa y `examples/interrupt_demo.cpp` para ejemplos.
### Soporte Gráfico: Pantalla de Texto (TextScreen)

- Nuevo dispositivo `TextScreen` que simula una pantalla de texto de 40x24 caracteres.
- Características principales:
  - **Buffer de video mapeado en memoria**: `$FC00-$FFFB` (960 bytes)
  - **Puerto de caracteres**: `$FFFF` - escribir caracteres directamente
  - **Control de cursor**: `$FFFC` (columna) y `$FFFD` (fila)
  - **Registro de control**: `$FFFE` (auto-scroll, limpiar pantalla, etc.)
  - **Soporte para caracteres de control**: `\n`, `\r`, `\t`, `\b`
  - **Auto-scroll automático** cuando se llena la pantalla
- Ejemplo de uso:
  ```cpp
  auto textScreen = std::make_shared<TextScreen>();
  cpu.registerIODevice(textScreen);
  
  // Escribir desde código 6502 mediante puerto de caracteres
  mem[0x8000] = 0xA9;  // LDA #'H'
  mem[0x8001] = 'H';
  mem[0x8002] = 0x8D;  // STA $FFFF
  mem[0x8003] = 0xFF;
  mem[0x8004] = 0xFF;
  
  // O usar la API C++ directamente
  textScreen->writeCharAtCursor('H');
  textScreen->writeCharAtCursor('i');
  std::cout << textScreen->getBuffer();
  ```
- Ver `docs/video_device.md` para documentación completa y `examples/text_screen_demo.cpp` para ejemplos.

### Soporte de Audio: Generador de Tonos (BasicAudio)

- Nuevo dispositivo `BasicAudio` que simula un generador de tonos simple para reproducir audio.
- Características principales:
  - **Generación de ondas cuadradas** con frecuencias de 20 Hz a 20 kHz
  - **Registros mapeados en memoria**: `$FB00-$FB05` para control completo desde 6502
  - **Control de frecuencia, duración y volumen** programable
  - **Reproducción en tiempo real** usando SDL2 Audio
  - **Ideal para música retro y efectos de sonido** como los chips de las computadoras clásicas
- Ejemplo de uso:
  ```cpp
  auto audio = std::make_shared<BasicAudio>();
  audio->initialize();
  cpu.registerIODevice(audio);
  
  // Reproducir La (440 Hz) durante 500 ms
  audio->playTone(440, 500, 200);
  
  // O desde código 6502:
  // LDA #$B8 : STA $FB00  ; Frecuencia baja
  // LDA #$01 : STA $FB01  ; Frecuencia alta
  // LDA #$F4 : STA $FB02  ; Duración baja (500 ms)
  // LDA #$01 : STA $FB03  ; Duración alta
  // LDA #$C8 : STA $FB04  ; Volumen (200)
  // LDA #$01 : STA $FB05  ; Reproducir
  ```
- Ejecutar la demo (¡escucha la escala musical!):
  ```bash
  cd build
  ./audio_demo
  ```
- Ver `docs/audio_device.md` para documentación completa y `examples/audio_demo.cpp` para ejemplos.

### Comunicación Serial sobre TCP (TcpSerial)

- Nuevo dispositivo `TcpSerial` que simula un puerto serial ACIA 6551 utilizando TCP/IP
- Características principales:
  - **Compatible con ACIA 6551**: Registros estándar `$FA00-$FA03` para código 6502 clásico
  - **Extensiones TCP**: Configuración de red mediante registros adicionales
  - **Modo cliente**: Conectar a servidores TCP remotos
  - **Modo servidor**: Escuchar conexiones entrantes en un puerto
  - **Comunicación bidireccional** en tiempo real
  - **Integración con herramientas modernas**: netcat, telnet, Python, etc.
- Ejemplo de uso:
  ```cpp
  auto tcpSerial = std::make_shared<TcpSerial>();
  tcpSerial->initialize();
  cpu.registerIODevice(tcpSerial);
  
  // Escuchar conexiones en puerto 12345
  tcpSerial->listen(12345);
  
  // Enviar/recibir datos
  while (tcpSerial->dataAvailable()) {
      uint8_t byte = tcpSerial->receiveByte();
      tcpSerial->transmitByte(byte);  // Echo
  }
  
  // O desde código 6502:
  // LDA #$39 : STA $FA04  ; Puerto bajo (12345)
  // LDA #$30 : STA $FA05  ; Puerto alto
  // LDA #$02 : STA $FA06  ; Activar modo escucha
  // ; Loop: LDA $FA01 : AND #$01 : BEQ Loop
  // LDA $FA00             ; Leer dato
  // STA $FA00             ; Enviar (echo)
  ```
- Ejecutar la demo (¡conéctate con `nc localhost 12345`!):
  ```bash
  cd build
  ./tcp_serial_demo
  ```
- Ver `docs/serial_device.md` para documentación completa y `examples/tcp_serial_demo.cpp` para ejemplos.

### Interfaz Gráfica Retro (EmulatorGUI)

- **Nueva GUI con estilo retro de los años 80** inspirada en Apple II, Commodore 64 y MSX
- Características principales:
  - **Pantalla de 40x24 caracteres** con aspecto vintage auténtico
  - **Paleta de 16 colores** estilo Apple II/Commodore 64
  - **Cursor de bloque parpadeante** como las terminales clásicas
  - **Renderizado con SDL2** para rendimiento fluido
  - **Entrada de teclado completa** con caracteres especiales
  - **Integración perfecta con TextScreen**
- Ejemplo de uso:
  ```cpp
  #include "gui/emulator_gui.hpp"
  #include "devices/text_screen.hpp"
  
  // Crear GUI con caracteres de 16x16 píxeles
  EmulatorGUI gui("6502 Retro Terminal", 16, 16);
  gui.initialize();
  
  // Conectar con TextScreen
  auto textScreen = std::make_shared<TextScreen>();
  gui.attachTextScreen(textScreen);
  
  // Bucle principal
  while (gui.isInitialized()) {
      if (gui.hasKey()) {
          char key = gui.getLastKey();
          textScreen->writeCharAtCursor(key);
      }
      gui.update();
      SDL_Delay(16);
  }
  ```
- Ejecutar la demo:
  ```bash
  cd build
  ./gui_demo
  ```
- Ver `docs/emulator_gui.md` o `docs/emulator_gui_es.md` para documentación completa

Consulta los archivos en `docs/` para detalles de arquitectura e instrucciones soportadas.
