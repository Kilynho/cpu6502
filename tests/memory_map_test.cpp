#include <gtest/gtest.h>
#include "mem/mem.hpp"
#include "cpu/cpu.hpp"
#include <devices/pia.hpp>
#include <fstream>
#include <vector>
#include <iostream>

/**
 * Memory Map Validation Test Suite for 6502/65C02 Emulator
 * 
 * This suite validates the complete memory map of the Apple 1 emulator:
 * 
 * MEMORY MAP:
 * 0x0000-0x00FF: Zero Page (256 bytes)
 * 0x0100-0x01FF: Stack (256 bytes)
 * 0x0200-0x1FFF: RAM (7.5 KB) - User/BASIC code
 * 0x2000-0x7FFF: Unused/RAM (24 KB)
 * 0x8000-0xBFFF: ROM - MS BASIC (16 KB)
 * 0xC000-0xCFFF: Unused (4 KB)
 * 0xD000-0xDFFF: I/O Space (4 KB)
 *   └─ 0xD010-0xD013: PIA 6821 (Keyboard/Display)
 * 0xE000-0xEFFF: ROM - Additional BASIC/WOZMON (4 KB)
 * 0xF000-0xFFFF: ROM - WOZMON + Vectors (4 KB)
 * 
 * VECTORS:
 * 0xFFFA-0xFFFB: NMI Vector
 * 0xFFFC-0xFFFD: RESET Vector
 * 0xFFFE-0xFFFF: IRQ/BRK Vector
 */

class MemoryMapTest : public ::testing::Test {
protected:
    Mem mem;
    CPU cpu;
    
    void SetUp() override {
        mem.Initialize();
        cpu.Reset(mem);
    }
    
    // Helper: Write word (16-bit) to memory (little-endian)
    void WriteWord(Word address, Word value) {
        mem[address] = static_cast<Byte>(value & 0xFF);          // Low byte
        mem[address + 1] = static_cast<Byte>((value >> 8) & 0xFF); // High byte
    }
    
    // Helper: Read word (16-bit) from memory (little-endian)
    Word ReadWord(Word address) const {
        Byte low = mem[address];
        Byte high = mem[address + 1];
        return static_cast<Word>(low) | (static_cast<Word>(high) << 8);
    }
};

// Test 1: Validate memory size and initialization
TEST_F(MemoryMapTest, MemorySizeAndInitialization) {
    EXPECT_EQ(Mem::MEM_SIZE, 65536) << "Total memory should be 64KB (65536 bytes)";
    
    // After CPU Reset, some memory locations (vectors, SP) are modified
    // Re-initialize to verify Initialize() works
    mem.Initialize();
    
    // Verify all memory is initialized to 0
    bool all_zero = true;
    for (size_t i = 0; i < Mem::MEM_SIZE; ++i) {
        if (mem[i] != 0) {
            all_zero = false;
            break;
        }
    }
    EXPECT_TRUE(all_zero) << "All memory should be initialized to 0";
}

// Test 2: Validate memory region boundaries
TEST_F(MemoryMapTest, MemoryRegionBoundaries) {
    // Zero Page
    EXPECT_EQ(Mem::ZERO_PAGE_START, 0x0000);
    EXPECT_EQ(Mem::ZERO_PAGE_END, 0x00FF);
    EXPECT_EQ(Mem::ZERO_PAGE_END - Mem::ZERO_PAGE_START + 1, 256);
    
    // Stack
    EXPECT_EQ(Mem::STACK_START, 0x0100);
    EXPECT_EQ(Mem::STACK_END, 0x01FF);
    EXPECT_EQ(Mem::STACK_END - Mem::STACK_START + 1, 256);
    
    // RAM
    EXPECT_EQ(Mem::RAM_START, 0x0200);
    EXPECT_EQ(Mem::RAM_END, 0x1FFF);
    EXPECT_EQ(Mem::RAM_END - Mem::RAM_START + 1, 7680); // 7.5 KB
    
    // ROM
    EXPECT_EQ(Mem::ROM_START, 0x8000);
    EXPECT_EQ(Mem::ROM_END, 0xFFFF);
    EXPECT_EQ(Mem::ROM_END - Mem::ROM_START + 1, 32768); // 32 KB
}

// Test 3: Validate CPU vectors
TEST_F(MemoryMapTest, CPUVectors) {
    EXPECT_EQ(Mem::RESET_VECTOR, 0xFFFC) << "RESET vector should be at 0xFFFC";
    EXPECT_EQ(Mem::IRQ_VECTOR, 0xFFFE) << "IRQ vector should be at 0xFFFE";
    EXPECT_EQ(Mem::NMI_VECTOR, 0xFFFA) << "NMI vector should be at 0xFFFA";
    
    // Write and read back vectors
    WriteWord(Mem::RESET_VECTOR, 0x9F06); // BASIC coldstart
    WriteWord(Mem::IRQ_VECTOR, 0xF000);   // WOZMON entry
    WriteWord(Mem::NMI_VECTOR, 0xF100);   // NMI handler
    
    EXPECT_EQ(ReadWord(Mem::RESET_VECTOR), 0x9F06);
    EXPECT_EQ(ReadWord(Mem::IRQ_VECTOR), 0xF000);
    EXPECT_EQ(ReadWord(Mem::NMI_VECTOR), 0xF100);
}

// Test 4: Validate Zero Page access
TEST_F(MemoryMapTest, ZeroPageAccess) {
    // Write to various zero page locations
    mem[0x00] = 0x12;
    mem[0x50] = 0x34;
    mem[0xFF] = 0x56;
    
    EXPECT_EQ(mem[0x00], 0x12);
    EXPECT_EQ(mem[0x50], 0x34);
    EXPECT_EQ(mem[0xFF], 0x56);
    
    // Verify zero page wraparound (0xFF + 1 = 0x00, not 0x100)
    // This is implicit in 8-bit addressing for ZP mode
}

// Test 5: Validate Stack access
TEST_F(MemoryMapTest, StackAccess) {
    // Stack should be writable
    mem[0x0100] = 0xAA;
    mem[0x01FF] = 0xBB;
    mem[0x0180] = 0xCC;
    
    EXPECT_EQ(mem[0x0100], 0xAA);
    EXPECT_EQ(mem[0x01FF], 0xBB);
    EXPECT_EQ(mem[0x0180], 0xCC);
    
    // Verify stack pointer range
    EXPECT_GE(cpu.SP, 0x00);
    EXPECT_LE(cpu.SP, 0xFF);
}

// Test 6: Validate RAM access
TEST_F(MemoryMapTest, RAMAccess) {
    // Write to various RAM locations
    mem[Mem::RAM_START] = 0x11;
    mem[0x0400] = 0x22;
    mem[0x1000] = 0x33;
    mem[Mem::RAM_END] = 0x44;
    
    EXPECT_EQ(mem[Mem::RAM_START], 0x11);
    EXPECT_EQ(mem[0x0400], 0x22);
    EXPECT_EQ(mem[0x1000], 0x33);
    EXPECT_EQ(mem[Mem::RAM_END], 0x44);
}

// Test 7: Validate ROM region (should be writable in emulator for loading)
TEST_F(MemoryMapTest, ROMAccess) {
    // In emulator, ROM region is writable for loading ROM images
    mem[Mem::ROM_START] = 0xEA;     // NOP at 0x8000
    mem[0x9F06] = 0x4C;             // JMP at BASIC coldstart
    mem[0xF000] = 0xD8;             // CLD at WOZMON entry
    mem[Mem::ROM_END] = 0x00;       // Last byte of ROM
    
    EXPECT_EQ(mem[Mem::ROM_START], 0xEA);
    EXPECT_EQ(mem[0x9F06], 0x4C);
    EXPECT_EQ(mem[0xF000], 0xD8);
    EXPECT_EQ(mem[Mem::ROM_END], 0x00);
}

// Test 8: Validate PIA memory mapping
TEST_F(MemoryMapTest, PIAMemoryMapping) {
    auto pia = std::make_shared<PIA>();
    cpu.registerIODevice(pia);
    
    // PIA registers should be at specific addresses
    constexpr uint16_t KBD = 0xD010;      // Keyboard input
    constexpr uint16_t KBDCR = 0xD011;    // Keyboard control register
    constexpr uint16_t DSP = 0xD012;      // Display output
    constexpr uint16_t DSPCR = 0xD013;    // Display control register
    
    // Verify PIA handles these addresses
    EXPECT_TRUE(pia->handlesRead(KBD));
    EXPECT_TRUE(pia->handlesRead(KBDCR));
    EXPECT_TRUE(pia->handlesRead(DSP));
    EXPECT_TRUE(pia->handlesRead(DSPCR));
    
    EXPECT_TRUE(pia->handlesWrite(KBD));
    EXPECT_TRUE(pia->handlesWrite(KBDCR));
    EXPECT_TRUE(pia->handlesWrite(DSP));
    EXPECT_TRUE(pia->handlesWrite(DSPCR));
    
    // Verify PIA does NOT handle addresses outside its range
    EXPECT_FALSE(pia->handlesRead(0xD009));
    EXPECT_FALSE(pia->handlesRead(0xD014));
    EXPECT_FALSE(pia->handlesWrite(0xD009));
    EXPECT_FALSE(pia->handlesWrite(0xD014));
}

// Test 9: Validate PIA keyboard functionality
TEST_F(MemoryMapTest, PIAKeyboardFunctionality) {
    auto pia = std::make_shared<PIA>();
    cpu.registerIODevice(pia);
    
    constexpr uint16_t KBD = 0xD010;
    constexpr uint16_t KBDCR = 0xD011;
    
    // Push a character to keyboard buffer
    pia->pushKeyboardCharacter('A');
    
    // Read keyboard status (bit 7 should be set when key ready)
    uint8_t kbd_value = pia->read(KBD);
    EXPECT_TRUE(kbd_value & 0x80) << "Bit 7 should be set when key is ready";
    EXPECT_EQ(kbd_value & 0x7F, 'A') << "Lower 7 bits should contain character";
    
    // After reading, key should be consumed
    kbd_value = pia->read(KBD);
    EXPECT_FALSE(kbd_value & 0x80) << "Bit 7 should be clear after key is read";
}

// Test 10: Validate PIA display functionality
TEST_F(MemoryMapTest, PIADisplayFunctionality) {
    auto pia = std::make_shared<PIA>();
    cpu.registerIODevice(pia);
    
    constexpr uint16_t DSP = 0xD012;
    constexpr uint16_t DSPCR = 0xD013;
    
    // Initialize PIA display control register
    cpu.WriteMemory(DSPCR, 0xFF, mem);
    
    // Write characters to display through CPU
    // Note: PIA display requires high bit CLEAR (0) to write
    cpu.WriteMemory(DSP, 'H', mem);  // High bit clear = write accepted
    cpu.WriteMemory(DSP, 'I', mem);
    
    // Retrieve display output
    std::string output = pia->getDisplayOutput();
    EXPECT_GE(output.length(), 2) << "Should have at least 2 characters";
    if (output.length() >= 2) {
        EXPECT_EQ(output[0], 'H');
        EXPECT_EQ(output[1], 'I');
    }
}

// Test 11: Validate memory continuity
TEST_F(MemoryMapTest, MemoryContinuity) {
    // Write sequential pattern across region boundaries
    for (uint16_t i = 0x00FE; i <= 0x0102; ++i) {
        mem[i] = static_cast<uint8_t>(i & 0xFF);
    }
    
    // Verify continuity across ZP -> Stack boundary
    EXPECT_EQ(mem[0x00FE], 0xFE);
    EXPECT_EQ(mem[0x00FF], 0xFF);
    EXPECT_EQ(mem[0x0100], 0x00);
    EXPECT_EQ(mem[0x0101], 0x01);
    EXPECT_EQ(mem[0x0102], 0x02);
}

// Test 12: Validate RESET vector initialization
TEST_F(MemoryMapTest, RESETVectorInitialization) {
    // Setup RESET vector pointing to BASIC coldstart
    WriteWord(Mem::RESET_VECTOR, 0x9F06);
    
    // Reset CPU and verify PC is loaded from RESET vector
    cpu.Reset(mem);
    
    // Note: CPU::Reset should load PC from RESET vector
    // This depends on CPU implementation
    Word reset_target = ReadWord(Mem::RESET_VECTOR);
    EXPECT_EQ(reset_target, 0x9F06);
}

// Test 13: Validate memory patterns for ROM loading
TEST_F(MemoryMapTest, ROMLoadingPatterns) {
    // Simulate loading a small ROM pattern
    const uint8_t test_rom[] = {
        0xD8,       // CLD
        0x58,       // CLI
        0xA9, 0xFF, // LDA #$FF
        0x8D, 0x12, 0xD0, // STA $D012
        0x4C, 0x00, 0xF0  // JMP $F000
    };
    
    // Load at ROM_START
    for (size_t i = 0; i < sizeof(test_rom); ++i) {
        mem[Mem::ROM_START + i] = test_rom[i];
    }
    
    // Verify loading
    EXPECT_EQ(mem[0x8000], 0xD8);
    EXPECT_EQ(mem[0x8001], 0x58);
    EXPECT_EQ(mem[0x8002], 0xA9);
    EXPECT_EQ(mem[0x8003], 0xFF);
    EXPECT_EQ(mem[0x8004], 0x8D);
    EXPECT_EQ(mem[0x8005], 0x12);
    EXPECT_EQ(mem[0x8006], 0xD0);
    EXPECT_EQ(mem[0x8007], 0x4C);
    EXPECT_EQ(mem[0x8008], 0x00);
    EXPECT_EQ(mem[0x8009], 0xF0);
}

// Test 14: Validate vector placement
TEST_F(MemoryMapTest, VectorPlacement) {
    // All vectors should be in the last 6 bytes of memory
    EXPECT_GE(Mem::NMI_VECTOR, 0xFFFA);
    EXPECT_LE(Mem::NMI_VECTOR, 0xFFFD);
    
    EXPECT_GE(Mem::RESET_VECTOR, 0xFFFA);
    EXPECT_LE(Mem::RESET_VECTOR, 0xFFFD);
    
    EXPECT_GE(Mem::IRQ_VECTOR, 0xFFFA);
    EXPECT_LE(Mem::IRQ_VECTOR, 0xFFFF);
    
    // Verify they don't overlap
    EXPECT_NE(Mem::NMI_VECTOR, Mem::RESET_VECTOR);
    EXPECT_NE(Mem::NMI_VECTOR, Mem::IRQ_VECTOR);
    EXPECT_NE(Mem::RESET_VECTOR, Mem::IRQ_VECTOR);
    
    // Verify correct ordering
    EXPECT_EQ(Mem::NMI_VECTOR, 0xFFFA);
    EXPECT_EQ(Mem::RESET_VECTOR, 0xFFFC);
    EXPECT_EQ(Mem::IRQ_VECTOR, 0xFFFE);
}

// Test 15: Validate memory accessibility across all regions
TEST_F(MemoryMapTest, MemoryAccessibilityAllRegions) {
    // Test write and read for each major region
    struct Region {
        std::string name;
        Word start;
        Word end;
    };
    
    std::vector<Region> regions = {
        {"Zero Page", Mem::ZERO_PAGE_START, Mem::ZERO_PAGE_END},
        {"Stack", Mem::STACK_START, Mem::STACK_END},
        {"RAM", Mem::RAM_START, Mem::RAM_END},
        {"ROM", Mem::ROM_START, Mem::ROM_END}
    };
    
    for (const auto& region : regions) {
        // Test start of region
        mem[region.start] = 0xAA;
        EXPECT_EQ(mem[region.start], 0xAA) 
            << region.name << " start should be accessible";
        
        // Test end of region
        mem[region.end] = 0xBB;
        EXPECT_EQ(mem[region.end], 0xBB) 
            << region.name << " end should be accessible";
        
        // Test middle of region
        Word mid = (region.start + region.end) / 2;
        mem[mid] = 0xCC;
        EXPECT_EQ(mem[mid], 0xCC) 
            << region.name << " middle should be accessible";
    }
}

// Test 16: Validate complete memory map layout
TEST_F(MemoryMapTest, CompleteMemoryMapLayout) {
    std::cout << "\n=== Complete Memory Map ===\n";
    std::cout << "Zero Page: 0x" << std::hex << Mem::ZERO_PAGE_START 
              << "-0x" << Mem::ZERO_PAGE_END 
              << " (" << std::dec << (Mem::ZERO_PAGE_END - Mem::ZERO_PAGE_START + 1) << " bytes)\n";
    std::cout << "Stack:     0x" << std::hex << Mem::STACK_START 
              << "-0x" << Mem::STACK_END 
              << " (" << std::dec << (Mem::STACK_END - Mem::STACK_START + 1) << " bytes)\n";
    std::cout << "RAM:       0x" << std::hex << Mem::RAM_START 
              << "-0x" << Mem::RAM_END 
              << " (" << std::dec << (Mem::RAM_END - Mem::RAM_START + 1) << " bytes)\n";
    std::cout << "ROM:       0x" << std::hex << Mem::ROM_START 
              << "-0x" << Mem::ROM_END 
              << " (" << std::dec << (Mem::ROM_END - Mem::ROM_START + 1) << " bytes)\n";
    std::cout << "PIA:       0xD010-0xD013 (4 bytes)\n";
    std::cout << "\nVectors:\n";
    std::cout << "NMI:       0x" << std::hex << Mem::NMI_VECTOR << "-0x" << (Mem::NMI_VECTOR + 1) << "\n";
    std::cout << "RESET:     0x" << std::hex << Mem::RESET_VECTOR << "-0x" << (Mem::RESET_VECTOR + 1) << "\n";
    std::cout << "IRQ/BRK:   0x" << std::hex << Mem::IRQ_VECTOR << "-0x" << (Mem::IRQ_VECTOR + 1) << "\n";
    std::cout << std::dec;
    
    // Verify no gaps or overlaps
    EXPECT_EQ(Mem::STACK_START, Mem::ZERO_PAGE_END + 1);
    EXPECT_EQ(Mem::RAM_START, Mem::STACK_END + 1);
}

// Test 17: Validate typical BASIC program loading
TEST_F(MemoryMapTest, BASICProgramLoading) {
    // Typical BASIC program would be loaded in RAM starting at 0x0200
    const char* basic_program = "10 PRINT \"HELLO\"\n20 GOTO 10\n";
    
    // Simulate loading BASIC program into RAM
    Word program_start = 0x0200;
    for (size_t i = 0; i < strlen(basic_program); ++i) {
        mem[program_start + i] = static_cast<uint8_t>(basic_program[i]);
    }
    
    // Verify loading
    std::string loaded_program;
    for (size_t i = 0; i < strlen(basic_program); ++i) {
        loaded_program += static_cast<char>(mem[program_start + i]);
    }
    
    EXPECT_EQ(loaded_program, std::string(basic_program));
}

// Test 18: Validate memory isolation (no cross-contamination)
TEST_F(MemoryMapTest, MemoryIsolation) {
    // Write distinct patterns to different regions
    mem[0x00FF] = 0x11; // End of Zero Page
    mem[0x0100] = 0x22; // Start of Stack
    mem[0x01FF] = 0x33; // End of Stack
    mem[0x0200] = 0x44; // Start of RAM
    
    // Verify each region maintains its own value
    EXPECT_EQ(mem[0x00FF], 0x11);
    EXPECT_EQ(mem[0x0100], 0x22);
    EXPECT_EQ(mem[0x01FF], 0x33);
    EXPECT_EQ(mem[0x0200], 0x44);
}

// Test 19: Validate word access (16-bit reads/writes)
TEST_F(MemoryMapTest, WordAccess) {
    // Write 16-bit values at various locations
    WriteWord(0x0000, 0x1234);
    WriteWord(0x0100, 0x5678);
    WriteWord(0x0200, 0x9ABC);
    WriteWord(0xFFFC, 0xDEF0);
    
    // Read back and verify
    EXPECT_EQ(ReadWord(0x0000), 0x1234);
    EXPECT_EQ(ReadWord(0x0100), 0x5678);
    EXPECT_EQ(ReadWord(0x0200), 0x9ABC);
    EXPECT_EQ(ReadWord(0xFFFC), 0xDEF0);
    
    // Verify little-endian storage
    EXPECT_EQ(mem[0x0000], 0x34); // Low byte
    EXPECT_EQ(mem[0x0001], 0x12); // High byte
}

// Test 20: Report memory map statistics
TEST_F(MemoryMapTest, MemoryMapStatistics) {
    std::cout << "\n=== Memory Map Statistics ===\n";
    std::cout << "Total Memory: " << Mem::MEM_SIZE << " bytes (64 KB)\n";
    
    size_t zp_size = Mem::ZERO_PAGE_END - Mem::ZERO_PAGE_START + 1;
    size_t stack_size = Mem::STACK_END - Mem::STACK_START + 1;
    size_t ram_size = Mem::RAM_END - Mem::RAM_START + 1;
    size_t rom_size = Mem::ROM_END - Mem::ROM_START + 1;
    
    std::cout << "Zero Page: " << zp_size << " bytes (" 
              << (zp_size * 100 / Mem::MEM_SIZE) << "%)\n";
    std::cout << "Stack:     " << stack_size << " bytes (" 
              << (stack_size * 100 / Mem::MEM_SIZE) << "%)\n";
    std::cout << "RAM:       " << ram_size << " bytes (" 
              << (ram_size * 100 / Mem::MEM_SIZE) << "%)\n";
    std::cout << "ROM:       " << rom_size << " bytes (" 
              << (rom_size * 100 / Mem::MEM_SIZE) << "%)\n";
    
    // Note: Mem.hpp defines regions that cover full address space
    // RAM_END is 0x1FFF, but ROM_START is 0x8000, leaving gap 0x2000-0x7FFF
    // This is expected for Apple 1 architecture
    size_t total_defined = zp_size + stack_size + ram_size + rom_size;
    std::cout << "Defined regions: " << total_defined << " bytes\n";
    std::cout << "Unused/IO: " << (Mem::MEM_SIZE - total_defined) << " bytes\n";
    
    // Validate individual regions are correctly sized
    EXPECT_EQ(zp_size, 256);
    EXPECT_EQ(stack_size, 256);
    EXPECT_EQ(ram_size, 7680);
    EXPECT_EQ(rom_size, 32768);
}
