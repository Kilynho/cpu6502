#include "devices/text_screen.hpp"
#include <algorithm>
#include <cstring>

TextScreen::TextScreen() 
    : videoBuffer(BUFFER_SIZE, ' '),  // Inicializar con espacios
      cursorCol(0),
      cursorRow(0),
      controlReg(CTRL_AUTO_SCROLL) {  // Auto-scroll habilitado por defecto
}

bool TextScreen::handlesRead(uint16_t address) const {
    return (address >= VIDEO_RAM_START && address <= CHAR_OUT_ADDR);
}

bool TextScreen::handlesWrite(uint16_t address) const {
    return (address >= VIDEO_RAM_START && address <= CHAR_OUT_ADDR);
}

uint8_t TextScreen::read(uint16_t address) {
    // Lectura del buffer de video
    if (address >= VIDEO_RAM_START && address <= VIDEO_RAM_END) {
        uint16_t offset = address - VIDEO_RAM_START;
        if (offset < BUFFER_SIZE) {
            return videoBuffer[offset];
        }
    }
    // Lectura de registros de control
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
        // Lectura del puerto de caracteres no hace nada, devuelve 0
        return 0;
    }
    
    return 0;
}

void TextScreen::write(uint16_t address, uint8_t value) {
    // Escritura en el buffer de video
    if (address >= VIDEO_RAM_START && address <= VIDEO_RAM_END) {
        uint16_t offset = address - VIDEO_RAM_START;
        if (offset < BUFFER_SIZE) {
            videoBuffer[offset] = value;
        }
    }
    // Escritura en registros de control
    else if (address == CURSOR_COL_ADDR) {
        cursorCol = value % WIDTH;  // Asegurar que está en rango
    }
    else if (address == CURSOR_ROW_ADDR) {
        cursorRow = value % HEIGHT;  // Asegurar que está en rango
    }
    else if (address == CONTROL_ADDR) {
        // Verificar si se solicita limpiar la pantalla
        if (value & CTRL_CLEAR_SCREEN) {
            clear();
            // Limpiar el bit de clear después de ejecutar
            controlReg = value & ~CTRL_CLEAR_SCREEN;
        } else {
            controlReg = value;
        }
    }
    else if (address == CHAR_OUT_ADDR) {
        // Escritura en el puerto de caracteres
        writeCharAtCursor(value);
    }
}

void TextScreen::refresh() {
    // En una implementación real con GUI (SDL/OpenGL), aquí se actualizaría la ventana
    // Por ahora, esta función no hace nada (la pantalla está siempre "sincronizada")
}

void TextScreen::clear() {
    std::fill(videoBuffer.begin(), videoBuffer.end(), ' ');
    cursorCol = 0;
    cursorRow = 0;
}

std::string TextScreen::getBuffer() const {
    std::string result;
    result.reserve(BUFFER_SIZE + HEIGHT);  // Espacio para caracteres + saltos de línea
    
    for (uint16_t row = 0; row < HEIGHT; ++row) {
        for (uint16_t col = 0; col < WIDTH; ++col) {
            uint16_t offset = row * WIDTH + col;
            char c = videoBuffer[offset];
            // Convertir caracteres no imprimibles a espacios
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
    // Mover todas las líneas una posición hacia arriba
    for (uint16_t row = 0; row < HEIGHT - 1; ++row) {
        for (uint16_t col = 0; col < WIDTH; ++col) {
            uint16_t dstOffset = row * WIDTH + col;
            uint16_t srcOffset = (row + 1) * WIDTH + col;
            videoBuffer[dstOffset] = videoBuffer[srcOffset];
        }
    }
    
    // Limpiar la última línea
    uint16_t lastLineOffset = (HEIGHT - 1) * WIDTH;
    std::fill(videoBuffer.begin() + lastLineOffset, 
              videoBuffer.begin() + lastLineOffset + WIDTH, 
              ' ');
    
    // Mover cursor al inicio de la última línea
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
                // Si no hay auto-scroll, volver al inicio
                cursorRow = 0;
            }
        }
    }
}

void TextScreen::processCharacter(char c) {
    // Procesar caracteres especiales
    if (c == '\n') {
        // Nueva línea: ir al inicio de la siguiente línea
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
        // Retorno de carro: ir al inicio de la línea actual
        cursorCol = 0;
        return;
    } else if (c == '\t') {
        // Tab: avanzar a la siguiente posición múltiplo de 8
        uint8_t nextTab = ((cursorCol / 8) + 1) * 8;
        while (cursorCol < nextTab && cursorCol < WIDTH) {
            uint16_t offset = getBufferOffset(cursorCol, cursorRow);
            videoBuffer[offset] = ' ';
            advanceCursor();
        }
        return;
    } else if (c == '\b') {
        // Backspace: retroceder una posición (sin borrar)
        if (cursorCol > 0) {
            cursorCol--;
        } else if (cursorRow > 0) {
            cursorRow--;
            cursorCol = WIDTH - 1;
        }
        return;
    }
    
    // Escribir carácter imprimible en la posición del cursor
    if (c >= 0x20 && c <= 0x7E) {
        uint16_t offset = getBufferOffset(cursorCol, cursorRow);
        videoBuffer[offset] = c;
        advanceCursor();
    }
    // Ignorar caracteres no imprimibles (excepto los ya procesados)
}

uint16_t TextScreen::getBufferOffset(uint8_t col, uint8_t row) const {
    return row * WIDTH + col;
}
