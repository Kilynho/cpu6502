# Video Device - Documentation

## Overview

The video module of the CPU 6502 emulator provides text screen capabilities through the `TextScreen` class, which simulates a classic 40-column x 24-line text display similar to 8-bit computers from the 1980s (Apple II, Commodore 64, etc.).

## Architecture

### Class Hierarchy

```
IODevice (base interface)
    └── VideoDevice (interface for video devices)
            └── TextScreen (text screen implementation)
```

### VideoDevice Interface

The `VideoDevice` interface defines the basic methods that every video device must implement:

```cpp
class VideoDevice : public IODevice {
public:
    virtual void refresh() = 0;                    // Refresh the screen
    virtual void clear() = 0;                      // Clear the screen
    virtual std::string getBuffer() const = 0;     // Get the buffer for debugging
    virtual uint16_t getWidth() const = 0;         // Width in characters/pixels
    virtual uint16_t getHeight() const = 0;        // Height in characters/pixels
};
```

## TextScreen - Text Screen

### Specifications

- **Dimensions:** 40 columns × 24 lines (960 characters)
- **Character set:** Printable ASCII (0x20-0x7E)
- **Special features:**
  - Auto-scroll when the screen is full
  - Cursor position control
  - Support for control characters (\\n, \\r, \\t, \\b)

### Memory Map

The TextScreen is mapped to the following memory addresses:

| Address | Description | Type | Range |
|-----------|-------------|------|-------|
| `$FC00-$FFFB` | Video buffer (960 bytes) | R/W | 0x00-0xFF |
| `$FFFC` | Cursor column | R/W | 0-39 |
| `$FFFD` | Cursor row | R/W | 0-23 |
| `$FFFE` | Control register | R/W | See bits below |
| `$FFFF` | Character output port | W | 0x00-0xFF |

#### Organization of the Video Buffer

The video buffer is organized in rows:
- Row 0: `$FC00-$FC27` (bytes 0-39)
- Row 1: `$FC28-$FC4F` (bytes 40-79)
- ...
- Row 23: `$FFD8-$FFFB` (bytes 920-959)

**Address formula:** `address = $FC00 + (row × 40) + column`

#### Control Register ($FFFE)

| Bit | Name | Description |
|-----|--------|-------------|
| 0 | AUTO_SCROLL | 1 = auto-scroll enabled, 0 = disabled |
| 1 | CLEAR_SCREEN | Writing 1 clears the screen (automatically resets) |
| 2-6 | Reserved | Not used |
| 7 | CURSOR_VISIBLE | 1 = cursor visible, 0 = invisible (for future GUI use) |

## Usage from C++

### Initialization

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "devices/text_screen.hpp"
#include <memory>

int main() {
    Mem mem;
    CPU cpu;
    auto textScreen = std::make_shared<TextScreen>();
    
    // Reset CPU and register the device
    cpu.Reset(mem);
    cpu.registerIODevice(textScreen);
    
    // Now the CPU can access the screen through mapped memory
}
```

### Direct C++ API

The `TextScreen` class provides C++ methods for direct control:

```cpp
// Write characters
textScreen->writeCharAtCursor('H');
textScreen->writeCharAtCursor('i');

// Cursor control
textScreen->setCursorPosition(10, 5);  // Column 10, row 5
uint8_t col, row;
textScreen->getCursorPosition(col, row);

// Clearing
textScreen->clear();

// Auto-scroll
textScreen->setAutoScroll(true);
bool enabled = textScreen->getAutoScroll();

// Get content (for debugging/testing)
std::string buffer = textScreen->getBuffer();
std::cout << buffer << std::endl;
```

## Usage from 6502 Code

### Example 1: Write "Hello, World!"

```asm
        ; Write through the character port ($FFFF)
        LDA #'H'
        STA $FFFF
        LDA #'e'
        STA $FFFF
        LDA #'l'
        STA $FFFF
        LDA #'l'
        STA $FFFF
        LDA #'o'
        STA $FFFF
```

### Example 2: Position Cursor and Write

```asm
        ; Position cursor at column 10, row 5
        LDA #10
        STA $FFFC      ; Column
        LDA #5
        STA $FFFD      ; Row
        
        ; Write 'X' at that position
        LDA #'X'
        STA $FFFF
```

### Example 3: Direct Write to Video Buffer

```asm
        ; Write 'A' at row 2, column 15
        ; Address = $FC00 + (2 × 40) + 15 = $FC00 + 80 + 15 = $FC4F
        LDA #'A'
        STA $FC4F
```

### Example 4: Clear Screen

```asm
        ; Clear screen through control register
        LDA #$02       ; Bit 1 = CLEAR_SCREEN
        STA $FFFE
```

### Example 5: Write Text Line with Newline

```asm
        ; Write "Line 1" followed by newline
        LDA #'L'
        STA $FFFF
        LDA #'i'
        STA $FFFF
        LDA #'n'
        STA $FFFF
        LDA #'e'
        STA $FFFF
        LDA #' '
        STA $FFFF
        LDA #'1'
        STA $FFFF
        LDA #$0A       ; '\n' = 0x0A
        STA $FFFF
```

## Control Characters

The TextScreen supports the following control characters:

| Character | Code | Description |
|----------|--------|-------------|
| `\n` | 0x0A | New line: moves the cursor to the start of the next line |
| `\r` | 0x0D | Carriage return: moves the cursor to the start of the current line |
| `\t` | 0x09 | Tab: advances to the next multiple of 8 position |
| `\b` | 0x08 | Backspace: moves the cursor back one position (without erasing) |

## Auto-Scroll

When auto-scroll is enabled (bit 0 of `$FFFE` = 1):
- Writing beyond the last line, all content scrolls up one line
- The first line is lost
- The last line is empty and ready for new writing
- The cursor is positioned at the start of the last line

When disabled:
- Writing beyond the last line, the cursor returns to the start (0,0)

## Integration with Future Systems

### Preparation for GUI (SDL/OpenGL)

The `VideoDevice` architecture is designed to allow future extensions:

1. **refresh() Method:** Currently does nothing, but in a real GUI implementation, it would update the SDL/OpenGL window.

2. **CURSOR_VISIBLE Bit:** Allows controlling the cursor's visibility in graphical implementations.

3. **Separation of logic and presentation:** The video buffer is completely separate from its visualization, making it easy to add graphical backends.

### Example of Future Integration

```cpp
class GraphicalTextScreen : public TextScreen {
protected:
    SDL_Window* window;
    SDL_Renderer* renderer;
    
public:
    void refresh() override {
        // Update SDL window with the buffer content
        renderTextToSDL();
        SDL_RenderPresent(renderer);
    }
};
```

## Performance

### Writing through Character Port

- **Advantage:** Simplifies 6502 code (just write to `$FFFF`)
- **Disadvantage:** Slower (processes control characters, moves cursor, etc.)
- **Recommended use:** Sequential text output, console messages

### Direct Write to Buffer

- **Advantage:** Maximum speed (direct memory access)
- **Disadvantage:** The programmer must calculate addresses and manage the cursor manually
- **Recommended use:** Text graphics, user interfaces, animations

## Complete Examples

See the following files for complete examples:

- **examples/text_screen_demo.cpp:** Full demonstration of all features
- **tests/test_text_screen.cpp:** Test cases showing all usage modes

## Troubleshooting

### Text not appearing

- Check that the device is registered: `cpu.registerIODevice(textScreen)`
- Check that you are writing to the correct addresses
- Ensure printable ASCII characters are being used (0x20-0x7E)

### Scroll not working

- Check that auto-scroll is enabled: write `0x01` to `$FFFE`
- Ensure writing is done through the character port (`$FFFF`)

### Characters appearing in incorrect positions

- Check address calculation: `address = $FC00 + (row × 40) + column`
- Remember that rows and columns are base 0 (0-23 and 0-39)

## Complete TextScreen API

### Public Methods

```cpp
// Constructor
TextScreen();

// IODevice interface
bool handlesRead(uint16_t address) const override;
bool handlesWrite(uint16_t address) const override;
uint8_t read(uint16_t address) override;
void write(uint16_t address, uint8_t value) override;

// VideoDevice interface
void refresh() override;
void clear() override;
std::string getBuffer() const override;
uint16_t getWidth() const override;
uint16_t getHeight() const override;

// Control methods
void setCursorPosition(uint8_t col, uint8_t row);
void getCursorPosition(uint8_t& col, uint8_t& row) const;
void writeCharAtCursor(char c);
void setAutoScroll(bool enabled);
bool getAutoScroll() const;
```

## Conclusion

The TextScreen video system provides a simple yet powerful way to add visual output capabilities to the CPU 6502 emulator. Its modular design allows for future extensions (graphics, color, multiple video modes) while maintaining simplicity and compatibility with classic 8-bit systems.
