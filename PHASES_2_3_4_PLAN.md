# Plan de Fases 2-4: Validación Timing, Memoria y BASIC READY

**Rama:** `feature/wozmon-integration`  
**Estado:** PLANIFICACIÓN PARA FASE 2  

---

## 🎯 FASE 2: Validación Completa de Timing 65C02

### Objetivos
1. Auditar ciclos de TODAS las instrucciones implementadas
2. Validar penalizaciones por cruce de página
3. Corregir discrepancias con referencia WDC 65C02

### Tareas Detalladas

#### 2.1 Crear Test de Timing Exhaustivo
- **Archivo:** `tests/timing_validation_test.cpp`
- **Scope:**
  - Para cada opcode implementado, verificar ciclos base
  - Validar penalizaciones (+1 ciclo en cruce de página para modos con índice)
  - Spot-check de instrucciones críticas

#### 2.2 Auditar Ciclos por Familia
- **Load/Store:** Verify all variants (imm, zp, zpx, abs, absx, absy, indx, indy, zp_ind)
- **Arithmetic (ADC/SBC):** 2 (imm), 3-6 (otros modos)
- **Logical (AND/EOR/ORA):** Similar a ADC
- **Branches:** 2 (no taken), 3+ (taken, +1 página)
- **Stack (PHA/PLA):** 3/4 en 65C02
- **Stack 65C02 (PHX/PLX/PHY/PLY):** 3/4
- **Shift/Rotate:** 2 (acc), 5-7 (mem variants)
- **Inc/Dec:** 2 (imm/reg), 5-7 (mem variants)
- **BIT:** Nuevas variantes (imm:2, zpx:4, absx:4)
- **STZ:** zp:3, zpx:4, abs:4, absx:5
- **TSB/TRB:** zp:5, abs:6
- **RMB/SMB/BBR/BBS:** 5 ciclos (todas)
- **WAI/STP:** 3 ciclos

#### 2.3 Resolver Discrepancias
- Comparar contra: http://6502.org/tutorials/6502opcodes.html (6502) y WDC 65C02 datasheet
- Actualizar `instruction_set.hpp` con valores correctos
- Documentar excepciones

### Deliverables
- [ ] `tests/timing_validation_test.cpp` (100+ tests)
- [ ] Actualización de ciclos en `instruction_set.hpp`
- [ ] Reporte de auditoría de timing

---

## 🎯 FASE 3: Auditoría Completa del Mapa de Memoria

### Objetivos
1. Verificar que cada segmento de memoria esté correctamente mapeado
2. Validar vectores de interrupción (RESET, NMI, IRQ)
3. Confirmar carga de ROM, BIOS, WOZMON
4. Verificar PIA y dispositivos periféricos

### Estructura de Memoria Esperada

```
0x0000-0x00FF   Zero Page (256 bytes)
                ├─ Usado por: Variables, flags, punteros
                └─ Critical para manejo de stack y operaciones

0x0100-0x01FF   Stack (256 bytes)
                ├─ Crecimiento descendente desde 0x01FF
                ├─ Controlado por SP (Stack Pointer)
                └─ Usado por: PHA/PLA, JSR/RTS, interrupciones

0x0200-0x7FFF   RAM General (31.75 KB)
                ├─ Espacio para variables globales
                ├─ Buffers de entrada/salida
                └─ Código dinámico

0x8000-0xBFFF   ROM/BIOS (16 KB)
                ├─ MS BASIC cargado aquí
                ├─ Punto de entrada: 0x9F06 (BASIC coldstart)
                └─ Offset: 0x1F06 en el archivo ROM

0xC000-0xCFFF   Expansión (4 KB) - No usado típicamente

0xD000-0xD0FF   Periféricos (256 bytes)
                ├─ 0xD010-0xD013: PIA 6821 (Keyboard/Display)
                ├─ 0xD000-0xD007: Otros dispositivos potenciales
                └─ Controlado por IODevice mapping

0xD100-0xDFFF   Expansion (3.75 KB)

0xE000-0xEFFF   Expansion (4 KB)

0xF000-0xFFFF   WOZMON / Vectores (4 KB)
                ├─ 0xF000-0xF1FF: WOZMON Monitor (512 bytes)
                ├─ 0xF200-0xFFFA: Espacio libre
                └─ Vectores de Interrupción (last 6 bytes):
                    ├─ 0xFFFA-0xFFFB: NMI (Non-Maskable Interrupt)
                    ├─ 0xFFFC-0xFFFD: RESET Vector → 0x8000 (BASIC)
                    └─ 0xFFFE-0xFFFF: IRQ (Interrupt Request)
```

### Tareas Detalladas

#### 3.1 Crear Test de Mapa de Memoria
- **Archivo:** `tests/memory_map_test.cpp`
- **Validaciones:**
  - [ ] Zero Page accesible (0x00-0xFF)
  - [ ] Stack funciona correctamente (0x0100-0x01FF)
  - [ ] RAM general escribible (0x0200-0x7FFF)
  - [ ] ROM no escribible (0x8000-0xBFFF)
  - [ ] WOZMON presente (0xF000-0xFFFF)
  - [ ] Vectores correctamente establecidos

#### 3.2 Auditar Carga de ROM
- **Ubicación:** `src/mem/mem.cpp` (Memory initialization)
- **Verificar:**
  - [ ] BASIC cargado en 0x8000
  - [ ] WOZMON cargado en 0xF000
  - [ ] Reset vector en 0xFFFC → 0x8000
  - [ ] IRQ/NMI vectores correctamente inicializados

#### 3.3 Auditar PIA 6821
- **Ubicación:** `src/devices/pia_device.*`
- **Direcciones:**
  - 0xD010: Control Register A (CRA)
  - 0xD011: Data Port B (DRB)  
  - 0xD012: Control Register B (CRB)
  - 0xD013: Data Port A (DRA) - Keyboard
- **Verificar:**
  - [ ] Lectura de teclado (0xD013)
  - [ ] Escritura a pantalla (0xD012)
  - [ ] Handshake correcto

### Deliverables
- [ ] `tests/memory_map_test.cpp` (15+ tests)
- [ ] Reporte de auditoría de memoria
- [ ] Documentación de asignación de memoria

---

## 🎯 FASE 4: Ejecución BASIC → READY

### Objetivos
1. Trazar ejecución desde ciclo inicial hasta comando READY
2. Identificar y corregir bloqueos/loops infinitos
3. Asegurar entrada/salida funciona correctamente

### Estrategia de Tracing

#### 4.1 Instrumentación de Ejecución
- **Agregar a `src/cpu/cpu.cpp`:**
  ```cpp
  void Execute(Mem& memory) {
      u32 instructionCount = 0;
      const u32 TRACE_INTERVAL = 1000;  // Log cada 1000 instrucciones
      
      while (!halted && instructionCount < MAX_INSTRUCTIONS) {
          if (instructionCount % TRACE_INTERVAL == 0) {
              LogTrace(instructionCount, PC, A, X, Y, SP);
          }
          // ... existing execute logic
      }
  }
  ```

#### 4.2 Puntos de Tracing Críticos
- **Inicio:** PC = 0x8000 (BASIC entry)
- **Coldstart:** PC = 0x9F06 (BASIC initialization)
- **Main loop:** PC around 0xA000-0xA100 (BASIC interpreter)
- **IO calls:** Detectar lectura de 0xD013 (keyboard), escritura a 0xD012 (display)
- **Stack operations:** Monitorear SP para detectar stack corruption

#### 4.3 Búsqueda de Stalls
- **Patrón:** Mismo PC repetido 1000+ veces
- **Solución:** 
  - Agregar breakpoint cuando PC no cambia
  - Imprimir instrucción y estado actual
  - Identificar loop infinito o opcode faltante

### Tareas Detalladas

#### 4.4 Crear Harness de Tracing
- **Archivo:** `examples/basic_trace_demo.cpp`
- **Funcionalidad:**
  - Cargar BASIC + WOZMON
  - Ejecutar con tracing detallado
  - Capturar primeros 10000 ciclos de instrucción
  - Mostrar PC, opcode, estado cada 100 ciclos

#### 4.5 Validar READY Prompt
- **Señal esperada:**
  ```
  Memory Size?<ESC> <- System pregunta tamaño RAM
  Terminal Width?<ESC> <- System pregunta ancho terminal
  <CR>
  READY <- Ready prompt
  ```
- **Implementar test:**
  ```cpp
  TEST(BASICIntegration, ColdStartToReady) {
      // Cargar BASIC
      // Ejecutar hasta que output contenga "READY"
      // Timeout: 1,000,000 ciclos
      // EXPECT que terminal output sea correcto
  }
  ```

#### 4.6 Debugging Interactivo
- **Si BASIC no alcanza READY:**
  1. Ejecutar `basic_trace_demo` y capturar output
  2. Identificar último PC válido
  3. Revisar instrucción en ese PC
  4. Verificar:
     - Opcode está implementado
     - Ciclos son correctos
     - Flags se actualizan adecuadamente
  5. Revisar llamadas a IO (PIA)

### Deliverables
- [ ] `examples/basic_trace_demo.cpp` (demo ejecutable)
- [ ] `tests/basic_integration_test.cpp` (test READY)
- [ ] Reporte de tracing y stalls resueltos

---

## 🔗 Dependencias Entre Fases

```
Fase 1: ✅ COMPLETADA
   └─ Tabla de metadatos + 8 tests
      │
      ├─→ Fase 2: Timing Validation
      │   └─ Auditar ciclos, crear tests
      │      │
      │      ├─→ Fase 3: Memory Audit
      │      │   └─ Mapa de memoria + vectores
      │      │      │
      │      │      └─→ Fase 4: BASIC READY
      │      │          └─ Tracing y ejecución
      │      │
      │      └─→ [Iterativo] Si timing incorrecto
      │          └─ Volver a auditar instructions.cpp
      │
      └─→ [Si error en tests]
          └─ Debugger para investigar

Final: BASIC runs → "READY" prompt ✅
```

---

## 📋 Checklist de Completitud

### Fase 1
- [x] Tabla de metadatos (256 opcodes)
- [x] 8 tests de cobertura
- [x] Documentación de cambios
- [x] Compilación exitosa

### Fase 2 (TODO)
- [ ] Test de timing (100+ casos)
- [ ] Auditoría de ciclos vs. referencia
- [ ] Correcciones si necesario
- [ ] Documentación de timing

### Fase 3 (TODO)
- [ ] Test de mapa de memoria (15+ casos)
- [ ] Auditoría de ROM loading
- [ ] Auditoría de PIA
- [ ] Vectores de interrupción validados

### Fase 4 (TODO)
- [ ] Demo de tracing
- [ ] Test de BASIC coldstart
- [ ] Test de READY prompt
- [ ] Debugging iterativo si sea necesario

---

## 📝 Notas y Sugerencias

### Para Debugging Rápido
1. Crear versión corta de BASIC (solo coldstart) para testing
2. Usar breakpoints en PIA 0xD010-0xD013
3. Monitorear stack pointer para detectar corrupción
4. Logging granular de instrucciones en zona critica (0xA000-0xA100)

### Riesgos Identificados
- **Ciclos incorrectos:** Pueden causar timing issues en BASIC
- **PIA no mapeado:** Keyboard/display no funcionan
- **Vectores incorrectos:** Interrupciones saltan a lugares errados
- **Stack corruption:** JSR/RTS pueden fallar silenciosamente

### Optimizaciones Futuras
- Cache de handlers para instrucciones frecuentes
- Ejecución JIT para secuencias largas
- Profiling para identificar instrucciones que consumen más ciclos

---

**Estado:** LISTO PARA INICIAR FASE 2  
**Próximo:** Timing validation suite
