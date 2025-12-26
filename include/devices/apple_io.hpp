#pragma once
#include "devices/io_device.hpp"
#include <queue>
#include <string>

class AppleIO : public IODevice {
public:
    AppleIO();
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    void pushInput(char c); // Para simular entrada de teclado
    std::string getScreenBuffer() const; // Para tests
private:
    std::queue<char> keyboardBuffer;
    std::string screenBuffer;
};
