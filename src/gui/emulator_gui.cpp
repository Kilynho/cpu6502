#include "gui/emulator_gui.hpp"
#include "devices/text_screen.hpp"
#include <iostream>

EmulatorGUI::EmulatorGUI(const std::string& title, int charWidth, int charHeight)
    : charWidth(charWidth),
      charHeight(charHeight),
      window(nullptr),
      renderer(nullptr),
      screenTexture(nullptr),
      initialized(false),
      textScreen(nullptr),
      windowTitle(title),
      lastKey(0),
      keyAvailable(false),
      cursorVisible(true),
      lastCursorBlink(0) {
}

EmulatorGUI::~EmulatorGUI() {
    if (screenTexture) {
        SDL_DestroyTexture(screenTexture);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    if (initialized) {
        SDL_Quit();
    }
}

bool EmulatorGUI::initialize() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Calculate window size based on character dimensions
    int windowWidth = SCREEN_WIDTH_CHARS * charWidth;
    int windowHeight = SCREEN_HEIGHT_CHARS * charHeight;
    
    // Create window
    window = SDL_CreateWindow(
        windowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    
    // Create screen texture for rendering
    screenTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        windowWidth,
        windowHeight
    );
    
    if (!screenTexture) {
        std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    
    initialized = true;
    lastCursorBlink = SDL_GetTicks();
    
    // Clear screen to black
    clear();
    
    std::cout << "EmulatorGUI initialized successfully" << std::endl;
    std::cout << "Window size: " << windowWidth << "x" << windowHeight << std::endl;
    std::cout << "Character size: " << charWidth << "x" << charHeight << std::endl;
    std::cout << "Display: " << SCREEN_WIDTH_CHARS << "x" << SCREEN_HEIGHT_CHARS << " characters" << std::endl;
    
    return true;
}

void EmulatorGUI::attachTextScreen(std::shared_ptr<TextScreen> screen) {
    textScreen = screen;
}

bool EmulatorGUI::update() {
    if (!initialized) {
        return false;
    }
    
    handleEvents();
    
    // Handle cursor blinking
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastCursorBlink >= CURSOR_BLINK_RATE) {
        cursorVisible = !cursorVisible;
        lastCursorBlink = currentTime;
    }
    
    render();
    
    return true;
}

void EmulatorGUI::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                initialized = false;
                break;
                
            case SDL_KEYDOWN:
                // Handle special keys
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        initialized = false;
                        break;
                    case SDLK_RETURN:
                        lastKey = '\n';
                        keyAvailable = true;
                        break;
                    case SDLK_BACKSPACE:
                        lastKey = '\b';
                        keyAvailable = true;
                        break;
                    case SDLK_TAB:
                        lastKey = '\t';
                        keyAvailable = true;
                        break;
                    default:
                        break;
                }
                break;
                
            case SDL_TEXTINPUT:
                // Handle regular text input
                if (event.text.text[0] >= 0x20 && event.text.text[0] <= 0x7E) {
                    lastKey = event.text.text[0];
                    keyAvailable = true;
                }
                break;
        }
    }
}

void EmulatorGUI::render() {
    if (!initialized) {
        return;
    }
    
    // Clear screen with background color
    SDL_Color bgColor = PALETTE[DEFAULT_BG_COLOR];
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(renderer);
    
    // Render text screen if attached
    if (textScreen) {
        renderTextScreen();
    }
    
    // Present the rendered frame
    SDL_RenderPresent(renderer);
}

void EmulatorGUI::renderTextScreen() {
    if (!textScreen) {
        return;
    }
    
    // Get the text buffer from TextScreen
    std::string buffer = textScreen->getBuffer();
    
    // Get cursor position
    uint8_t cursorCol, cursorRow;
    textScreen->getCursorPosition(cursorCol, cursorRow);
    
    // Render each character
    int charIndex = 0;
    for (int row = 0; row < SCREEN_HEIGHT_CHARS; row++) {
        for (int col = 0; col < SCREEN_WIDTH_CHARS; col++) {
            char c = ' ';
            
            // Calculate position in buffer (skip newlines)
            int bufferPos = row * (SCREEN_WIDTH_CHARS + 1) + col;
            if (bufferPos < static_cast<int>(buffer.size()) && buffer[bufferPos] != '\n') {
                c = buffer[bufferPos];
            }
            
            // Draw character
            drawChar(col, row, c, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
            
            // Draw cursor if at this position and visible
            if (col == cursorCol && row == cursorRow && cursorVisible) {
                drawCursor(col, row);
            }
        }
    }
}

void EmulatorGUI::drawChar(int x, int y, char c, int fgColor, int bgColor) {
    // Calculate pixel position
    int pixelX = x * charWidth;
    int pixelY = y * charHeight;
    
    // Draw background
    SDL_Rect bgRect = {pixelX, pixelY, charWidth, charHeight};
    SDL_Color bg = PALETTE[bgColor];
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Simple 8x8 bitmap font rendering (using basic shapes)
    // For a real implementation, you'd load a proper bitmap font
    // For now, we'll draw a simple representation using rectangles
    
    SDL_Color fg = PALETTE[fgColor];
    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, fg.a);
    
    // Draw a simplified version of each character using geometric shapes
    // This creates a retro "blocky" look similar to early computers
    
    if (c >= 0x20 && c <= 0x7E) {
        // For simplicity, we'll draw a small filled rectangle to represent each character
        // In a production version, you'd use a proper bitmap font
        
        int margin = 2;
        int charBlockWidth = charWidth - 2 * margin;
        int charBlockHeight = charHeight - 2 * margin;
        
        // Different patterns for different character ranges
        if (c >= 'A' && c <= 'Z') {
            // Draw uppercase letters as a tall rectangle
            SDL_Rect charRect = {
                pixelX + margin,
                pixelY + margin,
                charBlockWidth,
                charBlockHeight
            };
            SDL_RenderFillRect(renderer, &charRect);
            
            // Add a small gap to distinguish letters
            SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
            SDL_Rect gap = {
                pixelX + charWidth/2 - 1,
                pixelY + charHeight/2 - 1,
                2,
                2
            };
            SDL_RenderFillRect(renderer, &gap);
        } else if (c >= 'a' && c <= 'z') {
            // Draw lowercase letters as a smaller rectangle
            SDL_Rect charRect = {
                pixelX + margin,
                pixelY + charHeight/3,
                charBlockWidth,
                charBlockHeight * 2/3
            };
            SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, fg.a);
            SDL_RenderFillRect(renderer, &charRect);
        } else if (c >= '0' && c <= '9') {
            // Draw numbers as outlined rectangles
            SDL_Rect charRect = {
                pixelX + margin,
                pixelY + margin,
                charBlockWidth,
                charBlockHeight
            };
            SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, fg.a);
            SDL_RenderDrawRect(renderer, &charRect);
        } else {
            // Draw other characters as smaller blocks
            int smallMargin = charWidth / 4;
            SDL_Rect charRect = {
                pixelX + smallMargin,
                pixelY + smallMargin,
                charWidth - 2 * smallMargin,
                charHeight - 2 * smallMargin
            };
            SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, fg.a);
            SDL_RenderFillRect(renderer, &charRect);
        }
    }
}

void EmulatorGUI::drawCursor(int x, int y) {
    // Draw a solid block cursor (classic terminal style)
    int pixelX = x * charWidth;
    int pixelY = y * charHeight;
    
    SDL_Rect cursorRect = {pixelX, pixelY, charWidth, charHeight};
    SDL_Color cursorColor = PALETTE[CURSOR_COLOR];
    
    // Draw semi-transparent cursor
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, cursorColor.r, cursorColor.g, cursorColor.b, 128);
    SDL_RenderFillRect(renderer, &cursorRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void EmulatorGUI::clear() {
    if (!initialized) {
        return;
    }
    
    SDL_Color bgColor = PALETTE[DEFAULT_BG_COLOR];
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

char EmulatorGUI::getLastKey() {
    char key = lastKey;
    lastKey = 0;
    keyAvailable = false;
    return key;
}
