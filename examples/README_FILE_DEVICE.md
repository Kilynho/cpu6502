# FileDevice - Ejemplo de Uso

Este directorio contiene ejemplos de uso del dispositivo de almacenamiento `FileDevice`.

## Archivos

- **file_device_demo.cpp**: Demostración completa de todas las funcionalidades de FileDevice
- **apple_io_demo.cpp**: Demostración del dispositivo Apple II I/O (para comparación)

## Ejecutar los Ejemplos

### Usando Make

```bash
# Desde el directorio raíz del proyecto
make file_device_demo
```

### Directamente

```bash
# Compilar primero
make all

# Ejecutar
./build/file_device_demo
```

## Funcionalidades Demostradas

### file_device_demo.cpp

Este ejemplo demuestra:

1. **Carga de programas desde archivo**
   - Crear un programa 6502 en memoria
   - Guardarlo a un archivo
   - Cargarlo de vuelta usando FileDevice
   - Ejecutar el programa cargado

2. **Guardado de datos de memoria**
   - Guardar bloques de memoria a archivos
   - Usar la API directa de C++

3. **Uso de registros mapeados en memoria**
   - Escribir nombre de archivo en $FE10-$FE4F
   - Configurar dirección y longitud en $FE01-$FE04
   - Ejecutar operaciones mediante el registro de control $FE00
   - Verificar estado en $FE05

4. **Verificación de integridad**
   - Cargar y guardar datos
   - Verificar que los datos se mantienen íntegros

## Registros de FileDevice

| Dirección | Nombre | Descripción |
|-----------|--------|-------------|
| $FE00 | CONTROL | Control de operación (0=nada, 1=cargar, 2=guardar) |
| $FE01 | START_LO | Byte bajo de dirección de inicio |
| $FE02 | START_HI | Byte alto de dirección de inicio |
| $FE03 | LENGTH_LO | Byte bajo de longitud |
| $FE04 | LENGTH_HI | Byte alto de longitud |
| $FE05 | STATUS | Estado (0=éxito, 1=error) |
| $FE10-$FE4F | FILENAME | Buffer de nombre de archivo (64 bytes) |

## Ejemplo de Código 6502

```assembly
; Cargar archivo "PROG.BIN" en $8000
    
    ; Escribir nombre de archivo
    LDA #'P'
    STA $FE10
    LDA #'R'
    STA $FE11
    LDA #'O'
    STA $FE12
    LDA #'G'
    STA $FE13
    LDA #'.'
    STA $FE14
    LDA #'B'
    STA $FE15
    LDA #'I'
    STA $FE16
    LDA #'N'
    STA $FE17
    LDA #0        ; Null terminator
    STA $FE18
    
    ; Configurar dirección = $8000
    LDA #$00
    STA $FE01     ; Byte bajo
    LDA #$80
    STA $FE02     ; Byte alto
    
    ; Ejecutar LOAD
    LDA #1
    STA $FE00
    
    ; Verificar estado
    LDA $FE05
    BEQ success
    ; Error handling...
success:
    JMP $8000     ; Saltar al programa
```

## Ejemplo de API C++

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "devices/file_device.hpp"

int main() {
    Mem mem;
    CPU cpu;
    auto fileDevice = std::make_shared<FileDevice>(&mem);
    
    cpu.Reset(mem);
    cpu.registerIODevice(fileDevice);
    
    // Cargar programa
    if (fileDevice->loadBinary("programa.bin", 0x8000)) {
        cpu.PC = 0x8000;
        cpu.Execute(1000, mem);
    }
    
    // Guardar datos
    fileDevice->saveBinary("datos.bin", 0x0200, 256);
    
    return 0;
}
```

## Casos de Uso

### Desarrollo de Software
1. Ensamblar código 6502 externamente
2. Cargar el binario con FileDevice
3. Ejecutar y probar en el emulador

### Persistencia de Datos
1. Guardar estado del juego
2. Cargar estado guardado
3. Exportar/importar datos

### Debugging
1. Volcar memoria para análisis
2. Cargar estados de memoria específicos
3. Crear checkpoints de ejecución

## Limitaciones

- Tamaño máximo de archivo: 64KB (espacio de direcciones 6502)
- Longitud máxima de nombre: 64 caracteres
- Sobrescritura automática (sin confirmación)

## Más Información

Ver `docs/file_device.md` para documentación completa.
