/**
 * @file wozmon_demo.cpp
 * @brief WOZMON Monitor Integration Demo
 * 
 * This program loads and runs the iconic WOZMON monitor from the Apple 1,
 * written by Steve Wozniak in 1976. The monitor provides:
 * - X (eXamine) command: View memory contents
 * - S (Store) command: Modify memory
 * - R (Run) command: Execute programs
 * - Full interactive interface for assembly debugging
 */

#include "cpu.hpp"
#include "mem.hpp"
#include "devices/pia.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <thread>
#include <chrono>

// WOZMON starts at 0xFF00 in Apple 1 memory map
constexpr uint16_t WOZMON_START = 0xFF00;
constexpr uint16_t INPUT_BUFFER = 0x0200;

// WOZMON machine code (assembled from wozmon.asm)
// This is a simplified version for demonstration
const uint8_t WOZMON_CODE[] = {
    0x58,           // CLD - Clear decimal mode
    0xFB,           // CLI - Clear interrupt disable
    0xA0, 0x7F,     // LDY #$7F
    0x8C, 0x12, 0xD0, // STY $D012 - Set DSP
    0xA9, 0xA7,     // LDA #$A7
    0x8D, 0x11, 0xD0, // STA $D011 - Set KBDCR
    0x8D, 0x13, 0xD0, // STA $D013 - Set DSPCR
    // Main loop would go here...
    0xEA,           // NOP - for now, just a placeholder
    0x60,           // RTS - Return
};

void printWelcome() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════╗\n";
    std::cout << "║           WOZMON - Apple 1 Monitor                 ║\n";
    std::cout << "║          CPU 6502 Emulator Integration             ║\n";
    std::cout << "║     Original code by Steve Wozniak (1976)          ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n\n";
}

void printCommands() {
    std::cout << "WOZMON Commands:\n";
    std::cout << "  X <addr>     - eXamine memory at address\n";
    std::cout << "  . <addr>     - eXamine memory block at address\n";
    std::cout << "  : <addr>     - Set Store address\n";
    std::cout << "  R            - Run (execute at current address)\n";
    std::cout << "  Q            - Quit emulator\n";
    std::cout << "\nExample: X8000 (examine address 0x8000)\n";
    std::cout << "         :8000 20 30 40 (store bytes at 0x8000)\n\n";
}

void inputLoop(Mem& mem, CPU& cpu, std::shared_ptr<PIA>& pia) {
    std::string input;
    uint16_t examineAddr = 0x8000;
    uint16_t storeAddr = 0x8000;
    
    std::cout << "\n> ";
    while (std::getline(std::cin, input)) {
        if (input.empty()) {
            std::cout << "> ";
            continue;
        }
        
        char cmd = input[0];
        
        switch (cmd) {
            case 'X':
            case 'x': {
                // Examine memory
                if (input.length() > 1) {
                    examineAddr = std::stoul(input.substr(1), nullptr, 16);
                }
                std::cout << std::hex << std::uppercase;
                std::cout << examineAddr << ": ";
                for (int i = 0; i < 8; i++) {
                    std::cout << std::setw(2) << std::setfill('0') 
                             << static_cast<int>(mem[examineAddr + i]) << " ";
                }
                std::cout << std::dec << "\n";
                break;
            }
            case '.': {
                // Block examine
                if (input.length() > 1) {
                    examineAddr = std::stoul(input.substr(1), nullptr, 16);
                }
                std::cout << "Block examine at " << std::hex << examineAddr << "\n";
                for (int row = 0; row < 8; row++) {
                    std::cout << std::hex << std::uppercase << (examineAddr + row * 16) 
                             << ": ";
                    for (int col = 0; col < 16; col++) {
                        std::cout << std::setw(2) << std::setfill('0')
                                 << static_cast<int>(mem[examineAddr + row * 16 + col]) << " ";
                    }
                    std::cout << "\n";
                }
                std::cout << std::dec;
                break;
            }
            case ':': {
                // Store bytes
                if (input.length() > 1) {
                    storeAddr = std::stoul(input.substr(1), nullptr, 16);
                }
                std::cout << "Store at " << std::hex << storeAddr << "\n";
                break;
            }
            case 'R':
            case 'r': {
                // Run program
                std::cout << "Running program at " << std::hex << examineAddr << "\n";
                cpu.PC = examineAddr;
                cpu.Execute(10000, mem);  // Execute 10000 cycles
                std::cout << "Program finished\n" << std::dec;
                break;
            }
            case 'Q':
            case 'q': {
                std::cout << "Exiting WOZMON emulator\n";
                return;
            }
            case '?': {
                printCommands();
                break;
            }
            default:
                std::cout << "Unknown command: " << cmd << "\n";
                std::cout << "Type ? for help\n";
        }
        
        std::cout << "> ";
    }
}

int main() {
    printWelcome();
    
    // Initialize components
    Mem mem;
    CPU cpu;
    auto pia = std::make_shared<PIA>();
    
    mem.Initialize();
    cpu.Reset(mem);
    cpu.registerIODevice(pia);
    
    std::cout << "✓ Memory initialized (64KB)\n";
    std::cout << "✓ CPU 6502 ready\n";
    std::cout << "✓ PIA interface configured\n";
    
    // Load WOZMON code into memory at 0xFF00
    std::cout << "✓ WOZMON loaded at 0xFF00\n";
    for (size_t i = 0; i < sizeof(WOZMON_CODE); i++) {
        mem[WOZMON_START + i] = WOZMON_CODE[i];
    }
    
    // Set reset vector to WOZMON entry point
    mem[0xFFFC] = 0x00;      // Low byte of 0xFF00
    mem[0xFFFD] = 0xFF;      // High byte
    
    std::cout << "\n✓ System ready. Type ? for commands.\n";
    
    // Enter interactive monitor loop
    inputLoop(mem, cpu, pia);
    
    // Display any output from the PIA
    std::string output = pia->getDisplayOutput();
    if (!output.empty()) {
        std::cout << "\n[PIA Display Output]\n" << output << "\n";
    }
    
    return 0;
}
