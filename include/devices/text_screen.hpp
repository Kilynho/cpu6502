#pragma once
#include "../video_device.hpp"
#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Implementación de una pantalla de texto para el emulador 6502
 * 
 * TextScreen simula una pantalla de texto de 40 columnas x 24 líneas,
 * similar a las computadoras clásicas de 8 bits (Apple II, Commodore 64, etc.)
 * 
 * Direcciones mapeadas en memoria:
 * - 0xFC00-0xFFFB: Buffer de video (40x24 = 960 bytes)
 *   * Cada byte representa un carácter ASCII
 *   * Organizado en filas: fila 0 = 0xFC00-0xFC27, fila 1 = 0xFC28-0xFC4F, etc.
 * - 0xFFFC: Columna del cursor (0-39)
 * - 0xFFFD: Fila del cursor (0-23)
 * - 0xFFFE: Control de pantalla:
 *   * Bit 0: Auto-scroll (1=habilitado, 0=deshabilitado)
 *   * Bit 1: Clear screen (escribir 1 limpia la pantalla)
 *   * Bit 7: Cursor visible (1=visible, 0=invisible)
 * - 0xFFFF: Puerto de escritura de carácter (escribir aquí imprime en posición del cursor)
 * 
 * Características:
 * - Soporte para ASCII imprimible (0x20-0x7E)
 * - Auto-scroll cuando se llena la pantalla
 * - Control de cursor
 * - Limpieza de pantalla
 */
class TextScreen : public VideoDevice {
public:
    TextScreen();
    
    // Implementación de IODevice
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    
    // Implementación de VideoDevice
    void refresh() override;
    void clear() override;
    std::string getBuffer() const override;
    uint16_t getWidth() const override;
    uint16_t getHeight() const override;
    
    // Métodos adicionales para control
    void setCursorPosition(uint8_t col, uint8_t row);
    void getCursorPosition(uint8_t& col, uint8_t& row) const;
    void writeCharAtCursor(char c);
    void setAutoScroll(bool enabled);
    bool getAutoScroll() const;
    
private:
    // Constantes de pantalla
    static constexpr uint16_t WIDTH = 40;
    static constexpr uint16_t HEIGHT = 24;
    static constexpr uint16_t BUFFER_SIZE = WIDTH * HEIGHT;
    
    // Direcciones de memoria mapeada
    static constexpr uint16_t VIDEO_RAM_START = 0xFC00;  // Inicio del buffer de video
    static constexpr uint16_t VIDEO_RAM_END = 0xFFFB;    // Fin del buffer de video
    static constexpr uint16_t CURSOR_COL_ADDR = 0xFFFC;  // Registro de columna del cursor
    static constexpr uint16_t CURSOR_ROW_ADDR = 0xFFFD;  // Registro de fila del cursor
    static constexpr uint16_t CONTROL_ADDR = 0xFFFE;     // Registro de control
    static constexpr uint16_t CHAR_OUT_ADDR = 0xFFFF;    // Puerto de salida de caracteres
    
    // Bits del registro de control
    static constexpr uint8_t CTRL_AUTO_SCROLL = 0x01;    // Bit 0: Auto-scroll
    static constexpr uint8_t CTRL_CLEAR_SCREEN = 0x02;   // Bit 1: Limpiar pantalla
    static constexpr uint8_t CTRL_CURSOR_VISIBLE = 0x80; // Bit 7: Cursor visible
    
    // Estado interno
    std::vector<uint8_t> videoBuffer;  // Buffer de video (40x24)
    uint8_t cursorCol;                 // Columna del cursor (0-39)
    uint8_t cursorRow;                 // Fila del cursor (0-23)
    uint8_t controlReg;                // Registro de control
    
    // Métodos privados
    void scrollUp();                   // Desplaza el contenido una línea hacia arriba
    void advanceCursor();              // Avanza el cursor una posición
    void processCharacter(char c);     // Procesa un carácter (incluyendo \n, \r, etc.)
    uint16_t getBufferOffset(uint8_t col, uint8_t row) const; // Calcula offset en buffer
};
