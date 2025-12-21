/**
 * VIA Port Device (Simplified 6522 VIA)
 * 
 * Emulates basic port functionality for flow control.
 * Only implements PORTA (0x6001) and DDRA (0x6003).
 */

#ifndef VIA_PORT_DEVICE_HPP
#define VIA_PORT_DEVICE_HPP

#include "io_device.hpp"

class VIAPortDevice : public IODevice {
public:
    static constexpr uint16_t PORTA = 0x6001;
    static constexpr uint16_t DDRA  = 0x6003;
    
    VIAPortDevice();
    virtual ~VIAPortDevice() = default;
    
    // IODevice interface
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    
private:
    uint8_t portA;      // Port A data register
    uint8_t ddrA;       // Data Direction Register A (0=input, 1=output)
};

#endif // VIA_PORT_DEVICE_HPP
