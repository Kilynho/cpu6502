# FileDevice: File Storage Integration

## Introducción

El sistema `FileDevice` permite al emulador 6502 cargar y guardar binarios desde/hacia archivos del sistema host. Esta funcionalidad es esencial para:

- Cargar programas desde archivos binarios externos
- Guardar datos de memoria a archivos
- Persistir el estado de la memoria
- Facilitar el desarrollo y prueba de programas 6502

## Arquitectura

### Jerarquía de Clases

```
IODevice (interfaz base)
    └── StorageDevice (interfaz de almacenamiento)
            └── FileDevice (implementación basada en archivos)
```

### StorageDevice Interface

Interfaz base que define las operaciones de almacenamiento:

```cpp
class StorageDevice : public IODevice {
public:
    // Cargar un binario desde archivo a memoria
    virtual bool loadBinary(const std::string& filename, 
                           uint16_t startAddress) = 0;
    
    // Guardar un bloque de memoria a archivo
    virtual bool saveBinary(const std::string& filename, 
                           uint16_t startAddress, 
                           uint16_t length) = 0;
    
    // Verificar si un archivo existe
    virtual bool fileExists(const std::string& filename) const = 0;
};
```

### FileDevice Implementation

`FileDevice` implementa `StorageDevice` y proporciona dos modos de operación:

1. **API Directa**: Métodos C++ para control programático
2. **Registros Mapeados**: Control desde código 6502 mediante direcciones de memoria

## Registros Mapeados en Memoria

FileDevice mapea los siguientes registros en el rango `0xFE00-0xFE4F`:

| Dirección | Nombre | Tipo | Descripción |
|-----------|--------|------|-------------|
| `0xFE00` | CONTROL | W/R | Registro de control de operación |
| `0xFE01` | START_ADDR_LO | W/R | Byte bajo de dirección de inicio |
| `0xFE02` | START_ADDR_HI | W/R | Byte alto de dirección de inicio |
| `0xFE03` | LENGTH_LO | W/R | Byte bajo de longitud |
| `0xFE04` | LENGTH_HI | W/R | Byte alto de longitud |
| `0xFE05` | STATUS | W/R | Estado de la última operación |
| `0xFE10-0xFE4F` | FILENAME | W/R | Buffer de nombre de archivo (64 bytes) |

### Registro de Control (0xFE00)

Valores posibles:

| Valor | Operación | Descripción |
|-------|-----------|-------------|
| 0 | NONE | Sin operación |
| 1 | LOAD | Cargar archivo a memoria |
| 2 | SAVE | Guardar memoria a archivo |

**Nota**: La operación se ejecuta automáticamente al escribir un valor distinto de 0 en este registro.

### Registro de Estado (0xFE05)

Valores de retorno:

| Valor | Estado | Descripción |
|-------|--------|-------------|
| 0 | SUCCESS | Operación exitosa |
| 1 | ERROR | Error en la operación |

## Uso desde C++

### Inicialización

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "devices/file_device.hpp"

Mem mem;
CPU cpu;
auto fileDevice = std::make_shared<FileDevice>(&mem);

cpu.Reset(mem);
cpu.registerIODevice(fileDevice);
```

### Cargar un Binario

```cpp
// Cargar programa.bin en la dirección 0x8000
if (fileDevice->loadBinary("programa.bin", 0x8000)) {
    std::cout << "Programa cargado exitosamente\n";
    cpu.PC = 0x8000;  // Saltar al programa
    cpu.Execute(1000, mem);
} else {
    std::cerr << "Error al cargar el programa\n";
}
```

### Guardar Datos de Memoria

```cpp
// Guardar 256 bytes desde 0x0200 a datos.bin
if (fileDevice->saveBinary("datos.bin", 0x0200, 256)) {
    std::cout << "Datos guardados exitosamente\n";
} else {
    std::cerr << "Error al guardar datos\n";
}
```

## Uso desde Código 6502

### Ejemplo: Cargar un Archivo

```assembly
; Configurar nombre de archivo
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

; Configurar dirección de inicio = 0x8000
LDA #$00
STA $FE01     ; Byte bajo
LDA #$80
STA $FE02     ; Byte alto

; Ejecutar operación LOAD
LDA #1
STA $FE00

; Verificar estado
LDA $FE05
BEQ success   ; Si status = 0, éxito
; Manejo de error...
success:
JMP $8000     ; Saltar al programa cargado
```

### Ejemplo: Guardar Datos

```assembly
; Configurar nombre de archivo en $FE10-$FE4F
; (similar al ejemplo anterior)

; Configurar dirección de inicio = 0x0200
LDA #$00
STA $FE01
LDA #$02
STA $FE02

; Configurar longitud = 100 bytes
LDA #100
STA $FE03
LDA #0
STA $FE04

; Ejecutar operación SAVE
LDA #2
STA $FE00

; Verificar estado
LDA $FE05
BEQ success
; Manejo de error...
success:
; Continuar...
```

## Rutinas de Ayuda (Helper Routines)

### Escritura de Nombre de Archivo

```assembly
; Escribe una cadena terminada en null al buffer de filename
; Entrada: Dirección de la cadena en $00-$01 (zero page)
write_filename:
    LDY #0
.loop:
    LDA ($00),Y
    STA $FE10,Y
    BEQ .done
    INY
    CPY #64      ; Máximo 64 caracteres
    BNE .loop
.done:
    RTS
```

### Verificación de Estado

```assembly
; Verifica el estado de la última operación
; Retorna: Z=1 si éxito, Z=0 si error
check_status:
    LDA $FE05
    RTS
```

## Integración con Apple BASIC

Desde Apple BASIC se pueden usar las direcciones PEEK/POKE para controlar FileDevice:

```basic
10 REM Cargar archivo "PROG.BIN" en $8000
20 REM Escribir nombre de archivo
30 POKE 65040,80: REM 'P'
40 POKE 65041,82: REM 'R'
50 POKE 65042,79: REM 'O'
60 POKE 65043,71: REM 'G'
70 POKE 65044,46: REM '.'
80 POKE 65045,66: REM 'B'
90 POKE 65046,73: REM 'I'
100 POKE 65047,78: REM 'N'
110 POKE 65048,0: REM null
120 REM Configurar dirección
130 POKE 65025,0: REM byte bajo
140 POKE 65026,128: REM byte alto ($80)
150 REM Ejecutar LOAD
160 POKE 65024,1
170 REM Verificar estado
180 S = PEEK(65029)
190 IF S = 0 THEN PRINT "CARGADO OK"
200 IF S <> 0 THEN PRINT "ERROR"
```

## Integración con WOZMON

WOZMON (Wozniak Monitor) puede extenderse para incluir comandos de archivo:

### Comando LOAD (L)

```
L8000<prog.bin
```

Carga `prog.bin` en la dirección `$8000`.

### Comando SAVE (S)

```
S8000.81FF>datos.bin
```

Guarda desde `$8000` hasta `$81FF` a `datos.bin`.

**Nota**: Estas extensiones requieren modificar el código de WOZMON para usar los registros de FileDevice.

## Ejemplos Completos

### Demo Completo

Consulta `examples/file_device_demo.cpp` para un ejemplo completo que demuestra:

1. Cargar y ejecutar un programa desde archivo
2. Guardar datos de memoria
3. Uso de registros mapeados
4. Verificación de integridad

### Ejecutar el Demo

```bash
cd build
./file_device_demo
```

## Limitaciones y Consideraciones

### Limitaciones Actuales

1. **Tamaño de Archivo**: Los archivos no pueden exceder 64KB (límite del espacio de direcciones 6502)
2. **Nombres de Archivo**: Máximo 64 caracteres (null-terminated)
3. **Rutas**: Se soportan rutas relativas y absolutas del sistema host
4. **Concurrencia**: No hay protección contra accesos concurrentes

### Consideraciones de Seguridad

1. **Validación de Rutas**: FileDevice no valida rutas, confía en el sistema operativo
2. **Permisos**: Respeta los permisos del sistema de archivos del host
3. **Sobrescritura**: SAVE sobrescribirá archivos existentes sin confirmación

### Mejores Prácticas

1. Siempre verificar el estado después de una operación
2. Usar rutas absolutas para evitar ambigüedades
3. Validar tamaños antes de cargar/guardar
4. Mantener nombres de archivo cortos y compatibles

## Casos de Uso

### Desarrollo de Software 6502

```cpp
// Compilar código 6502 -> programa.bin
// Cargar y probar en el emulador
fileDevice->loadBinary("programa.bin", 0x8000);
cpu.PC = 0x8000;
cpu.Execute(10000, mem);
```

### Persistencia de Datos

```cpp
// Guardar estado del juego
fileDevice->saveBinary("savegame.dat", 0x2000, 0x1000);

// Cargar estado del juego
fileDevice->loadBinary("savegame.dat", 0x2000);
```

### Depuración

```cpp
// Guardar volcado de memoria para análisis
fileDevice->saveBinary("memdump.bin", 0x0000, 0x10000);
```

## Pruebas

### Ejecutar Tests

```bash
make test
# O directamente:
./build/runTests --gtest_filter=FileDevice*
```

### Cobertura de Pruebas

Los tests cubren:

- Creación y configuración del dispositivo
- Carga y guardado de archivos
- Registros mapeados
- Manejo de errores
- Integridad de datos
- Integración con CPU

Ver `tests/test_file_device.cpp` para detalles completos.

## Referencias

- **Código Fuente**:
  - `include/storage_device.hpp` - Interfaz StorageDevice
  - `include/devices/file_device.hpp` - Cabecera FileDevice
  - `src/devices/file_device.cpp` - Implementación FileDevice
  - `tests/test_file_device.cpp` - Suite de pruebas
  - `examples/file_device_demo.cpp` - Ejemplo completo

- **Documentación Relacionada**:
  - `docs/architecture.md` - Arquitectura del sistema IODevice
  - `README.md` - Información general del proyecto

## Extensiones Futuras

### Posibles Mejoras

1. **Búfer de Datos**: Transferencias más grandes con búfer intermedio
2. **Operaciones Asíncronas**: Carga/guardado en segundo plano
3. **Compresión**: Soporte para archivos comprimidos
4. **Formatos Especiales**: Soporte para formatos .PRG, .D64, etc.
5. **Sistema de Archivos Virtual**: Emular un sistema de archivos completo
6. **Callbacks**: Notificaciones cuando operaciones completan

### Compatibilidad Extendida

1. **Cassette Emulation**: Simular carga desde cinta
2. **Disk Emulation**: Emular unidades de disco
3. **Networking**: Transferencia de archivos por red
