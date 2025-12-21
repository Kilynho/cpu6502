#pragma once
#include "../io_device.hpp"
#include <cstdint>
#include <queue>
#include <string>

/**
 * @brief Peripheral Interface Adapter (PIA) emulation
 * 
 * Emulates the MOS Technology 6821 Peripheral Interface Adapter
 * used in the Apple 1 computer. Provides:
 * - Port A: Keyboard input (KBD) at $D010, Control (KBDCR) at $D011
 * - Port B: Display output (DSP) at $D012, Control (DSPCR) at $D013
 */
class PIA : public IODevice {
public:
    PIA();
    ~PIA();
    
    // IODevice interface
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    
    // Public API
    void pushKeyboardCharacter(char c);
    std::string getDisplayOutput() const;
    void clearDisplayOutput();
    bool hasKeyboardInput() const;
    
private:
    // Memory-mapped addresses
    static constexpr uint16_t KBD = 0xD010;      // Keyboard input
    static constexpr uint16_t KBDCR = 0xD011;    // Keyboard control register
    static constexpr uint16_t DSP = 0xD012;      // Display output
    static constexpr uint16_t DSPCR = 0xD013;    // Display control register
    
    // Internal state
    uint8_t kbdcr_;      // Keyboard control register
    uint8_t dspcr_;      // Display control register
    std::queue<uint8_t> keyboard_buffer_;
    std::string display_output_;
    bool key_ready_;     // Set when key is ready (for D010 bit 7)
    bool display_ready_; // Set when display is ready (for D012 bit 7)
};
