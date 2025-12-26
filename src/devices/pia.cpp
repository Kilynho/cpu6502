#include "devices/pia.hpp"
#include <iostream>

PIA::PIA()
    : kbdcr_(0), dspcr_(0), key_ready_(false), display_ready_(true) {}

PIA::~PIA() = default;

bool PIA::handlesRead(uint16_t address) const {
    return address >= 0xD010 && address <= 0xD013;
}

bool PIA::handlesWrite(uint16_t address) const {
    return address >= 0xD010 && address <= 0xD013;
}

uint8_t PIA::read(uint16_t address) {
    static int read_count = 0;
    switch (address) {
        case 0xD010: {  // KBD - Keyboard input
            // ...existing code...
            // Bit 7: Key ready flag (should always be 1 for valid char)
            // Bits 0-6: ASCII character with high bit set
            if (!keyboard_buffer_.empty()) {
                uint8_t c = keyboard_buffer_.front();
                keyboard_buffer_.pop();
                key_ready_ = !keyboard_buffer_.empty(); // Update after reading
                // ...existing code...
                return c; // Character already has bit 7 set
            }
            key_ready_ = false;
            return 0x00; // No key available
        }
        case 0xD011:  // KBDCR - Keyboard control register
            // ...existing code...
            // Check if there's a key available (before consuming it)
            key_ready_ = !keyboard_buffer_.empty();
            return kbdcr_ | (key_ready_ ? 0x80 : 0x00);
        
        case 0xD012: {  // DSP - Display output
            // Bit 7: Display ready (always 1 in this emulation)
            return 0x80;
        }
        case 0xD013:  // DSPCR - Display control register
            return dspcr_;
    }
    return 0;
}

void PIA::write(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xD010:  // KBD - Not writable
            break;
        
        case 0xD011:  // KBDCR - Keyboard control register
            kbdcr_ = value;
            break;
        
        case 0xD012: {  // DSP - Display output
            // High bit must be 0 to write
            if ((value & 0x80) == 0) {
                char c = value & 0x7F;
                display_output_ += c;
                // Echo to console
                if (c == '\r' || c == '\n') {
                    // Keep line endings for display
                } else if (c >= 32 && c < 127) {
                    // Printable ASCII
                }
            }
            break;
        }
        case 0xD013:  // DSPCR - Display control register
            dspcr_ = value;
            break;
    }
}


void PIA::pushKeyboardCharacter(char c) {
    // Set high bit for keyboard input
    keyboard_buffer_.push(static_cast<uint8_t>(c) | 0x80);
    key_ready_ = true;
}

std::string PIA::getDisplayOutput() const {
    return display_output_;
}

void PIA::clearDisplayOutput() {
    display_output_.clear();
}

bool PIA::hasKeyboardInput() const {
    return !keyboard_buffer_.empty();
}
