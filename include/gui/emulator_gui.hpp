#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <cstdint>

// Forward declaration
class TextScreen;

/**
 * @brief Emulator GUI - Retro-style terminal display
 * 
 * This class provides a graphical user interface that emulates the look and feel
 * of 1980s 8-bit computers (Apple II, Commodore 64, MSX, etc.).
 * 
 * Features:
 * - 40x24 character text mode display
 * - Retro color palette (16 colors similar to Apple II/Commodore 64)
 * - Block cursor (like vintage terminals)
 * - Monospace bitmap font rendering
 * - Keyboard input support
 * - Integration with TextScreen device
 * 
 * Color Palette (Apple II inspired):
 *  0: Black       4: Dark Green   8: Brown        12: Light Green
 *  1: Dark Red    5: Gray         9: Orange       13: Yellow
 *  2: Dark Blue   6: Medium Blue  10: Pink        14: Aqua
 *  3: Purple      7: Light Blue   11: Light Red   15: White
 */
class EmulatorGUI {
public:
    /**
     * @brief Constructor
     * @param title Window title
     * @param charWidth Character width in pixels
     * @param charHeight Character height in pixels
     */
    EmulatorGUI(const std::string& title = "6502 Emulator", 
                int charWidth = 16, 
                int charHeight = 16);
    
    /**
     * @brief Destructor - cleans up SDL resources
     */
    ~EmulatorGUI();
    
    /**
     * @brief Initialize the GUI window and renderer
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Attach a TextScreen device for display
     * @param screen Shared pointer to TextScreen
     */
    void attachTextScreen(std::shared_ptr<TextScreen> screen);
    
    /**
     * @brief Main event loop - process events and render
     * @return true to continue running, false to quit
     */
    bool update();
    
    /**
     * @brief Render the current screen content
     */
    void render();
    
    /**
     * @brief Clear the screen
     */
    void clear();
    
    /**
     * @brief Check if GUI is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return initialized; }
    
    /**
     * @brief Get last key pressed (ASCII)
     * @return ASCII code of last key, or 0 if none
     */
    char getLastKey();
    
    /**
     * @brief Check if a key is available
     * @return true if a key was pressed
     */
    bool hasKey() const { return keyAvailable; }

private:
    // Screen dimensions (in characters)
    static constexpr int SCREEN_WIDTH_CHARS = 40;
    static constexpr int SCREEN_HEIGHT_CHARS = 24;
    
    // Retro color palette (16 colors, Apple II/Commodore style)
    static const SDL_Color PALETTE[16];
    
    // Default colors
    static constexpr int DEFAULT_FG_COLOR = 12;  // Light Green (classic terminal)
    static constexpr int DEFAULT_BG_COLOR = 0;   // Black
    static constexpr int CURSOR_COLOR = 12;      // Light Green
    
    // Character rendering size
    int charWidth;
    int charHeight;
    
    // SDL components
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screenTexture;
    
    // State
    bool initialized;
    std::shared_ptr<TextScreen> textScreen;
    std::string windowTitle;
    
    // Keyboard input
    char lastKey;
    bool keyAvailable;
    
    // Cursor state
    bool cursorVisible;
    Uint32 lastCursorBlink;
    static constexpr Uint32 CURSOR_BLINK_RATE = 500; // milliseconds
    
    // Helper methods
    void handleEvents();
    void drawChar(int x, int y, char c, int fgColor, int bgColor);
    void drawCursor(int x, int y);
    void renderTextScreen();
    
    // Prevent copying
    EmulatorGUI(const EmulatorGUI&) = delete;
    EmulatorGUI& operator=(const EmulatorGUI&) = delete;
};
