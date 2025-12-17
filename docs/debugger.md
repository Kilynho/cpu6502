# Depurador Avanzado

Este proyecto ahora incluye un depurador básico con breakpoints, watchpoints, trazas de instrucciones y utilidades de inspección/modificación de estado.

## Características
- Breakpoints: pausa la ejecución cuando `PC` alcanza una dirección marcada.
- Watchpoints: registra y dispara evento al acceder/escribir una dirección de memoria.
- Trazas: captura `PC` y `opcode` de cada instrucción ejecutada.
- Inspección: lee estado de la CPU (`PC`, `SP`, `A`, `X`, `Y`, flags).
- Memoria: lectura y escritura directa de memoria.

## API
Clase `Debugger`:
- `attach(CPU* cpu, Mem* mem)`: asocia el depurador a CPU y Mem.
- `addBreakpoint(uint16_t addr)`, `removeBreakpoint(addr)`, `clearBreakpoints()`.
- `addWatchpoint(uint16_t addr)`, `removeWatchpoint(addr)`, `clearWatchpoints()`.
- `shouldBreak(uint16_t pc)`: comprobación interna usada por la CPU.
- `notifyBreakpoint(uint16_t pc)`: marcado de último breakpoint alcanzado.
- `traceInstruction(uint16_t pc, uint8_t opcode)`: añade evento de traza.
- `notifyMemoryAccess(uint16_t addr, uint8_t val, bool isWrite)`: evento de memoria.
- `inspectCPU() -> CpuState`: devuelve estado de registros y flags.
- `readMemory(addr)`, `writeMemory(addr, val)`: acceso directo a memoria.
- `hitBreakpoint()`, `lastBreakpoint()`: consulta estado del último disparo.

CPU:
- `setDebugger(Debugger*)`: activa integración de depuración.

## Uso Rápido
```cpp
Mem mem;
CPU cpu;
cpu.Reset(mem);

Debugger dbg;
dbg.attach(&cpu, &mem);
cpu.setDebugger(&dbg);

// Breakpoint en 0x8003
dbg.addBreakpoint(0x8003);

// Watchpoint en zp $00
dbg.addWatchpoint(0x00);

// Ejecutar
cpu.Execute(100, mem);

if (dbg.hitBreakpoint()) {
    auto st = dbg.inspectCPU();
    // Consultar st.pc, st.a, etc.
}
```

## Integración con Scripting
La `ScriptingAPI` expone `on_breakpoint` y `on_io`. Esta integración permanece disponible y puede conectarse al depurador para reenviar eventos si se desea (pendiente de diseño de acoplamiento opcional).

## Pruebas
Se añadieron pruebas en `tests/test_debugger.cpp` que validan:
- Parada por breakpoint de instrucción.
- Disparo por watchpoint de escritura.
- Inspección de estado de CPU.

## Construcción
No requiere dependencias adicionales. El archivo `src/debugger.cpp` se incorpora a la librería en `src/CMakeLists.txt`.

## Limitaciones y Próximos Pasos
- El depurador es no intrusivo y minimalista; no incluye CLI interactiva aún.
- Posible extensión: comandos de depuración en GUI/CLI, integración completa con `ScriptingAPI`.
