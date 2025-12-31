#pragma once
#include <cstdint>
#include <vector>

class IODevice {
public:
    virtual ~IODevice() = default;
    virtual bool handlesRead(uint16_t address) const = 0;
    virtual bool handlesWrite(uint16_t address) const = 0;
    virtual uint8_t read(uint16_t address) = 0;
    virtual void write(uint16_t address, uint8_t value) = 0;
};
