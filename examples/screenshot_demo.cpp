#include "emulator_gui.hpp"
#include "text_screen.hpp"
#include <memory>
#include <iostream>
#include <SDL2/SDL.h>

/**
 * @brief Screenshot demo for the Retro Terminal GUI
 * This version captures screenshots to demonstrate the GUI appearance
 */

void saveScreenshot(SDL_Window* window, SDL_Renderer* renderer, const char* filename) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    if (!surface) {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        return;
    }
    
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);
    
    if (SDL_SaveBMP(surface, filename) != 0) {
        std::cerr << "Failed to save screenshot: " << SDL_GetError() << std::endl;
    } else {
        std::cout << "Screenshot saved to " << filename << std::endl;
    }
    
    SDL_FreeSurface(surface);
}

int main() {
    std::cout << "=== Creating screenshots of 6502 Retro Terminal GUI ===" << std::endl;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Create window and renderer
    SDL_Window* window = SDL_CreateWindow(
        "6502 RETRO TERMINAL",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 384,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Create TextScreen
    auto textScreen = std::make_shared<TextScreen>();
    
    // Demo 1: Boot screen
    std::cout << "Creating boot screen screenshot..." << std::endl;
    textScreen->clear();
    
    const char* bootLines[] = {
        "",
        "    6502 MICROCOMPUTER SYSTEM",
        "",
        "   40K RAM SYSTEM  960 BYTES FREE",
        "",
        "",
        "MICROSOFT BASIC VERSION 1.0",
        "COPYRIGHT 1977-1983 MICROSOFT",
        "",
        "READY.",
        "",
        ">",
        nullptr
    };
    
    for (int i = 0; bootLines[i] != nullptr; i++) {
        for (const char* p = bootLines[i]; *p; p++) {
            textScreen->writeCharAtCursor(*p);
        }
        textScreen->writeCharAtCursor('\n');
    }
    
    // Render the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Simple rendering (similar to EmulatorGUI but simplified)
    std::string buffer = textScreen->getBuffer();
    uint8_t cursorCol, cursorRow;
    textScreen->getCursorPosition(cursorCol, cursorRow);
    
    // Define retro green color (classic terminal)
    SDL_Color fgColor = {0x00, 0xFF, 0x00, 0xFF};  // Bright green
    SDL_Color bgColor = {0x00, 0x00, 0x00, 0xFF};  // Black
    
    int charWidth = 16;
    int charHeight = 16;
    
    // Render text
    int row = 0, col = 0;
    for (size_t i = 0; i < buffer.size(); i++) {
        char c = buffer[i];
        
        if (c == '\n') {
            row++;
            col = 0;
            continue;
        }
        
        if (row >= 24) break;
        
        int pixelX = col * charWidth;
        int pixelY = row * charHeight;
        
        // Draw character background
        SDL_Rect bgRect = {pixelX, pixelY, charWidth, charHeight};
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &bgRect);
        
        // Draw character (simplified)
        if (c >= 0x20 && c <= 0x7E) {
            SDL_SetRenderDrawColor(renderer, fgColor.r, fgColor.g, fgColor.b, fgColor.a);
            
            int margin = 2;
            if (c >= 'A' && c <= 'Z') {
                SDL_Rect charRect = {
                    pixelX + margin,
                    pixelY + margin,
                    charWidth - 2 * margin,
                    charHeight - 2 * margin
                };
                SDL_RenderFillRect(renderer, &charRect);
            } else if (c >= 'a' && c <= 'z') {
                SDL_Rect charRect = {
                    pixelX + margin,
                    pixelY + charHeight/3,
                    charWidth - 2 * margin,
                    (charHeight - 2 * margin) * 2/3
                };
                SDL_RenderFillRect(renderer, &charRect);
            } else if (c >= '0' && c <= '9') {
                SDL_Rect charRect = {
                    pixelX + margin,
                    pixelY + margin,
                    charWidth - 2 * margin,
                    charHeight - 2 * margin
                };
                SDL_RenderDrawRect(renderer, &charRect);
            } else {
                int smallMargin = charWidth / 4;
                SDL_Rect charRect = {
                    pixelX + smallMargin,
                    pixelY + smallMargin,
                    charWidth - 2 * smallMargin,
                    charHeight - 2 * smallMargin
                };
                SDL_RenderFillRect(renderer, &charRect);
            }
        }
        
        col++;
        if (col >= 40) {
            col = 0;
            row++;
        }
    }
    
    // Draw cursor
    int cursorPixelX = cursorCol * charWidth;
    int cursorPixelY = cursorRow * charHeight;
    SDL_Rect cursorRect = {cursorPixelX, cursorPixelY, charWidth, charHeight};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, fgColor.r, fgColor.g, fgColor.b, 128);
    SDL_RenderFillRect(renderer, &cursorRect);
    
    SDL_RenderPresent(renderer);
    SDL_Delay(100);
    
    saveScreenshot(window, renderer, "retro_terminal_boot.bmp");
    
    // Demo 2: Welcome screen
    std::cout << "Creating welcome screen screenshot..." << std::endl;
    textScreen->clear();
    
    const char* welcomeLines[] = {
        "========================================",
        "  WELCOME TO THE 6502 EMULATOR GUI!   ",
        "========================================",
        "",
        "This GUI emulates the classic look of",
        "1980s 8-bit computers like:",
        "",
        "  * Apple II",
        "  * Commodore 64",
        "  * MSX computers",
        "",
        "Features:",
        "  - 40x24 character display",
        "  - 16 color retro palette",
        "  - Blinking block cursor",
        "  - Classic terminal feel",
        "",
        "A nostalgic trip to the golden age",
        "of personal computing!",
        nullptr
    };
    
    for (int i = 0; welcomeLines[i] != nullptr; i++) {
        for (const char* p = welcomeLines[i]; *p; p++) {
            textScreen->writeCharAtCursor(*p);
        }
        textScreen->writeCharAtCursor('\n');
    }
    
    // Render again
    buffer = textScreen->getBuffer();
    textScreen->getCursorPosition(cursorCol, cursorRow);
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    row = 0;
    col = 0;
    for (size_t i = 0; i < buffer.size(); i++) {
        char c = buffer[i];
        
        if (c == '\n') {
            row++;
            col = 0;
            continue;
        }
        
        if (row >= 24) break;
        
        int pixelX = col * charWidth;
        int pixelY = row * charHeight;
        
        SDL_Rect bgRect = {pixelX, pixelY, charWidth, charHeight};
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &bgRect);
        
        if (c >= 0x20 && c <= 0x7E) {
            SDL_SetRenderDrawColor(renderer, fgColor.r, fgColor.g, fgColor.b, fgColor.a);
            
            int margin = 2;
            if (c >= 'A' && c <= 'Z') {
                SDL_Rect charRect = {
                    pixelX + margin,
                    pixelY + margin,
                    charWidth - 2 * margin,
                    charHeight - 2 * margin
                };
                SDL_RenderFillRect(renderer, &charRect);
            } else if (c >= 'a' && c <= 'z') {
                SDL_Rect charRect = {
                    pixelX + margin,
                    pixelY + charHeight/3,
                    charWidth - 2 * margin,
                    (charHeight - 2 * margin) * 2/3
                };
                SDL_RenderFillRect(renderer, &charRect);
            } else if (c >= '0' && c <= '9') {
                SDL_Rect charRect = {
                    pixelX + margin,
                    pixelY + margin,
                    charWidth - 2 * margin,
                    charHeight - 2 * margin
                };
                SDL_RenderDrawRect(renderer, &charRect);
            } else {
                int smallMargin = charWidth / 4;
                SDL_Rect charRect = {
                    pixelX + smallMargin,
                    pixelY + smallMargin,
                    charWidth - 2 * smallMargin,
                    charHeight - 2 * smallMargin
                };
                SDL_RenderFillRect(renderer, &charRect);
            }
        }
        
        col++;
        if (col >= 40) {
            col = 0;
            row++;
        }
    }
    
    SDL_RenderPresent(renderer);
    SDL_Delay(100);
    
    saveScreenshot(window, renderer, "retro_terminal_welcome.bmp");
    
    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    std::cout << "Screenshots created successfully!" << std::endl;
    
    return 0;
}
