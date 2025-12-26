#include "cpu/cpu.hpp"
#include "mem/mem.hpp"
#include "util/logger.hpp"
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

    Mem mem; // Create a memory instance
    CPU cpu; // Create a CPU instance

    // Reset CPU and memory
    cpu.Reset(mem);
    
    // Write a test program to ROM memory
    mem[0x0040] = 0x55;                     // Value to load into accumulator
    mem[0x0050] = 0x77;                     // Value to load into accumulator
    mem[0x8000] = 0xA2;   // LDX Immediate instruction
    mem[0x8001] = 0x0f;   // Value to load into X register
    mem[0x8002] = 0xA9;   // LDA Immediate instruction
    mem[0x8003] = 0x84;   // Value to load into accumulator
    mem[0x8004] = 0xA5;   // LDA Zero Page instruction
    mem[0x8005] = 0x40;   // Zero page address to load into accumulator
    mem[0x8006] = 0xB5;   // LDA Zero Page X instruction
    mem[0x8007] = 0x41;   // Zero page address to load into accumulator with X offset
    mem[0x8008] = 0x85;   // STA Zero Page instruction
    mem[0x8009] = 0x42;   // Zero page address to store accumulator
    mem[0x800A] = 0x20;   // JSR (Jump to Subroutine) instruction
    mem[0x800B] = 0x00;   // Low byte of subroutine address
    mem[0x800C] = 0x80;   // High byte of subroutine address
    mem[0x8100] = 0x60;   // RTS (Return from Subroutine) instruction
    mem[0x800D] = 0xA9;   // LDA Immediate instruction
    mem[0x800E] = 0x99;   // Value to load into accumulator
    mem[0x800F] = 0x20;   // JSR (Jump to Subroutine) instruction
    mem[0x8010] = 0x00;   // Low byte of subroutine address
    mem[0x8011] = 0x80;   // High byte of subroutine address

    // Set the reset vector to point to the start address of the program
    mem[Mem::RESET_VECTOR] = 0x00;          // Low byte of reset vector address
    mem[Mem::RESET_VECTOR + 1] = 0x80;      // High byte of reset vector address

    // Determine the number of cycles to execute
    u32 cycles;
    if (infiniteCycles) {
        cycles = -1; // Infinite cycles
    } else {
        cycles = 1000; // Default to 1000 cycles for demo
    }

    // Execute the test program
    cpu.Execute(cycles, mem);

    return 0;
}