#ifndef CPU_ADDRESSING_HPP
#define CPU_ADDRESSING_HPP

#include <cstdint>
#include "mem.hpp"
#include "system_map.hpp"

using Byte = uint8_t;
using Word = uint16_t;
using u32 = uint32_t;

// Forward declaration
class CPU;

namespace Addressing {
    // Addressing mode functions - return the effective address for the instruction
    // Each function updates the cycle count and PC as needed
    
    Word Immediate(CPU& cpu, u32& cycles, SystemMap& bus);
    Word ZeroPage(CPU& cpu, u32& cycles, SystemMap& bus);
    Word ZeroPageX(CPU& cpu, u32& cycles, SystemMap& bus);
    Word ZeroPageY(CPU& cpu, u32& cycles, SystemMap& bus);
    Word Absolute(CPU& cpu, u32& cycles, SystemMap& bus);
    Word AbsoluteX(CPU& cpu, u32& cycles, SystemMap& bus, bool pageCrossPenalty = true);
    Word AbsoluteY(CPU& cpu, u32& cycles, SystemMap& bus, bool pageCrossPenalty = true);
    Word IndirectX(CPU& cpu, u32& cycles, SystemMap& bus);
    Word IndirectY(CPU& cpu, u32& cycles, SystemMap& bus, bool pageCrossPenalty = true);
    Word Indirect(CPU& cpu, u32& cycles, SystemMap& bus);
    
    // Helper to check if page boundary was crossed
    bool PagesCross(Word addr1, Word addr2);
}

#endif // CPU_ADDRESSING_HPP
