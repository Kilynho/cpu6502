# Sistema de Interrupciones (IRQ/NMI)

## Descripción General

El sistema de interrupciones permite a los dispositivos periféricos solicitar la atención de la CPU a través de señales de interrupción. El emulador 6502 soporta dos tipos de interrupciones:

- **IRQ (Interrupt Request)**: Interrupción estándar enmascarable
- **NMI (Non-Maskable Interrupt)**: Interrupción no enmascarable de alta prioridad

## Arquitectura

### Componentes Principales

1. **InterruptController**: Controlador centralizado que gestiona todas las fuentes de interrupción
2. **InterruptSource**: Interfaz que deben implementar los dispositivos que generan interrupciones
3. **CPU**: Hooks para verificar y manejar interrupciones

### Flujo de Interrupciones

```
Dispositivo → InterruptSource → InterruptController → CPU → Vector de Interrupción
```

## Uso Básico

### 1. Crear un Dispositivo con Interrupciones

Los dispositivos deben implementar la interfaz `InterruptSource`:

```cpp
#include "interrupt_controller.hpp"

class MiDispositivo : public IODevice, public InterruptSource {
public:
    bool hasIRQ() const override {
        return irqPending;
    }
    
    bool hasNMI() const override {
        return nmiPending;
    }
    
    void clearIRQ() override {
        irqPending = false;
    }
    
    void clearNMI() override {
        nmiPending = false;
    }
    
    // Método para disparar una IRQ
    void triggerInterrupt() {
        irqPending = true;
    }
    
private:
    bool irqPending = false;
    bool nmiPending = false;
};
```

### 2. Configurar el Sistema de Interrupciones

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "interrupt_controller.hpp"
#include "devices/basic_timer.hpp"

int main() {
    // Inicializar componentes
    Mem memoria;
    CPU cpu;
    InterruptController intCtrl;
    
    memoria.Initialize();
    cpu.Reset(memoria);
    
    // Configurar vectores de interrupción en memoria
    memoria[Mem::IRQ_VECTOR] = 0x00;      // Vector IRQ: 0x8000
    memoria[Mem::IRQ_VECTOR + 1] = 0x80;
    
    memoria[Mem::NMI_VECTOR] = 0x00;      // Vector NMI: 0x9000
    memoria[Mem::NMI_VECTOR + 1] = 0x90;
    
    // Conectar el controlador de interrupciones a la CPU
    cpu.setInterruptController(&intCtrl);
    
    // Crear y registrar un dispositivo con interrupciones
    auto timer = std::make_shared<BasicTimer>();
    timer->initialize();
    
    cpu.registerIODevice(timer);
    intCtrl.registerSource(timer);
    
    // Configurar el timer para generar IRQ cada 1000 ciclos
    timer->setLimit(1000);
    timer->write(0xFC08, 0x03);  // Enable | IRQ Enable
    
    return 0;
}
```

### 3. Manejo de Interrupciones en el Loop Principal

```cpp
// Loop principal de emulación
while (running) {
    // Actualizar dispositivos (timers, etc.)
    timer->tick(cycles);
    
    // Verificar y manejar interrupciones
    cpu.checkAndHandleInterrupts(memoria);
    
    // Ejecutar instrucciones de la CPU
    cpu.Execute(cycles, memoria);
}
```

## Ejemplo Completo: Timer con IRQ

```cpp
#include <iostream>
#include "cpu.hpp"
#include "mem.hpp"
#include "interrupt_controller.hpp"
#include "devices/basic_timer.hpp"

int main() {
    Mem mem;
    CPU cpu;
    InterruptController intCtrl;
    
    mem.Initialize();
    cpu.Reset(mem);
    
    // Configurar vector de IRQ apuntando a 0x8000
    mem[Mem::IRQ_VECTOR] = 0x00;
    mem[Mem::IRQ_VECTOR + 1] = 0x80;
    
    // Rutina de manejo de interrupción en 0x8000
    // RTI (Return from Interrupt)
    mem[0x8000] = 0x40;  // Opcode RTI
    
    // Conectar sistema de interrupciones
    cpu.setInterruptController(&intCtrl);
    
    // Crear y configurar timer
    auto timer = std::make_shared<BasicTimer>();
    timer->initialize();
    cpu.registerIODevice(timer);
    intCtrl.registerSource(timer);
    
    // Configurar timer: disparar IRQ cada 100 ciclos
    timer->setLimit(100);
    timer->write(0xFC08, 0x03);  // Enable | IRQ Enable
    
    std::cout << "Timer configurado para disparar IRQ cada 100 ciclos\n";
    
    // Simular 250 ciclos
    for (int i = 0; i < 5; i++) {
        std::cout << "\n--- Ciclo " << (i * 50) << " ---\n";
        
        // Ejecutar timer
        timer->tick(50);
        
        // Verificar y manejar interrupciones
        if (intCtrl.hasIRQ()) {
            std::cout << "¡IRQ detectada!\n";
            std::cout << "PC antes: 0x" << std::hex << cpu.PC << "\n";
            cpu.checkAndHandleInterrupts(mem);
            std::cout << "PC después: 0x" << std::hex << cpu.PC << "\n";
        }
    }
    
    // Limpiar
    cpu.unregisterIODevice(timer);
    timer->cleanup();
    
    return 0;
}
```

## Comportamiento de Interrupciones

### IRQ (Interrupt Request)

- **Enmascarable**: Puede ser deshabilitada estableciendo el flag I del CPU
- **Prioridad**: Menor que NMI
- **Vector**: 0xFFFE (dirección baja), 0xFFFF (dirección alta)
- **Proceso**:
  1. Si I=0 (interrupciones habilitadas) y hay IRQ pendiente
  2. Guardar PC (alto, luego bajo) en la pila
  3. Guardar registro de estado P en la pila
  4. Establecer I=1 (deshabilitar interrupciones)
  5. Cargar PC desde el vector de IRQ

### NMI (Non-Maskable Interrupt)

- **No enmascarable**: No puede ser deshabilitada por el flag I
- **Prioridad**: Mayor que IRQ
- **Vector**: 0xFFFA (dirección baja), 0xFFFB (dirección alta)
- **Proceso**: Similar a IRQ pero no puede ser bloqueada

### Prioridades

Cuando hay múltiples interrupciones pendientes:

1. **NMI** (prioridad más alta)
2. **IRQ** (si I=0)

## Referencia de API

### InterruptController

```cpp
class InterruptController {
public:
    // Registrar/eliminar fuentes de interrupción
    void registerSource(std::shared_ptr<InterruptSource> source);
    void unregisterSource(std::shared_ptr<InterruptSource> source);
    
    // Verificar interrupciones pendientes
    bool hasIRQ() const;
    bool hasNMI() const;
    
    // Reconocer interrupciones
    void acknowledgeIRQ();
    void acknowledgeNMI();
    void clearAll();
    
    // Información
    size_t getSourceCount() const;
};
```

### InterruptSource (Interfaz)

```cpp
class InterruptSource {
public:
    virtual bool hasIRQ() const = 0;
    virtual bool hasNMI() const = 0;
    virtual void clearIRQ() = 0;
    virtual void clearNMI() = 0;
};
```

### CPU

```cpp
class CPU {
public:
    // Configurar controlador de interrupciones
    void setInterruptController(InterruptController* controller);
    InterruptController* getInterruptController() const;
    
    // Manejar interrupciones
    void serviceIRQ(Mem& memory);
    void serviceNMI(Mem& memory);
    void checkAndHandleInterrupts(Mem& memory);
};
```

## Notas Importantes

1. **Vectores de Memoria**: Los vectores de interrupción deben estar configurados en memoria antes de que ocurran interrupciones.

2. **Flag I**: El flag I solo afecta a IRQ, no a NMI.

3. **Reconocimiento**: El controlador limpia automáticamente las interrupciones después de que la CPU las atiende.

4. **Múltiples Fuentes**: El controlador soporta múltiples dispositivos generando interrupciones simultáneamente.

5. **Thread Safety**: BasicTimer usa mutexes para operaciones thread-safe.

## Depuración

Para depurar problemas con interrupciones:

1. Verificar que los vectores de interrupción estén configurados correctamente
2. Comprobar el estado del flag I de la CPU
3. Verificar que el dispositivo está registrado en el InterruptController
4. Asegurarse de que `checkAndHandleInterrupts()` se llama regularmente

## Ejemplo de Código Assembly 6502

Rutina de manejo de IRQ típica:

```assembly
; Vector de IRQ apunta aquí (e.g., 0x8000)
IRQ_HANDLER:
    PHA           ; Guardar A en la pila
    TXA
    PHA           ; Guardar X en la pila
    TYA
    PHA           ; Guardar Y en la pila
    
    ; --- Manejar la interrupción ---
    LDA #$04      ; Limpiar IRQ del timer
    STA $FC08     ; Escribir al registro de control
    
    ; ... código de manejo específico ...
    
    ; --- Restaurar y retornar ---
    PLA
    TAY           ; Restaurar Y
    PLA
    TAX           ; Restaurar X
    PLA           ; Restaurar A
    RTI           ; Retornar de la interrupción
```

## Referencias

- [6502 Reference](http://www.6502.org/tutorials/interrupts.html)
- Documentación de BasicTimer: `include/devices/basic_timer.hpp`
- Tests de ejemplo: `tests/test_interrupt_controller.cpp`
