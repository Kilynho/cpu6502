#include "cpu.hpp"
#include "mem.hpp"
#include "apple_io.hpp"
#include <memory>
#include <iostream>

// Ejemplo de uso de AppleIO para simular E/S Apple II
// Programa que lee una tecla de $FD0C y la escribe en $FDED

int main() {

    SystemMap bus;
    CPU cpu;
    auto appleIO = std::make_shared<AppleIO>();
    cpu.registerIODevice(appleIO);


    // Simular entrada de teclado
    appleIO->pushInput('H');
    appleIO->pushInput('e');
    appleIO->pushInput('l');
    appleIO->pushInput('l');
    appleIO->pushInput('o');

    // Programa: leer 5 caracteres de $FD0C y escribirlos en $FDED
    // Loop 5 veces: LDA $FD0C, STA $FDED
    bus.write(0x8000, 0xA2);          // LDX #5 (contador de loop)
    bus.write(0x8001, 0x05);
    bus.write(0x8002, 0xAD);          // LDA $FD0C (leer teclado)
    bus.write(0x8003, 0x0C);
    bus.write(0x8004, 0xFD);
    bus.write(0x8005, 0x8D);          // STA $FDED (escribir pantalla)
    bus.write(0x8006, 0xED);
    bus.write(0x8007, 0xFD);
    bus.write(0x8008, 0xCA);          // DEX (decrementar X)
    bus.write(0x8009, 0xD0);          // BNE loop (si X != 0, volver a $8002)
    bus.write(0x800A, 0xF7);          // offset: -10 bytes

    // Configurar vector de reset
    bus.write(0xFFFC, 0x00);
    bus.write(0xFFFD, 0x80);

    std::cout << "Ejecutando demo de Apple IO...\n";
    std::cout << "Salida esperada: Hello\n";
    std::cout << "Salida real: ";

    // Configurar CPU registers
    cpu.PC = (bus.read(0xFFFC)) | (bus.read(0xFFFD) << 8);
    cpu.SP = 0xFD;
    // Ejecutar (aproximadamente 50 ciclos para el loop completo)
    cpu.Execute(50, bus);

    std::cout << "\n\nBuffer de pantalla capturado: " << appleIO->getScreenBuffer() << "\n";

    return 0;
}
