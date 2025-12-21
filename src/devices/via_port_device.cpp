/**
 * VIA Port Device Implementation
 */

#include "devices/via_port_device.hpp"

VIAPortDevice::VIAPortDevice() 
    : portA(0xFF)  // Default high (no flow control asserted)
    , ddrA(0x00)   // Default all inputs
{
}

bool VIAPortDevice::handlesRead(uint16_t address) const {
    return address == PORTA || address == DDRA;
}

bool VIAPortDevice::handlesWrite(uint16_t address) const {
    return address == PORTA || address == DDRA;
}

uint8_t VIAPortDevice::read(uint16_t address) {
    if (address == PORTA) {
        return portA;
    } else if (address == DDRA) {
        return ddrA;
    }
    return 0x00;
}

void VIAPortDevice::write(uint16_t address, uint8_t value) {
    if (address == PORTA) {
        // Only write bits configured as outputs
        portA = (portA & ~ddrA) | (value & ddrA);
    } else if (address == DDRA) {
        ddrA = value;
    }
}
