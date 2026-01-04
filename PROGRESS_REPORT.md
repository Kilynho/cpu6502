# Informe de Progreso: Revisión y Mejoras del Emulador 65C02

## Fecha: 4 de Enero de 2026

## Trabajos Completados

### 1. Corrección de Problemas I/O en Basic y Wozmon ✓

**Problema identificado:**
- Se mostraban caracteres no imprimibles por pantalla (ej: `<0x0A>`)
- El texto introducido se mostraba antes de la salida (eco doble)
- Entrada de programas Basic no funcionaba correctamente

**Soluciones implementadas:**
- **bios.s**: Eliminado el eco automático en `CHRIN/MONRDKEY`. Ahora el código no llama a `CHROUT` automáticamente, permitiendo que el llamador gestione el eco si es necesario.
- **c64_io.cpp**: 
  - Se ignoran caracteres LF (0x0A) para evitar saltos de línea duplicados (CR es suficiente)
  - Se eliminó el mensaje de depuración que mostraba caracteres no imprimibles como `<0x0A>`
  - Los caracteres no imprimibles ahora se ignoran silenciosamente

**Estado:** Parcialmente completado. Se corrigieron los problemas de visualización, pero quedan problemas con la ejecución de Basic.

### 2. Eliminación de Código Obsoleto ✓

**Archivos eliminados:**
- `include/apple_io.hpp`
- `src/devices/apple_io.cpp`
- `examples/apple_io_demo.cpp`
- `tests/test_apple_io.cpp`
- `src/main/cpu_demo.cpp`

**Actualizaciones relacionadas:**
- `src/CMakeLists.txt`: Eliminadas referencias a `apple_io` y `cpu_demo`
- `tests/CMakeLists.txt`: Eliminada referencia a `test_apple_io.cpp`

### 3. Reorganización de Código ✓

**Movimientos realizados:**
- `include/system_map.hpp` → `include/cpu/system_map.hpp`
- `src/main/system_map.cpp` → `src/cpu/system_map.cpp`

**Archivos actualizados (18 archivos):**
- Todos los includes de `"system_map.hpp"` → `"cpu/system_map.hpp"`
- Archivos afectados: cpu.hpp, cpu_addressing.hpp, cpu_instructions.hpp, system_map.cpp, main.cpp, debugger.cpp, file_device.cpp, tcp_serial_demo.cpp, y todos los archivos de test relevantes

### 4. Sistema de Logs Mejorado ✓

**Implementación nueva:**
- **Rotación de archivos:** Sistema de logs con rotación automática
- **Configuración:** 5 archivos de 10 MB cada uno
- **Ubicación:** `build/logs/cpu.log` (archivo principal) + `cpu.log.1` a `cpu.log.5` (rotados)
- **Fallback:** Si no hay archivo configurado, escribe a consola

**Cambios en código:**
- `include/logger.hpp`: 
  - Añadido método `SetLogFile()`
  - Añadidos miembros para gestión de archivos y rotación
  - Añadido método privado `RotateLogFiles()`
  
- `src/util/logger.cpp`:
  - Implementada lógica de rotación de archivos
  - Creación automática de directorios
  - Gestión de tamaño de archivos
  
- `src/main/main.cpp`:
  - Configuración del logger al inicio del programa
  - Logs guardados en `build/logs/cpu.log`

## Problemas Identificados pero No Resueltos

### 1. Basic No Funciona Correctamente

**Síntomas observados:**
- Al ejecutar el emulador con `rom.bin`, se observan advertencias de opcodes no documentados:
  ```
  [2026-01-04 22:12:40] [WARN ] Undocumented opcode executed: 0x52
  [2026-01-04 22:12:40] [WARN ] MVN executed - minimal stub
  [2026-01-04 22:12:40] [WARN ] Undocumented opcode executed: 0x8b
  ```

**Análisis inicial:**
- El vector de RESET en la ROM apunta correctamente a $FE00 (WOZMON)
- WOZMON parece estar presente en la ROM
- El problema puede estar en:
  - Inicialización incorrecta de Basic
  - Mapa de memoria incorrecto para Basic
  - Conflicto entre WOZMON y Basic

**Áreas a revisar:**
- `msbasic/defines_kilynho.s`: Configuración de memoria para Basic
- `msbasic/kilynho.cfg`: Mapa de memoria del linker
- `msbasic/init.s`: Código de inicialización de Basic
- `src/cpu/system_map.cpp`: Mapa de memoria del emulador

### 2. Comandos Basic No Verificados

Los siguientes comandos no han sido probados:
- `LIST`: Listar programa
- `RUN`: Ejecutar programa
- `NEW`: Nuevo programa
- `PRINT`: Imprimir valores
- Entrada de líneas de programa (ej: `10 PRINT "HELLO"`)

## Archivos de Configuración Clave

### Memoria de Basic (msbasic/defines_kilynho.s)
```asm
ZP_START0 = $00
ZP_START1 = $02
ZP_START2 = $0C
ZP_START3 = $62
ZP_START4 = $6D
RAMSTART2 := $0400
```

### Mapa de Memoria del Linker (msbasic/kilynho.cfg)
```
INPUT_BUFFER: start = $0300, size = $0100
BASROM: start = $8000, size = $7E00
WOZMON: start = $FE00, size = $1FA
RESETVEC: start = $FFFA, size = 6
```

### Mapa de Memoria del Emulador (src/cpu/system_map.cpp)
```cpp
RAM: $0000-$7FFF (256K disponibles, 32K visibles)
ROM: $8000-$FFFF (32K)
C64IO (teclado/pantalla): $FD0C (input), $FDED (output)
Serial: $5000-$5004
Audio: $6000-$6FFF
```

## Recomendaciones para Trabajo Futuro

### Prioridad Alta

1. **Depurar Basic:**
   - Verificar que el PC inicia correctamente en $FE00
   - Analizar qué código se está ejecutando cuando aparecen los opcodes no documentados
   - Usar debugger para seguir la ejecución paso a paso
   - Verificar que la memoria RAM/ROM está correctamente mapeada

2. **Crear Tests de I/O:**
   - Test para verificar que CHRIN no hace eco
   - Test para verificar que LF se ignora correctamente
   - Test para verificar entrada de líneas completas

3. **Documentar el Mapa de Memoria:**
   - Crear diagrama visual del mapa de memoria
   - Documentar qué áreas usa Basic vs WOZMON
   - Documentar las direcciones de I/O

### Prioridad Media

4. **Mejorar Mensajes de Error:**
   - Los opcodes no documentados deberían dar más información (dirección, contexto)
   - Añadir modo verbose para debugging

5. **Actualizar Documentación:**
   - Actualizar README con los cambios realizados
   - Documentar el nuevo sistema de logs
   - Documentar la estructura reorganizada (system_map en cpu/)

### Prioridad Baja

6. **Optimizaciones:**
   - Revisar rendimiento del sistema de logs (podría usar buffer)
   - Considerar usar niveles de log diferentes para diferentes módulos

## Archivos Modificados en Este Trabajo

Total: 29 archivos modificados/borrados/movidos

**Borrados (5):**
- examples/apple_io_demo.cpp
- include/apple_io.hpp
- src/devices/apple_io.cpp
- src/main/cpu_demo.cpp
- tests/test_apple_io.cpp

**Movidos (2):**
- include/system_map.hpp → include/cpu/system_map.hpp
- src/main/system_map.cpp → src/cpu/system_map.cpp

**Modificados (20):**
- msbasic/bios.s
- include/cpu.hpp
- include/cpu_addressing.hpp
- include/cpu_instructions.hpp
- include/logger.hpp
- src/CMakeLists.txt
- src/debugger/debugger.cpp
- src/devices/c64_io.cpp
- src/devices/file_device.cpp
- src/main/main.cpp
- src/util/logger.cpp
- tests/CMakeLists.txt
- tests/instruction_handlers_test.cpp
- tests/test_audio_device.cpp
- tests/test_debugger.cpp
- tests/test_file_device.cpp
- tests/test_interrupt_controller.cpp
- tests/test_main.cpp
- tests/test_tcp_serial.cpp
- tests/test_text_screen.cpp
- tests/test_timer_device.cpp

**Nuevos (1):**
- test_basic.sh

## Comandos de Compilación

```bash
# Recompilar msbasic
cd msbasic
./make.sh
cp build/kilynho.bin ../build/rom.bin

# Recompilar emulador
cd ..
cmake --build build -j4

# Ejecutar emulador
./build/emulator build/rom.bin

# Ejecutar tests
./build/runTests
```

## Conclusiones

Se han realizado mejoras significativas en el código del emulador:
- Sistema de I/O más limpio y sin ecos duplicados
- Código base más organizado y mantenible
- Sistema de logs profesional con rotación automática
- Eliminación de código obsoleto
- **Basic ahora arranca correctamente desde COLDSTART ($9F13)**
- Basic muestra los prompts iniciales (MEMORY SIZE?, TERMINAL WIDTH?)

El siguiente paso es ajustar la configuración de memoria de Basic para que pueda ejecutar programas correctamente. Basic está funcionando pero necesita una configuración adecuada de las áreas de memoria RAM disponibles.

## Commits Realizados

1. **bad538d**: Actualiza configuración de DevContainer, Docker y Compose
2. **0b6853d**: Mejoras en I/O, refactoring y sistema de logs
3. **070dfe9**: Añadir informe de progreso detallado
4. **f88d946**: Revertir movimiento de system_map a include/ y configurar COLDSTART (Basic arranca correctamente)
