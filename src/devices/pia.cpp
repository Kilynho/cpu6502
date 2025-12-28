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
    uint8_t result = 0;
    switch (address) {
        case 0xD010: {  // KBD - Keyboard input
            if (!keyboard_buffer_.empty()) {
                uint8_t c = keyboard_buffer_.front();
                keyboard_buffer_.pop();
                key_ready_ = !keyboard_buffer_.empty();
                result = c;
            } else {
                key_ready_ = false;
                result = 0x00;
            }
            std::cout << "[PIA::read] KBD ($D010) -> 0x" << std::hex << (int)result << std::dec << std::endl;
            return result;
        }
        case 0xD011:  // KBDCR - Keyboard control register
            key_ready_ = !keyboard_buffer_.empty();
            result = kbdcr_ | (key_ready_ ? 0x80 : 0x00);
            std::cout << "[PIA::read] KBDCR ($D011) -> 0x" << std::hex << (int)result << std::dec << std::endl;
            return result;
        case 0xD012: {  // DSP - Display output
            // Return DSP status: bit7 = 1 when display busy, 0 when ready.
            result = dspcr_ | (display_ready_ ? 0x00 : 0x80);
            std::cout << "[PIA::read] DSP ($D012) -> 0x" << std::hex << (int)result << std::dec
                      << " (display_output_size=" << display_output_.size() << ")" << std::endl;
            return result;
        }
        case 0xD013:  // DSPCR - Display control register
            result = dspcr_;
            std::cout << "[PIA::read] DSPCR ($D013) -> 0x" << std::hex << (int)result << std::dec << std::endl;
            return result;
    }
    std::cout << "[PIA::read] Unknown ($" << std::hex << address << ") -> 0x0" << std::dec << std::endl;
    return 0;
}

void PIA::write(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xD010:  // KBD - Not writable
            break;
        
        case 0xD011:  // KBDCR - Keyboard control register
            kbdcr_ = value;
            std::cout << "[PIA::write] KBDCR ($D011) <- 0x" << std::hex << (int)kbdcr_ << std::dec << std::endl;
            break;
        
        case 0xD012: {  // DSP - Display output
            // High bit must be 0 to write
            if ((value & 0x80) == 0) {
                char c = value & 0x7F;
                display_output_ += c;
                // Traza para depuración de salida WOZMON/BASIC
                std::cout << "[PIA::write] OUT: '" << (c >= 32 && c < 127 ? c : '.') << "' (0x" << std::hex << (int)(unsigned char)c << ")" << std::dec << std::endl;
                // Simulate display becoming busy briefly
                display_ready_ = false;
                // For this simple emulator model, mark ready immediately after writing.
                display_ready_ = true;
            }
            break;
        }
        case 0xD013:  // DSPCR - Display control register
            dspcr_ = value;
            std::cout << "[PIA::write] DSPCR ($D013) <- 0x" << std::hex << (int)dspcr_ << std::dec << std::endl;
            break;
    }
}


void PIA::pushKeyboardCharacter(char c) {
    // Set high bit for keyboard input
    keyboard_buffer_.push(static_cast<uint8_t>(c) | 0x80);
    key_ready_ = true;
    std::cout << "[PIA::pushKeyboardCharacter] queued 0x" << std::hex << (int)(static_cast<uint8_t>(c) | 0x80) << std::dec << std::endl;
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
