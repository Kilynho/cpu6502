#pragma once
#include "peripheral_device.hpp"

class VIADevice : public PeripheralDevice {
public:
    void writeRegister(uint16_t address, uint8_t value) override;
    uint8_t readRegister(uint16_t address) override;
};
