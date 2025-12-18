#pragma once

class PeripheralDevice {
public:
    virtual ~PeripheralDevice() = default;
    virtual void writeRegister(uint16_t address, uint8_t value) = 0;
    virtual uint8_t readRegister(uint16_t address) = 0;
};

