#include "gui/emulator_gui.hpp"
#include "devices/text_screen.hpp"
#include "cpu/cpu.hpp"
#include "mem/mem.hpp"
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>

/**
 * @brief Retro Terminal GUI Demo
 * 
 * This demo showcases the EmulatorGUI with a retro 1980s computer terminal look.
 * Features demonstrated:
 * - Retro color palette
 * - 40x24 character display
 * - Blinking block cursor
 * - Text rendering from TextScreen device
 * - Simulated boot sequence (like Microsoft BASIC)
 */

void simulateRetroBootSequence(std::shared_ptr<TextScreen> screen) {
    // Clear screen
    screen->clear();
    
    // Simulate a classic 1980s computer boot sequence
    const char* bootMsg1 = "    6502 MICROCOMPUTER SYSTEM";
    const char* bootMsg2 = "   40K RAM SYSTEM  960 BYTES FREE";
    const char* bootMsg3 = "";
    const char* bootMsg4 = "MICROSOFT BASIC VERSION 1.0";
    const char* bootMsg5 = "COPYRIGHT 1977-1983 MICROSOFT";
    const char* bootMsg6 = "";
    const char* bootMsg7 = "READY.";
    const char* prompt = ">";
    
    // Write boot messages
    for (const char* p = bootMsg1; *p; p++) screen->writeCharAtCursor(*p);
    screen->writeCharAtCursor('\n');
    screen->writeCharAtCursor('\n');
    
    for (const char* p = bootMsg2; *p; p++) screen->writeCharAtCursor(*p);
    screen->writeCharAtCursor('\n');
    screen->writeCharAtCursor('\n');
    screen->writeCharAtCursor('\n');
    
    for (const char* p = bootMsg4; *p; p++) screen->writeCharAtCursor(*p);
    screen->writeCharAtCursor('\n');
    
    for (const char* p = bootMsg5; *p; p++) screen->writeCharAtCursor(*p);
    screen->writeCharAtCursor('\n');
    screen->writeCharAtCursor('\n');
    
    for (const char* p = bootMsg7; *p; p++) screen->writeCharAtCursor(*p);
    screen->writeCharAtCursor('\n');
    screen->writeCharAtCursor('\n');
    
    for (const char* p = prompt; *p; p++) screen->writeCharAtCursor(*p);
}

void printWelcomeMessage(std::shared_ptr<TextScreen> screen) {
    screen->clear();
    
    const char* lines[] = {
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
        "Press ESC to exit",
        "",
        "Switching to boot sequence in 3s...",
        nullptr
    };
    
    for (int i = 0; lines[i] != nullptr; i++) {
        for (const char* p = lines[i]; *p; p++) {
            screen->writeCharAtCursor(*p);
        }
        screen->writeCharAtCursor('\n');
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== 6502 Retro Terminal GUI Demo ===" << std::endl;
    std::cout << "Initializing retro-style display..." << std::endl;
    
    // Create the GUI (16x16 pixel characters for a nice retro look)
    EmulatorGUI gui("6502 RETRO TERMINAL", 16, 16);
    
    if (!gui.initialize()) {
        std::cerr << "Failed to initialize GUI!" << std::endl;
        return 1;
    }
    
    // Create a TextScreen device
    auto textScreen = std::make_shared<TextScreen>();
    
    // Attach the text screen to the GUI
    gui.attachTextScreen(textScreen);
    
    std::cout << "GUI initialized successfully!" << std::endl;
    std::cout << "Displaying welcome message..." << std::endl;
    
    // Show welcome message
    printWelcomeMessage(textScreen);
    
    // Wait 3 seconds before showing boot sequence
    auto startTime = std::chrono::steady_clock::now();
    auto welcomeDuration = std::chrono::seconds(3);
    
    bool inWelcome = true;
    bool inBoot = false;
    
    // Main event loop
    while (gui.isInitialized()) {
        // Update current time
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
        
        // Switch from welcome to boot sequence after 3 seconds
        if (inWelcome && elapsed >= welcomeDuration) {
            inWelcome = false;
            inBoot = true;
            std::cout << "Showing boot sequence..." << std::endl;
            simulateRetroBootSequence(textScreen);
            startTime = currentTime; // Reset timer for interactive mode
        }
        
        // Handle keyboard input in boot mode
        if (inBoot && gui.hasKey()) {
            char key = gui.getLastKey();
            if (key >= 0x20 && key <= 0x7E) {
                // Echo printable characters
                textScreen->writeCharAtCursor(key);
            } else if (key == '\n') {
                // Handle enter key
                textScreen->writeCharAtCursor('\n');
                
                // Write a new prompt
                textScreen->writeCharAtCursor('>');
            } else if (key == '\b') {
                // Handle backspace (simple implementation)
                textScreen->writeCharAtCursor('\b');
                textScreen->writeCharAtCursor(' ');
                textScreen->writeCharAtCursor('\b');
            }
        }
        
        // Update and render the GUI
        if (!gui.update()) {
            break;
        }
        
        // Small delay to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
    
    std::cout << "GUI closed. Goodbye!" << std::endl;
    return 0;
}
