#pragma once
#include "devices/video_device.hpp"
#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Implementation of a text screen for the 6502 emulator
 *
 * TextScreen simulates a 40-column x 24-row text screen,
 * similar to classic 8-bit computers (Apple II, Commodore 64, etc.)
 *
 * Memory-mapped addresses:
 * - 0xFC00-0xFFFB: Video buffer (40x24 = 960 bytes)
 *   * Each byte represents an ASCII character
 *   * Organized by rows: row 0 = 0xFC00-0xFC27, row 1 = 0xFC28-0xFC4F, etc.
 * - 0xFFFC: Cursor column (0-39)
 * - 0xFFFD: Cursor row (0-23)
 * - 0xFFFE: Screen control:
 *   * Bit 0: Auto-scroll (1=enabled, 0=disabled)
 *   * Bit 1: Clear screen (writing 1 clears the screen)
 *   * Bit 7: Cursor visible (1=visible, 0=invisible)
 * - 0xFFFF: Character write port (writing here prints at cursor position)
 * 
 * Features:
 * - Support for printable ASCII (0x20-0x7E)
 * - Auto-scroll when the screen is full
 * - Cursor control
 * - Screen clearing
 */
class TextScreen : public VideoDevice {
public:
    TextScreen();
    
    // IODevice implementation
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    
    // VideoDevice implementation
    void refresh() override;
    void clear() override;
    std::string getBuffer() const override;
    uint16_t getWidth() const override;
    uint16_t getHeight() const override;
    
    // Additional control methods
    void setCursorPosition(uint8_t col, uint8_t row);
    void getCursorPosition(uint8_t& col, uint8_t& row) const;
    void writeCharAtCursor(char c);
    void setAutoScroll(bool enabled);
    bool getAutoScroll() const;
    
private:
    // Screen constants
    static constexpr uint16_t WIDTH = 40;
    static constexpr uint16_t HEIGHT = 24;
    static constexpr uint16_t BUFFER_SIZE = WIDTH * HEIGHT;
    
    // Memory-mapped address ranges
    static constexpr uint16_t VIDEO_RAM_START = 0xFC00;  // Start of video buffer
    static constexpr uint16_t VIDEO_RAM_END = 0xFFFB;    // End of video buffer
    static constexpr uint16_t CURSOR_COL_ADDR = 0xFFFC;  // Cursor column register
    static constexpr uint16_t CURSOR_ROW_ADDR = 0xFFFD;  // Cursor row register
    static constexpr uint16_t CONTROL_ADDR = 0xFFFE;     // Control register
    static constexpr uint16_t CHAR_OUT_ADDR = 0xFFFF;    // Character output port
    
    // Control register bits
    static constexpr uint8_t CTRL_AUTO_SCROLL = 0x01;    // Bit 0: Auto-scroll
    static constexpr uint8_t CTRL_CLEAR_SCREEN = 0x02;   // Bit 1: Clear screen
    static constexpr uint8_t CTRL_CURSOR_VISIBLE = 0x80; // Bit 7: Cursor visible
    
    // Internal state
    std::vector<uint8_t> videoBuffer;  // Video buffer (40x24)
    uint8_t cursorCol;                 // Cursor column (0-39)
    uint8_t cursorRow;                 // Cursor row (0-23)
    uint8_t controlReg;                // Control register
    
    // Private methods
    void scrollUp();                   // Scrolls content up one line
    void advanceCursor();              // Advances the cursor by one position
    void processCharacter(char c);     // Processes a character (including \n, \r, etc.)
    uint16_t getBufferOffset(uint8_t col, uint8_t row) const; // Calculates buffer offset
};
