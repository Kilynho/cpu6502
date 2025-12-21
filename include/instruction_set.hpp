#pragma once
#include <cstdint>
#include <array>
#include <string>

namespace Instructions {

// Instruction metadata for all 256 opcodes
struct InstructionMetadata {
    uint8_t opcode;
    const char* mnemonic;
    const char* addressing_mode;  // "Implied", "Accumulator", "Immediate", "Zero Page", etc.
    uint8_t cycles;                // 65C02 timing
    const char* description;
    bool is_65c02_only;            // true if this is a new 65C02 opcode
};

// Complete instruction set reference: all 256 possible opcodes
// Organized by function family for clarity
class InstructionSet {
public:
    static constexpr std::array<InstructionMetadata, 256> OPCODES = {{
        // 0x00 - 0x0F: BRK, ORA, undocumented, TSB, etc.
        {0x00, "BRK", "Implied", 7, "Break (software interrupt)", false},
        {0x01, "ORA", "(Indirect,X)", 6, "Bitwise OR with accumulator", false},
        {0x02, "---", "Implied", 2, "Unimplemented", false},
        {0x03, "---", "Implied", 2, "Unimplemented", false},
        {0x04, "TSB", "Zero Page", 5, "Test and Set Bits", true},
        {0x05, "ORA", "Zero Page", 3, "Bitwise OR with accumulator", false},
        {0x06, "ASL", "Zero Page", 5, "Arithmetic Shift Left", false},
        {0x07, "RMB0", "Zero Page", 5, "Reset Memory Bit 0", true},
        {0x08, "PHP", "Implied", 3, "Push Processor Status", false},
        {0x09, "ORA", "Immediate", 2, "Bitwise OR with accumulator", false},
        {0x0A, "ASL", "Accumulator", 2, "Arithmetic Shift Left", false},
        {0x0B, "---", "Implied", 2, "Unimplemented", false},
        {0x0C, "TSB", "Absolute", 6, "Test and Set Bits", true},
        {0x0D, "ORA", "Absolute", 4, "Bitwise OR with accumulator", false},
        {0x0E, "ASL", "Absolute", 6, "Arithmetic Shift Left", false},
        {0x0F, "BBR0", "Relative", 5, "Branch on Bit Reset 0", true},

        // 0x10 - 0x1F: BPL, ORA, undocumented, BIT, etc.
        {0x10, "BPL", "Relative", 2, "Branch if Plus", false},
        {0x11, "ORA", "(Indirect),Y", 5, "Bitwise OR with accumulator", false},
        {0x12, "ORA", "(Zero Page)", 5, "Bitwise OR with accumulator", true},
        {0x13, "---", "Implied", 2, "Unimplemented", false},
        {0x14, "TRB", "Zero Page", 5, "Test and Reset Bits", true},
        {0x15, "ORA", "Zero Page,X", 4, "Bitwise OR with accumulator", false},
        {0x16, "ASL", "Zero Page,X", 6, "Arithmetic Shift Left", false},
        {0x17, "RMB1", "Zero Page", 5, "Reset Memory Bit 1", true},
        {0x18, "CLC", "Implied", 2, "Clear Carry", false},
        {0x19, "ORA", "Absolute,Y", 4, "Bitwise OR with accumulator", false},
        {0x1A, "INC", "Accumulator", 2, "Increment Accumulator", true},
        {0x1B, "---", "Implied", 2, "Unimplemented", false},
        {0x1C, "TRB", "Absolute", 6, "Test and Reset Bits", true},
        {0x1D, "ORA", "Absolute,X", 4, "Bitwise OR with accumulator", false},
        {0x1E, "ASL", "Absolute,X", 7, "Arithmetic Shift Left", false},
        {0x1F, "BBR1", "Relative", 5, "Branch on Bit Reset 1", true},

        // 0x20 - 0x2F: JSR, AND, undocumented, BIT, etc.
        {0x20, "JSR", "Absolute", 6, "Jump to Subroutine", false},
        {0x21, "AND", "(Indirect,X)", 6, "Bitwise AND with accumulator", false},
        {0x22, "---", "Implied", 2, "Unimplemented", false},
        {0x23, "---", "Implied", 2, "Unimplemented", false},
        {0x24, "BIT", "Zero Page", 3, "Bit Test", false},
        {0x25, "AND", "Zero Page", 3, "Bitwise AND with accumulator", false},
        {0x26, "ROL", "Zero Page", 5, "Rotate Left", false},
        {0x27, "RMB2", "Zero Page", 5, "Reset Memory Bit 2", true},
        {0x28, "PLP", "Implied", 4, "Pull Processor Status", false},
        {0x29, "AND", "Immediate", 2, "Bitwise AND with accumulator", false},
        {0x2A, "ROL", "Accumulator", 2, "Rotate Left", false},
        {0x2B, "---", "Implied", 2, "Unimplemented", false},
        {0x2C, "BIT", "Absolute", 4, "Bit Test", false},
        {0x2D, "AND", "Absolute", 4, "Bitwise AND with accumulator", false},
        {0x2E, "ROL", "Absolute", 6, "Rotate Left", false},
        {0x2F, "BBR2", "Relative", 5, "Branch on Bit Reset 2", true},

        // 0x30 - 0x3F: BMI, EOR, undocumented, etc.
        {0x30, "BMI", "Relative", 2, "Branch if Minus", false},
        {0x31, "AND", "(Indirect),Y", 5, "Bitwise AND with accumulator", false},
        {0x32, "AND", "(Zero Page)", 5, "Bitwise AND with accumulator", true},
        {0x33, "---", "Implied", 2, "Unimplemented", false},
        {0x34, "BIT", "Zero Page,X", 4, "Bit Test", true},
        {0x35, "AND", "Zero Page,X", 4, "Bitwise AND with accumulator", false},
        {0x36, "ROL", "Zero Page,X", 6, "Rotate Left", false},
        {0x37, "RMB3", "Zero Page", 5, "Reset Memory Bit 3", true},
        {0x38, "SEC", "Implied", 2, "Set Carry", false},
        {0x39, "AND", "Absolute,Y", 4, "Bitwise AND with accumulator", false},
        {0x3A, "DEC", "Accumulator", 2, "Decrement Accumulator", true},
        {0x3B, "---", "Implied", 2, "Unimplemented", false},
        {0x3C, "BIT", "Absolute,X", 4, "Bit Test", true},
        {0x3D, "AND", "Absolute,X", 4, "Bitwise AND with accumulator", false},
        {0x3E, "ROL", "Absolute,X", 7, "Rotate Left", false},
        {0x3F, "BBR3", "Relative", 5, "Branch on Bit Reset 3", true},

        // 0x40 - 0x4F: RTI, EOR, undocumented, etc.
        {0x40, "RTI", "Implied", 6, "Return from Interrupt", false},
        {0x41, "EOR", "(Indirect,X)", 6, "Bitwise XOR with accumulator", false},
        {0x42, "---", "Implied", 2, "Unimplemented", false},
        {0x43, "---", "Implied", 2, "Unimplemented", false},
        {0x44, "---", "Implied", 3, "Unimplemented", false},
        {0x45, "EOR", "Zero Page", 3, "Bitwise XOR with accumulator", false},
        {0x46, "LSR", "Zero Page", 5, "Logical Shift Right", false},
        {0x47, "RMB4", "Zero Page", 5, "Reset Memory Bit 4", true},
        {0x48, "PHA", "Implied", 3, "Push Accumulator", false},
        {0x49, "EOR", "Immediate", 2, "Bitwise XOR with accumulator", false},
        {0x4A, "LSR", "Accumulator", 2, "Logical Shift Right", false},
        {0x4B, "---", "Implied", 2, "Unimplemented", false},
        {0x4C, "JMP", "Absolute", 3, "Jump", false},
        {0x4D, "EOR", "Absolute", 4, "Bitwise XOR with accumulator", false},
        {0x4E, "LSR", "Absolute", 6, "Logical Shift Right", false},
        {0x4F, "BBR4", "Relative", 5, "Branch on Bit Reset 4", true},

        // 0x50 - 0x5F: BVC, EOR, undocumented, etc.
        {0x50, "BVC", "Relative", 2, "Branch if Overflow Clear", false},
        {0x51, "EOR", "(Indirect),Y", 5, "Bitwise XOR with accumulator", false},
        {0x52, "EOR", "(Zero Page)", 5, "Bitwise XOR with accumulator", true},
        {0x53, "---", "Implied", 2, "Unimplemented", false},
        {0x54, "---", "Implied", 3, "Unimplemented", false},
        {0x55, "EOR", "Zero Page,X", 4, "Bitwise XOR with accumulator", false},
        {0x56, "LSR", "Zero Page,X", 6, "Logical Shift Right", false},
        {0x57, "RMB5", "Zero Page", 5, "Reset Memory Bit 5", true},
        {0x58, "CLI", "Implied", 2, "Clear Interrupt Disable", false},
        {0x59, "EOR", "Absolute,Y", 4, "Bitwise XOR with accumulator", false},
        {0x5A, "PHY", "Implied", 3, "Push Y", true},
        {0x5B, "---", "Implied", 2, "Unimplemented", false},
        {0x5C, "---", "Implied", 3, "Unimplemented", false},
        {0x5D, "EOR", "Absolute,X", 4, "Bitwise XOR with accumulator", false},
        {0x5E, "LSR", "Absolute,X", 7, "Logical Shift Right", false},
        {0x5F, "BBR5", "Relative", 5, "Branch on Bit Reset 5", true},

        // 0x60 - 0x6F: RTS, ADC, undocumented, etc.
        {0x60, "RTS", "Implied", 6, "Return from Subroutine", false},
        {0x61, "ADC", "(Indirect,X)", 6, "Add with Carry", false},
        {0x62, "---", "Implied", 2, "Unimplemented", false},
        {0x63, "---", "Implied", 2, "Unimplemented", false},
        {0x64, "STZ", "Zero Page", 3, "Store Zero", true},
        {0x65, "ADC", "Zero Page", 3, "Add with Carry", false},
        {0x66, "ROR", "Zero Page", 5, "Rotate Right", false},
        {0x67, "RMB6", "Zero Page", 5, "Reset Memory Bit 6", true},
        {0x68, "PLA", "Implied", 4, "Pull Accumulator", false},
        {0x69, "ADC", "Immediate", 2, "Add with Carry", false},
        {0x6A, "ROR", "Accumulator", 2, "Rotate Right", false},
        {0x6B, "---", "Implied", 2, "Unimplemented", false},
        {0x6C, "JMP", "(Indirect)", 5, "Jump Indirect", false},
        {0x6D, "ADC", "Absolute", 4, "Add with Carry", false},
        {0x6E, "ROR", "Absolute", 6, "Rotate Right", false},
        {0x6F, "BBR6", "Relative", 5, "Branch on Bit Reset 6", true},

        // 0x70 - 0x7F: BVS, ADC, undocumented, etc.
        {0x70, "BVS", "Relative", 2, "Branch if Overflow Set", false},
        {0x71, "ADC", "(Indirect),Y", 5, "Add with Carry", false},
        {0x72, "ADC", "(Zero Page)", 5, "Add with Carry", true},
        {0x73, "---", "Implied", 2, "Unimplemented", false},
        {0x74, "STZ", "Zero Page,X", 4, "Store Zero", true},
        {0x75, "ADC", "Zero Page,X", 4, "Add with Carry", false},
        {0x76, "ROR", "Zero Page,X", 6, "Rotate Right", false},
        {0x77, "RMB7", "Zero Page", 5, "Reset Memory Bit 7", true},
        {0x78, "SEI", "Implied", 2, "Set Interrupt Disable", false},
        {0x79, "ADC", "Absolute,Y", 4, "Add with Carry", false},
        {0x7A, "PLY", "Implied", 4, "Pull Y", true},
        {0x7B, "---", "Implied", 2, "Unimplemented", false},
        {0x7C, "JMP", "Absolute,X", 6, "Jump Indirect X", true},
        {0x7D, "ADC", "Absolute,X", 4, "Add with Carry", false},
        {0x7E, "ROR", "Absolute,X", 7, "Rotate Right", false},
        {0x7F, "BBR7", "Relative", 5, "Branch on Bit Reset 7", true},

        // 0x80 - 0x8F: BRA, STA, undocumented, etc.
        {0x80, "BRA", "Relative", 3, "Branch Always", true},
        {0x81, "STA", "(Indirect,X)", 6, "Store Accumulator", false},
        {0x82, "---", "Implied", 2, "Unimplemented", false},
        {0x83, "---", "Implied", 2, "Unimplemented", false},
        {0x84, "STY", "Zero Page", 3, "Store Y Register", false},
        {0x85, "STA", "Zero Page", 3, "Store Accumulator", false},
        {0x86, "STX", "Zero Page", 3, "Store X Register", false},
        {0x87, "SMB0", "Zero Page", 5, "Set Memory Bit 0", true},
        {0x88, "DEY", "Implied", 2, "Decrement Y", false},
        {0x89, "BIT", "Immediate", 2, "Bit Test", true},
        {0x8A, "TXA", "Implied", 2, "Transfer X to Accumulator", false},
        {0x8B, "---", "Implied", 2, "Unimplemented", false},
        {0x8C, "STY", "Absolute", 4, "Store Y Register", false},
        {0x8D, "STA", "Absolute", 4, "Store Accumulator", false},
        {0x8E, "STX", "Absolute", 4, "Store X Register", false},
        {0x8F, "BBS0", "Relative", 5, "Branch on Bit Set 0", true},

        // 0x90 - 0x9F: BCC, STA, undocumented, etc.
        {0x90, "BCC", "Relative", 2, "Branch if Carry Clear", false},
        {0x91, "STA", "(Indirect),Y", 6, "Store Accumulator", false},
        {0x92, "STA", "(Zero Page)", 5, "Store Accumulator", true},
        {0x93, "---", "Implied", 2, "Unimplemented", false},
        {0x94, "STY", "Zero Page,X", 4, "Store Y Register", false},
        {0x95, "STA", "Zero Page,X", 4, "Store Accumulator", false},
        {0x96, "STX", "Zero Page,Y", 4, "Store X Register", false},
        {0x97, "SMB1", "Zero Page", 5, "Set Memory Bit 1", true},
        {0x98, "TYA", "Implied", 2, "Transfer Y to Accumulator", false},
        {0x99, "STA", "Absolute,Y", 5, "Store Accumulator", false},
        {0x9A, "TXS", "Implied", 2, "Transfer X to Stack Pointer", false},
        {0x9B, "---", "Implied", 2, "Unimplemented", false},
        {0x9C, "STZ", "Absolute", 4, "Store Zero", true},
        {0x9D, "STA", "Absolute,X", 5, "Store Accumulator", false},
        {0x9E, "STZ", "Absolute,X", 5, "Store Zero", true},
        {0x9F, "BBS1", "Relative", 5, "Branch on Bit Set 1", true},

        // 0xA0 - 0xAF: LDY, LDA, undocumented, etc.
        {0xA0, "LDY", "Immediate", 2, "Load Y Register", false},
        {0xA1, "LDA", "(Indirect,X)", 6, "Load Accumulator", false},
        {0xA2, "LDX", "Immediate", 2, "Load X Register", false},
        {0xA3, "---", "Implied", 2, "Unimplemented", false},
        {0xA4, "LDY", "Zero Page", 3, "Load Y Register", false},
        {0xA5, "LDA", "Zero Page", 3, "Load Accumulator", false},
        {0xA6, "LDX", "Zero Page", 3, "Load X Register", false},
        {0xA7, "SMB2", "Zero Page", 5, "Set Memory Bit 2", true},
        {0xA8, "TAY", "Implied", 2, "Transfer Accumulator to Y", false},
        {0xA9, "LDA", "Immediate", 2, "Load Accumulator", false},
        {0xAA, "TAX", "Implied", 2, "Transfer Accumulator to X", false},
        {0xAB, "---", "Implied", 2, "Unimplemented", false},
        {0xAC, "LDY", "Absolute", 4, "Load Y Register", false},
        {0xAD, "LDA", "Absolute", 4, "Load Accumulator", false},
        {0xAE, "LDX", "Absolute", 4, "Load X Register", false},
        {0xAF, "BBS2", "Relative", 5, "Branch on Bit Set 2", true},

        // 0xB0 - 0xBF: BCS, LDA, undocumented, etc.
        {0xB0, "BCS", "Relative", 2, "Branch if Carry Set", false},
        {0xB1, "LDA", "(Indirect),Y", 5, "Load Accumulator", false},
        {0xB2, "LDA", "(Zero Page)", 5, "Load Accumulator", true},
        {0xB3, "---", "Implied", 2, "Unimplemented", false},
        {0xB4, "LDY", "Zero Page,X", 4, "Load Y Register", false},
        {0xB5, "LDA", "Zero Page,X", 4, "Load Accumulator", false},
        {0xB6, "LDX", "Zero Page,Y", 4, "Load X Register", false},
        {0xB7, "SMB3", "Zero Page", 5, "Set Memory Bit 3", true},
        {0xB8, "CLV", "Implied", 2, "Clear Overflow", false},
        {0xB9, "LDA", "Absolute,Y", 4, "Load Accumulator", false},
        {0xBA, "TSX", "Implied", 2, "Transfer Stack Pointer to X", false},
        {0xBB, "---", "Implied", 2, "Unimplemented", false},
        {0xBC, "LDY", "Absolute,X", 4, "Load Y Register", false},
        {0xBD, "LDA", "Absolute,X", 4, "Load Accumulator", false},
        {0xBE, "LDX", "Absolute,Y", 4, "Load X Register", false},
        {0xBF, "BBS3", "Relative", 5, "Branch on Bit Set 3", true},

        // 0xC0 - 0xCF: CPY, CMP, undocumented, etc.
        {0xC0, "CPY", "Immediate", 2, "Compare Y Register", false},
        {0xC1, "CMP", "(Indirect,X)", 6, "Compare Accumulator", false},
        {0xC2, "---", "Implied", 2, "Unimplemented", false},
        {0xC3, "---", "Implied", 2, "Unimplemented", false},
        {0xC4, "CPY", "Zero Page", 3, "Compare Y Register", false},
        {0xC5, "CMP", "Zero Page", 3, "Compare Accumulator", false},
        {0xC6, "DEC", "Zero Page", 5, "Decrement", false},
        {0xC7, "SMB4", "Zero Page", 5, "Set Memory Bit 4", true},
        {0xC8, "INY", "Implied", 2, "Increment Y", false},
        {0xC9, "CMP", "Immediate", 2, "Compare Accumulator", false},
        {0xCA, "DEX", "Implied", 2, "Decrement X", false},
        {0xCB, "WAI", "Implied", 3, "Wait for Interrupt", true},
        {0xCC, "CPY", "Absolute", 4, "Compare Y Register", false},
        {0xCD, "CMP", "Absolute", 4, "Compare Accumulator", false},
        {0xCE, "DEC", "Absolute", 6, "Decrement", false},
        {0xCF, "BBS4", "Relative", 5, "Branch on Bit Set 4", true},

        // 0xD0 - 0xDF: BNE, CMP, undocumented, etc.
        {0xD0, "BNE", "Relative", 2, "Branch if Not Equal", false},
        {0xD1, "CMP", "(Indirect),Y", 5, "Compare Accumulator", false},
        {0xD2, "CMP", "(Zero Page)", 5, "Compare Accumulator", true},
        {0xD3, "---", "Implied", 2, "Unimplemented", false},
        {0xD4, "---", "Implied", 3, "Unimplemented", false},
        {0xD5, "CMP", "Zero Page,X", 4, "Compare Accumulator", false},
        {0xD6, "DEC", "Zero Page,X", 6, "Decrement", false},
        {0xD7, "SMB5", "Zero Page", 5, "Set Memory Bit 5", true},
        {0xD8, "CLD", "Implied", 2, "Clear Decimal", false},
        {0xD9, "CMP", "Absolute,Y", 4, "Compare Accumulator", false},
        {0xDA, "PHX", "Implied", 3, "Push X", true},
        {0xDB, "STP", "Implied", 3, "Stop (Halt)", true},
        {0xDC, "---", "Implied", 3, "Unimplemented", false},
        {0xDD, "CMP", "Absolute,X", 4, "Compare Accumulator", false},
        {0xDE, "DEC", "Absolute,X", 7, "Decrement", false},
        {0xDF, "BBS5", "Relative", 5, "Branch on Bit Set 5", true},

        // 0xE0 - 0xEF: CPX, SBC, undocumented, etc.
        {0xE0, "CPX", "Immediate", 2, "Compare X Register", false},
        {0xE1, "SBC", "(Indirect,X)", 6, "Subtract with Carry", false},
        {0xE2, "---", "Implied", 2, "Unimplemented", false},
        {0xE3, "---", "Implied", 2, "Unimplemented", false},
        {0xE4, "CPX", "Zero Page", 3, "Compare X Register", false},
        {0xE5, "SBC", "Zero Page", 3, "Subtract with Carry", false},
        {0xE6, "INC", "Zero Page", 5, "Increment", false},
        {0xE7, "SMB6", "Zero Page", 5, "Set Memory Bit 6", true},
        {0xE8, "INX", "Implied", 2, "Increment X", false},
        {0xE9, "SBC", "Immediate", 2, "Subtract with Carry", false},
        {0xEA, "NOP", "Implied", 2, "No Operation", false},
        {0xEB, "---", "Implied", 2, "Unimplemented", false},
        {0xEC, "CPX", "Absolute", 4, "Compare X Register", false},
        {0xED, "SBC", "Absolute", 4, "Subtract with Carry", false},
        {0xEE, "INC", "Absolute", 6, "Increment", false},
        {0xEF, "BBS6", "Relative", 5, "Branch on Bit Set 6", true},

        // 0xF0 - 0xFF: BEQ, SBC, undocumented, etc.
        {0xF0, "BEQ", "Relative", 2, "Branch if Equal", false},
        {0xF1, "SBC", "(Indirect),Y", 5, "Subtract with Carry", false},
        {0xF2, "SBC", "(Zero Page)", 5, "Subtract with Carry", true},
        {0xF3, "---", "Implied", 2, "Unimplemented", false},
        {0xF4, "---", "Implied", 3, "Unimplemented", false},
        {0xF5, "SBC", "Zero Page,X", 4, "Subtract with Carry", false},
        {0xF6, "INC", "Zero Page,X", 6, "Increment", false},
        {0xF7, "SMB7", "Zero Page", 5, "Set Memory Bit 7", true},
        {0xF8, "SED", "Implied", 2, "Set Decimal", false},
        {0xF9, "SBC", "Absolute,Y", 4, "Subtract with Carry", false},
        {0xFA, "PLX", "Implied", 4, "Pull X", true},
        {0xFB, "---", "Implied", 2, "Unimplemented", false},
        {0xFC, "---", "Implied", 3, "Unimplemented", false},
        {0xFD, "SBC", "Absolute,X", 4, "Subtract with Carry", false},
        {0xFE, "INC", "Absolute,X", 7, "Increment", false},
        {0xFF, "BBS7", "Relative", 5, "Branch on Bit Set 7", true},
    }};

    // Query helpers
    static const InstructionMetadata& getMetadata(uint8_t opcode) {
        return OPCODES[opcode];
    }

    static bool is65C02Opcode(uint8_t opcode) {
        return OPCODES[opcode].is_65c02_only;
    }

    static bool isImplemented(uint8_t opcode) {
        return OPCODES[opcode].mnemonic[0] != '-';
    }
};

}  // namespace Instructions
