#include <gtest/gtest.h>
#include "cpu.hpp"
#include "system_map.hpp"
#include "basic_timer.hpp"
#include <memory>
#include <thread>
#include <chrono>

class BasicTimerTest : public testing::Test {
public:
    SystemMap bus;
    CPU cpu;
    std::shared_ptr<BasicTimer> timer;

    virtual void SetUp() {
        cpu.PC = 0x8000;
        cpu.SP = 0xFD;
        cpu.A = cpu.X = cpu.Y = 0;
        cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
        bus.clearRAM();
        timer = std::make_shared<BasicTimer>();
        ASSERT_TRUE(timer->initialize()) << "Failed to initialize timer device";
        bus.registerIODevice(timer);
    }

    virtual void TearDown() {
        bus.unregisterIODevice(timer);
        timer->cleanup();
    }
};

// Test: Inicialización del dispositivo de timer
TEST_F(BasicTimerTest, Initialization) {
    // Verificar que el timer se inicializó correctamente
    EXPECT_FALSE(timer->isEnabled());
    EXPECT_EQ(timer->getCounter(), 0);
    EXPECT_EQ(timer->getLimit(), 0);
    EXPECT_FALSE(timer->hasIRQ());
}

// Test: Lectura y escritura del contador
TEST_F(BasicTimerTest, CounterReadWrite) {
    // Escribir valor en el contador (0x12345678)
    timer->write(0xFC00, 0x78);  // Byte bajo
    timer->write(0xFC01, 0x56);  // Byte medio 1
    timer->write(0xFC02, 0x34);  // Byte medio 2
    timer->write(0xFC03, 0x12);  // Byte alto
    
    // Leer de vuelta el contador
    uint8_t byte0 = timer->read(0xFC00);
    uint8_t byte1 = timer->read(0xFC01);
    uint8_t byte2 = timer->read(0xFC02);
    uint8_t byte3 = timer->read(0xFC03);
    
    EXPECT_EQ(byte0, 0x78);
    EXPECT_EQ(byte1, 0x56);
    EXPECT_EQ(byte2, 0x34);
    EXPECT_EQ(byte3, 0x12);
    
    uint32_t counterValue = timer->getCounter();
    EXPECT_EQ(counterValue, 0x12345678);
}

// Test: Lectura y escritura del límite
TEST_F(BasicTimerTest, LimitReadWrite) {
    // Escribir límite (1000000 = 0x000F4240)
    timer->write(0xFC04, 0x40);  // Byte bajo
    timer->write(0xFC05, 0x42);  // Byte medio 1
    timer->write(0xFC06, 0x0F);  // Byte medio 2
    timer->write(0xFC07, 0x00);  // Byte alto
    
    // Leer de vuelta el límite
    uint8_t byte0 = timer->read(0xFC04);
    uint8_t byte1 = timer->read(0xFC05);
    uint8_t byte2 = timer->read(0xFC06);
    uint8_t byte3 = timer->read(0xFC07);
    
    EXPECT_EQ(byte0, 0x40);
    EXPECT_EQ(byte1, 0x42);
    EXPECT_EQ(byte2, 0x0F);
    EXPECT_EQ(byte3, 0x00);
    
    uint32_t limitValue = timer->getLimit();
    EXPECT_EQ(limitValue, 1000000);
}

// Test: Habilitación y deshabilitación del timer
TEST_F(BasicTimerTest, EnableDisable) {
    // Habilitar timer
    timer->write(0xFC08, 0x01);  // Enable bit
    
    EXPECT_TRUE(timer->isEnabled());
    
    // Verify status register
    uint8_t status = timer->read(0xFC09);
    EXPECT_TRUE((status & BasicTimer::STATUS_ENABLED) != 0);  // Enabled bit
    
    // Disable timer
    timer->write(0xFC08, 0x00);
    
    EXPECT_FALSE(timer->isEnabled());
    
    status = timer->read(0xFC09);
    EXPECT_TRUE((status & BasicTimer::STATUS_ENABLED) == 0);
}

// Test: Conteo de ciclos
TEST_F(BasicTimerTest, CountingCycles) {
    // Reiniciar contador
    timer->reset();
    EXPECT_EQ(timer->getCounter(), 0);
    
    // Habilitar timer
    timer->setEnabled(true);
    
    // Ejecutar algunos ciclos
    timer->tick(100);
    EXPECT_EQ(timer->getCounter(), 100);
    
    timer->tick(50);
    EXPECT_EQ(timer->getCounter(), 150);
    
    timer->tick(25);
    EXPECT_EQ(timer->getCounter(), 175);
}

// Test: Generación de IRQ al alcanzar el límite
TEST_F(BasicTimerTest, IRQGeneration) {
    // Configurar límite a 1000 ciclos
    timer->setLimit(1000);
    
    // Habilitar timer e IRQ
    timer->write(0xFC08, 0x03);  // Enable | IRQ Enable
    
    EXPECT_TRUE(timer->isEnabled());
    EXPECT_TRUE(timer->isIRQEnabled());
    EXPECT_FALSE(timer->hasIRQ());
    
    // Ejecutar 500 ciclos - no debe generar IRQ aún
    timer->tick(500);
    EXPECT_FALSE(timer->hasIRQ());
    EXPECT_EQ(timer->getCounter(), 500);
    
    // Ejecutar 500 ciclos más - debe generar IRQ
    timer->tick(500);
    EXPECT_TRUE(timer->hasIRQ());
    EXPECT_EQ(timer->getCounter(), 1000);
    
    // Verify status register
    uint8_t status = timer->read(0xFC09);
    EXPECT_TRUE((status & BasicTimer::STATUS_IRQ_PENDING) != 0);  // IRQ Pending bit
    EXPECT_TRUE((status & BasicTimer::STATUS_LIMIT_REACHED) != 0);  // Limit Reached bit
}

// Test: Limpiar IRQ
TEST_F(BasicTimerTest, ClearIRQ) {
    // Configurar y generar IRQ
    timer->setLimit(100);
    timer->write(0xFC08, 0x03);  // Enable | IRQ Enable
    timer->tick(100);
    
    EXPECT_TRUE(timer->hasIRQ());
    
    // Limpiar IRQ escribiendo el bit IRQ_FLAG
    timer->write(0xFC08, 0x04);
    
    EXPECT_FALSE(timer->hasIRQ());
    
    // Verify status register
    uint8_t status = timer->read(0xFC09);
    EXPECT_TRUE((status & BasicTimer::STATUS_IRQ_PENDING) == 0);  // IRQ Pending bit should be 0
}

// Test: Auto-reload
TEST_F(BasicTimerTest, AutoReload) {
    // Configurar límite a 100 ciclos
    timer->setLimit(100);
    
    // Habilitar timer, IRQ y auto-reload
    timer->write(0xFC08, 0x13);  // Enable | IRQ Enable | Auto-reload
    
    EXPECT_TRUE(timer->isEnabled());
    EXPECT_TRUE(timer->isIRQEnabled());
    EXPECT_TRUE(timer->isAutoReload());
    
    // Ejecutar 150 ciclos
    timer->tick(150);
    
    // Debe haber generado IRQ
    EXPECT_TRUE(timer->hasIRQ());
    
    // El contador debe haberse reiniciado con overflow (150 - 100 = 50)
    EXPECT_EQ(timer->getCounter(), 50);
    
    // El timer debe seguir habilitado
    EXPECT_TRUE(timer->isEnabled());
}

// Test: Detener al alcanzar el límite (sin auto-reload)
TEST_F(BasicTimerTest, StopAtLimit) {
    // Configurar límite a 100 ciclos
    timer->setLimit(100);
    
    // Habilitar timer e IRQ, pero NO auto-reload
    timer->write(0xFC08, 0x03);  // Enable | IRQ Enable
    
    EXPECT_TRUE(timer->isEnabled());
    EXPECT_FALSE(timer->isAutoReload());
    
    // Ejecutar 150 ciclos
    timer->tick(150);
    
    // Debe haber generado IRQ
    EXPECT_TRUE(timer->hasIRQ());
    
    // El contador debe haberse detenido en el límite
    EXPECT_EQ(timer->getCounter(), 100);
    
    // El timer debe haberse deshabilitado
    EXPECT_FALSE(timer->isEnabled());
}

// Test: Reset del contador
TEST_F(BasicTimerTest, ResetCounter) {
    // Configurar contador y límite
    timer->setCounter(500);
    timer->setLimit(1000);
    timer->setEnabled(true);
    
    EXPECT_EQ(timer->getCounter(), 500);
    
    // Reiniciar usando el bit de control
    timer->write(0xFC08, 0x08);  // Reset bit
    
    EXPECT_EQ(timer->getCounter(), 0);
    EXPECT_FALSE(timer->hasIRQ());
}

// Test: Leer valor de tiempo desde la CPU (simulación)
TEST_F(BasicTimerTest, ReadTimeFromCPU) {
    // Este test simula cómo la CPU leería el tiempo del timer
    
    // Configurar y ejecutar el timer
    timer->setLimit(10000);
    timer->setEnabled(true);
    timer->tick(5432);  // Simular 5432 ciclos
    
    // Leer el contador como lo haría la CPU (byte por byte)
    uint8_t byte0 = cpu.ReadMemory(0xFC00, bus);
    uint8_t byte1 = cpu.ReadMemory(0xFC01, bus);
    uint8_t byte2 = cpu.ReadMemory(0xFC02, bus);
    uint8_t byte3 = cpu.ReadMemory(0xFC03, bus);
    
    // Reconstruir el valor de 32 bits
    uint32_t timerValue = byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);
    
    EXPECT_EQ(timerValue, 5432);
}

// Test: IRQ periódica (simulación)
TEST_F(BasicTimerTest, PeriodicIRQ) {
    // Configurar timer para IRQ cada 1000 ciclos con auto-reload
    timer->setLimit(1000);
    timer->write(0xFC08, 0x13);  // Enable | IRQ Enable | Auto-reload
    
    // Primera IRQ
    timer->tick(1000);
    EXPECT_TRUE(timer->hasIRQ());
    timer->clearIRQ();
    EXPECT_FALSE(timer->hasIRQ());
    
    // Segunda IRQ
    timer->tick(1000);
    EXPECT_TRUE(timer->hasIRQ());
    timer->clearIRQ();
    EXPECT_FALSE(timer->hasIRQ());
    
    // Tercera IRQ
    timer->tick(1000);
    EXPECT_TRUE(timer->hasIRQ());
    
    // Verificar que el contador se reinició correctamente
    EXPECT_EQ(timer->getCounter(), 0);
}

// Test: Control register bits
TEST_F(BasicTimerTest, ControlRegisterBits) {
    // Write all control bits (Enable | IRQ Enable | IRQ Flag | Reset | Auto-reload)
    uint8_t allBits = 0x01 | 0x02 | 0x04 | 0x08 | 0x10;  // 0x1F
    timer->write(0xFC08, allBits);
    
    uint8_t control = timer->read(0xFC08);
    EXPECT_EQ(control, allBits);
    
    EXPECT_TRUE(timer->isEnabled());
    EXPECT_TRUE(timer->isIRQEnabled());
    EXPECT_TRUE(timer->isAutoReload());
}

// Test: No IRQ si no está habilitado
TEST_F(BasicTimerTest, NoIRQWhenDisabled) {
    // Configurar límite
    timer->setLimit(100);
    
    // Habilitar timer pero NO IRQ
    timer->write(0xFC08, 0x01);  // Solo Enable
    
    EXPECT_TRUE(timer->isEnabled());
    EXPECT_FALSE(timer->isIRQEnabled());
    
    // Ejecutar ciclos hasta alcanzar el límite
    timer->tick(100);
    
    // Should not generate IRQ
    EXPECT_FALSE(timer->hasIRQ());
    
    // But limit should have been reached
    uint8_t status = timer->read(0xFC09);
    EXPECT_TRUE((status & BasicTimer::STATUS_LIMIT_REACHED) != 0);  // Limit Reached bit
}
