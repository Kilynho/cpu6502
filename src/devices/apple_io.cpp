
#include "apple_io.hpp"
#include <iostream>

// AppleIO simula el teclado y la pantalla de un Apple II básico.
// Expone dos direcciones: una para leer teclado y otra para escribir en pantalla.

#define APPLE_KBD_ADDR 0xFD0C
#define APPLE_SCREEN_ADDR 0xFDED


// Constructor vacío
AppleIO::AppleIO() {}

// Indica si la dirección corresponde a lectura de teclado
bool AppleIO::handlesRead(uint16_t address) const {
    return address == APPLE_KBD_ADDR;
}

// Indica si la dirección corresponde a escritura en pantalla
bool AppleIO::handlesWrite(uint16_t address) const {
    return address == APPLE_SCREEN_ADDR;
}

// Lee un byte del teclado (si hay datos en el buffer), si no retorna 0
uint8_t AppleIO::read(uint16_t address) {
    if (address == APPLE_KBD_ADDR) {
        if (!keyboardBuffer.empty()) {
            char c = keyboardBuffer.front();
            keyboardBuffer.pop();
            return static_cast<uint8_t>(c);
        }
        // Si no hay tecla, retorna 0
        return 0;
    }
    // Dirección no válida para este dispositivo
    return 0;
}

// Escribe un byte en la pantalla (lo agrega al buffer y lo imprime por consola)
void AppleIO::write(uint16_t address, uint8_t value) {
    if (address == APPLE_SCREEN_ADDR) {
        screenBuffer += static_cast<char>(value);
        std::cout << static_cast<char>(value);
    }
}

// Inserta un carácter en el buffer de teclado (simula pulsación de tecla)
void AppleIO::pushInput(char c) {
    keyboardBuffer.push(c);
}

// Devuelve el contenido actual de la pantalla simulada
std::string AppleIO::getScreenBuffer() const {
    return screenBuffer;
}
