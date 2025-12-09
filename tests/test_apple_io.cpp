#include <gtest/gtest.h>
#include "cpu.hpp"
#include "mem.hpp"
#include "apple_io.hpp"
#include <memory>

class AppleIOTest : public testing::Test {
public:
    Mem mem;
    CPU cpu;
    std::shared_ptr<AppleIO> appleIO;

    virtual void SetUp() {
        cpu.Reset(mem);
        appleIO = std::make_shared<AppleIO>();
        cpu.registerIODevice(appleIO);
        // El PC ya está en $8000 después de Reset()
    }

    virtual void TearDown() {
        cpu.unregisterIODevice(appleIO);
    }
};

// Test: Leer de $FD0C sin entrada devuelve 0
TEST_F(AppleIOTest, ReadKeyboardEmpty) {
    mem[0x8000] = 0xAD;  // LDA $FD0C
    mem[0x8001] = 0x0C;
    mem[0x8002] = 0xFD;

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0x00);
}

// Test: Leer de $FD0C con entrada devuelve el carácter
TEST_F(AppleIOTest, ReadKeyboardWithInput) {
    appleIO->pushInput('A');

    mem[0x8000] = 0xAD;  // LDA $FD0C
    mem[0x8001] = 0x0C;
    mem[0x8002] = 0xFD;

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 'A');
}

// Test: Leer múltiples caracteres del teclado
TEST_F(AppleIOTest, ReadKeyboardMultiple) {
    appleIO->pushInput('H');
    appleIO->pushInput('i');

    mem[0x8000] = 0xAD;  // LDA $FD0C
    mem[0x8001] = 0x0C;
    mem[0x8002] = 0xFD;

    cpu.Execute(4, mem);
    EXPECT_EQ(cpu.A, 'H');

    cpu.PC = 0x8000;  // Reset PC para leer de nuevo
    cpu.Execute(4, mem);
    EXPECT_EQ(cpu.A, 'i');

    cpu.PC = 0x8000;  // Reset PC para leer de nuevo (ya no hay más)
    cpu.Execute(4, mem);
    EXPECT_EQ(cpu.A, 0x00);
}

// Test: Escribir en $FDED actualiza el buffer de pantalla
TEST_F(AppleIOTest, WriteScreen) {
    cpu.A = 'X';

    mem[0x8000] = 0x8D;  // STA $FDED
    mem[0x8001] = 0xED;
    mem[0x8002] = 0xFD;

    cpu.Execute(4, mem);

    EXPECT_EQ(appleIO->getScreenBuffer(), "X");
}

// Test: Escribir múltiples caracteres en $FDED
TEST_F(AppleIOTest, WriteScreenMultiple) {
    cpu.A = 'A';
    mem[0x8000] = 0x8D;  // STA $FDED
    mem[0x8001] = 0xED;
    mem[0x8002] = 0xFD;
    cpu.Execute(4, mem);

    cpu.A = 'B';
    cpu.PC = 0x8000;
    cpu.Execute(4, mem);

    cpu.A = 'C';
    cpu.PC = 0x8000;
    cpu.Execute(4, mem);

    EXPECT_EQ(appleIO->getScreenBuffer(), "ABC");
}

// Test: Leer y escribir en el mismo ciclo (eco)
TEST_F(AppleIOTest, EchoTest) {
    appleIO->pushInput('!');

    // LDA $FD0C, STA $FDED
    mem[0x8000] = 0xAD;  // LDA $FD0C
    mem[0x8001] = 0x0C;
    mem[0x8002] = 0xFD;
    mem[0x8003] = 0x8D;  // STA $FDED
    mem[0x8004] = 0xED;
    mem[0x8005] = 0xFD;

    cpu.Execute(8, mem);

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
    mem[0x8000] = 0xAD;  // LDA $FD0C
    mem[0x8001] = 0x0C;
    mem[0x8002] = 0xFD;

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 'Z');

    cpu.unregisterIODevice(anotherIO);
}
