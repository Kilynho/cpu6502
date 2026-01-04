#include <gtest/gtest.h>
#include "interrupt_controller.hpp"
#include "cpu.hpp"
#include "system_map.hpp"
#include "basic_timer.hpp"

static constexpr uint16_t IRQ_VECTOR_ADDR = 0xFFFE;
static constexpr uint16_t NMI_VECTOR_ADDR = 0xFFFA;
#include <memory>

// Mock interrupt source para pruebas
class MockInterruptSource : public InterruptSource {
public:
    MockInterruptSource() : irqPending(false), nmiPending(false) {}
    
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
    
    void triggerIRQ() {
        irqPending = true;
    }
    
    void triggerNMI() {
        nmiPending = true;
    }
    
private:
    bool irqPending;
    bool nmiPending;
};

class InterruptControllerTest : public testing::Test {
public:
    InterruptController intCtrl;
    std::shared_ptr<MockInterruptSource> mockSource1;
    std::shared_ptr<MockInterruptSource> mockSource2;
    
    virtual void SetUp() {
        mockSource1 = std::make_shared<MockInterruptSource>();
        mockSource2 = std::make_shared<MockInterruptSource>();
    }
    
    virtual void TearDown() {
    }
};

// Test: Inicialización del controlador de interrupciones
TEST_F(InterruptControllerTest, Initialization) {
    EXPECT_EQ(intCtrl.getSourceCount(), 0);
    EXPECT_FALSE(intCtrl.hasIRQ());
    EXPECT_FALSE(intCtrl.hasNMI());
}

// Test: Registrar fuentes de interrupción
TEST_F(InterruptControllerTest, RegisterSources) {
    intCtrl.registerSource(mockSource1);
    EXPECT_EQ(intCtrl.getSourceCount(), 1);
    
    intCtrl.registerSource(mockSource2);
    EXPECT_EQ(intCtrl.getSourceCount(), 2);
}

// Test: Eliminar fuentes de interrupción
TEST_F(InterruptControllerTest, UnregisterSources) {
    intCtrl.registerSource(mockSource1);
    intCtrl.registerSource(mockSource2);
    EXPECT_EQ(intCtrl.getSourceCount(), 2);
    
    intCtrl.unregisterSource(mockSource1);
    EXPECT_EQ(intCtrl.getSourceCount(), 1);
    
    intCtrl.unregisterSource(mockSource2);
    EXPECT_EQ(intCtrl.getSourceCount(), 0);
}

// Test: Detectar IRQ pendiente
TEST_F(InterruptControllerTest, DetectIRQ) {
    intCtrl.registerSource(mockSource1);
    
    EXPECT_FALSE(intCtrl.hasIRQ());
    
    mockSource1->triggerIRQ();
    EXPECT_TRUE(intCtrl.hasIRQ());
}

// Test: Detectar NMI pendiente
TEST_F(InterruptControllerTest, DetectNMI) {
    intCtrl.registerSource(mockSource1);
    
    EXPECT_FALSE(intCtrl.hasNMI());
    
    mockSource1->triggerNMI();
    EXPECT_TRUE(intCtrl.hasNMI());
}

// Test: Reconocer IRQ
TEST_F(InterruptControllerTest, AcknowledgeIRQ) {
    intCtrl.registerSource(mockSource1);
    mockSource1->triggerIRQ();
    
    EXPECT_TRUE(intCtrl.hasIRQ());
    
    intCtrl.acknowledgeIRQ();
    EXPECT_FALSE(intCtrl.hasIRQ());
}

// Test: Reconocer NMI
TEST_F(InterruptControllerTest, AcknowledgeNMI) {
    intCtrl.registerSource(mockSource1);
    mockSource1->triggerNMI();
    
    EXPECT_TRUE(intCtrl.hasNMI());
    
    intCtrl.acknowledgeNMI();
    EXPECT_FALSE(intCtrl.hasNMI());
}

// Test: Múltiples fuentes de IRQ
TEST_F(InterruptControllerTest, MultipleIRQSources) {
    intCtrl.registerSource(mockSource1);
    intCtrl.registerSource(mockSource2);
    
    mockSource1->triggerIRQ();
    mockSource2->triggerIRQ();
    
    EXPECT_TRUE(intCtrl.hasIRQ());
    
    // Reconocer debe limpiar todas las fuentes
    intCtrl.acknowledgeIRQ();
    EXPECT_FALSE(intCtrl.hasIRQ());
}

// Test: Limpiar todas las interrupciones
TEST_F(InterruptControllerTest, ClearAll) {
    intCtrl.registerSource(mockSource1);
    mockSource1->triggerIRQ();
    mockSource1->triggerNMI();
    
    EXPECT_TRUE(intCtrl.hasIRQ());
    EXPECT_TRUE(intCtrl.hasNMI());
    
    intCtrl.clearAll();
    EXPECT_FALSE(intCtrl.hasIRQ());
    EXPECT_FALSE(intCtrl.hasNMI());
}

// Test: Integración CPU - IRQ básica
TEST_F(InterruptControllerTest, CPUIntegrationBasicIRQ) {
    SystemMap bus;
    CPU cpu;
    bus.clearRAM();
    cpu.PC = 0x0000;
    cpu.SP = 0xFD;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
    
    // Configurar vector de IRQ a 0x8000
    bus.write(IRQ_VECTOR_ADDR, 0x00);
    bus.write(IRQ_VECTOR_ADDR + 1, 0x80);
    
    // Configurar controlador de interrupciones
    cpu.setInterruptController(&intCtrl);
    intCtrl.registerSource(mockSource1);
    
    // Estado inicial de la CPU
    Word initialPC = cpu.PC;
    Byte initialSP = cpu.SP;
    
    // Disparar IRQ
    mockSource1->triggerIRQ();
    EXPECT_TRUE(intCtrl.hasIRQ());
    
    // Verificar que I está limpio (IRQ habilitado)
    EXPECT_FALSE(cpu.I);
    
    // Manejar la interrupción
    cpu.checkAndHandleInterrupts(bus);
    
    // Verificar que PC apunta al vector de IRQ
    EXPECT_EQ(cpu.PC, 0x8000);
    
    // Verificar que I ahora está establecido
    EXPECT_TRUE(cpu.I);
    
    // Verificar que SP decrementó (3 bytes: PCH, PCL, P)
    EXPECT_EQ(cpu.SP, initialSP - 3);
    
    // Verificar que IRQ fue reconocida
    EXPECT_FALSE(intCtrl.hasIRQ());
}

// Test: Integración CPU - NMI básica
TEST_F(InterruptControllerTest, CPUIntegrationBasicNMI) {
    SystemMap bus;
    CPU cpu;
    bus.clearRAM();
    cpu.PC = 0x0000;
    cpu.SP = 0xFD;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
    
    // Configurar vector de NMI a 0x9000
    bus.write(NMI_VECTOR_ADDR, 0x00);
    bus.write(NMI_VECTOR_ADDR + 1, 0x90);
    
    // Configurar controlador de interrupciones
    cpu.setInterruptController(&intCtrl);
    intCtrl.registerSource(mockSource1);
    
    Word initialPC = cpu.PC;
    Byte initialSP = cpu.SP;
    
    // Disparar NMI
    mockSource1->triggerNMI();
    EXPECT_TRUE(intCtrl.hasNMI());
    
    // Manejar la interrupción
    cpu.checkAndHandleInterrupts(bus);
    
    // Verificar que PC apunta al vector de NMI
    EXPECT_EQ(cpu.PC, 0x9000);
    
    // Verificar que I está establecido
    EXPECT_TRUE(cpu.I);
    
    // Verificar que SP decrementó
    EXPECT_EQ(cpu.SP, initialSP - 3);
    
    // Verificar que NMI fue reconocida
    EXPECT_FALSE(intCtrl.hasNMI());
}

// Test: IRQ enmascarada por flag I
TEST_F(InterruptControllerTest, IRQMaskedByIFlag) {
    SystemMap bus;
    CPU cpu;
    bus.clearRAM();
    cpu.PC = 0x0000;
    cpu.SP = 0xFD;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
    
    // Configurar vector de IRQ
    bus.write(IRQ_VECTOR_ADDR, 0x00);
    bus.write(IRQ_VECTOR_ADDR + 1, 0x80);
    
    cpu.setInterruptController(&intCtrl);
    intCtrl.registerSource(mockSource1);
    
    // Establecer flag I (deshabilitar interrupciones)
    cpu.I = 1;
    
    Word initialPC = cpu.PC;
    
    // Disparar IRQ
    mockSource1->triggerIRQ();
    EXPECT_TRUE(intCtrl.hasIRQ());
    
    // Intentar manejar la interrupción
    cpu.checkAndHandleInterrupts(bus);
    
    // PC no debe cambiar porque IRQ está enmascarada
    EXPECT_EQ(cpu.PC, initialPC);
    
    // IRQ sigue pendiente
    EXPECT_TRUE(intCtrl.hasIRQ());
}

// Test: NMI no puede ser enmascarada
TEST_F(InterruptControllerTest, NMINotMasked) {
    SystemMap bus;
    CPU cpu;
    bus.clearRAM();
    cpu.PC = 0x0000;
    cpu.SP = 0xFD;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
    
    // Configurar vector de NMI
    bus.write(NMI_VECTOR_ADDR, 0x00);
    bus.write(NMI_VECTOR_ADDR + 1, 0x90);
    
    cpu.setInterruptController(&intCtrl);
    intCtrl.registerSource(mockSource1);
    
    // Establecer flag I (intentar deshabilitar interrupciones)
    cpu.I = 1;
    
    Word initialPC = cpu.PC;
    
    // Disparar NMI
    mockSource1->triggerNMI();
    EXPECT_TRUE(intCtrl.hasNMI());
    
    // Manejar la interrupción
    cpu.checkAndHandleInterrupts(bus);
    
    // PC debe cambiar porque NMI no puede ser enmascarada
    EXPECT_EQ(cpu.PC, 0x9000);
    
    // NMI fue reconocida
    EXPECT_FALSE(intCtrl.hasNMI());
}

// Test: Prioridad NMI sobre IRQ
TEST_F(InterruptControllerTest, NMIPriority) {
    SystemMap bus;
    CPU cpu;
    bus.clearRAM();
    cpu.PC = 0x0000;
    cpu.SP = 0xFD;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
    
    // Configurar vectores
    bus.write(IRQ_VECTOR_ADDR, 0x00);
    bus.write(IRQ_VECTOR_ADDR + 1, 0x80);
    bus.write(NMI_VECTOR_ADDR, 0x00);
    bus.write(NMI_VECTOR_ADDR + 1, 0x90);
    
    cpu.setInterruptController(&intCtrl);
    intCtrl.registerSource(mockSource1);
    
    // Disparar ambas interrupciones
    mockSource1->triggerIRQ();
    mockSource1->triggerNMI();
    
    EXPECT_TRUE(intCtrl.hasIRQ());
    EXPECT_TRUE(intCtrl.hasNMI());
    
    // Manejar interrupciones
    cpu.checkAndHandleInterrupts(bus);
    
    // NMI debe tener prioridad, así que PC apunta al vector de NMI
    EXPECT_EQ(cpu.PC, 0x9000);
    
    // NMI fue reconocida, pero IRQ sigue pendiente
    EXPECT_FALSE(intCtrl.hasNMI());
    EXPECT_TRUE(intCtrl.hasIRQ());
}

// Test: Integración con BasicTimer
TEST_F(InterruptControllerTest, BasicTimerIntegration) {
    SystemMap bus;
    CPU cpu;
    InterruptController timerIntCtrl;
    bus.clearRAM();
    cpu.PC = 0x0000;
    cpu.SP = 0xFD;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
    
    // Configurar vector de IRQ
    bus.write(IRQ_VECTOR_ADDR, 0x00);
    bus.write(IRQ_VECTOR_ADDR + 1, 0x80);
    
    // Crear y configurar timer
    auto timer = std::make_shared<BasicTimer>();
    ASSERT_TRUE(timer->initialize());
    
    cpu.registerIODevice(timer);
    cpu.setInterruptController(&timerIntCtrl);
    timerIntCtrl.registerSource(timer);
    
    // Configurar timer para disparar IRQ después de 100 ciclos
    timer->setLimit(100);
    timer->write(0xFC08, 0x03);  // Enable | IRQ Enable
    
    EXPECT_TRUE(timer->isEnabled());
    EXPECT_TRUE(timer->isIRQEnabled());
    EXPECT_FALSE(timerIntCtrl.hasIRQ());
    
    // Ejecutar 50 ciclos - no debe generar IRQ aún
    timer->tick(50);
    EXPECT_FALSE(timerIntCtrl.hasIRQ());
    
    // Ejecutar 50 ciclos más - debe generar IRQ
    timer->tick(50);
    EXPECT_TRUE(timerIntCtrl.hasIRQ());
    
    Word initialPC = cpu.PC;
    
    // Manejar la interrupción
    cpu.checkAndHandleInterrupts(bus);
    
    // Verificar que la CPU saltó al vector de IRQ
    EXPECT_EQ(cpu.PC, 0x8000);
    
    // IRQ debe estar limpia
    EXPECT_FALSE(timerIntCtrl.hasIRQ());
    
    // Limpiar
    cpu.unregisterIODevice(timer);
    timer->cleanup();
}

// Test: Sin controlador de interrupciones
TEST_F(InterruptControllerTest, NoInterruptController) {
    SystemMap bus;
    CPU cpu;
    bus.clearRAM();
    cpu.PC = 0x0000;
    cpu.SP = 0xFD;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
    
    // No configurar controlador de interrupciones
    EXPECT_EQ(cpu.getInterruptController(), nullptr);
    
    Word initialPC = cpu.PC;
    
    // Intentar manejar interrupciones - no debe hacer nada
    cpu.checkAndHandleInterrupts(bus);
    
    EXPECT_EQ(cpu.PC, initialPC);
}
