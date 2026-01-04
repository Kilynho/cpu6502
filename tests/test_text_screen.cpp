#include <gtest/gtest.h>
#include "cpu.hpp"
#include "cpu/system_map.hpp"
#include "text_screen.hpp"
#include <memory>

class TextScreenTest : public testing::Test {
public:
    SystemMap bus;
    CPU cpu;
    std::shared_ptr<TextScreen> screen;

    virtual void SetUp() {
        cpu.PC = 0x8000;
        cpu.SP = 0xFD;
        cpu.A = cpu.X = cpu.Y = 0;
        cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
        bus.clearRAM();
        screen = std::make_shared<TextScreen>();
        bus.registerIODevice(screen);
    }

    virtual void TearDown() {
        bus.unregisterIODevice(screen);
    }
};

// Test: Inicialización de la pantalla
TEST_F(TextScreenTest, Initialization) {
    EXPECT_EQ(screen->getWidth(), 40);
    EXPECT_EQ(screen->getHeight(), 24);
    
    // Verificar que la pantalla está vacía (llena de espacios)
    std::string buffer = screen->getBuffer();
    for (char c : buffer) {
        if (c != ' ' && c != '\n') {
            FAIL() << "Buffer debería estar lleno de espacios, encontrado: " << c;
        }
    }
}

// Test: Escritura directa en buffer de video
TEST_F(TextScreenTest, DirectVideoMemoryWrite) {
    // Escribir 'H' en la primera posición
    bus.write(0x8000, 0xA9);  // LDA #'H'
    bus.write(0x8001, 'H');
    bus.write(0x8002, 0x8D);  // STA $FC00
    bus.write(0x8003, 0x00);
    bus.write(0x8004, 0xFC);
    
    cpu.Execute(8, bus);
    
    std::string buffer = screen->getBuffer();
    EXPECT_EQ(buffer[0], 'H');
}

// Test: Escritura mediante el puerto de caracteres
TEST_F(TextScreenTest, CharacterPortWrite) {
    // Escribir 'A' mediante el puerto de caracteres
    bus.write(0x8000, 0xA9);  // LDA #'A'
    bus.write(0x8001, 'A');
    bus.write(0x8002, 0x8D);  // STA $FFFF (puerto de caracteres)
    bus.write(0x8003, 0xFF);
    bus.write(0x8004, 0xFF);
    
    cpu.Execute(8, bus);
    
    std::string buffer = screen->getBuffer();
    EXPECT_EQ(buffer[0], 'A');
}

// Test: Escritura de múltiples caracteres
TEST_F(TextScreenTest, MultipleCharacterWrite) {
    // Escribir "HELLO" mediante el puerto de caracteres
    const char* text = "HELLO";
    uint16_t addr = 0x8000;
    
    for (int i = 0; text[i] != '\0'; i++) {
        bus.write(addr++, 0xA9);  // LDA #char
        bus.write(addr++, text[i]);
        bus.write(addr++, 0x8D);  // STA $FFFF
        bus.write(addr++, 0xFF);
        bus.write(addr++, 0xFF);
    }
    
    cpu.Execute(40, bus);
    
    std::string buffer = screen->getBuffer();
    EXPECT_EQ(buffer.substr(0, 5), "HELLO");
}

// Test: Control de posición del cursor
TEST_F(TextScreenTest, CursorPositioning) {
    // Posicionar cursor en columna 10, fila 5
    bus.write(0x8000, 0xA9);  // LDA #10
    bus.write(0x8001, 10);
    bus.write(0x8002, 0x8D);  // STA $FFFC (cursor col)
    bus.write(0x8003, 0xFC);
    bus.write(0x8004, 0xFF);
    
    bus.write(0x8005, 0xA9);  // LDA #5
    bus.write(0x8006, 5);
    bus.write(0x8007, 0x8D);  // STA $FFFD (cursor row)
    bus.write(0x8008, 0xFD);
    bus.write(0x8009, 0xFF);
    
    bus.write(0x800A, 0xA9);  // LDA #'X'
    bus.write(0x800B, 'X');
    bus.write(0x800C, 0x8D);  // STA $FFFF (escribir en cursor)
    bus.write(0x800D, 0xFF);
    bus.write(0x800E, 0xFF);
    
    cpu.Execute(24, bus);
    
    std::string buffer = screen->getBuffer();
    // Calcular offset: fila 5 * 40 + columna 10 = 210
    // En el string con saltos de línea: 5 líneas de 40 caracteres + 5 saltos + 10 = 215
    int offset = 5 * 41 + 10;  // 41 porque hay un '\n' al final de cada línea
    EXPECT_EQ(buffer[offset], 'X');
}

// Test: Limpieza de pantalla
TEST_F(TextScreenTest, ClearScreen) {
    // Escribir algunos caracteres
    screen->writeCharAtCursor('H');
    screen->writeCharAtCursor('I');
    
    // Limpiar pantalla mediante registro de control
    bus.write(0x8000, 0xA9);  // LDA #0x02 (bit de clear)
    bus.write(0x8001, 0x02);
    bus.write(0x8002, 0x8D);  // STA $FFFE (control)
    bus.write(0x8003, 0xFE);
    bus.write(0x8004, 0xFF);
    
    cpu.Execute(8, bus);
    
    std::string buffer = screen->getBuffer();
    for (char c : buffer) {
        if (c != ' ' && c != '\n') {
            FAIL() << "Buffer debería estar vacío después de clear";
        }
    }
    
    uint8_t col, row;
    screen->getCursorPosition(col, row);
    EXPECT_EQ(col, 0);
    EXPECT_EQ(row, 0);
}

// Test: Salto de línea
TEST_F(TextScreenTest, NewlineHandling) {
    screen->writeCharAtCursor('A');
    screen->writeCharAtCursor('\n');
    screen->writeCharAtCursor('B');
    
    std::string buffer = screen->getBuffer();
    EXPECT_EQ(buffer[0], 'A');
    // 'B' debe estar en la segunda línea (offset 41 con '\n')
    EXPECT_EQ(buffer[41], 'B');
}

// Test: Auto-scroll cuando se llena la pantalla
TEST_F(TextScreenTest, AutoScroll) {
    screen->setAutoScroll(true);
    
    // Llenar las primeras 2 líneas con 'A'
    for (int i = 0; i < 40; i++) {
        screen->writeCharAtCursor('A');
    }
    for (int i = 0; i < 40; i++) {
        screen->writeCharAtCursor('B');
    }
    
    // Mover cursor a la última línea
    screen->setCursorPosition(0, 23);
    
    // Escribir más allá de la última línea debería hacer scroll
    for (int i = 0; i < 40; i++) {
        screen->writeCharAtCursor('Z');
    }
    screen->writeCharAtCursor('X');  // Esta debería causar scroll
    
    std::string buffer = screen->getBuffer();
    
    // La primera línea original ('A's) debería haber desaparecido
    // La segunda línea ('B's) debería estar ahora en la primera posición
    EXPECT_EQ(buffer[0], 'B');
}

// Test: Lectura de registros de cursor
TEST_F(TextScreenTest, ReadCursorRegisters) {
    screen->setCursorPosition(15, 10);
    
    // Leer columna del cursor
    bus.write(0x8000, 0xAD);  // LDA $FFFC
    bus.write(0x8001, 0xFC);
    bus.write(0x8002, 0xFF);
    
    cpu.Execute(4, bus);
    EXPECT_EQ(cpu.A, 15);
    
    // Leer fila del cursor
    bus.write(0x8003, 0xAD);  // LDA $FFFD
    bus.write(0x8004, 0xFD);
    bus.write(0x8005, 0xFF);
    
    cpu.Execute(4, bus);
    EXPECT_EQ(cpu.A, 10);
}

// Test: Escritura de caracteres de control (tab)
TEST_F(TextScreenTest, TabHandling) {
    screen->writeCharAtCursor('A');
    screen->writeCharAtCursor('\t');  // Tab a posición 8
    screen->writeCharAtCursor('B');
    
    std::string buffer = screen->getBuffer();
    EXPECT_EQ(buffer[0], 'A');
    EXPECT_EQ(buffer[8], 'B');
}

// Test: Backspace
TEST_F(TextScreenTest, BackspaceHandling) {
    screen->writeCharAtCursor('A');
    screen->writeCharAtCursor('B');
    screen->writeCharAtCursor('C');
    
    uint8_t col, row;
    screen->getCursorPosition(col, row);
    EXPECT_EQ(col, 3);  // Después de escribir 3 caracteres
    
    screen->writeCharAtCursor('\b');
    screen->getCursorPosition(col, row);
    EXPECT_EQ(col, 2);  // Retroceder una posición
}

// Test: Manejo del método clear()
TEST_F(TextScreenTest, ClearMethod) {
    screen->writeCharAtCursor('T');
    screen->writeCharAtCursor('E');
    screen->writeCharAtCursor('S');
    screen->writeCharAtCursor('T');
    
    screen->clear();
    
    std::string buffer = screen->getBuffer();
    for (char c : buffer) {
        if (c != ' ' && c != '\n') {
            FAIL() << "Buffer debería estar vacío después de clear()";
        }
    }
}

// Test: Escritura en todo el buffer
TEST_F(TextScreenTest, FullBufferWrite) {
    // Deshabilitar auto-scroll para que no se pierdan caracteres
    screen->setAutoScroll(false);
    
    // Llenar todo el buffer con 'X'
    for (uint16_t i = 0; i < 40 * 24; i++) {
        screen->writeCharAtCursor('X');
    }
    
    std::string buffer = screen->getBuffer();
    int xCount = 0;
    for (char c : buffer) {
        if (c == 'X') xCount++;
    }
    EXPECT_EQ(xCount, 40 * 24);
}
