#ifndef CPU_ADDRESSING_HPP
#define CPU_ADDRESSING_HPP

#include <cstdint>
#include "mem.hpp"

using Byte = uint8_t;
using Word = uint16_t;
using u32 = uint32_t;

// Forward declaration
class CPU;

namespace Addressing {
    // Addressing mode functions - return the effective address for the instruction
    // Each function updates the cycle count and PC as needed
    
    Word Immediate(CPU& cpu, u32& cycles, Mem& memory);
    Word ZeroPage(CPU& cpu, u32& cycles, Mem& memory);
    Word ZeroPageX(CPU& cpu, u32& cycles, Mem& memory);
    Word ZeroPageY(CPU& cpu, u32& cycles, Mem& memory);
    Word Absolute(CPU& cpu, u32& cycles, Mem& memory);
    Word AbsoluteX(CPU& cpu, u32& cycles, Mem& memory, bool pageCrossPenalty = true);
    Word AbsoluteY(CPU& cpu, u32& cycles, Mem& memory, bool pageCrossPenalty = true);
    Word IndirectX(CPU& cpu, u32& cycles, Mem& memory);
    Word IndirectY(CPU& cpu, u32& cycles, Mem& memory, bool pageCrossPenalty = true);
    // 65C02: (Zero Page) Indirect addressing
    Word IndirectZeroPage(CPU& cpu, u32& cycles, Mem& memory);
    Word Indirect(CPU& cpu, u32& cycles, Mem& memory);
    
    // Helper to check if page boundary was crossed
    bool PagesCross(Word addr1, Word addr2);
}

#endif // CPU_ADDRESSING_HPP
