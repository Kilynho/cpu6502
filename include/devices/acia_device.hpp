/**
 * ACIA Device (6551 Compatible)
 * 
 * Emulates a 6551 ACIA (Asynchronous Communications Interface Adapter)
 * for serial I/O as used in Ben Eater's 6502 computer.
 * 
 * Memory Map:
 * - 0x5000: ACIA_DATA    - Data register (read/write)
 * - 0x5001: ACIA_STATUS  - Status register (read only)
 * - 0x5002: ACIA_CMD     - Command register (write only)
 * - 0x5003: ACIA_CTRL    - Control register (write only)
 */

#ifndef ACIA_DEVICE_HPP
#define ACIA_DEVICE_HPP

#include "io_device.hpp"
#include <queue>
#include <string>

class ACIADevice : public IODevice {
public:
    static constexpr uint16_t ACIA_DATA   = 0x5000;
    static constexpr uint16_t ACIA_STATUS = 0x5001;
    static constexpr uint16_t ACIA_CMD    = 0x5002;
    static constexpr uint16_t ACIA_CTRL   = 0x5003;
    
    // Status register bits
    static constexpr uint8_t STATUS_PARITY_ERROR   = 0b00000001;
    static constexpr uint8_t STATUS_FRAMING_ERROR  = 0b00000010;
    static constexpr uint8_t STATUS_OVERRUN        = 0b00000100;
    static constexpr uint8_t STATUS_RX_DATA_READY  = 0b00001000;  // Data available to read
    static constexpr uint8_t STATUS_TX_DATA_EMPTY  = 0b00010000;  // Ready to transmit
    static constexpr uint8_t STATUS_DCD            = 0b00100000;  // Data Carrier Detect
    static constexpr uint8_t STATUS_DSR            = 0b01000000;  // Data Set Ready
    static constexpr uint8_t STATUS_IRQ            = 0b10000000;  // Interrupt Request
    
    ACIADevice();
    virtual ~ACIADevice() = default;
    
    // IODevice interface
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    
    // Input/Output management
    void pushInput(char c);
    void pushInput(const std::string& str);
    std::string getOutput() const;
    void clearOutput();
    
private:
    std::queue<uint8_t> rxBuffer;  // Receive buffer (input to CPU)
    std::string txBuffer;          // Transmit buffer (output from CPU)
    
    uint8_t statusRegister;
    uint8_t commandRegister;
    uint8_t controlRegister;
    
    void updateStatus();
};

#endif // ACIA_DEVICE_HPP
