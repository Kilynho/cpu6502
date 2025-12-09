#include "apple_io.hpp"
#include <iostream>

#define APPLE_KBD_ADDR 0xFD0C
#define APPLE_SCREEN_ADDR 0xFDED

AppleIO::AppleIO() {}

bool AppleIO::handlesRead(uint16_t address) const {
    return address == APPLE_KBD_ADDR;
}

bool AppleIO::handlesWrite(uint16_t address) const {
    return address == APPLE_SCREEN_ADDR;
}

uint8_t AppleIO::read(uint16_t address) {
    if (address == APPLE_KBD_ADDR) {
        if (!keyboardBuffer.empty()) {
            char c = keyboardBuffer.front();
            keyboardBuffer.pop();
            return static_cast<uint8_t>(c);
        }
        return 0;
    }
    return 0;
}

void AppleIO::write(uint16_t address, uint8_t value) {
    if (address == APPLE_SCREEN_ADDR) {
        screenBuffer += static_cast<char>(value);
        std::cout << static_cast<char>(value);
    }
}

void AppleIO::pushInput(char c) {
    keyboardBuffer.push(c);
}

std::string AppleIO::getScreenBuffer() const {
    return screenBuffer;
}
