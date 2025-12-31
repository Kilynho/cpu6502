#include "c64_io.hpp"
#include <iostream>

#define C64_SCREEN_ADDR_START 0x0400
#define C64_SCREEN_ADDR_END   0x07FF
#define C64_KEYBOARD_MATRIX_BASE 0xDC00
#define WOZMON_KBD_IN 0xFD0C      // WOZMON keyboard input
#define WOZMON_CHAR_OUT 0xFDED    // WOZMON character output

C64IO::C64IO() : screenRAM(1024, 32) {} // 32 = espacio en PETSCII

// Indica si la dirección corresponde a lectura de teclado (matriz simplificada)
bool C64IO::handlesRead(uint16_t address) const {
    // El teclado se lee en $DC00-$DC01 (matriz de teclado) o $FD0C (WOZMON)
    return (address == C64_KEYBOARD_MATRIX_BASE || address == C64_KEYBOARD_MATRIX_BASE + 1 || address == WOZMON_KBD_IN);
}

// Indica si la dirección corresponde a escritura en pantalla
bool C64IO::handlesWrite(uint16_t address) const {
    // La pantalla está en $0400-$07FF o $FDED (WOZMON output)
    return (address >= C64_SCREEN_ADDR_START && address <= C64_SCREEN_ADDR_END) || (address == WOZMON_CHAR_OUT);
}

// Lee un byte del teclado
// Retorna carácter con bit 7 claro si disponible
// Retorna 0x00 si no disponible (permitiendo que CHRIN continue esperando)
uint8_t C64IO::read(uint16_t address) {
    if (address == C64_KEYBOARD_MATRIX_BASE || address == WOZMON_KBD_IN) {
        if (!keyboardBuffer.empty()) {
            char c = keyboardBuffer.front();
            keyboardBuffer.pop();
            // Return character with bit 7 clear to indicate character ready
            return static_cast<uint8_t>(c) & 0x7F;
        }
        // No character ready - return a value with bit7 set so CHRIN will loop
        // CHRIN checks 'bmi CHRIN' (branch if negative), so a negative value
        // indicates 'no character yet'. Use 0x80 as a sentinel.
        return 0x80;
    }
    return 0xFF;
}

// Escribe un byte en la pantalla (lo agrega al buffer y lo imprime por consola)
void C64IO::write(uint16_t address, uint8_t value) {
    if (address >= C64_SCREEN_ADDR_START && address <= C64_SCREEN_ADDR_END) {
        size_t pos = address - C64_SCREEN_ADDR_START;
        screenRAM[pos] = value;
        // Para mostrar en consola, convertimos PETSCII a ASCII básico
        char out = (value >= 32 && value < 128) ? value : '?';
        screenBuffer += out;
        std::cout << out;
    } else if (address == WOZMON_CHAR_OUT) {
        // WOZMON character output - display directly
        char out = static_cast<char>(value);
        screenBuffer += out;
        std::cout << out << std::flush;
    }
}

// Inserta un carácter en el buffer de teclado (simula pulsación de tecla)
void C64IO::pushInput(char c) {
    keyboardBuffer.push(c);
}

// Devuelve el contenido actual de la pantalla simulada
std::string C64IO::getScreenBuffer() const {
    return screenBuffer;
}
