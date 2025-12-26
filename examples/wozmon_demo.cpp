/**
 * @file wozmon_demo.cpp
 * @brief MS BASIC + WOZMON Integration Demo
 * 
 * This program loads and runs Microsoft BASIC and WOZMON monitor from the Apple 1,
 * written by Steve Wozniak and Microsoft. The system provides:
 * - MS BASIC interpreter with full language support
 * - WOZMON monitor for memory examination and debugging
 * - Interactive interface for programming and debugging
 */

#include "cpu/cpu.hpp"
#include "mem/mem.hpp"
#include "devices/pia.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <thread>
#include <chrono>
#include <cstdlib>

// MS BASIC + WOZMON starts at 0x8000 in Apple 1 memory map (BASIC coldstart at 0xE000)
constexpr uint16_t ROM_START = 0x8000;
constexpr uint16_t INPUT_BUFFER = 0x0200;

// WOZMON machine code (assembled from wozmon.asm)
// This is a simplified version for demonstration
const uint8_t WOZMON_CODE[] = {
    0xD8,       // CLD
    0x58,       // CLI
    0xA0, 0x7F, // LDY #$7F
    0x8C, 0x12, 0xD0, // STY $D012 (DSP)
    0xA9, 0xA7, // LDA #$A7
    0x8D, 0x11, 0xD0, // STA $D011 (KBDCR)
    0x8D, 0x13, 0xD0, // STA $D013 (DSPCR)
    0xC9, 0xFF, // CMP #'_'+$80
    0xF0, 0x5B, // BEQ BACKSPACE
    0xC9, 0x9B, // CMP #$9B (ESC)
    0xF0, 0x5E, // BEQ ESCAPE
    0xC8,       // INY
    0x10, 0x5F, // BPL NEXTCHAR
    0xA9, 0xAB, // LDA #'\'+$80
    0x20, 0xE2, 0xFF, // JSR ECHO
    0xA9, 0x8D, // LDA #$8D
    0x20, 0xE2, 0xFF, // JSR ECHO
    0xA0, 0x01, // LDY #$01
    0x88,       // DEY
    0x30, 0xF6, // BMI GETLINE
    0xAD, 0x11, 0xD0, // LDA KBDCR
    0x10, 0xF9, // BPL NEXTCHAR
    0xAD, 0x10, 0xD0, // LDA KBD
    0x99, 0x00, 0x02, // STA IN,Y
    0x20, 0xE2, 0xFF, // JSR ECHO
    0xC9, 0x8D, // CMP #$8D
    0xD0, 0xE3, // BNE NOTCR
    0xA0, 0xFF, // LDY #$FF
    0xA9, 0x00, // LDA #$00
    0xAA,       // TAX
    0x0A,       // ASL
    0x85, 0x2B, // STA MODE
    0xC8,       // INY
    0xB9, 0x00, 0x02, // LDA IN,Y
    0xC9, 0x8D, // CMP #$8D
    0xF0, 0xD6, // BEQ GETLINE
    0xC9, 0xAE, // CMP #'.'+$80
    0x90, 0xF6, // BCC BLSKIP
    0xF0, 0xE7, // BEQ SETMODE
    0xC9, 0xBA, // CMP #':'+$80
    0xF0, 0xE2, // BEQ SETSTOR
    0xC9, 0xD2, // CMP #'R'+$80
    0xF0, 0xF2, // BEQ RUN
    0x86, 0x28, // STX L
    0x86, 0x29, // STX H
    0x84, 0x2A, // STY YSAV
    0xB9, 0x00, 0x02, // LDA IN,Y
    0x49, 0xB0, // EOR #$B0
    0xC9, 0x0A, // CMP #$0A
    0x90, 0x0A, // BCC DIG
    0x69, 0x88, // ADC #$88
    0xC9, 0xFA, // CMP #$FA
    0x90, 0xE7, // BCC NOTHEX
    0x0A,       // ASL
    0x0A,       // ASL
    0x0A,       // ASL
    0x0A,       // ASL
    0xA2, 0x04, // LDX #$04
    0x0A,       // ASL
    0x26, 0x28, // ROL L
    0x26, 0x29, // ROL H
    0xCA,       // DEX
    0xD0, 0xF8, // BNE HEXSHIFT
    0xC8,       // INY
    0xD0, 0xE1, // BNE NEXTHEX
    0xC4, 0x2A, // CPY YSAV
    0xF0, 0xC7, // BEQ ESCAPE
    0x24, 0x2B, // BIT MODE
    0x50, 0x03, // BVC NOTSTOR
    0xA5, 0x28, // LDA L
    0x81, 0x26, // STA (STL,X)
    0xE6, 0x26, // INC STL
    0xD0, 0x02, // BNE NEXTITEM
    0xE6, 0x27, // INC STH
    0x4C, 0x6B, 0xFF, // JMP NEXTITEM
    0x6C, 0x24, 0x00, // JMP (XAML)
    0x30, 0x0B, // BMI XAMNEXT
    0xA2, 0x02, // LDX #$02
    0xBD, 0x27, 0x00, // LDA L-1,X
    0x9D, 0x25, 0x00, // STA STL-1,X
    0x9D, 0x23, 0x00, // STA XAML-1,X
    0xCA,       // DEX
    0xD0, 0xF5, // BNE SETADR
    0xD0, 0x03, // BNE PRDATA
    0xA9, 0x8D, // LDA #$8D
    0x20, 0xE2, 0xFF, // JSR ECHO
    0xA5, 0x25, // LDA XAMH
    0x20, 0xD7, 0xFF, // JSR PRBYTE
    0xA5, 0x24, // LDA XAML
    0x20, 0xD7, 0xFF, // JSR PRBYTE
    0xA9, 0xBA, // LDA #':'+$80
    0x20, 0xE2, 0xFF, // JSR ECHO
    0xA9, 0xA0, // LDA #$A0
    0x20, 0xE2, 0xFF, // JSR ECHO
    0xA1, 0x24, // LDA (XAML,X)
    0x20, 0xD7, 0xFF, // JSR PRBYTE
    0x86, 0x2B, // STX MODE
    0xA5, 0x24, // LDA XAML
    0xC5, 0x28, // CMP L
    0xA5, 0x25, // LDA XAMH
    0xE5, 0x29, // SBC H
    0xB0, 0xC2, // BCS TONEXTITEM
    0xE6, 0x24, // INC XAML
    0xD0, 0x02, // BNE MOD8CHK
    0xE6, 0x25, // INC XAMH
    0xA5, 0x24, // LDA XAML
    0x29, 0x07, // AND #$07
    0x10, 0xD1, // BPL NXTPRNT
    0x48,       // PHA
    0x4A,       // LSR
    0x4A,       // LSR
    0x4A,       // LSR
    0x4A,       // LSR
    0x20, 0xF6, 0xFF, // JSR PRHEX
    0x68,       // PLA
    0x29, 0x0F, // AND #$0F
    0x09, 0xB0, // ORA #'0'+$80
    0xC9, 0xBA, // CMP #$BA
    0x90, 0x03, // BCC ECHO
    0x69, 0x06, // ADC #$06
    0x2C, 0x12, 0xD0, // BIT DSP
    0x30, 0xFB, // BMI ECHO
    0x8D, 0x12, 0xD0, // STA DSP
    0x60,       // RTS
    0x00,       // BRK (unused)
    0x00,       // BRK (unused)
    0x00, 0x0F, // .WORD $0F00 (NMI vector)
    0x00, 0xFF, // .WORD RESET (RESET vector)
    0x00, 0x00  // .WORD $0000 (BRK/IRQ vector)
};

// Función para cargar binario desde archivo
bool loadBinaryFromFile(const std::string& filename, Mem& mem, uint16_t startAddr) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: No se pudo abrir el archivo: " << filename << std::endl;
        return false;
    }
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (startAddr + i >= 0x10000) break; // Evitar overflow de memoria
        mem[startAddr + i] = buffer[i];
    }
    return true;
}

void printWelcome() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════╗\n";
    std::cout << "║         MS BASIC + WOZMON - Apple 1 System         ║\n";
    std::cout << "║          CPU 6502 Emulator Integration             ║\n";
    std::cout << "║     Microsoft BASIC & WOZMON by Steve Wozniak      ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n\n";
}

void printCommands() {
    std::cout << "MS BASIC Commands:\n";
    std::cout << "  PRINT, LET, IF, FOR, NEXT, GOTO, GOSUB, RETURN, etc.\n";
    std::cout << "  Example: PRINT \"HELLO WORLD\"\n";
    std::cout << "           10 PRINT \"HELLO\"; : GOTO 10\n\n";
    std::cout << "WOZMON Commands (when monitor is active):\n";
    std::cout << "  X <addr>     - eXamine memory at address\n";
    std::cout << "  . <addr>     - eXamine memory block at address\n";
    std::cout << "  : <addr>     - Set Store address\n";
    std::cout << "  R            - Run (execute at current address)\n";
    std::cout << "  Q            - Quit emulator\n\n";
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
                uint16_t runAddr = examineAddr;
                if (input.length() > 1) {
                    runAddr = std::stoul(input.substr(1), nullptr, 16);
                }
                std::cout << "Running program at " << std::hex << runAddr << "\n";
                cpu.PC = runAddr;
                std::cout << "PC set to " << std::hex << cpu.PC << std::dec << "\n";
                cpu.Execute(1000000, mem); // Run for a million cycles
                return; // Exit after running
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

int main(int argc, char* argv[]) {
    printWelcome();
    
    // Initialize components
    Mem mem;
    CPU cpu;
    auto pia = std::make_shared<PIA>();
    
    mem.Initialize();
    // cpu.Reset(mem);  // Mover después de cargar el binario
    cpu.registerIODevice(pia);
    
    // ...existing code...
    
    // Load MS BASIC + WOZMON ROM from binary file (priority: CLI arg > env > default)
    std::string romPath = "../src/wozmon/rom.bin";
    if (argc > 1 && argv[1][0] != '\0') {
        romPath = argv[1];
    } else if (const char* envRom = std::getenv("WOZMON_BIN")) {
        if (*envRom != '\0') romPath = envRom;
    }

    if (!loadBinaryFromFile(romPath, mem, ROM_START)) {
        // Fallback to embedded code
        for (size_t i = 0; i < sizeof(WOZMON_CODE); i++) {
            mem[ROM_START + i] = WOZMON_CODE[i];
        }
    }
    
    // Set reset vector to BASIC coldstart at 0x9F06 (COLD_START)
    mem[0xFFFC] = 0x00;      // Low byte of 0x9F06
    mem[0xFFFD] = 0xFE;      // High byte
    cpu.Reset(mem);
    
    // Execute directly from BASIC coldstart until we see output, then drop to interactive inputLoop
    // ...existing code...

    // Run in larger batches until BASIC emits its prompts/output
    const u32 batchInstructions = 200000;  // larger batch to reduce warning spam
    int batches_run = 0;
    while (batches_run < 10) {
        cpu.Execute(batchInstructions, mem);
        batches_run++;
        std::string output = pia->getDisplayOutput();
        if (!output.empty()) {
            std::cout << output << std::endl;
            break;
        }
    }

    // Enter interactive loop: forward stdin to PIA so BASIC sees user input (with high bit set)
    // ...existing code...
    std::string line;
    std::cout << "> " << std::flush;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            pia->pushKeyboardCharacter('\r' | 0x80); // Only CR with high bit
        } else {
            for (char c : line) pia->pushKeyboardCharacter(static_cast<Byte>(c) | 0x80);
            pia->pushKeyboardCharacter('\r' | 0x80);
        }

        // Run a large batch to ensure prompt advances
        cpu.Execute(1000000, mem);

        std::string output = pia->getDisplayOutput();
        if (!output.empty()) {
            std::cout << output;
        }
        std::cout << "> " << std::flush;
    }

    std::cout << "\nExiting WOZMON emulator\n";
    return 0;
}
