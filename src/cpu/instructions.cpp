#include "cpu/cpu_instructions.hpp"
#include "cpu/cpu.hpp"
#include "cpu/cpu_addressing.hpp"
#include "util/logger.hpp"
#include <iomanip>
#include <sstream>
#include <array>

namespace Instructions {

// Global instruction handler table - indexed by opcode
static std::array<InstrHandler, 256> instructionTable;

// Helper function implementations
void UpdateZeroAndNegativeFlags(CPU& cpu, Byte value) {
    cpu.Z = (value == 0);
    cpu.N = (value & 0x80) != 0;
}

void UpdateCarryFlag(CPU& cpu, bool carry) {
    cpu.C = carry ? 1 : 0;
}

void UpdateOverflowFlag(CPU& cpu, bool overflow) {
    cpu.V = overflow ? 1 : 0;
}

// Load/Store Instructions
void LDA(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.A = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, cpu.A, false);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void LDX(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.X = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, cpu.X, false);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void LDY(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.Y = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, cpu.Y, false);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

void STA(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.WriteMemory(address, cpu.A, memory);
    cpu.LogMemoryAccess(address, cpu.A, true);
    cycles--;
}

void STX(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.WriteMemory(address, cpu.X, memory);
    cpu.LogMemoryAccess(address, cpu.X, true);
    cycles--;
}

void STY(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.WriteMemory(address, cpu.Y, memory);
    cpu.LogMemoryAccess(address, cpu.Y, true);
    cycles--;
}

// Transfer Instructions
void TAX(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.X = cpu.A;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void TAY(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.Y = cpu.A;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

void TXA(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.A = cpu.X;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void TYA(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.A = cpu.Y;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void TSX(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.X = cpu.SP;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void TXS(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.SP = cpu.X;
    cycles--;
    // TXS does not affect flags
}

// Stack Instructions
void PHA(CPU& cpu, u32& cycles, Mem& memory) {
    memory[cpu.SPToAddress()] = cpu.A;
    cpu.LogMemoryAccess(cpu.SPToAddress(), cpu.A, true);
    cpu.SP--;
    cycles -= 3; // 65C02: PHA takes 3 cycles
}

void PHP(CPU& cpu, u32& cycles, Mem& memory) {
    // Push processor status with B flag set
    Byte status = (cpu.N << 7) | (cpu.V << 6) | (1 << 5) | (1 << 4) | 
                  (cpu.D << 3) | (cpu.I << 2) | (cpu.Z << 1) | cpu.C;
    memory[cpu.SPToAddress()] = status;
    cpu.LogMemoryAccess(cpu.SPToAddress(), status, true);
    cpu.SP--;
    cycles -= 3; // 65C02: PHP takes 3 cycles
}

void PLA(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.SP++;
    cpu.A = memory[cpu.SPToAddress()];
    cpu.LogMemoryAccess(cpu.SPToAddress(), cpu.A, false);
    cycles -= 4; // 65C02: PLA takes 4 cycles
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void PLP(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.SP++;
    Byte status = memory[cpu.SPToAddress()];
    cpu.LogMemoryAccess(cpu.SPToAddress(), status, false);

    cpu.N = (status >> 7) & 1;
    cpu.V = (status >> 6) & 1;
    cpu.D = (status >> 3) & 1;
    cpu.I = (status >> 2) & 1;
    cpu.Z = (status >> 1) & 1;
    cpu.C = status & 1;
    cycles -= 4; // 65C02: PLP takes 4 cycles
}

// Logical Instructions
void AND(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.A &= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void EOR(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.A ^= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void ORA(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.A |= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void BIT(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    
    cpu.Z = ((cpu.A & value) == 0);
    cpu.N = (value & 0x80) != 0;
    cpu.V = (value & 0x40) != 0;
}

// Arithmetic Instructions
void ADC(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    
    Word sum = cpu.A + value + cpu.C;
    
    // Set carry if result > 255
    UpdateCarryFlag(cpu, sum > 0xFF);
    
    // Set overflow if sign bit is incorrect
    // V = (A^result) & (value^result) & 0x80
    bool overflow = ((cpu.A ^ sum) & (value ^ sum) & 0x80) != 0;
    UpdateOverflowFlag(cpu, overflow);
    
    cpu.A = sum & 0xFF;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void SBC(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    
    // SBC is equivalent to ADC with inverted operand
    Word diff = cpu.A - value - (1 - cpu.C);
    
    // Carry is set if no borrow (result >= 0)
    UpdateCarryFlag(cpu, diff <= 0xFF);
    
    // Set overflow if sign bit is incorrect
    bool overflow = ((cpu.A ^ value) & (cpu.A ^ diff) & 0x80) != 0;
    UpdateOverflowFlag(cpu, overflow);
    
    cpu.A = diff & 0xFF;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void CMP(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    
    Word result = cpu.A - value;
    UpdateCarryFlag(cpu, cpu.A >= value);
    UpdateZeroAndNegativeFlags(cpu, result & 0xFF);
}

void CPX(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    
    Word result = cpu.X - value;
    UpdateCarryFlag(cpu, cpu.X >= value);
    UpdateZeroAndNegativeFlags(cpu, result & 0xFF);
}

void CPY(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    
    Word result = cpu.Y - value;
    UpdateCarryFlag(cpu, cpu.Y >= value);
    UpdateZeroAndNegativeFlags(cpu, result & 0xFF);
}

// Inc/Dec Instructions
void INC(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    
    value++;
    cpu.WriteMemory(address, value, memory);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;
    
    UpdateZeroAndNegativeFlags(cpu, value);
}

void INX(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.X++;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void INY(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.Y++;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

void DEC(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    
    value--;
    cpu.WriteMemory(address, value, memory);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;
    
    UpdateZeroAndNegativeFlags(cpu, value);
}

void DEX(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.X--;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void DEY(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.Y--;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

// Shift Instructions
void ASL(CPU& cpu, u32& cycles, Mem& memory, Word address, bool accumulator) {
    Byte value;
    
    if (accumulator) {
        value = cpu.A;
        cycles--;
    } else {
        value = cpu.ReadMemory(address, memory);
        cpu.LogMemoryAccess(address, value, false);
        cycles--;
    }
    
    UpdateCarryFlag(cpu, (value & 0x80) != 0);
    value <<= 1;
    
    if (accumulator) {
        cpu.A = value;
    } else {
        cpu.WriteMemory(address, value, memory);
        cpu.LogMemoryAccess(address, value, true);
        cycles--;
    }
    
    UpdateZeroAndNegativeFlags(cpu, value);
}

void LSR(CPU& cpu, u32& cycles, Mem& memory, Word address, bool accumulator) {
    Byte value;
    
    if (accumulator) {
        value = cpu.A;
        cycles--;
    } else {
        value = cpu.ReadMemory(address, memory);
        cpu.LogMemoryAccess(address, value, false);
        cycles--;
    }
    
    UpdateCarryFlag(cpu, (value & 0x01) != 0);
    value >>= 1;
    
    if (accumulator) {
        cpu.A = value;
    } else {
        cpu.WriteMemory(address, value, memory);
        cpu.LogMemoryAccess(address, value, true);
        cycles--;
    }
    
    UpdateZeroAndNegativeFlags(cpu, value);
}

void ROL(CPU& cpu, u32& cycles, Mem& memory, Word address, bool accumulator) {
    Byte value;
    
    if (accumulator) {
        value = cpu.A;
        cycles--;
    } else {
        value = cpu.ReadMemory(address, memory);
        cpu.LogMemoryAccess(address, value, false);
        cycles--;
    }
    
    bool oldCarry = cpu.C;
    UpdateCarryFlag(cpu, (value & 0x80) != 0);
    value = (value << 1) | (oldCarry ? 1 : 0);
    
    if (accumulator) {
        cpu.A = value;
    } else {
        cpu.WriteMemory(address, value, memory);
        cpu.LogMemoryAccess(address, value, true);
        cycles--;
    }
    
    UpdateZeroAndNegativeFlags(cpu, value);
}

void ROR(CPU& cpu, u32& cycles, Mem& memory, Word address, bool accumulator) {
    Byte value;
    
    if (accumulator) {
        value = cpu.A;
        cycles--;
    } else {
        value = cpu.ReadMemory(address, memory);
        cpu.LogMemoryAccess(address, value, false);
        cycles--;
    }
    
    bool oldCarry = cpu.C;
    UpdateCarryFlag(cpu, (value & 0x01) != 0);
    value = (value >> 1) | (oldCarry ? 0x80 : 0);
    
    if (accumulator) {
        cpu.A = value;
    } else {
        cpu.WriteMemory(address, value, memory);
        cpu.LogMemoryAccess(address, value, true);
        cycles--;
    }
    
    UpdateZeroAndNegativeFlags(cpu, value);
}

// Jump/Branch Instructions
void JMP(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.PC = address;
    // No additional cycles needed - already consumed in addressing mode
}

void JSR(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.PushPCToStack(cycles, memory);
    cpu.PC = address;
    cycles--;
}

void RTS(CPU& cpu, u32& cycles, Mem& memory) {
    cycles--; // Internal operation
    cpu.SP++;
    cycles--;
    
    Word lowByte = memory[0x0100 + cpu.SP];
    cpu.LogMemoryAccess(0x0100 + cpu.SP, lowByte, false);
    cpu.SP++;
    cycles--;
    
    Word highByte = memory[0x0100 + cpu.SP];
    cpu.LogMemoryAccess(0x0100 + cpu.SP, highByte, false);
    
    cpu.PC = (highByte << 8) | lowByte;
    cycles--;
    cpu.PC++;
    cycles--;
}

void Branch(CPU& cpu, u32& cycles, Mem& memory, bool condition) {
    int8_t offset = static_cast<int8_t>(cpu.FetchByte(cycles, memory));
    
    // Base cycle cost after reading offset
    cycles--;

    if (condition) {
        Word oldPC = cpu.PC;
        cpu.PC += offset;
        // Additional cycle for taken branch
        cycles--;
        
        // Additional cycle if page boundary crossed
        if (Addressing::PagesCross(oldPC, cpu.PC)) {
            cycles--;
        }
    }
}

// Flag Instructions
void CLC(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.C = 0;
    cycles--;
}

void CLD(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.D = 0;
    cycles--;
}

void CLI(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.I = 0;
    cycles--;
}

void CLV(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.V = 0;
    cycles--;
}

void SEC(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.C = 1;
    cycles--;
}

void SED(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.D = 1;
    cycles--;
}

void SEI(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.I = 1;
    cycles--;
}

// System Instructions
void BRK(CPU& cpu, u32& cycles, Mem& memory) {
    // For test/integration: treat BRK as a stop (set cycles=0 and increment PC by 2)
    // 6502/65C02: BRK is a 2-byte instruction (BRK + padding byte)
    cpu.PC += 2;
    cycles = 0;
    // If you want to emulate full interrupt, restore the original code above.
}

void RTI(CPU& cpu, u32& cycles, Mem& memory) {
    // Pull processor status
    cpu.SP++;
    Byte status = memory[cpu.SPToAddress()];
    cpu.LogMemoryAccess(cpu.SPToAddress(), status, false);
    cycles--;
    
    cpu.N = (status >> 7) & 1;
    cpu.V = (status >> 6) & 1;
    cpu.D = (status >> 3) & 1;
    cpu.I = (status >> 2) & 1;
    cpu.Z = (status >> 1) & 1;
    cpu.C = status & 1;
    
    // Pull PC
    cpu.SP++;
    Word lowByte = memory[cpu.SPToAddress()];
    cpu.LogMemoryAccess(cpu.SPToAddress(), lowByte, false);
    cpu.SP++;
    cycles--;
    
    Word highByte = memory[cpu.SPToAddress()];
    cpu.LogMemoryAccess(cpu.SPToAddress(), highByte, false);
    
    cpu.PC = (highByte << 8) | lowByte;
    cycles -= 2;
}

void NOP(CPU& cpu, u32& cycles, Mem& memory) {
    cycles--;
}

// 65C02: Accumulator INC/DEC
void INC_A(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.A = static_cast<Byte>(cpu.A + 1);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void DEC_A(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.A = static_cast<Byte>(cpu.A - 1);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

// 65C02: Push/Pop X/Y
void PHX(CPU& cpu, u32& cycles, Mem& memory) {
    memory[cpu.SPToAddress()] = cpu.X;
    cpu.LogMemoryAccess(cpu.SPToAddress(), cpu.X, true);
    cpu.SP--;
    cycles -= 3;
}

void PHY(CPU& cpu, u32& cycles, Mem& memory) {
    memory[cpu.SPToAddress()] = cpu.Y;
    cpu.LogMemoryAccess(cpu.SPToAddress(), cpu.Y, true);
    cpu.SP--;
    cycles -= 3;
}

void PLX(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.SP++;
    cpu.X = memory[cpu.SPToAddress()];
    cpu.LogMemoryAccess(cpu.SPToAddress(), cpu.X, false);
    cycles -= 4;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void PLY(CPU& cpu, u32& cycles, Mem& memory) {
    cpu.SP++;
    cpu.Y = memory[cpu.SPToAddress()];
    cpu.LogMemoryAccess(cpu.SPToAddress(), cpu.Y, false);
    cycles -= 4;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

// 65C02: STZ (Store Zero)
void STZ(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    cpu.WriteMemory(address, 0x00, memory);
    // cycles for store handled by addressing + write
}

// 65C02: TSB/TRB
void TSB(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.Z = ((value & cpu.A) == 0);
    value |= cpu.A;
    cpu.WriteMemory(address, value, memory);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;
}

void TRB(CPU& cpu, u32& cycles, Mem& memory, Word address) {
    Byte value = cpu.ReadMemory(address, memory);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.Z = ((value & cpu.A) == 0);
    value &= static_cast<Byte>(~cpu.A);
    cpu.WriteMemory(address, value, memory);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;
}

// 65C02: RMB/SMB (Reset/Set Memory Bit) - zero page only
void RMB(CPU& cpu, u32& cycles, Mem& memory, Byte zpAddr, int bit) {
    Byte value = memory[zpAddr];
    cpu.LogMemoryAccess(zpAddr, value, false);
    cycles--; // read
    value &= static_cast<Byte>(~(1 << bit));
    memory[zpAddr] = value;
    cpu.LogMemoryAccess(zpAddr, value, true);
    cycles--; // write
}

void SMB(CPU& cpu, u32& cycles, Mem& memory, Byte zpAddr, int bit) {
    Byte value = memory[zpAddr];
    cpu.LogMemoryAccess(zpAddr, value, false);
    cycles--; // read
    value |= static_cast<Byte>(1 << bit);
    memory[zpAddr] = value;
    cpu.LogMemoryAccess(zpAddr, value, true);
    cycles--; // write
}

// 65C02: BBR/BBS (Branch on Bit Reset/Set) - zero page, relative
void BBR_BBS(CPU& cpu, u32& cycles, Mem& memory, int bit, bool branchOnSet) {
    Byte zpAddr = cpu.FetchByte(cycles, memory);
    Byte value = memory[zpAddr];
    cpu.LogMemoryAccess(zpAddr, value, false);
    cycles--; // read
    bool isSet = (value & (1 << bit)) != 0;
    bool cond = branchOnSet ? isSet : !isSet;
    Branch(cpu, cycles, memory, cond);
}

// 65C02: STP/WAI
void STP(CPU& cpu, u32& cycles, Mem& memory) {
    // Stop the processor; end execution window
    cycles = 0;
}

void WAI(CPU& cpu, u32& cycles, Mem& memory) {
    // Wait for interrupt; for now, halt execution slice
    cycles = 0;
}

// Initialize instruction table with all 256 opcodes
void InitializeInstructionTable() {
    // Initialize all opcodes to NOP by default
    for (int i = 0; i < 256; i++) {
        instructionTable[i] = [i](CPU& cpu, u32& cycles, Mem& memory) {
            std::stringstream ss;
            ss << "Unimplemented opcode: 0x" << std::hex << std::setw(2) << std::setfill('0') << i 
               << " at PC=0x" << std::hex << std::setw(4) << (cpu.PC - 1);
            util::LogWarn(ss.str());
            cycles--;
        };
    }
    
    // COP - Coprocessor instruction (65C02, treat as 1-byte NOP for compatibility)
    instructionTable[0x02] = [](CPU& cpu, u32& cycles, Mem& memory) {
        cycles--;  // 1 cycle for COP instruction
    };
    
    // LDA - Load Accumulator
    instructionTable[0xA9] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDA(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0xA5] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDA(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xB5] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDA(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0xAD] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDA(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0xBD] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDA(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory));
    };
    instructionTable[0xB9] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDA(cpu, cycles, memory, Addressing::AbsoluteY(cpu, cycles, memory));
    };
    instructionTable[0xA1] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDA(cpu, cycles, memory, Addressing::IndirectX(cpu, cycles, memory));
    };
    instructionTable[0xB1] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDA(cpu, cycles, memory, Addressing::IndirectY(cpu, cycles, memory));
    };
    
    // LDX - Load X Register
    instructionTable[0xA2] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDX(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0xA6] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDX(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xB6] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDX(cpu, cycles, memory, Addressing::ZeroPageY(cpu, cycles, memory));
    };
    instructionTable[0xAE] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDX(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0xBE] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDX(cpu, cycles, memory, Addressing::AbsoluteY(cpu, cycles, memory));
    };
    
    // LDY - Load Y Register
    instructionTable[0xA0] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDY(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0xA4] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDY(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xB4] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDY(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0xAC] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDY(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0xBC] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LDY(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory));
    };
    
    // STA - Store Accumulator
    instructionTable[0x85] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STA(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x95] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STA(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0x8D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STA(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0x9D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STA(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory, false));
    };
    instructionTable[0x99] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STA(cpu, cycles, memory, Addressing::AbsoluteY(cpu, cycles, memory, false));
    };
    instructionTable[0x81] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STA(cpu, cycles, memory, Addressing::IndirectX(cpu, cycles, memory));
    };
    instructionTable[0x91] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STA(cpu, cycles, memory, Addressing::IndirectY(cpu, cycles, memory, false));
    };
    
    // STX - Store X Register
    instructionTable[0x86] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STX(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x96] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STX(cpu, cycles, memory, Addressing::ZeroPageY(cpu, cycles, memory));
    };
    instructionTable[0x8E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STX(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    
    // STY - Store Y Register
    instructionTable[0x84] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STY(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x94] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STY(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0x8C] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STY(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    
    // Transfer Instructions
    instructionTable[0xAA] = TAX;
    instructionTable[0xA8] = TAY;
    instructionTable[0x8A] = TXA;
    instructionTable[0x98] = TYA;
    instructionTable[0xBA] = TSX;
    instructionTable[0x9A] = TXS;
    
    // Stack Instructions
    instructionTable[0x48] = PHA;
    instructionTable[0x08] = PHP;
    instructionTable[0x68] = PLA;
    instructionTable[0x28] = PLP;
    
    // Logical Instructions - AND
    instructionTable[0x29] = [](CPU& cpu, u32& cycles, Mem& memory) {
        AND(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0x25] = [](CPU& cpu, u32& cycles, Mem& memory) {
        AND(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x35] = [](CPU& cpu, u32& cycles, Mem& memory) {
        AND(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0x2D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        AND(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0x3D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        AND(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory));
    };
    instructionTable[0x39] = [](CPU& cpu, u32& cycles, Mem& memory) {
        AND(cpu, cycles, memory, Addressing::AbsoluteY(cpu, cycles, memory));
    };
    instructionTable[0x21] = [](CPU& cpu, u32& cycles, Mem& memory) {
        AND(cpu, cycles, memory, Addressing::IndirectX(cpu, cycles, memory));
    };
    instructionTable[0x31] = [](CPU& cpu, u32& cycles, Mem& memory) {
        AND(cpu, cycles, memory, Addressing::IndirectY(cpu, cycles, memory));
    };
    
    // Logical Instructions - EOR
    instructionTable[0x49] = [](CPU& cpu, u32& cycles, Mem& memory) {
        EOR(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0x45] = [](CPU& cpu, u32& cycles, Mem& memory) {
        EOR(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x55] = [](CPU& cpu, u32& cycles, Mem& memory) {
        EOR(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0x4D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        EOR(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0x5D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        EOR(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory));
    };
    instructionTable[0x59] = [](CPU& cpu, u32& cycles, Mem& memory) {
        EOR(cpu, cycles, memory, Addressing::AbsoluteY(cpu, cycles, memory));
    };
    instructionTable[0x41] = [](CPU& cpu, u32& cycles, Mem& memory) {
        EOR(cpu, cycles, memory, Addressing::IndirectX(cpu, cycles, memory));
    };
    instructionTable[0x51] = [](CPU& cpu, u32& cycles, Mem& memory) {
        EOR(cpu, cycles, memory, Addressing::IndirectY(cpu, cycles, memory));
    };
    
    // Logical Instructions - ORA
    instructionTable[0x09] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ORA(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0x05] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ORA(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x15] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ORA(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0x0D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ORA(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0x1D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ORA(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory));
    };
    instructionTable[0x19] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ORA(cpu, cycles, memory, Addressing::AbsoluteY(cpu, cycles, memory));
    };
    instructionTable[0x01] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ORA(cpu, cycles, memory, Addressing::IndirectX(cpu, cycles, memory));
    };
    instructionTable[0x11] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ORA(cpu, cycles, memory, Addressing::IndirectY(cpu, cycles, memory));
    };
    
    // BIT - Bit Test
    instructionTable[0x24] = [](CPU& cpu, u32& cycles, Mem& memory) {
        BIT(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x2C] = [](CPU& cpu, u32& cycles, Mem& memory) {
        BIT(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    
    // ADC - Add with Carry
    instructionTable[0x69] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ADC(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0x65] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ADC(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x75] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ADC(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0x6D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ADC(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0x7D] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ADC(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory));
    };
    instructionTable[0x79] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ADC(cpu, cycles, memory, Addressing::AbsoluteY(cpu, cycles, memory));
    };
    instructionTable[0x61] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ADC(cpu, cycles, memory, Addressing::IndirectX(cpu, cycles, memory));
    };
    instructionTable[0x71] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ADC(cpu, cycles, memory, Addressing::IndirectY(cpu, cycles, memory));
    };
    
    // SBC - Subtract with Carry
    instructionTable[0xE9] = [](CPU& cpu, u32& cycles, Mem& memory) {
        SBC(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0xE5] = [](CPU& cpu, u32& cycles, Mem& memory) {
        SBC(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xF5] = [](CPU& cpu, u32& cycles, Mem& memory) {
        SBC(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0xED] = [](CPU& cpu, u32& cycles, Mem& memory) {
        SBC(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0xFD] = [](CPU& cpu, u32& cycles, Mem& memory) {
        SBC(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory));
    };
    instructionTable[0xF9] = [](CPU& cpu, u32& cycles, Mem& memory) {
        SBC(cpu, cycles, memory, Addressing::AbsoluteY(cpu, cycles, memory));
    };
    instructionTable[0xE1] = [](CPU& cpu, u32& cycles, Mem& memory) {
        SBC(cpu, cycles, memory, Addressing::IndirectX(cpu, cycles, memory));
    };
    instructionTable[0xF1] = [](CPU& cpu, u32& cycles, Mem& memory) {
        SBC(cpu, cycles, memory, Addressing::IndirectY(cpu, cycles, memory));
    };
    
    // CMP - Compare Accumulator
    instructionTable[0xC9] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CMP(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0xC5] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CMP(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xD5] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CMP(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0xCD] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CMP(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0xDD] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CMP(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory));
    };
    instructionTable[0xD9] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CMP(cpu, cycles, memory, Addressing::AbsoluteY(cpu, cycles, memory));
    };
    instructionTable[0xC1] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CMP(cpu, cycles, memory, Addressing::IndirectX(cpu, cycles, memory));
    };
    instructionTable[0xD1] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CMP(cpu, cycles, memory, Addressing::IndirectY(cpu, cycles, memory));
    };
    
    // CPX - Compare X Register
    instructionTable[0xE0] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CPX(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0xE4] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CPX(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xEC] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CPX(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    
    // CPY - Compare Y Register
    instructionTable[0xC0] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CPY(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };
    instructionTable[0xC4] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CPY(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xCC] = [](CPU& cpu, u32& cycles, Mem& memory) {
        CPY(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    
    // INC - Increment Memory
    instructionTable[0xE6] = [](CPU& cpu, u32& cycles, Mem& memory) {
        INC(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xF6] = [](CPU& cpu, u32& cycles, Mem& memory) {
        INC(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0xEE] = [](CPU& cpu, u32& cycles, Mem& memory) {
        INC(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0xFE] = [](CPU& cpu, u32& cycles, Mem& memory) {
        INC(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory, false));
    };
    
    // INX, INY
    instructionTable[0xE8] = INX;
    instructionTable[0xC8] = INY;
    
    // DEC - Decrement Memory
    instructionTable[0xC6] = [](CPU& cpu, u32& cycles, Mem& memory) {
        DEC(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xD6] = [](CPU& cpu, u32& cycles, Mem& memory) {
        DEC(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    instructionTable[0xCE] = [](CPU& cpu, u32& cycles, Mem& memory) {
        DEC(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0xDE] = [](CPU& cpu, u32& cycles, Mem& memory) {
        DEC(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory, false));
    };
    
    // DEX, DEY
    instructionTable[0xCA] = DEX;
    instructionTable[0x88] = DEY;
    
    // ASL - Arithmetic Shift Left
    instructionTable[0x0A] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ASL(cpu, cycles, memory, 0, true);
    };
    instructionTable[0x06] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ASL(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory), false);
    };
    instructionTable[0x16] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ASL(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory), false);
    };
    instructionTable[0x0E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ASL(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory), false);
    };
    instructionTable[0x1E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ASL(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory, false), false);
    };
    
    // LSR - Logical Shift Right
    instructionTable[0x4A] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LSR(cpu, cycles, memory, 0, true);
    };
    instructionTable[0x46] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LSR(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory), false);
    };
    instructionTable[0x56] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LSR(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory), false);
    };
    instructionTable[0x4E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LSR(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory), false);
    };
    instructionTable[0x5E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        LSR(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory, false), false);
    };
    
    // ROL - Rotate Left
    instructionTable[0x2A] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROL(cpu, cycles, memory, 0, true);
    };
    instructionTable[0x26] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROL(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory), false);
    };
    instructionTable[0x36] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROL(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory), false);
    };
    instructionTable[0x2E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROL(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory), false);
    };
    instructionTable[0x3E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROL(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory, false), false);
    };
    
    // ROR - Rotate Right
    instructionTable[0x6A] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROR(cpu, cycles, memory, 0, true);
    };
    instructionTable[0x66] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROR(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory), false);
    };
    instructionTable[0x76] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROR(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory), false);
    };
    instructionTable[0x6E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROR(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory), false);
    };
    instructionTable[0x7E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        ROR(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory, false), false);
    };
    
    // JMP - Jump
    instructionTable[0x4C] = [](CPU& cpu, u32& cycles, Mem& memory) {
        JMP(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0x6C] = [](CPU& cpu, u32& cycles, Mem& memory) {
        JMP(cpu, cycles, memory, Addressing::Indirect(cpu, cycles, memory));
    };
    
    // JSR - Jump to Subroutine
    instructionTable[0x20] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Word address = cpu.FetchWord(cycles, memory);
        JSR(cpu, cycles, memory, address);
    };
    
    // RTS - Return from Subroutine
    instructionTable[0x60] = RTS;
    
    // Branch Instructions
    instructionTable[0x10] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Branch(cpu, cycles, memory, cpu.N == 0); // BPL - Branch if Positive
    };
    instructionTable[0x30] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Branch(cpu, cycles, memory, cpu.N == 1); // BMI - Branch if Minus
    };
    instructionTable[0x50] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Branch(cpu, cycles, memory, cpu.V == 0); // BVC - Branch if Overflow Clear
    };
    instructionTable[0x70] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Branch(cpu, cycles, memory, cpu.V == 1); // BVS - Branch if Overflow Set
    };
    instructionTable[0x90] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Branch(cpu, cycles, memory, cpu.C == 0); // BCC - Branch if Carry Clear
    };
    instructionTable[0xB0] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Branch(cpu, cycles, memory, cpu.C == 1); // BCS - Branch if Carry Set
    };
    instructionTable[0xD0] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Branch(cpu, cycles, memory, cpu.Z == 0); // BNE - Branch if Not Equal
    };
    instructionTable[0xF0] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Branch(cpu, cycles, memory, cpu.Z == 1); // BEQ - Branch if Equal
    };
    
    // Flag Instructions
    instructionTable[0x18] = CLC; // Clear Carry
    instructionTable[0xD8] = CLD; // Clear Decimal
    instructionTable[0x58] = CLI; // Clear Interrupt
    instructionTable[0xB8] = CLV; // Clear Overflow
    instructionTable[0x38] = SEC; // Set Carry
    instructionTable[0xF8] = SED; // Set Decimal
    instructionTable[0x78] = SEI; // Set Interrupt
    
    // System Instructions
    instructionTable[0x00] = BRK; // Break
    instructionTable[0x40] = RTI; // Return from Interrupt
    instructionTable[0xEA] = NOP; // No Operation

    // 65C02: INC/DEC A
    instructionTable[0x1A] = INC_A;
    instructionTable[0x3A] = DEC_A;

    // Unofficial NOPs (illegal opcodes) - keep others
    instructionTable[0x5A] = NOP;
    instructionTable[0x7A] = NOP;
    // 65C02 PHX/PLX
    instructionTable[0xDA] = PHX;
    instructionTable[0xFA] = PLX;
    // 65C02 PHY/PLY
    instructionTable[0x5A] = PHY;
    instructionTable[0x7A] = PLY;

    // 65C02 BRA (Branch Always)
    instructionTable[0x80] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Branch(cpu, cycles, memory, true);
    };
    // 65C02 BIT immediate
    instructionTable[0x89] = [](CPU& cpu, u32& cycles, Mem& memory) {
        BIT(cpu, cycles, memory, Addressing::Immediate(cpu, cycles, memory));
    };

    // 65C02 TSB/TRB zero page
    instructionTable[0x04] = [](CPU& cpu, u32& cycles, Mem& memory) {
        TSB(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x14] = [](CPU& cpu, u32& cycles, Mem& memory) {
        TRB(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };
    // 65C02 STZ zero page
    instructionTable[0x64] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STZ(cpu, cycles, memory, Addressing::ZeroPage(cpu, cycles, memory));
    };

    // 65C02 BIT ZeroPage,X
    instructionTable[0x34] = [](CPU& cpu, u32& cycles, Mem& memory) {
        BIT(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };
    // 65C02 STZ ZeroPage,X
    instructionTable[0x74] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STZ(cpu, cycles, memory, Addressing::ZeroPageX(cpu, cycles, memory));
    };

    // 65C02 TSB/TRB absolute
    instructionTable[0x0C] = [](CPU& cpu, u32& cycles, Mem& memory) {
        TSB(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0x1C] = [](CPU& cpu, u32& cycles, Mem& memory) {
        TRB(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };

    // 65C02 BIT Absolute,X
    instructionTable[0x3C] = [](CPU& cpu, u32& cycles, Mem& memory) {
        BIT(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory));
    };
    // 65C02 STZ Absolute/Absolute,X
    instructionTable[0x9C] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STZ(cpu, cycles, memory, Addressing::Absolute(cpu, cycles, memory));
    };
    instructionTable[0x9E] = [](CPU& cpu, u32& cycles, Mem& memory) {
        STZ(cpu, cycles, memory, Addressing::AbsoluteX(cpu, cycles, memory, false));
    };
    // 65C02 JMP Absolute,X
    instructionTable[0x7C] = [](CPU& cpu, u32& cycles, Mem& memory) {
        Word addr = Addressing::AbsoluteX(cpu, cycles, memory, false);
        // JMP abs: 3 cycles; abs,X is 6 cycles total. Account for extra cycles:
        cpu.PC = addr;
        cycles -= 3; // extra timing to reach 6 (FetchWord=2 + this 3 + JMP internal 1)
    };

    // 65C02 Indirect (zp) addressing variants
    instructionTable[0x12] = [](CPU& cpu, u32& cycles, Mem& memory) { // ORA (zp)
        ORA(cpu, cycles, memory, Addressing::IndirectZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x32] = [](CPU& cpu, u32& cycles, Mem& memory) { // AND (zp)
        AND(cpu, cycles, memory, Addressing::IndirectZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x52] = [](CPU& cpu, u32& cycles, Mem& memory) { // EOR (zp)
        EOR(cpu, cycles, memory, Addressing::IndirectZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x72] = [](CPU& cpu, u32& cycles, Mem& memory) { // ADC (zp)
        ADC(cpu, cycles, memory, Addressing::IndirectZeroPage(cpu, cycles, memory));
    };
    instructionTable[0x92] = [](CPU& cpu, u32& cycles, Mem& memory) { // STA (zp)
        STA(cpu, cycles, memory, Addressing::IndirectZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xB2] = [](CPU& cpu, u32& cycles, Mem& memory) { // LDA (zp)
        LDA(cpu, cycles, memory, Addressing::IndirectZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xD2] = [](CPU& cpu, u32& cycles, Mem& memory) { // CMP (zp)
        CMP(cpu, cycles, memory, Addressing::IndirectZeroPage(cpu, cycles, memory));
    };
    instructionTable[0xF2] = [](CPU& cpu, u32& cycles, Mem& memory) { // SBC (zp)
        SBC(cpu, cycles, memory, Addressing::IndirectZeroPage(cpu, cycles, memory));
    };

    // 65C02 RMB (Reset Memory Bit) zero page
    instructionTable[0x07] = [](CPU& cpu, u32& cycles, Mem& memory) { RMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 0); };
    instructionTable[0x17] = [](CPU& cpu, u32& cycles, Mem& memory) { RMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 1); };
    instructionTable[0x27] = [](CPU& cpu, u32& cycles, Mem& memory) { RMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 2); };
    instructionTable[0x37] = [](CPU& cpu, u32& cycles, Mem& memory) { RMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 3); };
    instructionTable[0x47] = [](CPU& cpu, u32& cycles, Mem& memory) { RMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 4); };
    instructionTable[0x57] = [](CPU& cpu, u32& cycles, Mem& memory) { RMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 5); };
    instructionTable[0x67] = [](CPU& cpu, u32& cycles, Mem& memory) { RMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 6); };
    instructionTable[0x77] = [](CPU& cpu, u32& cycles, Mem& memory) { RMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 7); };

    // 65C02 SMB (Set Memory Bit) zero page
    instructionTable[0x87] = [](CPU& cpu, u32& cycles, Mem& memory) { SMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 0); };
    instructionTable[0x97] = [](CPU& cpu, u32& cycles, Mem& memory) { SMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 1); };
    instructionTable[0xA7] = [](CPU& cpu, u32& cycles, Mem& memory) { SMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 2); };
    instructionTable[0xB7] = [](CPU& cpu, u32& cycles, Mem& memory) { SMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 3); };
    instructionTable[0xC7] = [](CPU& cpu, u32& cycles, Mem& memory) { SMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 4); };
    instructionTable[0xD7] = [](CPU& cpu, u32& cycles, Mem& memory) { SMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 5); };
    instructionTable[0xE7] = [](CPU& cpu, u32& cycles, Mem& memory) { SMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 6); };
    instructionTable[0xF7] = [](CPU& cpu, u32& cycles, Mem& memory) { SMB(cpu, cycles, memory, cpu.FetchByte(cycles, memory), 7); };

    // 65C02 BBR/BBS (Zero Page, Relative)
    instructionTable[0x0F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 0, false); };
    instructionTable[0x1F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 1, false); };
    instructionTable[0x2F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 2, false); };
    instructionTable[0x3F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 3, false); };
    instructionTable[0x4F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 4, false); };
    instructionTable[0x5F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 5, false); };
    instructionTable[0x6F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 6, false); };
    instructionTable[0x7F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 7, false); };
    instructionTable[0x8F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 0, true); };
    instructionTable[0x9F] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 1, true); };
    instructionTable[0xAF] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 2, true); };
    instructionTable[0xBF] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 3, true); };
    instructionTable[0xCF] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 4, true); };
    instructionTable[0xDF] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 5, true); };
    instructionTable[0xEF] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 6, true); };
    instructionTable[0xFF] = [](CPU& cpu, u32& cycles, Mem& memory) { BBR_BBS(cpu, cycles, memory, 7, true); };

    // 65C02 WAI/STP
    instructionTable[0xCB] = WAI;
    instructionTable[0xDB] = STP;
}

InstrHandler GetHandler(Byte opcode) {
    return instructionTable[opcode];
}

} // namespace Instructions
