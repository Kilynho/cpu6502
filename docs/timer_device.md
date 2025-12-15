# TimerDevice - Documentación del Dispositivo de Temporización

## Descripción General

El `TimerDevice` es una interfaz que define las operaciones básicas para dispositivos de temporización en el emulador 6502. La implementación `BasicTimer` proporciona un temporizador programable que cuenta ciclos de CPU y puede generar interrupciones IRQ periódicas, similar a los chips temporizadores de las computadoras clásicas de 8 bits (VIA 6522, CIA 6526, PIA 6821, etc.).

## Características

- **Contador de 32 bits**: Capacidad para contar hasta 4,294,967,295 ciclos
- **Límite configurable**: Define cuándo generar IRQ o detener el contador
- **Generación de IRQ**: Interrupciones periódicas cuando el contador alcanza el límite
- **Modo auto-reload**: Reinicio automático del contador para temporizadores periódicos
- **Registros mapeados en memoria**: Acceso directo desde código 6502
- **API C++**: Control programático desde el código del emulador
- **Thread-safe**: Operaciones seguras para uso multi-hilo

## Arquitectura

### Interfaz TimerDevice

```cpp
class TimerDevice : public IODevice {
public:
    virtual bool initialize() = 0;
    virtual uint32_t getCounter() const = 0;
    virtual void setCounter(uint32_t value) = 0;
    virtual void reset() = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual bool hasIRQ() const = 0;
    virtual void clearIRQ() = 0;
    virtual void tick(uint32_t cycles) = 0;
    virtual void cleanup() = 0;
};
```

### Implementación BasicTimer

`BasicTimer` implementa la interfaz `TimerDevice` con un contador de 32 bits que se incrementa con cada ciclo de CPU ejecutado.

## Mapa de Memoria

Los registros del timer están mapeados en las siguientes direcciones:

| Dirección | Nombre | Descripción |
|-----------|--------|-------------|
| `$FC00` | COUNTER_LOW | Byte bajo del contador (bits 0-7) |
| `$FC01` | COUNTER_MID1 | Byte medio 1 del contador (bits 8-15) |
| `$FC02` | COUNTER_MID2 | Byte medio 2 del contador (bits 16-23) |
| `$FC03` | COUNTER_HIGH | Byte alto del contador (bits 24-31) |
| `$FC04` | LIMIT_LOW | Byte bajo del límite (bits 0-7) |
| `$FC05` | LIMIT_MID1 | Byte medio 1 del límite (bits 8-15) |
| `$FC06` | LIMIT_MID2 | Byte medio 2 del límite (bits 16-23) |
| `$FC07` | LIMIT_HIGH | Byte alto del límite (bits 24-31) |
| `$FC08` | CONTROL | Control y configuración |
| `$FC09` | STATUS | Estado (solo lectura) |

### Registro de Control ($FC08)

| Bit | Nombre | Descripción |
|-----|--------|-------------|
| 0 | ENABLE | 1=Timer habilitado, 0=Deshabilitado |
| 1 | IRQ_ENABLE | 1=Generar IRQ al alcanzar límite, 0=No generar |
| 2 | IRQ_FLAG | Escritura: 1=Limpiar IRQ pendiente |
| 3 | RESET | Escritura: 1=Reiniciar contador a 0 |
| 4 | AUTO_RELOAD | 1=Reiniciar automáticamente, 0=Detener al límite |
| 5-7 | - | Reservado |

### Registro de Status ($FC09)

| Bit | Nombre | Descripción |
|-----|--------|-------------|
| 0 | ENABLED | 1=Timer habilitado, 0=Deshabilitado |
| 1 | IRQ_PENDING | 1=IRQ pendiente, 0=Sin IRQ |
| 2 | LIMIT_REACHED | 1=Contador alcanzó el límite, 0=No alcanzado |
| 3-7 | - | Reservado |

## Uso desde Código 6502

### Ejemplo 1: Leer el Tiempo Transcurrido

```assembly
; Leer el contador actual (32 bits)
LDA $FC00       ; Byte bajo
STA TIME        
LDA $FC01       ; Byte medio 1
STA TIME+1      
LDA $FC02       ; Byte medio 2
STA TIME+2      
LDA $FC03       ; Byte alto
STA TIME+3      

; Ahora TIME contiene los ciclos transcurridos
```

### Ejemplo 2: Configurar IRQ Periódica

```assembly
; Configurar límite: 1,000,000 ciclos (0x000F4240)
LDA #$40        ; Byte bajo
STA $FC04       
LDA #$42        ; Byte medio 1
STA $FC05       
LDA #$0F        ; Byte medio 2
STA $FC06       
LDA #$00        ; Byte alto
STA $FC07       

; Habilitar timer con IRQ y auto-reload
LDA #$13        ; Enable | IRQ Enable | Auto-reload
STA $FC08       

; El timer ahora generará una IRQ cada 1,000,000 ciclos
```

### Ejemplo 3: Manejador de Interrupciones

```assembly
; Configurar vector de IRQ
; En 6502, el vector IRQ está en $FFFE/$FFFF
    .org $8000
    
INIT:
    ; Configurar límite para IRQ cada 100,000 ciclos
    LDA #$A0        ; 100000 & 0xFF
    STA $FC04
    LDA #$86        ; (100000 >> 8) & 0xFF
    STA $FC05
    LDA #$01        ; (100000 >> 16) & 0xFF
    STA $FC06
    LDA #$00        ; 100000 >> 24
    STA $FC07
    
    ; Habilitar timer con IRQ y auto-reload
    LDA #$13
    STA $FC08
    
    ; Habilitar interrupciones
    CLI             ; Clear Interrupt Disable
    
LOOP:
    ; Programa principal
    JMP LOOP

; Manejador de IRQ
IRQ_HANDLER:
    PHA             ; Guardar registros
    TXA
    PHA
    TYA
    PHA
    
    ; Limpiar IRQ del timer
    LDA #$04        ; IRQ_FLAG bit
    STA $FC08
    
    ; Incrementar contador de ticks
    INC TICK_COUNT
    
    ; Hacer algo cada tick...
    ; (tu código aquí)
    
    PLA             ; Restaurar registros
    TAY
    PLA
    TAX
    PLA
    RTI             ; Return from Interrupt

TICK_COUNT:
    .byte 0

    ; Configurar vector de IRQ
    .org $FFFE
    .word IRQ_HANDLER
```

### Ejemplo 4: Espera Temporizada (Delay)

```assembly
; Delay de aproximadamente 10,000 ciclos
DELAY_10K:
    ; Reiniciar y configurar timer
    LDA #$08        ; Reset bit
    STA $FC08
    
    ; Configurar límite a 10,000 (0x00002710)
    LDA #$10        ; Byte bajo
    STA $FC04
    LDA #$27        ; Byte medio 1
    STA $FC05
    LDA #$00        ; Byte medio 2
    STA $FC06
    LDA #$00        ; Byte alto
    STA $FC07
    
    ; Habilitar timer sin IRQ ni auto-reload
    LDA #$01        ; Solo Enable
    STA $FC08
    
WAIT_LOOP:
    ; Leer status
    LDA $FC09
    AND #$04        ; Verificar LIMIT_REACHED
    BEQ WAIT_LOOP   ; Si no se alcanzó, seguir esperando
    
    RTS
```

### Ejemplo 5: Medir Duración de Ejecución

```assembly
; Medir cuántos ciclos toma ejecutar una rutina
MEASURE_ROUTINE:
    ; Reiniciar timer
    LDA #$08
    STA $FC08
    
    ; Habilitar timer
    LDA #$01
    STA $FC08
    
    ; Ejecutar rutina a medir
    JSR MY_ROUTINE
    
    ; Deshabilitar timer
    LDA #$00
    STA $FC08
    
    ; Leer contador
    LDA $FC00
    STA CYCLES
    LDA $FC01
    STA CYCLES+1
    LDA $FC02
    STA CYCLES+2
    LDA $FC03
    STA CYCLES+3
    
    RTS

MY_ROUTINE:
    ; Rutina a medir
    NOP
    NOP
    NOP
    RTS

CYCLES:
    .word 0, 0      ; 32 bits para almacenar el resultado
```

## Uso desde C++

### Inicialización

```cpp
#include "devices/basic_timer.hpp"

// Crear dispositivo de timer
auto timer = std::make_shared<BasicTimer>();

// Inicializar
if (!timer->initialize()) {
    std::cerr << "Error: No se pudo inicializar el timer" << std::endl;
    return 1;
}

// Registrar con la CPU
cpu.registerIODevice(timer);
```

### Configurar Timer Periódico

```cpp
// Configurar límite a 1,000,000 ciclos
timer->setLimit(1000000);

// Habilitar timer con IRQ y auto-reload
timer->setEnabled(true);
timer->write(0xFC08, 0x13);  // Enable | IRQ Enable | Auto-reload

// En el loop principal
while (running) {
    // Ejecutar CPU
    cpu.Execute(1000, mem);
    
    // Actualizar timer
    timer->tick(1000);
    
    // Verificar si hay IRQ
    if (timer->hasIRQ()) {
        // Manejar IRQ
        handleTimerInterrupt();
        
        // Limpiar IRQ
        timer->clearIRQ();
    }
}
```

### Leer Tiempo Transcurrido

```cpp
// Obtener valor actual del contador
uint32_t cycles = timer->getCounter();
std::cout << "Ciclos transcurridos: " << cycles << std::endl;
```

### Reiniciar Timer

```cpp
// Reiniciar contador a cero
timer->reset();

// O usando el registro de control
timer->write(0xFC08, 0x08);  // RESET bit
```

### Limpiar

```cpp
// Antes de salir del programa
timer->cleanup();
cpu.unregisterIODevice(timer);
```

## Integración con el Sistema de Interrupciones

El `BasicTimer` puede generar interrupciones IRQ que la CPU debe manejar. Aquí hay un ejemplo completo:

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "devices/basic_timer.hpp"

int main() {
    Mem mem;
    CPU cpu;
    
    // Inicializar memoria
    mem.Initialize();
    cpu.Reset(mem);
    
    // Crear y configurar timer
    auto timer = std::make_shared<BasicTimer>();
    timer->initialize();
    cpu.registerIODevice(timer);
    
    // Configurar límite a 50,000 ciclos (aproximadamente 50ms a 1 MHz)
    timer->setLimit(50000);
    
    // Habilitar con IRQ y auto-reload
    timer->write(0xFC08, 0x13);
    
    // Cargar programa que maneja IRQ
    // ... (cargar código 6502 con manejador de IRQ)
    
    // Loop principal
    bool running = true;
    while (running) {
        // Ejecutar 1000 ciclos de CPU
        cpu.Execute(1000, mem);
        
        // Actualizar timer
        timer->tick(1000);
        
        // Verificar IRQ
        if (timer->hasIRQ() && !cpu.I) {  // I = Interrupt Disable flag
            // Generar IRQ en la CPU
            // 1. Guardar PC y flags en pila
            cpu.PushPCToStack(cycles, mem);
            cpu.WriteByte(cycles, cpu.SPToAddress(), 
                         (cpu.N << 7) | (cpu.V << 6) | (cpu.B << 4) | 
                         (cpu.D << 3) | (cpu.I << 2) | (cpu.Z << 1) | cpu.C,
                         mem);
            cpu.SP--;
            
            // 2. Establecer I flag
            cpu.I = 1;
            
            // 3. Saltar a vector IRQ
            cpu.PC = mem.ReadWord(Mem::IRQ_VECTOR);
            
            // Limpiar IRQ del timer
            timer->clearIRQ();
        }
    }
    
    timer->cleanup();
    return 0;
}
```

## Casos de Uso

### 1. Reloj en Tiempo Real (RTC)

```cpp
// Configurar timer para incrementar cada segundo a 1 MHz
timer->setLimit(1000000);  // 1 millón de ciclos = 1 segundo a 1 MHz
timer->write(0xFC08, 0x13); // Auto-reload con IRQ

// En el manejador de IRQ, incrementar segundos
uint32_t seconds = 0;
if (timer->hasIRQ()) {
    seconds++;
    timer->clearIRQ();
}
```

### 2. Frame Timing para Juegos

```cpp
// 60 FPS = 16.67ms por frame
// A 1 MHz, 16.67ms = ~16,670 ciclos
timer->setLimit(16670);
timer->write(0xFC08, 0x13);

// En el loop del juego
if (timer->hasIRQ()) {
    updateGame();
    renderFrame();
    timer->clearIRQ();
}
```

### 3. Profiling de Código

```cpp
// Medir rendimiento de una función
timer->reset();
timer->setEnabled(true);

// Ejecutar código a medir
functionToProfile();

// Leer ciclos
uint32_t cycles = timer->getCounter();
std::cout << "Función tomó " << cycles << " ciclos" << std::endl;
```

### 4. Generación de Eventos Periódicos

```cpp
// Actualizar sensores cada 100,000 ciclos
timer->setLimit(100000);
timer->write(0xFC08, 0x13);

if (timer->hasIRQ()) {
    readSensors();
    updateDisplay();
    timer->clearIRQ();
}
```

## Limitaciones

- **Precisión**: El timer cuenta ciclos de CPU, no tiempo real
- **Resolución**: Un ciclo de CPU (a 1 MHz = 1 microsegundo)
- **Rango máximo**: 4,294,967,295 ciclos (~71.6 minutos a 1 MHz)
- **IRQ manual**: El emulador debe llamar a `tick()` y manejar IRQ manualmente

## Consideraciones de Rendimiento

- El timer usa operaciones atómicas para thread-safety
- Las operaciones de lectura/escritura son O(1)
- El método `tick()` es muy eficiente y puede llamarse frecuentemente
- El mutex solo se bloquea durante operaciones críticas

## Ejemplos de Temporización

A una velocidad de CPU de 1 MHz (1,000,000 ciclos/segundo):

| Límite | Tiempo Aproximado |
|--------|-------------------|
| 1,000 | 1 ms |
| 10,000 | 10 ms |
| 16,667 | ~16.67 ms (60 FPS) |
| 20,000 | 20 ms (50 Hz) |
| 50,000 | 50 ms |
| 100,000 | 100 ms |
| 1,000,000 | 1 segundo |
| 60,000,000 | 1 minuto |

## Pruebas

El timer incluye 14 tests exhaustivos que verifican:

1. Inicialización correcta
2. Lectura/escritura de registros de 32 bits
3. Habilitación/deshabilitación
4. Conteo de ciclos
5. Generación de IRQ
6. Limpieza de IRQ
7. Modo auto-reload
8. Detención al alcanzar límite
9. Reset del contador
10. Lectura desde CPU
11. IRQ periódica
12. Bits de control
13. Comportamiento sin IRQ habilitado

Ejecuta las pruebas con:

```bash
cd build
./runTests --gtest_filter="BasicTimerTest.*"
```

## Integración con Otros Dispositivos

El timer puede coordinarse con otros dispositivos:

```cpp
// Sincronizar audio con timer
if (timer->getCounter() % 44100 == 0) {  // Cada segundo a 44.1 kHz
    audio->playTone(440, 100, 128);
}

// Actualizar pantalla en intervalos regulares
if (timer->hasIRQ()) {
    textScreen->refresh();
    timer->clearIRQ();
}
```

## Referencia de API

### BasicTimer::initialize()
Inicializa el timer y todos sus registros.
- **Retorna**: `true` si se inicializó correctamente

### BasicTimer::tick(cycles)
Incrementa el contador y verifica límites.
- **cycles**: Número de ciclos a añadir al contador

### BasicTimer::getCounter()
Obtiene el valor actual del contador.
- **Retorna**: Valor del contador (0-4294967295)

### BasicTimer::setCounter(value)
Establece el valor del contador.
- **value**: Nuevo valor del contador

### BasicTimer::getLimit()
Obtiene el límite configurado.
- **Retorna**: Valor del límite

### BasicTimer::setLimit(value)
Establece el límite para IRQ.
- **value**: Nuevo valor del límite

### BasicTimer::reset()
Reinicia el contador a 0 y limpia IRQ.

### BasicTimer::isEnabled()
Verifica si el timer está habilitado.
- **Retorna**: `true` si está habilitado

### BasicTimer::setEnabled(enabled)
Habilita o deshabilita el timer.
- **enabled**: `true` para habilitar, `false` para deshabilitar

### BasicTimer::hasIRQ()
Verifica si hay IRQ pendiente.
- **Retorna**: `true` si hay IRQ pendiente y habilitada

### BasicTimer::clearIRQ()
Limpia la bandera de IRQ.

### BasicTimer::isIRQEnabled()
Verifica si la generación de IRQ está habilitada.
- **Retorna**: `true` si IRQ está habilitada

### BasicTimer::isAutoReload()
Verifica si el modo auto-reload está habilitado.
- **Retorna**: `true` si auto-reload está habilitado

### BasicTimer::cleanup()
Libera recursos y deshabilita el timer.

## Ver También

- [Audio Device Documentation](audio_device.md)
- [Serial Device Documentation](serial_device.md)
- [Architecture Documentation](architecture.md)
- [File Device Documentation](file_device.md)
