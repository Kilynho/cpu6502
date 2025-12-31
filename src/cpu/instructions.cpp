#include "cpu_instructions.hpp"
#include "cpu.hpp"
#include "cpu_addressing.hpp"
#include "logger.hpp"
#include <array>
#include <sstream>
#include <iomanip>

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
void LDA(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    cpu.A = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, cpu.A, false);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void LDX(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    cpu.X = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, cpu.X, false);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void LDY(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    cpu.Y = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, cpu.Y, false);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

void STA(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    cpu.WriteMemory(address, cpu.A, bus);
    cpu.LogMemoryAccess(address, cpu.A, true);
    cycles--;
}

void STX(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    cpu.WriteMemory(address, cpu.X, bus);
    cpu.LogMemoryAccess(address, cpu.X, true);
    cycles--;
}

void STY(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    cpu.WriteMemory(address, cpu.Y, bus);
    cpu.LogMemoryAccess(address, cpu.Y, true);
    cycles--;
}

// Transfer Instructions
void TAX(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.X = cpu.A;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void TAY(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.Y = cpu.A;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

void TXA(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.A = cpu.X;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void TYA(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.A = cpu.Y;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void TSX(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.X = cpu.SP;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void TXS(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.SP = cpu.X;
    cycles--;
    // TXS does not affect flags
}

// Stack Instructions

void PHA(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.WriteMemory(cpu.SPToAddress(), cpu.A, bus);
    cpu.SP--;
    cycles -= 2;
}


void PHP(CPU& cpu, u32& cycles, SystemMap& bus) {
    // Push processor status with B flag set
    Byte status = (cpu.N << 7) | (cpu.V << 6) | (1 << 5) | (1 << 4) | 
                  (cpu.D << 3) | (cpu.I << 2) | (cpu.Z << 1) | cpu.C;
    bus.write(cpu.SPToAddress(), status);
    cpu.WriteMemory(cpu.SPToAddress(), status, bus);
    cpu.SP--;
    cycles -= 2;
}

void PLA(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.SP++;
    cpu.A = bus.read(cpu.SPToAddress());
    cpu.LogMemoryAccess(cpu.SPToAddress(), cpu.A, false);
    cycles -= 3;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void PLP(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.SP++;
    Byte status = bus.read(cpu.SPToAddress());
    cpu.LogMemoryAccess(cpu.SPToAddress(), status, false);
    cpu.N = (status >> 7) & 1;
    cpu.V = (status >> 6) & 1;
    cpu.D = (status >> 3) & 1;
    cpu.I = (status >> 2) & 1;
    cpu.Z = (status >> 1) & 1;
    cpu.C = status & 1;
    cycles -= 3;
}

// Push/Pop X register (65C02)

void PHX(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.WriteMemory(cpu.SPToAddress(), cpu.X, bus);
    cpu.SP--;
    cycles -= 2;
}


void PLX(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.SP++;
    cpu.X = bus.read(cpu.SPToAddress());
    cpu.LogMemoryAccess(cpu.SPToAddress(), cpu.X, false);
    cycles -= 3;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

// Logical Instructions
void AND(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.A &= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void EOR(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.A ^= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void ORA(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.A |= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void BIT(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.Z = ((cpu.A & value) == 0);
    cpu.N = (value & 0x80) != 0;
    cpu.V = (value & 0x40) != 0;
}

// Arithmetic Instructions
void ADC(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    Word sum = cpu.A + value + cpu.C;
    UpdateCarryFlag(cpu, sum > 0xFF);
    bool overflow = ((cpu.A ^ sum) & (value ^ sum) & 0x80) != 0;
    UpdateOverflowFlag(cpu, overflow);
    cpu.A = sum & 0xFF;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void SBC(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    Word diff = cpu.A - value - (1 - cpu.C);
    UpdateCarryFlag(cpu, diff <= 0xFF);
    bool overflow = ((cpu.A ^ value) & (cpu.A ^ diff) & 0x80) != 0;
    UpdateOverflowFlag(cpu, overflow);
    cpu.A = diff & 0xFF;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

void CMP(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    Word result = cpu.A - value;
    UpdateCarryFlag(cpu, cpu.A >= value);
    UpdateZeroAndNegativeFlags(cpu, result & 0xFF);
}

void CPX(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    Word result = cpu.X - value;
    UpdateCarryFlag(cpu, cpu.X >= value);
    UpdateZeroAndNegativeFlags(cpu, result & 0xFF);
}

void CPY(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    Word result = cpu.Y - value;
    UpdateCarryFlag(cpu, cpu.Y >= value);
    UpdateZeroAndNegativeFlags(cpu, result & 0xFF);
}

// Inc/Dec Instructions
void INC(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    value++;
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, value);
}

void INX(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.X++;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void INY(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.Y++;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

void DEC(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    value--;
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, value);
}

void DEX(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.X--;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.X);
}

void DEY(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.Y--;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

// DEC A (Accumulator) - 65C02

void DECA(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.A--;
    cycles--;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

// Shift Instructions
void ASL(CPU& cpu, u32& cycles, SystemMap& bus, Word address, bool accumulator) {
    if (accumulator) {
        Byte old = cpu.A;
        cpu.C = (old & 0x80) != 0;
        cpu.A <<= 1;
        UpdateZeroAndNegativeFlags(cpu, cpu.A);
        cycles--;
    } else {
        Byte value = cpu.ReadMemory(address, bus);
        cpu.C = (value & 0x80) != 0;
        value <<= 1;
        cpu.WriteMemory(address, value, bus);
        UpdateZeroAndNegativeFlags(cpu, value);
        cycles--;
    }
}

void LSR(CPU& cpu, u32& cycles, SystemMap& bus, Word address, bool accumulator) {
    if (accumulator) {
        cpu.C = cpu.A & 0x01;
        cpu.A >>= 1;
        UpdateZeroAndNegativeFlags(cpu, cpu.A);
        cycles--;
    } else {
        Byte value = cpu.ReadMemory(address, bus);
        cpu.C = value & 0x01;
        value >>= 1;
        cpu.WriteMemory(address, value, bus);
        UpdateZeroAndNegativeFlags(cpu, value);
        cycles--;
    }
}

void ROL(CPU& cpu, u32& cycles, SystemMap& bus, Word address, bool accumulator) {
    if (accumulator) {
        Byte old = cpu.A;
        bool newC = (old & 0x80) != 0;
        cpu.A = (cpu.A << 1) | cpu.C;
        cpu.C = newC;
        UpdateZeroAndNegativeFlags(cpu, cpu.A);
        cycles--;
    } else {
        Byte value = cpu.ReadMemory(address, bus);
        bool newC = (value & 0x80) != 0;
        value = (value << 1) | cpu.C;
        cpu.C = newC;
        cpu.WriteMemory(address, value, bus);
        UpdateZeroAndNegativeFlags(cpu, value);
        cycles--;
    }
}

void ROR(CPU& cpu, u32& cycles, SystemMap& bus, Word address, bool accumulator) {
    if (accumulator) {
        bool newC = cpu.A & 0x01;
        cpu.A = (cpu.A >> 1) | (cpu.C << 7);
        cpu.C = newC;
        UpdateZeroAndNegativeFlags(cpu, cpu.A);
        cycles--;
    } else {
        Byte value = cpu.ReadMemory(address, bus);
        bool newC = value & 0x01;
        value = (value >> 1) | (cpu.C << 7);
        cpu.C = newC;
        cpu.WriteMemory(address, value, bus);
        UpdateZeroAndNegativeFlags(cpu, value);
        cycles--;
    }
}

// Test and Set Bits (TSB) - 65C02

void TSB(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;

    Byte result = value & cpu.A;
    cpu.Z = (result == 0);

    value = value | cpu.A;
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;
}

// Test and Reset Bits (TRB) - 65C02

void TRB(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;

    Byte result = value & cpu.A;
    cpu.Z = (result == 0);

    value = value & (~cpu.A);
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;
}

// STZ - Store Zero (65C02)

void STZ(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    cpu.WriteMemory(address, 0x00, bus);
    cpu.LogMemoryAccess(address, 0x00, true);
    cycles--;
}

// PHY / PLY - Push/Pull Y (65C02)

void PHY(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.WriteMemory(cpu.SPToAddress(), cpu.Y, bus);
    cpu.SP--;
    cycles -= 2;
}


void PLY(CPU& cpu, u32& cycles, SystemMap& bus) {
    cpu.SP++;
    cpu.Y = bus.read(cpu.SPToAddress());
    cpu.LogMemoryAccess(cpu.SPToAddress(), cpu.Y, false);
    cycles -= 3;
    UpdateZeroAndNegativeFlags(cpu, cpu.Y);
}

// Jump/Branch Instructions
void JMP(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    cpu.PC = address;
    cycles--;
}

void JSR(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    // Push return address (address of last byte of JSR operand)
    Word returnAddress = cpu.PC - 1;
    Byte returnLow = static_cast<Byte>(returnAddress & 0x00FF);
    Byte returnHigh = static_cast<Byte>((returnAddress >> 8) & 0x00FF);

    bus.write(cpu.SPToAddress(), returnHigh);
    cpu.SP--;
    bus.write(cpu.SPToAddress(), returnLow);
    cpu.SP--;

    cpu.PC = address;
    // Account remaining cycles (fetch already consumed 3)
    cycles -= 3;
}

void RTS(CPU& cpu, u32& cycles, SystemMap& bus) {
    // Pull return address from stack (low byte first)
    cpu.SP++;
    Byte low = bus.read(cpu.SPToAddress());
    cpu.SP++;
    Byte high = bus.read(cpu.SPToAddress());
    cpu.PC = static_cast<Word>((high << 8) | low);
    cpu.PC++;
    // Account remaining cycles (fetch already consumed 1)
    cycles -= 5;
}

void Branch(CPU& cpu, u32& cycles, SystemMap& bus, bool condition) {
    Byte offset = bus.read(cpu.PC);
    cpu.PC++; // move past offset
    cycles--; // fetch offset

    if (condition) {
        int8_t rel = static_cast<int8_t>(offset);
        cpu.PC = static_cast<Word>(cpu.PC + rel);
        cycles--; // branch taken cost
    }
}

// Flag Instructions
void CLC(CPU& cpu, u32& cycles, SystemMap& bus) { cpu.C = 0; cycles--; }
void CLD(CPU& cpu, u32& cycles, SystemMap& bus) { cpu.D = 0; cycles--; }
void CLI(CPU& cpu, u32& cycles, SystemMap& bus) { cpu.I = 0; cycles--; }
void CLV(CPU& cpu, u32& cycles, SystemMap& bus) { cpu.V = 0; cycles--; }
void SEC(CPU& cpu, u32& cycles, SystemMap& bus) { cpu.C = 1; cycles--; }
void SED(CPU& cpu, u32& cycles, SystemMap& bus) { cpu.D = 1; cycles--; }
void SEI(CPU& cpu, u32& cycles, SystemMap& bus) { cpu.I = 1; cycles--; }

// System Instructions
void BRK(CPU& cpu, u32& cycles, SystemMap& bus) { /* TODO: implementar interrupción */ cycles--; }
void RTI(CPU& cpu, u32& cycles, SystemMap& bus) { /* TODO: migrar PullPCFromStack a SystemMap& bus */ cycles--; }
void NOP(CPU& cpu, u32& cycles, SystemMap& bus) { cycles--; }

// ---------- Undocumented / common illegal opcodes (functional implementations) ----------

// SLO - ASL then ORA (memory)

void SLO(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;

    // ASL
    UpdateCarryFlag(cpu, (value & 0x80) != 0);
    value <<= 1;
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;

    // ORA with resulting value
    cpu.A |= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

// RLA - ROL then AND (memory)

void RLA(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;

    bool oldCarry = cpu.C;
    UpdateCarryFlag(cpu, (value & 0x80) != 0);
    value = (value << 1) | (oldCarry ? 1 : 0);
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;

    cpu.A &= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

// SRE - LSR then EOR (memory)

void SRE(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;

    UpdateCarryFlag(cpu, (value & 0x01) != 0);
    value >>= 1;
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;

    cpu.A ^= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

// RRA - ROR then ADC (memory)

void RRA(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;

    bool oldCarry = cpu.C;
    UpdateCarryFlag(cpu, (value & 0x01) != 0);
    value = (value >> 1) | (oldCarry ? 0x80 : 0);
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;

    // ADC with value (without duplicating ADC flags logic precisely)
    Word sum = cpu.A + value + cpu.C;
    UpdateCarryFlag(cpu, sum > 0xFF);
    bool overflow = ((cpu.A ^ sum) & (value ^ sum) & 0x80) != 0;
    UpdateOverflowFlag(cpu, overflow);
    cpu.A = sum & 0xFF;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

// DCP - DEC then CMP

void DCP(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;

    value--;
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;

    Word result = cpu.A - value;
    UpdateCarryFlag(cpu, cpu.A >= value);
    UpdateZeroAndNegativeFlags(cpu, result & 0xFF);
}

// ISB (a.k.a. INS) - INC then SBC

void ISB(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;

    value++;
    cpu.WriteMemory(address, value, bus);
    cpu.LogMemoryAccess(address, value, true);
    cycles--;

    // SBC with value
    Word diff = cpu.A - value - (1 - cpu.C);
    UpdateCarryFlag(cpu, diff <= 0xFF);
    bool overflow = ((cpu.A ^ value) & (cpu.A ^ diff) & 0x80) != 0;
    UpdateOverflowFlag(cpu, overflow);
    cpu.A = diff & 0xFF;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

// LAX - Load A and X

void LAX(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte value = cpu.ReadMemory(address, bus);
    cpu.LogMemoryAccess(address, value, false);
    cycles--;
    cpu.A = value;
    cpu.X = value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
}

// SAX - Store A & X
void SAX(CPU& cpu, u32& cycles, SystemMap& bus, Word address) {
    Byte out = cpu.A & cpu.X;
    cpu.WriteMemory(address, out, bus);
    cpu.LogMemoryAccess(address, out, true);
    cycles--;
}

// ANC - AND then set C from bit 7 (immediate)
void ANC(CPU& cpu, u32& cycles, SystemMap& bus) {
    Byte value = cpu.FetchByte(cycles, bus);
    cpu.A &= value;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
    cpu.C = (cpu.A & 0x80) ? 1 : 0;
    cycles--;
}

// ALR - AND then LSR (immediate)
void ALR(CPU& cpu, u32& cycles, SystemMap& bus) {
    Byte value = cpu.FetchByte(cycles, bus);
    cpu.A &= value;
    UpdateCarryFlag(cpu, (cpu.A & 0x01) != 0);
    cpu.A >>= 1;
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
    cycles--;
}

// ARR - AND then ROR (immediate) - approximate behavior
void ARR(CPU& cpu, u32& cycles, SystemMap& bus) {
    Byte value = cpu.FetchByte(cycles, bus);
    cpu.A &= value;
    bool oldCarry = cpu.C;
    UpdateCarryFlag(cpu, (cpu.A & 0x01) != 0);
    cpu.A = (cpu.A >> 1) | (oldCarry ? 0x80 : 0);
    UpdateZeroAndNegativeFlags(cpu, cpu.A);
    cycles--;
}

// Initialize instruction table with all 256 opcodes
void InitializeInstructionTable() {
    // Initialize all opcodes to a default handler that logs the opcode
    for (int i = 0; i < 256; i++) {
        instructionTable[i] = [i](CPU& cpu, u32& cycles, SystemMap& bus) {
            std::ostringstream _oss;
            _oss << "Unimplemented opcode: 0x" << std::hex << std::setw(2) << std::setfill('0') << (i & 0xFF);
            util::LogWarn(_oss.str());
            // Consume at least one cycle to avoid tight loops
            cycles--;
        };
    }
    
    // LDA - Load Accumulator
    instructionTable[0xA9] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDA(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0xA5] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDA(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xB5] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDA(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0xAD] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDA(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0xBD] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDA(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus));
    };
    instructionTable[0xB9] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDA(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus));
    };
    instructionTable[0xA1] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDA(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus));
    };
    instructionTable[0xB1] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDA(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus));
    };
    
    // LDX - Load X Register
    instructionTable[0xA2] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDX(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0xA6] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDX(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xB6] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDX(cpu, cycles, bus, Addressing::ZeroPageY(cpu, cycles, bus));
    };
    instructionTable[0xAE] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDX(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0xBE] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDX(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus));
    };
    // INX - Increment X Register
    instructionTable[0xE8] = INX;
    
    // LDY - Load Y Register
    instructionTable[0xA0] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDY(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0xA4] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDY(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xB4] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDY(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0xAC] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDY(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0xBC] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LDY(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus));
    };
    
    // STA - Store Accumulator
    instructionTable[0x85] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STA(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x95] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STA(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0x8D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STA(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0x9D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STA(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false));
    };
    instructionTable[0x99] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STA(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus, false));
    };
    instructionTable[0x81] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STA(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus));
    };
    instructionTable[0x91] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STA(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus, false));
    };
    
    // STX - Store X Register
    instructionTable[0x86] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STX(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x96] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STX(cpu, cycles, bus, Addressing::ZeroPageY(cpu, cycles, bus));
    };
    instructionTable[0x8E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STX(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    
    // STY - Store Y Register
    instructionTable[0x84] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STY(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x94] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STY(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0x8C] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STY(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
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
    // 65C02 Push/Pop X
    instructionTable[0xDA] = PHX; // PHX - push X
    instructionTable[0xFA] = PLX; // PLX - pull X
    
    // Logical Instructions - AND
    instructionTable[0x29] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        AND(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0x25] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        AND(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x35] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        AND(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0x2D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        AND(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0x3D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        AND(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus));
    };
    instructionTable[0x39] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        AND(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus));
    };
    instructionTable[0x21] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        AND(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus));
    };
    instructionTable[0x31] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        AND(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus));
    };
    
    // Logical Instructions - EOR
    instructionTable[0x49] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        EOR(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0x45] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        EOR(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x55] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        EOR(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0x4D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        EOR(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0x5D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        EOR(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus));
    };
    instructionTable[0x59] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        EOR(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus));
    };
    instructionTable[0x41] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        EOR(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus));
    };
    instructionTable[0x51] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        EOR(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus));
    };
    
    // Logical Instructions - ORA
    instructionTable[0x09] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ORA(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0x05] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ORA(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x15] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ORA(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0x0D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ORA(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0x1D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ORA(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus));
    };
    instructionTable[0x19] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ORA(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus));
    };
    instructionTable[0x01] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ORA(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus));
    };
    instructionTable[0x11] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ORA(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus));
    };
    
    // BIT - Bit Test
    instructionTable[0x24] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        BIT(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x2C] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        BIT(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    
    // ADC - Add with Carry
    instructionTable[0x69] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ADC(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0x65] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ADC(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x75] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ADC(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0x6D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ADC(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0x7D] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ADC(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus));
    };
    instructionTable[0x79] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ADC(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus));
    };
    instructionTable[0x61] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ADC(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus));
    };
    instructionTable[0x71] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ADC(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus));
    };
    
    // SBC - Subtract with Carry
    instructionTable[0xE9] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        SBC(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0xE5] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        SBC(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xF5] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        SBC(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0xED] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        SBC(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0xFD] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        SBC(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus));
    };
    instructionTable[0xF9] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        SBC(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus));
    };
    instructionTable[0xE1] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        SBC(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus));
    };
    instructionTable[0xF1] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        SBC(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus));
    };
    
    // CMP - Compare Accumulator
    instructionTable[0xC9] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CMP(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0xC5] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CMP(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xD5] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CMP(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0xCD] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CMP(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0xDD] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CMP(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus));
    };
    instructionTable[0xD9] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CMP(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus));
    };
    instructionTable[0xC1] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CMP(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus));
    };
    instructionTable[0xD1] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CMP(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus));
    };
    
    // CPX - Compare X Register
    instructionTable[0xE0] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CPX(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0xE4] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CPX(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xEC] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CPX(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    
    // CPY - Compare Y Register
    instructionTable[0xC0] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CPY(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };
    instructionTable[0xC4] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CPY(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xCC] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        CPY(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    
    // INC - Increment Memory
    instructionTable[0xE6] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        INC(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xE6] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        INC(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xF6] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        INC(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0xEE] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        INC(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0xFE] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        INC(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus));
    };
    instructionTable[0xC8] = INY;
    
    // DEC - Decrement Memory
    instructionTable[0xC6] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        DEC(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0xD6] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        DEC(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0xCE] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        DEC(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0xDE] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        DEC(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false));
    };
    
    // DEX, DEY
    instructionTable[0xCA] = DEX;
    instructionTable[0x88] = DEY;
    // DEC A (Accumulator) - 65C02
    instructionTable[0x3A] = DECA;
    
    // Shift Instructions
    instructionTable[0x0A] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ASL(cpu, cycles, bus, 0, true);
    };
    instructionTable[0x06] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ASL(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus), false);
    };
    instructionTable[0x16] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ASL(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus), false);
    };
    instructionTable[0x0E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ASL(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus), false);
    };
    instructionTable[0x1E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ASL(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false), false);
    };
    
    // LSR - Logical Shift Right
    instructionTable[0x4A] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LSR(cpu, cycles, bus, 0, true);
    };
    instructionTable[0x46] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LSR(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus), false);
    };
    instructionTable[0x56] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LSR(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus), false);
    };
    instructionTable[0x4E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LSR(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus), false);
    };
    instructionTable[0x5E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        LSR(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false), false);
    };
    
    // ROL - Rotate Left
    instructionTable[0x2A] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROL(cpu, cycles, bus, 0, true);
    };
    instructionTable[0x26] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROL(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus), false);
    };
    instructionTable[0x36] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROL(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus), false);
    };
    instructionTable[0x2E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROL(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus), false);
    };
    instructionTable[0x3E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROL(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false), false);
    };
    
    // ROR - Rotate Right
    instructionTable[0x6A] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROR(cpu, cycles, bus, 0, true);
    };
    instructionTable[0x66] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROR(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus), false);
    };
    instructionTable[0x76] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROR(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus), false);
    };
    instructionTable[0x6E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROR(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus), false);
    };
    instructionTable[0x7E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        ROR(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false), false);
    };
    
    // JMP - Jump
    instructionTable[0x4C] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        JMP(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0x6C] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        JMP(cpu, cycles, bus, Addressing::Indirect(cpu, cycles, bus));
    };

    // 65C02 additions
    // BIT immediate
    instructionTable[0x89] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        BIT(cpu, cycles, bus, Addressing::Immediate(cpu, cycles, bus));
    };

    // TSB / TRB
    instructionTable[0x04] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        TSB(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x0C] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        TSB(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0x14] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        TRB(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x1C] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        TRB(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };

    // STZ
    instructionTable[0x64] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STZ(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus));
    };
    instructionTable[0x74] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STZ(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus));
    };
    instructionTable[0x9C] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STZ(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus));
    };
    instructionTable[0x9E] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        STZ(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false));
    };

    // BRA - Branch Always
    instructionTable[0x80] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Branch(cpu, cycles, bus, true);
    };
    // PER, WAI, STP
    // instructionTable[0x62] = PER; // PER - Push Effective Relative (stubbed)
    // instructionTable[0xCB] = WAI; // WAI - Wait for Interrupt (simplified) (stubbed)
    // instructionTable[0xDB] = STP; // STP - Stop Processor (simplified) (stubbed)

    // PHY / PLY
    instructionTable[0x5A] = PHY;
    instructionTable[0x7A] = PLY;
    
    // JSR - Jump to Subroutine
    instructionTable[0x20] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Word address = cpu.FetchWord(cycles, bus);
        JSR(cpu, cycles, bus, address);
    };
    
    // RTS - Return from Subroutine
    instructionTable[0x60] = RTS;
    
    // Branch Instructions
    instructionTable[0x10] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Branch(cpu, cycles, bus, cpu.N == 0); // BPL - Branch if Positive
    };
    instructionTable[0x30] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Branch(cpu, cycles, bus, cpu.N == 1); // BMI - Branch if Minus
    };
    instructionTable[0x50] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Branch(cpu, cycles, bus, cpu.V == 0); // BVC - Branch if Overflow Clear
    };
    instructionTable[0x70] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Branch(cpu, cycles, bus, cpu.V == 1); // BVS - Branch if Overflow Set
    };
    instructionTable[0x90] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Branch(cpu, cycles, bus, cpu.C == 0); // BCC - Branch if Carry Clear
    };
    // RMB / SMB (reset/set bit in zero page) - opcodes with low nibble 0x7
    for (int hi = 0; hi < 16; ++hi) {
        Byte op = (hi << 4) | 0x7;
        int bit = (hi & 0x7);
        bool isSMB = (op & 0x80) != 0; // high bit selects SMB group
        instructionTable[op] = [bit, isSMB](CPU& cpu, u32& cycles, SystemMap& bus) {
            // RMB_SMB(cpu, cycles, bus, bit, isSMB); // stubbed
            util::LogWarn("RMB/SMB opcode stubbed");
            cycles--;
        };
    }

    // BBR / BBS (branch if bit reset/set) - opcodes with low nibble 0xF
    for (int hi = 0; hi < 16; ++hi) {
        Byte op = (hi << 4) | 0xF;
        int bit = (hi & 0x7);
        bool isBBS = (op & 0x80) != 0; // high bit selects BBS group
        instructionTable[op] = [bit, isBBS](CPU& cpu, u32& cycles, SystemMap& bus) {
            // BBR_BBS_Handler(cpu, cycles, bus, bit, isBBS, 0); // stubbed
            util::LogWarn("BBR/BBS opcode stubbed");
            cycles--;
        };
    }

    // MVN / MVP block move (65C02) - minimal implementation
    instructionTable[0x54] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        // MVN srcBank, dstBank, count
        Byte src = cpu.FetchByte(cycles, bus);
        Byte dst = cpu.FetchByte(cycles, bus);
        Byte cnt = cpu.FetchByte(cycles, bus);
        // Minimal: just consume cycles and log - full implementation requires 24-bit addressing
        util::LogWarn("MVN executed - minimal stub (no actual block move)");
        cycles -= 3;
    };
    instructionTable[0x44] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        // MVP srcBank, dstBank, count - minimal stub
        Byte src = cpu.FetchByte(cycles, bus);
        Byte dst = cpu.FetchByte(cycles, bus);
        Byte cnt = cpu.FetchByte(cycles, bus);
        util::LogWarn("MVP executed - minimal stub (no actual block move)");
        cycles -= 3;
    };

    // ---------- Map common undocumented opcodes (families) ----------
    // SLO family (ASL then ORA)
    instructionTable[0x03] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SLO(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus)); };
    instructionTable[0x07] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SLO(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus)); };
    instructionTable[0x0F] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SLO(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus)); };
    instructionTable[0x13] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SLO(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus)); };
    instructionTable[0x17] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SLO(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus)); };
    instructionTable[0x1F] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SLO(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false)); };

    // RLA family (ROL then AND)
    instructionTable[0x23] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RLA(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus)); };
    instructionTable[0x27] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RLA(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus)); };
    instructionTable[0x2F] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RLA(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus)); };
    instructionTable[0x33] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RLA(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus)); };
    instructionTable[0x37] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RLA(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus)); };
    instructionTable[0x3F] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RLA(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false)); };

    // SRE family (LSR then EOR)
    instructionTable[0x43] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SRE(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus)); };
    instructionTable[0x47] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SRE(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus)); };
    instructionTable[0x4F] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SRE(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus)); };
    instructionTable[0x53] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SRE(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus)); };
    instructionTable[0x57] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SRE(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus)); };
    instructionTable[0x5F] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SRE(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false)); };

    // RRA family (ROR then ADC)
    instructionTable[0x63] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RRA(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus)); };
    instructionTable[0x67] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RRA(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus)); };
    instructionTable[0x6F] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RRA(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus)); };
    instructionTable[0x73] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RRA(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus)); };
    instructionTable[0x77] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RRA(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus)); };
    instructionTable[0x7F] = [](CPU& cpu, u32& cycles, SystemMap& bus) { RRA(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false)); };

    // DCP family (DEC then CMP)
    instructionTable[0xC3] = [](CPU& cpu, u32& cycles, SystemMap& bus) { DCP(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus)); };
    instructionTable[0xC7] = [](CPU& cpu, u32& cycles, SystemMap& bus) { DCP(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus)); };
    instructionTable[0xCF] = [](CPU& cpu, u32& cycles, SystemMap& bus) { DCP(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus)); };
    instructionTable[0xD3] = [](CPU& cpu, u32& cycles, SystemMap& bus) { DCP(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus)); };
    instructionTable[0xD7] = [](CPU& cpu, u32& cycles, SystemMap& bus) { DCP(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus)); };
    instructionTable[0xDF] = [](CPU& cpu, u32& cycles, SystemMap& bus) { DCP(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false)); };

    // ISB family (INC then SBC)
    instructionTable[0xE3] = [](CPU& cpu, u32& cycles, SystemMap& bus) { ISB(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus)); };
    instructionTable[0xE7] = [](CPU& cpu, u32& cycles, SystemMap& bus) { ISB(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus)); };
    instructionTable[0xEF] = [](CPU& cpu, u32& cycles, SystemMap& bus) { ISB(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus)); };
    instructionTable[0xF3] = [](CPU& cpu, u32& cycles, SystemMap& bus) { ISB(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus)); };
    instructionTable[0xF7] = [](CPU& cpu, u32& cycles, SystemMap& bus) { ISB(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus)); };
    instructionTable[0xFF] = [](CPU& cpu, u32& cycles, SystemMap& bus) { ISB(cpu, cycles, bus, Addressing::AbsoluteX(cpu, cycles, bus, false)); };

    // LAX family (load A and X)
    instructionTable[0xA3] = [](CPU& cpu, u32& cycles, SystemMap& bus) { LAX(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus)); };
    instructionTable[0xA7] = [](CPU& cpu, u32& cycles, SystemMap& bus) { LAX(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus)); };
    instructionTable[0xAF] = [](CPU& cpu, u32& cycles, SystemMap& bus) { LAX(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus)); };
    instructionTable[0xB3] = [](CPU& cpu, u32& cycles, SystemMap& bus) { LAX(cpu, cycles, bus, Addressing::IndirectY(cpu, cycles, bus)); };
    instructionTable[0xB7] = [](CPU& cpu, u32& cycles, SystemMap& bus) { LAX(cpu, cycles, bus, Addressing::ZeroPageX(cpu, cycles, bus)); };
    instructionTable[0xBF] = [](CPU& cpu, u32& cycles, SystemMap& bus) { LAX(cpu, cycles, bus, Addressing::AbsoluteY(cpu, cycles, bus)); };

    // SAX family (store A & X)
    instructionTable[0x83] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SAX(cpu, cycles, bus, Addressing::IndirectX(cpu, cycles, bus)); };
    instructionTable[0x87] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SAX(cpu, cycles, bus, Addressing::ZeroPage(cpu, cycles, bus)); };
    instructionTable[0x8F] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SAX(cpu, cycles, bus, Addressing::Absolute(cpu, cycles, bus)); };
    instructionTable[0x97] = [](CPU& cpu, u32& cycles, SystemMap& bus) { SAX(cpu, cycles, bus, Addressing::ZeroPageY(cpu, cycles, bus)); };

    // ANC / ALR / ARR immediate variants
    instructionTable[0x0B] = ANC;
    instructionTable[0x2B] = ANC;
    instructionTable[0x4B] = ALR;
    instructionTable[0x6B] = ARR;

    // Generic handlers for any remaining undocumented opcodes that weren't mapped above.
    // These will log the opcode and consume one operand byte if present to advance PC.
    auto make_generic_undoc = [](Byte op) {
        return [op](CPU& cpu, u32& cycles, SystemMap& bus) {
            std::ostringstream _oss;
            _oss << "Undocumented opcode executed: 0x" << std::hex << std::setw(2) << std::setfill('0') << (op & 0xFF);
            util::LogWarn(_oss.str());
            // Try to consume one operand byte safely so PC advances; this avoids tight loops.
            cpu.FetchByte(cycles, bus);
            cycles--;
        };
    };

    // Assign generic handler for any table slots still containing the default logger
    for (int i = 0; i < 256; ++i) {
        // If the entry still matches the default logger (we set it earlier), replace it.
        // We detect default by calling GetHandler and comparing to nullptr isn't possible,
        // so instead for safety, assign generic handlers only where the current handler
        // will still log "Unimplemented opcode" pattern. We'll conservatively overwrite
        // entries that haven't been specifically set by checking a small whitelist of
        // opcodes we expect were implemented. To keep things simple, overwrite any
        // entry that currently logs the default by assigning our generic handler.
        // (This loop will overwrite some that were already set above only if they
        // remain with the original lambda; in practice earlier explicit assignments
        // remain because they were set after defaults.)
        // For determinism, just fill any remaining empty slots using our maker.
        instructionTable[i] = instructionTable[i];
    }

    // Now explicitly assign the remaining known-missing opcodes to a generic handler
    Byte remaining[] = {0x02,0x12,0x1A,0x1B,0x22,0x32,0x34,0x3B,0x3C,0x42,0x52,0x5B,0x5C,0x72,0x7B,0x7C,0x82,0x8B,0x92,0x93,0x9B,0x9F,0xAB,0xB2,0xBB,0xC2,0xD2,0xD4,0xDC,0xE2,0xEB,0xF2,0xF4,0xFB,0xFC};
    for (Byte op : remaining) {
        instructionTable[op] = make_generic_undoc(op);
    }
    instructionTable[0xB0] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Branch(cpu, cycles, bus, cpu.C == 1); // BCS - Branch if Carry Set
    };
    instructionTable[0xD0] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Branch(cpu, cycles, bus, cpu.Z == 0); // BNE - Branch if Not Equal
    };
    instructionTable[0xF0] = [](CPU& cpu, u32& cycles, SystemMap& bus) {
        Branch(cpu, cycles, bus, cpu.Z == 1); // BEQ - Branch if Equal
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

    // Provide opcode metadata for CPU to query. Keep in sync with InitializeInstructionTable.
    auto GetOpcodeInfo = [](Byte opcode) -> OpcodeInfo {
        switch (opcode) {
            case 0x20: return {6, 3}; // JSR
            case 0x60: return {6, 1}; // RTS
            case 0xA9: return {2, 2}; // LDA immediate
            case 0xA5: return {3, 2}; // LDA zero page
            case 0xB5: return {4, 2}; // LDA zero page,X
            case 0xAD: return {4, 3}; // LDA absolute
            case 0xBD: return {4, 3}; // LDA absolute,X
            case 0xB9: return {4, 3}; // LDA absolute,Y
            case 0xA1: return {6, 2}; // LDA (indirect,X)
            case 0xB1: return {5, 2}; // LDA (indirect),Y
            case 0xA2: return {2, 2}; // LDX immediate
            case 0xA6: return {3, 2}; // LDX zero page
            case 0xB6: return {4, 2}; // LDX zero page,Y
            case 0xAE: return {4, 3}; // LDX absolute
            case 0xBE: return {4, 3}; // LDX absolute,Y
            case 0xA0: return {2, 2}; // LDY immediate
            case 0xA4: return {3, 2}; // LDY zero page
            case 0xB4: return {4, 2}; // LDY zero page,X
            case 0xAC: return {4, 3}; // LDY absolute
            case 0xBC: return {4, 3}; // LDY absolute,X
            case 0x85: return {3, 2}; // STA zero page
            case 0x95: return {4, 2}; // STA zero page,X
            case 0x8D: return {4, 3}; // STA absolute
            case 0x9D: return {5, 3}; // STA absolute,X
            case  0x99: return {5, 3}; // STA absolute,Y
            case 0x81: return {6, 2}; // STA (indirect,X)
            case 0x91: return {6, 2}; // STA (indirect),Y
            default: return {0,1};
        }
    };

    // Export functions
    instructionTable[0xFF] = [GetOpcodeInfo](CPU& cpu, u32& cycles, SystemMap& bus) { /* no-op placeholder */ };
    // Note: real GetHandler/GetOpcodeInfo functions below use instructionTable and the local mapping.
}

InstrHandler GetHandler(Byte opcode) {
    return instructionTable[opcode];
}

OpcodeInfo GetOpcodeInfo(Byte opcode) {
    // Delegate to the same mapping used in InitializeInstructionTable.
    switch (opcode) {
        case 0x20: return {6, 3};
        case 0x60: return {6, 1};
        case 0xA9: return {2, 2};
        case 0xA5: return {3, 2};
        case 0xB5: return {4, 2};
        case 0xAD: return {4, 3};
        case 0xBD: return {4, 3};
        case 0xB9: return {4, 3};
        case 0xA1: return {6, 2};
        case 0xB1: return {5, 2};
        case 0xA2: return {2, 2};
        case 0xA6: return {3, 2};
        case 0xB6: return {4, 2};
        case 0xAE: return {4, 3};
        case 0xBE: return {4, 3};
        case 0xA0: return {2, 2};
        case 0xA4: return {3, 2};
        case 0xB4: return {4, 2};
        case 0xAC: return {4, 3};
        case 0xBC: return {4, 3};
        case 0x85: return {3, 2};
        case 0x95: return {4, 2};
        case 0x8D: return {4, 3};
        case 0x9D: return {5, 3};
        case 0x99: return {5, 3};
        case 0x81: return {6, 2};
        case 0x91: return {6, 2};
        default: return {0,1};
    }
}

} // namespace Instructions
