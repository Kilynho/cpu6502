#include "devices/text_screen.hpp"
#include <algorithm>
#include <cstring>

TextScreen::TextScreen() 
    : videoBuffer(BUFFER_SIZE, ' '),  // Initialize with spaces
      cursorCol(0),
      cursorRow(0),
      controlReg(CTRL_AUTO_SCROLL) {  // Auto-scroll enabled by default
}

bool TextScreen::handlesRead(uint16_t address) const {
    return (address >= VIDEO_RAM_START && address <= CHAR_OUT_ADDR);
}

bool TextScreen::handlesWrite(uint16_t address) const {
    return (address >= VIDEO_RAM_START && address <= CHAR_OUT_ADDR);
}

uint8_t TextScreen::read(uint16_t address) {
    // Reading from the video buffer
    if (address >= VIDEO_RAM_START && address <= VIDEO_RAM_END) {
        uint16_t offset = address - VIDEO_RAM_START;
        if (offset < BUFFER_SIZE) {
            return videoBuffer[offset];
        }
    }
    // Reading control registers
    else if (address == CURSOR_COL_ADDR) {
        return cursorCol;
    }
    else if (address == CURSOR_ROW_ADDR) {
        return cursorRow;
    }
    else if (address == CONTROL_ADDR) {
        return controlReg;
    }
    else if (address == CHAR_OUT_ADDR) {
        // Reading from the character port does nothing, returns 0
        return 0;
    }
    
    return 0;
}

void TextScreen::write(uint16_t address, uint8_t value) {
    // Writing to the video buffer
    if (address >= VIDEO_RAM_START && address <= VIDEO_RAM_END) {
        uint16_t offset = address - VIDEO_RAM_START;
        if (offset < BUFFER_SIZE) {
            videoBuffer[offset] = value;
        }
    }
    // Writing to control registers
    else if (address == CURSOR_COL_ADDR) {
        cursorCol = value % WIDTH;  // Ensure it's in range
    }
    else if (address == CURSOR_ROW_ADDR) {
        cursorRow = value % HEIGHT;  // Ensure it's in range
    }
    else if (address == CONTROL_ADDR) {
        // Check if screen clear is requested
        if (value & CTRL_CLEAR_SCREEN) {
            clear();
            // Clear the bit after execution
            controlReg = value & ~CTRL_CLEAR_SCREEN;
        } else {
            controlReg = value;
        }
    }
    else if (address == CHAR_OUT_ADDR) {
        // Writing to the character port
        writeCharAtCursor(value);
    }
}

void TextScreen::refresh() {
    // In a real implementation with GUI (SDL/OpenGL), the window would be updated here
    // For now, this function does nothing (the screen is always "synchronized")
}

void TextScreen::clear() {
    std::fill(videoBuffer.begin(), videoBuffer.end(), ' ');
    cursorCol = 0;
    cursorRow = 0;
}

std::string TextScreen::getBuffer() const {
    std::string result;
    result.reserve(BUFFER_SIZE + HEIGHT);  // Space for characters + newlines
    
    for (uint16_t row = 0; row < HEIGHT; ++row) {
        for (uint16_t col = 0; col < WIDTH; ++col) {
            uint16_t offset = row * WIDTH + col;
            char c = videoBuffer[offset];
            // Convert non-printable characters to spaces
            if (c < 0x20 || c > 0x7E) {
                result += ' ';
            } else {
                result += c;
            }
        }
        if (row < HEIGHT - 1) {
            result += '\n';
        }
    }
    
    return result;
}

uint16_t TextScreen::getWidth() const {
    return WIDTH;
}

uint16_t TextScreen::getHeight() const {
    return HEIGHT;
}

void TextScreen::setCursorPosition(uint8_t col, uint8_t row) {
    cursorCol = col % WIDTH;
    cursorRow = row % HEIGHT;
}

void TextScreen::getCursorPosition(uint8_t& col, uint8_t& row) const {
    col = cursorCol;
    row = cursorRow;
}

void TextScreen::writeCharAtCursor(char c) {
    processCharacter(c);
}

void TextScreen::setAutoScroll(bool enabled) {
    if (enabled) {
        controlReg |= CTRL_AUTO_SCROLL;
    } else {
        controlReg &= ~CTRL_AUTO_SCROLL;
    }
}

bool TextScreen::getAutoScroll() const {
    return (controlReg & CTRL_AUTO_SCROLL) != 0;
}

void TextScreen::scrollUp() {
    // Move all lines up by one
    for (uint16_t row = 0; row < HEIGHT - 1; ++row) {
        for (uint16_t col = 0; col < WIDTH; ++col) {
            uint16_t dstOffset = row * WIDTH + col;
            uint16_t srcOffset = (row + 1) * WIDTH + col;
            videoBuffer[dstOffset] = videoBuffer[srcOffset];
        }
    }
    
    // Clear the last line
    uint16_t lastLineOffset = (HEIGHT - 1) * WIDTH;
    std::fill(videoBuffer.begin() + lastLineOffset, 
              videoBuffer.begin() + lastLineOffset + WIDTH, 
              ' ');
    
    // Move cursor to the start of the last line
    cursorCol = 0;
    cursorRow = HEIGHT - 1;
}

void TextScreen::advanceCursor() {
    cursorCol++;
    if (cursorCol >= WIDTH) {
        cursorCol = 0;
        cursorRow++;
        if (cursorRow >= HEIGHT) {
            if (controlReg & CTRL_AUTO_SCROLL) {
                scrollUp();
            } else {
                // If no auto-scroll, go back to the start
                cursorRow = 0;
            }
        }
    }
}

void TextScreen::processCharacter(char c) {
    // Process special characters
    if (c == '\n') {
        // New line: go to the start of the next line
        cursorCol = 0;
        cursorRow++;
        if (cursorRow >= HEIGHT) {
            if (controlReg & CTRL_AUTO_SCROLL) {
                scrollUp();
            } else {
                cursorRow = 0;
            }
        }
        return;
    } else if (c == '\r') {
        // Carriage return: go to the start of the current line
        cursorCol = 0;
        return;
    } else if (c == '\t') {
        // Tab: advance to the next multiple of 8
        uint8_t nextTab = ((cursorCol / 8) + 1) * 8;
        while (cursorCol < nextTab && cursorCol < WIDTH) {
            uint16_t offset = getBufferOffset(cursorCol, cursorRow);
            videoBuffer[offset] = ' ';
            advanceCursor();
        }
        return;
    } else if (c == '\b') {
        // Backspace: move back one position (without erasing)
        if (cursorCol > 0) {
            cursorCol--;
        } else if (cursorRow > 0) {
            cursorRow--;
            cursorCol = WIDTH - 1;
        }
        return;
    }
    
    // Write printable character at the cursor position
    if (c >= 0x20 && c <= 0x7E) {
        uint16_t offset = getBufferOffset(cursorCol, cursorRow);
        videoBuffer[offset] = c;
        advanceCursor();
    }
    // Ignore non-printable characters (except for those already processed)
}

uint16_t TextScreen::getBufferOffset(uint8_t col, uint8_t row) const {
    return row * WIDTH + col;
}
