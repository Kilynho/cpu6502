#ifndef CPU_INSTRUCTIONS_HPP
#define CPU_INSTRUCTIONS_HPP

#include <cstdint>
#include <functional>
#include "mem.hpp"
#include "system_map.hpp"

using Byte = uint8_t;
using Word = uint16_t;
using u32 = uint32_t;

// Forward declaration
class CPU;

// Instruction handler type
using InstrHandler = std::function<void(CPU&, u32&, SystemMap&)>;

namespace Instructions {
    // Opcode metadata used by the emulator (cycles and instruction length)
    struct OpcodeInfo { uint8_t cycles; uint8_t bytes; };

    // Public opcode constants for use by other modules/tests.
    constexpr Byte OP_LDA_IM = 0xA9;
    constexpr Byte OP_LDA_ZP = 0xA5;
    constexpr Byte OP_LDA_ZPX = 0xB5;
    constexpr Byte OP_LDA_ABS = 0xAD;
    constexpr Byte OP_LDA_ABSX = 0xBD;
    constexpr Byte OP_LDA_ABSY = 0xB9;
    constexpr Byte OP_LDX_IM = 0xA2;
    constexpr Byte OP_LDX_ZP = 0xA6;
    constexpr Byte OP_LDX_ZPX = 0xB6;
    constexpr Byte OP_LDX_ABS = 0xAE;
    constexpr Byte OP_LDX_ABSY = 0xBE;
    constexpr Byte OP_STA_ZP = 0x85;
    constexpr Byte OP_STA_ZPX = 0x95;
    constexpr Byte OP_STA_ABS = 0x8D;
    constexpr Byte OP_JSR = 0x20;
    constexpr Byte OP_RTS = 0x60;
    // Helper functions for flag updates
    void UpdateZeroAndNegativeFlags(CPU& cpu, Byte value);
    void UpdateCarryFlag(CPU& cpu, bool carry);
    void UpdateOverflowFlag(CPU& cpu, bool overflow);
    
    // Initialize the instruction table
    void InitializeInstructionTable();
    
    // Get the handler for a specific opcode
    InstrHandler GetHandler(Byte opcode);

    // Get opcode metadata (cycles and bytes). Returns {0,1} for unknown opcodes.
    OpcodeInfo GetOpcodeInfo(Byte opcode);
    
    // Load/Store Instructions
    void LDA(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void LDX(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void LDY(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void STA(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void STX(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void STY(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    
    // Transfer Instructions
    void TAX(CPU& cpu, u32& cycles, SystemMap& bus);
    void TAY(CPU& cpu, u32& cycles, SystemMap& bus);
    void TXA(CPU& cpu, u32& cycles, SystemMap& bus);
    void TYA(CPU& cpu, u32& cycles, SystemMap& bus);
    void TSX(CPU& cpu, u32& cycles, SystemMap& bus);
    void TXS(CPU& cpu, u32& cycles, SystemMap& bus);
    
    // Stack Instructions
    void PHA(CPU& cpu, u32& cycles, SystemMap& bus);
    void PHP(CPU& cpu, u32& cycles, SystemMap& bus);
    void PLA(CPU& cpu, u32& cycles, SystemMap& bus);
    void PLP(CPU& cpu, u32& cycles, SystemMap& bus);
    
    // Logical Instructions
    void AND(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void EOR(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void ORA(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void BIT(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    
    // Arithmetic Instructions
    void ADC(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void SBC(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void CMP(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void CPX(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void CPY(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    
    // Inc/Dec Instructions
    void INC(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void INX(CPU& cpu, u32& cycles, SystemMap& bus);
    void INY(CPU& cpu, u32& cycles, SystemMap& bus);
    void DEC(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void DEX(CPU& cpu, u32& cycles, SystemMap& bus);
    void DEY(CPU& cpu, u32& cycles, SystemMap& bus);
    
    // Shift Instructions
    void ASL(CPU& cpu, u32& cycles, SystemMap& bus, Word address, bool accumulator);
    void LSR(CPU& cpu, u32& cycles, SystemMap& bus, Word address, bool accumulator);
    void ROL(CPU& cpu, u32& cycles, SystemMap& bus, Word address, bool accumulator);
    void ROR(CPU& cpu, u32& cycles, SystemMap& bus, Word address, bool accumulator);
    
    // Jump/Branch Instructions
    void JMP(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void JSR(CPU& cpu, u32& cycles, SystemMap& bus, Word address);
    void RTS(CPU& cpu, u32& cycles, SystemMap& bus);
    void Branch(CPU& cpu, u32& cycles, SystemMap& bus, bool condition);
    
    // Flag Instructions
    void CLC(CPU& cpu, u32& cycles, SystemMap& bus);
    void CLD(CPU& cpu, u32& cycles, SystemMap& bus);
    void CLI(CPU& cpu, u32& cycles, SystemMap& bus);
    void CLV(CPU& cpu, u32& cycles, SystemMap& bus);
    void SEC(CPU& cpu, u32& cycles, SystemMap& bus);
    void SED(CPU& cpu, u32& cycles, SystemMap& bus);
    void SEI(CPU& cpu, u32& cycles, SystemMap& bus);
    
    // System Instructions
    void BRK(CPU& cpu, u32& cycles, SystemMap& bus);
    void RTI(CPU& cpu, u32& cycles, SystemMap& bus);
    void NOP(CPU& cpu, u32& cycles, SystemMap& bus);
}

#endif // CPU_INSTRUCTIONS_HPP
