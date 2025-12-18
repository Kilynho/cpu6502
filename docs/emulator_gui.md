# EmulatorGUI - Retro-Style Terminal Interface

## Overview

The EmulatorGUI provides a graphical user interface that emulates the look and feel of 1980s 8-bit computers like the Apple II, Commodore 64, and MSX. It features a classic 40x24 character display with retro colors and a blinking block cursor, bringing back the nostalgic experience of vintage computing.

## Features

- **Authentic Retro Display**: 40 columns × 24 rows character-based display
- **16-Color Palette**: Apple II/Commodore 64-inspired color scheme
- **Block Cursor**: Classic blinking cursor like vintage terminals
- **Keyboard Input**: Full keyboard support with special character handling
- **SDL2 Backend**: Hardware-accelerated rendering for smooth performance
- **TextScreen Integration**: Seamlessly connects with the existing TextScreen device

## Color Palette

The GUI implements a 16-color palette inspired by classic 8-bit computers:

| Index | Color Name   | RGB Value    | Usage                    |
|-------|-------------|--------------|--------------------------|
| 0     | Black       | #000000      | Background               |
| 1     | Dark Red    | #880000      | Error messages           |
| 2     | Dark Blue   | #0000CC      | Headers                  |
| 3     | Purple      | #DD22DD      | Special text             |
| 4     | Dark Green  | #008800      | Success messages         |
| 5     | Gray        | #808080      | Disabled text            |
| 6     | Medium Blue | #2222FF      | Links/Commands           |
| 7     | Light Blue  | #66DDFF      | Highlights               |
| 8     | Brown       | #885500      | Warnings                 |
| 9     | Orange      | #FF6600      | Important notices        |
| 10    | Pink        | #CCCCCC      | Light gray               |
| 11    | Light Red   | #FF9999      | Light errors             |
| 12    | Light Green | #00FF00      | **Default Text/Cursor**  |
| 13    | Yellow      | #FFFF00      | Bright highlights        |
| 14    | Aqua        | #00FFFF      | Special highlights       |
| 15    | White       | #FFFFFF      | Bright text              |

The default configuration uses **Light Green (#00FF00)** for text on a **Black (#000000)** background, mimicking classic terminal displays.

## Building

The GUI requires SDL2 to be installed on your system:

### Ubuntu/Debian
```bash
sudo apt-get install libsdl2-dev
```

### Build with CMake
```bash
mkdir build && cd build
cmake ..
make gui_demo
```

## Usage

### Basic Example

```cpp
#include "gui/emulator_gui.hpp"
#include "devices/text_screen.hpp"
#include <memory>

int main() {
    // Create GUI with 16x16 pixel characters
    EmulatorGUI gui("My 6502 Computer", 16, 16);
    
    // Initialize the window
    if (!gui.initialize()) {
        return 1;
    }
    
    // Create and attach TextScreen
    auto textScreen = std::make_shared<TextScreen>();
    gui.attachTextScreen(textScreen);
    
    // Write welcome message
    const char* msg = "Hello, World!";
    for (const char* p = msg; *p; p++) {
        textScreen->writeCharAtCursor(*p);
    }
    
    // Main loop
    while (gui.isInitialized()) {
        // Handle keyboard input
        if (gui.hasKey()) {
            char key = gui.getLastKey();
            textScreen->writeCharAtCursor(key);
        }
        
        // Update and render
        gui.update();
        
        // Small delay for ~60 FPS
        SDL_Delay(16);
    }
    
    return 0;
}
```

### Running the Demo

The included demo showcases the GUI features:

```bash
cd build
./gui_demo
```

The demo displays:
1. **Welcome Screen**: Information about the GUI features
2. **Boot Sequence**: Simulated BASIC startup (like Microsoft BASIC 1.0)
3. **Interactive Mode**: Type on the keyboard and see characters appear

**Controls:**
- Type any keys to see them appear on screen
- Press **ENTER** to create a new prompt
- Press **ESC** to exit

## API Reference

### Constructor

```cpp
EmulatorGUI(const std::string& title = "6502 Emulator", 
            int charWidth = 16, 
            int charHeight = 16);
```

- `title`: Window title
- `charWidth`: Character width in pixels (default: 16)
- `charHeight`: Character height in pixels (default: 16)

Window size is automatically calculated: `640x384` pixels (40×16 by 24×16)

### Methods

#### `bool initialize()`
Initializes SDL2 and creates the window. Must be called before using the GUI.

**Returns:** `true` on success, `false` on failure

#### `void attachTextScreen(std::shared_ptr<TextScreen> screen)`
Attaches a TextScreen device for display output.

**Parameters:**
- `screen`: Shared pointer to a TextScreen instance

#### `bool update()`
Main update loop - processes events, handles cursor blinking, and renders the screen.

**Returns:** `true` to continue running, `false` to quit

#### `void render()`
Renders the current screen content to the window.

#### `void clear()`
Clears the screen to the background color.

#### `char getLastKey()`
Gets the last key pressed and clears the key buffer.

**Returns:** ASCII code of the key, or 0 if no key was pressed

#### `bool hasKey() const`
Checks if a key is available in the buffer.

**Returns:** `true` if a key was pressed

#### `bool isInitialized() const`
Checks if the GUI is initialized and running.

**Returns:** `true` if initialized

## Integration with CPU Emulator

The GUI integrates seamlessly with the 6502 CPU emulator and TextScreen device:

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "gui/emulator_gui.hpp"
#include "devices/text_screen.hpp"

int main() {
    // Initialize components
    Mem mem;
    CPU cpu;
    auto textScreen = std::make_shared<TextScreen>();
    EmulatorGUI gui("6502 Computer", 16, 16);
    
    // Setup
    cpu.Reset(mem);
    cpu.registerIODevice(textScreen);
    gui.initialize();
    gui.attachTextScreen(textScreen);
    
    // Load program into memory (example)
    mem[0x8000] = 0xA9; // LDA #'H'
    mem[0x8001] = 'H';
    mem[0x8002] = 0x8D; // STA $FFFF (write to screen)
    mem[0x8003] = 0xFF;
    mem[0x8004] = 0xFF;
    
    // Main loop
    while (gui.isInitialized()) {
        // Execute CPU cycles
        cpu.Execute(100, mem);
        
        // Handle keyboard input
        if (gui.hasKey()) {
            char key = gui.getLastKey();
            // Could write to keyboard buffer at $FD0C
        }
        
        // Update GUI
        gui.update();
        SDL_Delay(16);
    }
    
    return 0;
}
```

## Architecture

### Character Rendering

The GUI uses a simplified geometric rendering approach:
- **Uppercase letters (A-Z)**: Filled rectangles with a small gap
- **Lowercase letters (a-z)**: Smaller rectangles in the lower portion
- **Numbers (0-9)**: Outlined rectangles
- **Other characters**: Small filled blocks

This creates a distinctive "blocky" retro aesthetic similar to early computer displays.

### Cursor

The cursor is rendered as a semi-transparent block that blinks every 500ms, mimicking the behavior of vintage terminals. The blinking rate can be adjusted by modifying the `CURSOR_BLINK_RATE` constant.

### Screen Buffer

The GUI reads directly from the TextScreen's 40×24 character buffer and renders it in real-time. Changes to the TextScreen are immediately reflected on the display.

## Technical Specifications

- **Display Resolution**: 640×384 pixels (configurable via character size)
- **Character Grid**: 40 columns × 24 rows (960 characters)
- **Character Size**: 16×16 pixels (default, configurable)
- **Frame Rate**: ~60 FPS (with 16ms delay)
- **Cursor Blink Rate**: 500ms (2 Hz)
- **Memory Footprint**: Minimal - uses TextScreen's existing buffer

## Screenshots

### Boot Screen
![Boot Screen](retro_terminal_boot.png)

The classic Microsoft BASIC 1.0 boot sequence, showing the system information and ready prompt.

### Welcome Screen
![Welcome Screen](retro_terminal_welcome.png)

Welcome message demonstrating the 40×24 character display and retro color palette.

## Future Enhancements

Potential improvements for future versions:

1. **Bitmap Font Support**: Load custom retro fonts (e.g., Apple II, C64 fonts)
2. **Color Attributes**: Per-character foreground/background colors
3. **Scanline Effects**: CRT-style scanlines for authenticity
4. **Phosphor Glow**: Simulated phosphor persistence
5. **Multiple Screen Modes**: Graphics modes in addition to text
6. **Sound Support**: Beeper/speaker emulation
7. **Fullscreen Mode**: Immersive retro experience
8. **Configurable Palettes**: Switch between different vintage computer palettes

## Troubleshooting

### SDL2 Initialization Failed
**Problem:** SDL initialization fails with an error message.

**Solution:** Ensure SDL2 is properly installed:
```bash
sudo apt-get install libsdl2-dev
pkg-config --modversion sdl2
```

### Window Doesn't Appear
**Problem:** Window is created but not visible.

**Solution:** 
- Check if running in a headless environment
- Use `xvfb-run -a ./gui_demo` for virtual display testing
- Verify display environment variables are set

### Characters Not Rendering
**Problem:** Screen is blank despite TextScreen having content.

**Solution:**
- Ensure `attachTextScreen()` was called
- Verify TextScreen contains printable ASCII characters (0x20-0x7E)
- Check that `update()` is being called in the main loop

## License

This GUI is part of the cpu6502 emulator project and follows the same license terms.

## Credits

Inspired by:
- Apple II (1977)
- Commodore 64 (1982)
- MSX Computers (1983)
- Classic terminal emulators

The retro aesthetic pays homage to the golden age of 8-bit computing.
