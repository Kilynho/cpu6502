# Microsoft BASIC en cpu6502

## Descripción

Esta guía documenta la integración y uso de Microsoft BASIC (msbasic) en el emulador cpu6502. Permite cargar, ejecutar y trazar programas BASIC históricos sobre una emulación precisa del 65C02/6502.

---

## Contenidos

- [Requisitos](#requisitos)
- [Ubicación de binarios y fuentes](#ubicación-de-binarios-y-fuentes)
- [Ejecución rápida](#ejecución-rápida)
- [Demo de traza BASIC](#demo-de-traza-basic)
- [Opciones de integración](#opciones-de-integración)
- [Notas técnicas](#notas-técnicas)
- [Recursos adicionales](#recursos-adicionales)

---

## Requisitos

- cpu6502 compilado (ver README principal)
- Binarios de Microsoft BASIC (ver carpeta `msbasic/`)

---

## Ubicación de binarios y fuentes

- Los binarios y fuentes de Microsoft BASIC se encuentran en:
  - `msbasic/` (submódulo o carpeta importada)
  - Archivos relevantes: `init.s`, `kbd_extra.s`, `zeropage.s`, etc.

---

## Ejecución rápida

1. Compila el proyecto:
   ```bash
   mkdir build && cd build
   cmake ..
   make -j$(nproc)
   ```
2. Ejecuta el demo de traza BASIC:
   ```bash
   ./basic_trace_demo
   ```
3. Observa la salida de la ejecución, estadísticas y traza de instrucciones.

---

## Demo de traza BASIC

El archivo `examples/basic_trace_demo.cpp` muestra cómo cargar y ejecutar Microsoft BASIC en el emulador, con traza de instrucciones y detección de eventos clave (como el prompt READY).

**Fragmento de uso:**
```cpp
#include "cpu.hpp"
#include "mem.hpp"

Mem mem;
CPU cpu;
// Cargar binario BASIC en memoria (ver ejemplo)
// ...
cpu.Reset(mem);
cpu.Execute(100000, mem); // Ejecutar hasta el prompt o límite
```

El demo imprime estadísticas de ejecución, estado final de la CPU y muestra la salida generada por BASIC.

---

## Opciones de integración

- Puedes cargar otros binarios BASIC modificando el demo o usando la API de memoria.
- Es posible conectar dispositivos virtuales (teclado, pantalla) para interacción completa.
- La integración con el depurador permite inspeccionar el estado de la CPU durante la ejecución de BASIC.

---

## Notas técnicas

- El intérprete BASIC requiere una inicialización de memoria y vectores adecuada (ver ejemplos).
- El emulador soporta instrucciones y timings requeridos por BASIC, incluyendo extensiones 65C02 si se desea.
- Se recomienda consultar la documentación de msbasic para detalles de mapeo de memoria y personalización.

---

## Recursos adicionales

- [examples/basic_trace_demo.cpp](examples/basic_trace_demo.cpp): Demo de traza y ejecución
- [msbasic/](msbasic/): Binarios y fuentes de Microsoft BASIC
- [WOZMON_README.md](WOZMON_README.md): Integración Apple 1 Monitor
- [README.md](README.md): Documentación principal del emulador

---

**¿Dudas o sugerencias?** Abre un issue o consulta la documentación avanzada.
