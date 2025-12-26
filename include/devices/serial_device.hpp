#pragma once
#include "devices/io_device.hpp"
#include <cstdint>
#include <string>

/**
 * @brief Base interface for serial devices
 *
 * This interface defines the basic operations that serial devices must implement
 * (UART, ACIA 6551, TCP/IP, etc.)
 */
class SerialDevice : public IODevice {
public:
    virtual ~SerialDevice() = default;
    
    /**
     * @brief Initializes the serial device
     * @return true if successfully initialized, false otherwise
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Connects the serial device to an address/port
     * @param address Address or port to connect (format depends on implementation)
     * @return true if successfully connected, false otherwise
     */
    virtual bool connect(const std::string& address) = 0;
    
    /**
     * @brief Disconnects the serial device
     */
    virtual void disconnect() = 0;
    
    /**
     * @brief Checks if data is available to read
     * @return true if data is available, false otherwise
     */
    virtual bool dataAvailable() const = 0;
    
    /**
     * @brief Reads a byte from the serial device
     * @return Byte read (0 if no data available)
     */
    virtual uint8_t receiveByte() = 0;
    
    /**
     * @brief Sends a byte through the serial device
     * @param data Byte to send
     * @return true if sent successfully, false otherwise
     */
    virtual bool transmitByte(uint8_t data) = 0;
    
    /**
     * @brief Checks if the device is connected
     * @return true if connected, false otherwise
     */
    virtual bool isConnected() const = 0;
    
    /**
     * @brief Cleans up the serial system and frees resources
     */
    virtual void cleanup() = 0;
};
