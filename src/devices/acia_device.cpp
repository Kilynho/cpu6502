/**
 * ACIA Device Implementation
 */

#include "devices/acia_device.hpp"
#include <iostream>

ACIADevice::ACIADevice() 
    : statusRegister(STATUS_TX_DATA_EMPTY | STATUS_DSR)  // Ready to transmit, DSR active
    , commandRegister(0)
    , controlRegister(0)
{
}

bool ACIADevice::handlesRead(uint16_t address) const {
    return address == ACIA_DATA || address == ACIA_STATUS;
}

bool ACIADevice::handlesWrite(uint16_t address) const {
    return address == ACIA_DATA || address == ACIA_CMD || address == ACIA_CTRL;
}

uint8_t ACIADevice::read(uint16_t address) {
    if (address == ACIA_DATA) {
        // Read data register
        if (!rxBuffer.empty()) {
            uint8_t data = rxBuffer.front();
            rxBuffer.pop();
            updateStatus();
            return data;
        }
        return 0x00;  // No data available
        
    } else if (address == ACIA_STATUS) {
        // Read status register
        return statusRegister;
    }
    
    return 0x00;
}

void ACIADevice::write(uint16_t address, uint8_t value) {
    if (address == ACIA_DATA) {
        // Write data register (transmit)
        txBuffer += static_cast<char>(value);
        
        // Echo to console for debugging
        std::cout << static_cast<char>(value) << std::flush;
        
        // Transmission is instantaneous in emulation
        // Status remains TX_DATA_EMPTY
        
    } else if (address == ACIA_CMD) {
        // Write command register
        commandRegister = value;
        
    } else if (address == ACIA_CTRL) {
        // Write control register
        controlRegister = value;
        
        // Handle soft reset (bit 0-1 = 00)
        if ((value & 0x03) == 0x00) {
            // Soft reset requested
            // Clear buffers and reset status
            while (!rxBuffer.empty()) rxBuffer.pop();
            txBuffer.clear();
            statusRegister = STATUS_TX_DATA_EMPTY | STATUS_DSR;
        }
    }
}

void ACIADevice::pushInput(char c) {
    rxBuffer.push(static_cast<uint8_t>(c));
    updateStatus();
}

void ACIADevice::pushInput(const std::string& str) {
    for (char c : str) {
        rxBuffer.push(static_cast<uint8_t>(c));
    }
    updateStatus();
}

std::string ACIADevice::getOutput() const {
    return txBuffer;
}

void ACIADevice::clearOutput() {
    txBuffer.clear();
}

void ACIADevice::updateStatus() {
    // Update RX_DATA_READY flag based on buffer state
    if (!rxBuffer.empty()) {
        statusRegister |= STATUS_RX_DATA_READY;
    } else {
        statusRegister &= ~STATUS_RX_DATA_READY;
    }
    
    // TX_DATA_EMPTY is always set (instant transmission)
    statusRegister |= STATUS_TX_DATA_EMPTY;
}
