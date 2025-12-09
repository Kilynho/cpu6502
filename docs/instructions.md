## Best Practices

1. **Always update flags correctly** - This is critical for program correctness
2. **Test edge cases** - 0x00, 0xFF, boundary crossings
3. **Document cycle counts** - Include comments about timing
4. **Use helper functions** - `UpdateZeroAndNegativeFlags`, etc.
5. **Log memory access** - For debugging and tracing

## Ejemplo de uso con binario externo

```
./cpu_demo file ../examples/demo_program.bin
```

## Instrucciones soportadas

- LDA (Immediate, Zero Page, Zero Page,X, Absolute, Absolute,X, Absolute,Y)
- LDX (Immediate)
- STA (Zero Page)
- JSR, RTS

Consulta el código fuente para detalles de implementación y logging.

## Integración con dispositivos de E/S

### Apple II I/O

El emulador soporta la simulación de E/S Apple II mediante la clase `AppleIO`:

- **Lectura de teclado ($FD0C)**: Devuelve el siguiente carácter del buffer de entrada, o 0 si no hay entrada.
- **Escritura en pantalla ($FDED)**: Envía el carácter al buffer de pantalla y lo imprime en la consola.

### Ejemplo de código

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "apple_io.hpp"

CPU cpu;
Mem mem;
auto appleIO = std::make_shared<AppleIO>();

cpu.Reset(mem);
cpu.registerIODevice(appleIO);

// Simular entrada de teclado
appleIO->pushInput('A');

// Ejecutar código que lea de $FD0C
// mem[0x8000] = 0xAD; mem[0x8001] = 0x0C; mem[0x8002] = 0xFD; // LDA $FD0C
// cpu.Execute(4, mem);
// El acumulador ahora contiene 'A'

// Escribir en pantalla
// mem[0x8003] = 0x8D; mem[0x8004] = 0xED; mem[0x8005] = 0xFD; // STA $FDED
// cpu.Execute(4, mem);
// La pantalla ahora muestra 'A'
```

### Extensibilidad

Puedes crear tus propios dispositivos de E/S implementando la interfaz `IODevice` y registrándolos en la CPU.
