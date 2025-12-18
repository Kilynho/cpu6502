/**
 * @file interrupt_demo.cpp
 * @brief Demonstration of the IRQ/NMI interrupt system
 *
 * This example shows how to set up and use the interrupt system
 * with a timer that triggers IRQs periodically.
 */

#include <iostream>
#include <iomanip>
#include "cpu.hpp"
#include "mem.hpp"
#include "interrupt_controller.hpp"
#include "devices/basic_timer.hpp"

void printCPUState(const CPU& cpu, const std::string& context) {
    std::cout << "  [" << context << "] PC: 0x" 
              << std::hex << std::setw(4) << std::setfill('0') << cpu.PC
              << ", SP: 0x" << std::setw(2) << (int)cpu.SP
              << ", I: " << (int)cpu.I
              << std::dec << "\n";
}

int main() {
    std::cout << "=== Demostración del Sistema de Interrupciones ===" << std::endl;
    std::cout << std::endl;
    
    // Inicializar componentes
    Mem mem;
    CPU cpu;
    InterruptController intCtrl;
    
    mem.Initialize();
    cpu.Reset(mem);
    
    std::cout << "1. Configurando vectores de interrupción..." << std::endl;
    
    // Configurar vector de IRQ apuntando a 0x8000
    mem[Mem::IRQ_VECTOR] = 0x00;
    mem[Mem::IRQ_VECTOR + 1] = 0x80;
    std::cout << "   - Vector IRQ: 0x8000" << std::endl;
    
    // Configurar vector de NMI apuntando a 0x9000
    mem[Mem::NMI_VECTOR] = 0x00;
    mem[Mem::NMI_VECTOR + 1] = 0x90;
    std::cout << "   - Vector NMI: 0x9000" << std::endl;
    std::cout << std::endl;
    
    // Rutina de manejo de interrupción simple (RTI - Return from Interrupt)
    // En un programa real, aquí habría código para manejar la interrupción
    mem[0x8000] = 0x40;  // RTI en el handler de IRQ
    mem[0x9000] = 0x40;  // RTI en el handler de NMI
    
    // Conectar el controlador de interrupciones a la CPU
    cpu.setInterruptController(&intCtrl);
    std::cout << "2. Controlador de interrupciones conectado a la CPU" << std::endl;
    std::cout << std::endl;
    
    // Crear y configurar timer
    auto timer = std::make_shared<BasicTimer>();
    timer->initialize();
    cpu.registerIODevice(timer);
    intCtrl.registerSource(timer);
    
    std::cout << "3. Timer registrado como fuente de interrupciones" << std::endl;
    std::cout << "   - Fuentes registradas: " << intCtrl.getSourceCount() << std::endl;
    std::cout << std::endl;
    
    // Configurar timer para disparar IRQ cada 100 ciclos
    timer->setLimit(100);
    timer->write(0xFC08, 0x03);  // Enable | IRQ Enable
    
    std::cout << "4. Timer configurado:" << std::endl;
    std::cout << "   - Límite: 100 ciclos" << std::endl;
    std::cout << "   - IRQ habilitada: " << (timer->isIRQEnabled() ? "Sí" : "No") << std::endl;
    std::cout << "   - Auto-reload: " << (timer->isAutoReload() ? "Sí" : "No") << std::endl;
    std::cout << std::endl;
    
    std::cout << "5. Ejecutando simulación..." << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    // Estado inicial
    printCPUState(cpu, "Estado inicial");
    
    // Simular varios ciclos y manejar interrupciones
    int totalCycles = 0;
    int irqCount = 0;
    
    for (int i = 0; i < 6; i++) {
        int cyclesThisIteration = 50;
        totalCycles += cyclesThisIteration;
        
        std::cout << "\n--- Iteración " << (i + 1) << " (Ciclos totales: " << totalCycles << ") ---" << std::endl;
        
        // Ejecutar timer
        timer->tick(cyclesThisIteration);
        std::cout << "  Timer tick: +" << cyclesThisIteration << " ciclos" << std::endl;
        std::cout << "  Contador del timer: " << timer->getCounter() << std::endl;
        
        // Verificar y manejar interrupciones
        if (intCtrl.hasIRQ()) {
            irqCount++;
            std::cout << "\n  *** IRQ #" << irqCount << " DETECTADA ***" << std::endl;
            printCPUState(cpu, "Antes de IRQ");
            
            cpu.checkAndHandleInterrupts(mem);
            
            printCPUState(cpu, "Después de IRQ");
            std::cout << "  Pila afectada: " << (cpu.SP != 0xFF ? "Sí" : "No") << std::endl;
            
            // Resetear PC para la siguiente iteración (en un programa real, RTI lo haría)
            cpu.PC = 0xFFFC;
            cpu.I = 0;  // Habilitar interrupciones nuevamente
        } else {
            std::cout << "  No hay interrupciones pendientes" << std::endl;
        }
    }
    
    std::cout << std::string(60, '-') << std::endl;
    std::cout << "\n6. Resumen de la simulación:" << std::endl;
    std::cout << "   - Total de ciclos ejecutados: " << totalCycles << std::endl;
    std::cout << "   - IRQs disparadas: " << irqCount << std::endl;
    std::cout << "   - Contador final del timer: " << timer->getCounter() << std::endl;
    std::cout << std::endl;
    
    std::cout << "7. Demostrando NMI (no enmascarable)..." << std::endl;
    
    // Crear una fuente mock de NMI
    class MockNMISource : public InterruptSource {
    public:
        bool hasIRQ() const override { return false; }
        bool hasNMI() const override { return nmiActive; }
        void clearIRQ() override {}
        void clearNMI() override { nmiActive = false; }
        void triggerNMI() { nmiActive = true; }
    private:
        bool nmiActive = false;
    };
    
    auto nmiSource = std::make_shared<MockNMISource>();
    intCtrl.registerSource(nmiSource);
    
    // Establecer flag I para intentar bloquear interrupciones
    cpu.I = 1;
    std::cout << "   - Flag I establecido (interrupciones deshabilitadas)" << std::endl;
    
    // Disparar NMI
    nmiSource->triggerNMI();
    std::cout << "   - NMI disparada" << std::endl;
    
    printCPUState(cpu, "Antes de NMI");
    cpu.checkAndHandleInterrupts(mem);
    printCPUState(cpu, "Después de NMI");
    
    std::cout << "   - NMI se ejecutó a pesar del flag I (no enmascarable)" << std::endl;
    std::cout << "   - PC apunta a: 0x" << std::hex << cpu.PC << std::dec << " (vector de NMI)" << std::endl;
    std::cout << std::endl;
    
    // Limpiar
    cpu.unregisterIODevice(timer);
    timer->cleanup();
    
    std::cout << "=== Demostración completada exitosamente ===" << std::endl;
    
    return 0;
}
