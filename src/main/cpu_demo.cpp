#include "cpu.hpp"
#include "mem.hpp"
#include "logger.hpp"
#include "cpu_instructions.hpp"
#include <cstdio>
#include <unistd.h>
#include <cstring>

int main(int argc, char* argv[]) {
    bool infiniteCycles = false;
  
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "infinite") == 0) {
            infiniteCycles = true;
        }
    }


    SystemMap bus; // Create a system bus instance
    CPU cpu; // Create a CPU instance

    // Manually set CPU registers to reset state
    cpu.PC = (bus.read(0xFFFC)) | (bus.read(0xFFFD) << 8); // Set PC from reset vector
    cpu.SP = 0xFD;
    // Optionally set status flags if needed (e.g., cpu.Status = 0x24;)

    // Write a test program to ROM/RAM via the bus
    bus.write(0x0040, 0x55);                     // Value to load into accumulator
    bus.write(0x0050, 0x77);                     // Value to load into accumulator
    bus.write(0x8000, Instructions::OP_LDX_IM);   // LDX Immediate instruction
    bus.write(0x8001, 0x0f);                     // Value to load into X register
    bus.write(0x8002, Instructions::OP_LDA_IM);   // LDA Immediate instruction
    bus.write(0x8003, 0x84);                     // Value to load into accumulator
    bus.write(0x8004, Instructions::OP_LDA_ZP);   // LDA Zero Page instruction
    bus.write(0x8005, 0x40);                     // Zero page address to load into accumulator
    bus.write(0x8006, Instructions::OP_LDA_ZPX);  // LDA Zero Page X instruction
    bus.write(0x8007, 0x41);                     // Zero page address to load into accumulator with X offset
    bus.write(0x8008, Instructions::OP_STA_ZP);   // STA Zero Page instruction
    bus.write(0x8009, 0x42);                     // Zero page address to store accumulator
    bus.write(0x800A, Instructions::OP_JSR);      // JSR (Jump to Subroutine) instruction
    bus.write(0x800B, 0x00);                     // Low byte of subroutine address
    bus.write(0x800C, 0x80);                     // High byte of subroutine address
    bus.write(0x8100, Instructions::OP_RTS);      // RTS (Return from Subroutine) instruction
    bus.write(0x800D, Instructions::OP_LDA_IM);   // LDA Immediate instruction
    bus.write(0x800E, 0x99);                     // Value to load into accumulator
    bus.write(0x800F, Instructions::OP_JSR);      // JSR (Jump to Subroutine) instruction
    bus.write(0x8010, 0x00);                     // Low byte of subroutine address
    bus.write(0x8011, 0x80);                     // High byte of subroutine address

    // Set the reset vector to point to the start address of the program
    bus.write(0xFFFC, 0x00);          // Low byte of reset vector address
    bus.write(0xFFFD, 0x80);          // High byte of reset vector address

    // Determine the number of cycles to execute
    u32 cycles;
    if (infiniteCycles) {
        cycles = -1; // Infinite cycles
    } else {
        cycles = 1000; // Use a fixed cycle count for demo
    }

    // Execute the test program
    cpu.Execute(cycles, bus);

    return 0;
}