#pragma once
#include <queue>
#include <string>
#include <vector>
#include <cstdint>

// Simula el acceso a la pantalla y teclado del Commodore 64 y WOZMON
class C64IO {
public:
    C64IO();

    // El C64 lee el teclado mediante una matriz (matriz de 8x8)
    // WOZMON usa $FD0C para entrada y $FDED para salida
    bool handlesRead(uint16_t address) const;
    bool handlesWrite(uint16_t address) const;

    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

    void pushInput(char c);
    std::string getScreenBuffer() const;

private:
    std::queue<char> keyboardBuffer;
    std::string screenBuffer;
    std::vector<uint8_t> screenRAM; // 1000-17FF (0x0400-0x07FF)
};
