#ifndef CPU_INSTRUCTIONS_HPP
#define CPU_INSTRUCTIONS_HPP

#include <cstdint>
#include <functional>
#include "mem/mem.hpp"

using Byte = uint8_t;
using Word = uint16_t;
using u32 = uint32_t;

// Forward declaration
class CPU;

// Instruction handler type
using InstrHandler = std::function<void(CPU&, u32&, Mem&)>;

namespace Instructions {
    // Helper functions for flag updates
    void UpdateZeroAndNegativeFlags(CPU& cpu, Byte value);
    void UpdateCarryFlag(CPU& cpu, bool carry);
    void UpdateOverflowFlag(CPU& cpu, bool overflow);
    
    // Initialize the instruction table
    void InitializeInstructionTable();
    
    // Get the handler for a specific opcode
    InstrHandler GetHandler(Byte opcode);
    
    // Load/Store Instructions
    void LDA(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void LDX(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void LDY(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void STA(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void STX(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void STY(CPU& cpu, u32& cycles, Mem& memory, Word address);
    
    // Transfer Instructions
    void TAX(CPU& cpu, u32& cycles, Mem& memory);
    void TAY(CPU& cpu, u32& cycles, Mem& memory);
    void TXA(CPU& cpu, u32& cycles, Mem& memory);
    void TYA(CPU& cpu, u32& cycles, Mem& memory);
    void TSX(CPU& cpu, u32& cycles, Mem& memory);
    void TXS(CPU& cpu, u32& cycles, Mem& memory);
    
    // Stack Instructions
    void PHA(CPU& cpu, u32& cycles, Mem& memory);
    void PHP(CPU& cpu, u32& cycles, Mem& memory);
    void PLA(CPU& cpu, u32& cycles, Mem& memory);
    void PLP(CPU& cpu, u32& cycles, Mem& memory);
    
    // Logical Instructions
    void AND(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void EOR(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void ORA(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void BIT(CPU& cpu, u32& cycles, Mem& memory, Word address);
    
    // Arithmetic Instructions
    void ADC(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void SBC(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void CMP(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void CPX(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void CPY(CPU& cpu, u32& cycles, Mem& memory, Word address);
    
    // Inc/Dec Instructions
    void INC(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void INX(CPU& cpu, u32& cycles, Mem& memory);
    void INY(CPU& cpu, u32& cycles, Mem& memory);
    void DEC(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void DEX(CPU& cpu, u32& cycles, Mem& memory);
    void DEY(CPU& cpu, u32& cycles, Mem& memory);
    
    // Shift Instructions
    void ASL(CPU& cpu, u32& cycles, Mem& memory, Word address, bool accumulator);
    void LSR(CPU& cpu, u32& cycles, Mem& memory, Word address, bool accumulator);
    void ROL(CPU& cpu, u32& cycles, Mem& memory, Word address, bool accumulator);
    void ROR(CPU& cpu, u32& cycles, Mem& memory, Word address, bool accumulator);
    
    // Jump/Branch Instructions
    void JMP(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void JSR(CPU& cpu, u32& cycles, Mem& memory, Word address);
    void RTS(CPU& cpu, u32& cycles, Mem& memory);
    void Branch(CPU& cpu, u32& cycles, Mem& memory, bool condition);
    
    // Flag Instructions
    void CLC(CPU& cpu, u32& cycles, Mem& memory);
    void CLD(CPU& cpu, u32& cycles, Mem& memory);
    void CLI(CPU& cpu, u32& cycles, Mem& memory);
    void CLV(CPU& cpu, u32& cycles, Mem& memory);
    void SEC(CPU& cpu, u32& cycles, Mem& memory);
    void SED(CPU& cpu, u32& cycles, Mem& memory);
    void SEI(CPU& cpu, u32& cycles, Mem& memory);
    
    // System Instructions
    void BRK(CPU& cpu, u32& cycles, Mem& memory);
    void RTI(CPU& cpu, u32& cycles, Mem& memory);
    void NOP(CPU& cpu, u32& cycles, Mem& memory);
}

#endif // CPU_INSTRUCTIONS_HPP
