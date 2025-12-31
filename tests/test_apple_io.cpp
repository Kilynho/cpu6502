#include <gtest/gtest.h>
#include "cpu.hpp"
#include "system_map.hpp"
#include "apple_io.hpp"
#include <memory>

class AppleIOTest : public testing::Test {
public:
    SystemMap bus;
    CPU cpu;
    std::shared_ptr<AppleIO> appleIO;

    virtual void SetUp() {
        cpu.PC = 0x8000;
        cpu.SP = 0xFD;
        cpu.A = cpu.X = cpu.Y = 0;
        cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
        appleIO = std::make_shared<AppleIO>();
        cpu.registerIODevice(appleIO);
    }

    virtual void TearDown() {
        cpu.unregisterIODevice(appleIO);
    }
};

// Test: Leer de $FD0C sin entrada devuelve 0
TEST_F(AppleIOTest, ReadKeyboardEmpty) {
    bus.write(0x8000, 0xAD);  // LDA $FD0C
    bus.write(0x8001, 0x0C);
    bus.write(0x8002, 0xFD);
    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0x00);
}

// Test: Leer de $FD0C con entrada devuelve el carácter
TEST_F(AppleIOTest, ReadKeyboardWithInput) {
    appleIO->pushInput('A');

    bus.write(0x8000, 0xAD);  // LDA $FD0C
    bus.write(0x8001, 0x0C);
    bus.write(0x8002, 0xFD);
    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 'A');
}

// Test: Leer múltiples caracteres del teclado
TEST_F(AppleIOTest, ReadKeyboardMultiple) {
    appleIO->pushInput('H');
    appleIO->pushInput('i');

    bus.write(0x8000, 0xAD);  // LDA $FD0C
    bus.write(0x8001, 0x0C);
    bus.write(0x8002, 0xFD);
    cpu.Execute(4, bus);
    EXPECT_EQ(cpu.A, 'H');
    cpu.PC = 0x8000;  // Reset PC para leer de nuevo
    cpu.Execute(4, bus);
    EXPECT_EQ(cpu.A, 'i');
    cpu.PC = 0x8000;  // Reset PC para leer de nuevo (ya no hay más)
    cpu.Execute(4, bus);
    EXPECT_EQ(cpu.A, 0x00);
}

// Test: Escribir en $FDED actualiza el buffer de pantalla
TEST_F(AppleIOTest, WriteScreen) {
    cpu.A = 'X';

    bus.write(0x8000, 0x8D);  // STA $FDED
    bus.write(0x8001, 0xED);
    bus.write(0x8002, 0xFD);
    cpu.Execute(4, bus);

    EXPECT_EQ(appleIO->getScreenBuffer(), "X");
}

// Test: Escribir múltiples caracteres en $FDED
TEST_F(AppleIOTest, WriteScreenMultiple) {
    cpu.A = 'A';
    bus.write(0x8000, 0x8D);  // STA $FDED
    bus.write(0x8001, 0xED);
    bus.write(0x8002, 0xFD);
    cpu.Execute(4, bus);
    cpu.A = 'B';
    cpu.PC = 0x8000;
    cpu.Execute(4, bus);
    cpu.A = 'C';
    cpu.PC = 0x8000;
    cpu.Execute(4, bus);
    EXPECT_EQ(appleIO->getScreenBuffer(), "ABC");
}

// Test: Leer y escribir en el mismo ciclo (eco)
TEST_F(AppleIOTest, EchoTest) {
    appleIO->pushInput('!');

    // LDA $FD0C, STA $FDED
    bus.write(0x8000, 0xAD);  // LDA $FD0C
    bus.write(0x8001, 0x0C);
    bus.write(0x8002, 0xFD);
    bus.write(0x8003, 0x8D);  // STA $FDED
    bus.write(0x8004, 0xED);
    bus.write(0x8005, 0xFD);
    cpu.Execute(8, bus);

    EXPECT_EQ(cpu.A, '!');
    EXPECT_EQ(appleIO->getScreenBuffer(), "!");
}

// Test: Registrar y desregistrar IODevice
TEST_F(AppleIOTest, RegisterUnregisterDevice) {
    auto anotherIO = std::make_shared<AppleIO>();
    cpu.registerIODevice(anotherIO);

    // Ambos dispositivos deberían estar registrados
    // Desregistrar el primero
    cpu.unregisterIODevice(appleIO);

    // Ahora solo anotherIO debería manejar las operaciones
    anotherIO->pushInput('Z');
    bus.write(0x8000, 0xAD);  // LDA $FD0C
    bus.write(0x8001, 0x0C);
    bus.write(0x8002, 0xFD);
    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 'Z');

    cpu.unregisterIODevice(anotherIO);
}
