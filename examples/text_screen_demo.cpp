#include "cpu.hpp"
#include "mem.hpp"
#include "devices/text_screen.hpp"
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>

/**
 * @brief Demostración de la pantalla de texto (TextScreen)
 * 
 * Este programa muestra las capacidades del dispositivo de video TextScreen:
 * - Escritura de texto mediante el puerto de caracteres
 * - Control de posición del cursor
 * - Limpieza de pantalla
 * - Saltos de línea y formato
 * - Auto-scroll cuando se llena la pantalla
 */

void printScreen(const std::shared_ptr<TextScreen>& screen) {
    std::cout << "\n" << std::string(42, '=') << "\n";
    std::cout << screen->getBuffer();
    std::cout << "\n" << std::string(42, '=') << "\n";
}

int main() {
    Mem mem;
    CPU cpu;
    auto textScreen = std::make_shared<TextScreen>();

    // Reiniciar la CPU y registrar el dispositivo de video
    cpu.Reset(mem);
    cpu.registerIODevice(textScreen);

    std::cout << "=== Demostración de TextScreen para CPU 6502 ===\n\n";

    // Demo 1: Escribir texto simple usando el puerto de caracteres
    std::cout << "Demo 1: Escribiendo 'Hello, World!' en la pantalla...\n";
    
    const char* message1 = "Hello, World!";
    uint16_t addr = 0x8000;
    
    for (int i = 0; message1[i] != '\0'; i++) {
        mem[addr++] = 0xA9;  // LDA #char
        mem[addr++] = message1[i];
        mem[addr++] = 0x8D;  // STA $FFFF (puerto de caracteres)
        mem[addr++] = 0xFF;
        mem[addr++] = 0xFF;
    }
    
    cpu.Execute(65, mem);
    printScreen(textScreen);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Demo 2: Posicionar cursor y escribir en diferentes posiciones
    std::cout << "\nDemo 2: Posicionando cursor y escribiendo en diferentes ubicaciones...\n";
    
    // Limpiar pantalla
    textScreen->clear();
    
    // Escribir "CPU 6502" en posición (5, 2)
    addr = 0x8000;
    mem[addr++] = 0xA9;  // LDA #5
    mem[addr++] = 5;
    mem[addr++] = 0x8D;  // STA $FFFC (cursor col)
    mem[addr++] = 0xFC;
    mem[addr++] = 0xFF;
    
    mem[addr++] = 0xA9;  // LDA #2
    mem[addr++] = 2;
    mem[addr++] = 0x8D;  // STA $FFFD (cursor row)
    mem[addr++] = 0xFD;
    mem[addr++] = 0xFF;
    
    const char* message2 = "CPU 6502 Emulator";
    for (int i = 0; message2[i] != '\0'; i++) {
        mem[addr++] = 0xA9;  // LDA #char
        mem[addr++] = message2[i];
        mem[addr++] = 0x8D;  // STA $FFFF
        mem[addr++] = 0xFF;
        mem[addr++] = 0xFF;
    }
    
    cpu.Execute(100, mem);
    
    // Escribir "Video Device Demo" en posición (8, 5)
    addr = 0x8100;
    mem[addr++] = 0xA9;  // LDA #8
    mem[addr++] = 8;
    mem[addr++] = 0x8D;  // STA $FFFC
    mem[addr++] = 0xFC;
    mem[addr++] = 0xFF;
    
    mem[addr++] = 0xA9;  // LDA #5
    mem[addr++] = 5;
    mem[addr++] = 0x8D;  // STA $FFFD
    mem[addr++] = 0xFD;
    mem[addr++] = 0xFF;
    
    const char* message3 = "Video Device Demo";
    for (int i = 0; message3[i] != '\0'; i++) {
        mem[addr++] = 0xA9;  // LDA #char
        mem[addr++] = message3[i];
        mem[addr++] = 0x8D;  // STA $FFFF
        mem[addr++] = 0xFF;
        mem[addr++] = 0xFF;
    }
    
    // Configurar PC a 0x8100 para ejecutar el segundo bloque
    cpu.PC = 0x8100;
    cpu.Execute(100, mem);
    
    printScreen(textScreen);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    // Demo 3: Escribir con saltos de línea
    std::cout << "\nDemo 3: Escribiendo texto con múltiples líneas...\n";
    
    textScreen->clear();
    
    // Escribir un mensaje de varias líneas usando la API C++
    textScreen->writeCharAtCursor('L');
    textScreen->writeCharAtCursor('i');
    textScreen->writeCharAtCursor('n');
    textScreen->writeCharAtCursor('e');
    textScreen->writeCharAtCursor(' ');
    textScreen->writeCharAtCursor('1');
    textScreen->writeCharAtCursor('\n');
    textScreen->writeCharAtCursor('L');
    textScreen->writeCharAtCursor('i');
    textScreen->writeCharAtCursor('n');
    textScreen->writeCharAtCursor('e');
    textScreen->writeCharAtCursor(' ');
    textScreen->writeCharAtCursor('2');
    textScreen->writeCharAtCursor('\n');
    textScreen->writeCharAtCursor('L');
    textScreen->writeCharAtCursor('i');
    textScreen->writeCharAtCursor('n');
    textScreen->writeCharAtCursor('e');
    textScreen->writeCharAtCursor(' ');
    textScreen->writeCharAtCursor('3');
    
    printScreen(textScreen);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    // Demo 4: Demostración de auto-scroll
    std::cout << "\nDemo 4: Demostrando auto-scroll al llenar la pantalla...\n";
    
    textScreen->clear();
    textScreen->setAutoScroll(true);
    
    // Escribir 26 líneas (más que las 24 que caben)
    for (char c = 'A'; c <= 'Z'; c++) {
        for (int i = 0; i < 30; i++) {
            textScreen->writeCharAtCursor(c);
        }
        textScreen->writeCharAtCursor('\n');
    }
    
    printScreen(textScreen);
    std::cout << "\nNota: Las primeras líneas (A, B) han sido desplazadas fuera de la pantalla.\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // Demo 5: Escritura directa en memoria de video
    std::cout << "\nDemo 5: Escritura directa en la memoria de video...\n";
    
    textScreen->clear();
    
    // Escribir directamente en el buffer de video (sin usar el cursor)
    const char* directMsg = "Direct Memory Access!";
    addr = 0x8000;
    
    for (int i = 0; directMsg[i] != '\0'; i++) {
        uint16_t videoAddr = 0xFC00 + 10 * 40 + 10 + i;  // Fila 10, col 10+i
        mem[addr++] = 0xA9;  // LDA #char
        mem[addr++] = directMsg[i];
        mem[addr++] = 0x8D;  // STA videoAddr
        mem[addr++] = videoAddr & 0xFF;
        mem[addr++] = (videoAddr >> 8) & 0xFF;
    }
    
    cpu.PC = 0x8000;
    cpu.Execute(105, mem);
    
    printScreen(textScreen);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    // Demo 6: Crear un marco decorativo
    std::cout << "\nDemo 6: Creando un marco decorativo...\n";
    
    textScreen->clear();
    
    // Dibujar marco superior
    textScreen->setCursorPosition(5, 5);
    for (int i = 0; i < 30; i++) {
        textScreen->writeCharAtCursor(i == 0 ? '+' : (i == 29 ? '+' : '-'));
    }
    
    // Dibujar lados
    for (int row = 6; row < 15; row++) {
        textScreen->setCursorPosition(5, row);
        textScreen->writeCharAtCursor('|');
        textScreen->setCursorPosition(34, row);
        textScreen->writeCharAtCursor('|');
    }
    
    // Dibujar marco inferior
    textScreen->setCursorPosition(5, 15);
    for (int i = 0; i < 30; i++) {
        textScreen->writeCharAtCursor(i == 0 ? '+' : (i == 29 ? '+' : '-'));
    }
    
    // Escribir mensaje en el centro
    textScreen->setCursorPosition(11, 10);
    const char* boxMsg = "Text Screen!";
    for (int i = 0; boxMsg[i] != '\0'; i++) {
        textScreen->writeCharAtCursor(boxMsg[i]);
    }
    
    printScreen(textScreen);

    std::cout << "\n=== Demostración completada ===\n";
    std::cout << "\nCaracterísticas demostradas:\n";
    std::cout << "  - Escritura mediante puerto de caracteres ($FFFF)\n";
    std::cout << "  - Control de posición del cursor ($FFFC, $FFFD)\n";
    std::cout << "  - Limpieza de pantalla (registro de control $FFFE)\n";
    std::cout << "  - Saltos de línea y formato de texto\n";
    std::cout << "  - Auto-scroll automático\n";
    std::cout << "  - Escritura directa en memoria de video ($FC00-$FFFB)\n";
    std::cout << "\nLa pantalla tiene 40 columnas x 24 líneas.\n";

    return 0;
}
