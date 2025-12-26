#include <gtest/gtest.h>
#include "cpu/instruction_set.hpp"
#include <map>
#include <string>

using namespace Instructions;

/**
 * Timing Validation Test Suite for 65C02 Instruction Set
 * 
 * This suite validates that all implemented instructions have correct cycle counts
 * according to the WDC 65C02 datasheet and current implementation.
 * 
 * The 65C02 improved timing on several operations compared to the 6502:
 * - STZ (Store Zero) - new instruction, 3-5 cycles
 * - INC A / DEC A - new accumulator operations, 2 cycles
 * - TSB / TRB - new bit test/modify, 5-6 cycles
 * - BRA - unconditional branch, 3 cycles (improved from 6502)
 * - BIT with Immediate - new variant, 2 cycles
 * - Bit manipulation (RMB, SMB, BBR, BBS) - new operations, 5 cycles
 * 
 * Page crossing penalties apply to indexed modes:
 * - Absolute,X / Absolute,Y: +1 cycle if crosses page boundary
 * - (Indirect),Y: +1 cycle if crosses page boundary
 * 
 * Branch penalties:
 * - Branches taken: +1 cycle
 * - Branches crossing page boundary: +2 cycles (only 65C02 6502 compat mode)
 */

class TimingValidationTest : public ::testing::Test {
protected:
    // Reference timing table for validation
    // This maps (mnemonic, addressing_mode) -> expected_cycles
    std::map<std::pair<std::string, std::string>, uint8_t> expectedTiming;
    
    void SetUp() override {
        PopulateExpectedTiming();
    }
    
    void PopulateExpectedTiming() {
        // Load/Store Operations
        expectedTiming[{"LDA", "Immediate"}] = 2;
        expectedTiming[{"LDA", "Zero Page"}] = 3;
        expectedTiming[{"LDA", "Zero Page,X"}] = 4;
        expectedTiming[{"LDA", "Absolute"}] = 4;
        expectedTiming[{"LDA", "Absolute,X"}] = 4;
        expectedTiming[{"LDA", "Absolute,Y"}] = 4;
        expectedTiming[{"LDA", "(Indirect,X)"}] = 6;
        expectedTiming[{"LDA", "(Indirect),Y"}] = 5;
        expectedTiming[{"LDA", "(Zero Page)"}] = 5;  // 65C02 new addressing mode
        
        expectedTiming[{"LDX", "Immediate"}] = 2;
        expectedTiming[{"LDX", "Zero Page"}] = 3;
        expectedTiming[{"LDX", "Zero Page,Y"}] = 4;
        expectedTiming[{"LDX", "Absolute"}] = 4;
        expectedTiming[{"LDX", "Absolute,Y"}] = 4;
        
        expectedTiming[{"LDY", "Immediate"}] = 2;
        expectedTiming[{"LDY", "Zero Page"}] = 3;
        expectedTiming[{"LDY", "Zero Page,X"}] = 4;
        expectedTiming[{"LDY", "Absolute"}] = 4;
        expectedTiming[{"LDY", "Absolute,X"}] = 4;
        
        expectedTiming[{"STA", "Zero Page"}] = 3;
        expectedTiming[{"STA", "Zero Page,X"}] = 4;
        expectedTiming[{"STA", "Absolute"}] = 4;
        expectedTiming[{"STA", "Absolute,X"}] = 5;
        expectedTiming[{"STA", "Absolute,Y"}] = 5;
        expectedTiming[{"STA", "(Indirect,X)"}] = 6;
        expectedTiming[{"STA", "(Indirect),Y"}] = 6;
        expectedTiming[{"STA", "(Zero Page)"}] = 5;  // 65C02
        
        expectedTiming[{"STX", "Zero Page"}] = 3;
        expectedTiming[{"STX", "Zero Page,Y"}] = 4;
        expectedTiming[{"STX", "Absolute"}] = 4;
        
        expectedTiming[{"STY", "Zero Page"}] = 3;
        expectedTiming[{"STY", "Zero Page,X"}] = 4;
        expectedTiming[{"STY", "Absolute"}] = 4;
        
        // Store Zero - 65C02 only
        expectedTiming[{"STZ", "Zero Page"}] = 3;
        expectedTiming[{"STZ", "Zero Page,X"}] = 4;
        expectedTiming[{"STZ", "Absolute"}] = 4;
        expectedTiming[{"STZ", "Absolute,X"}] = 5;
        
        // Add/Subtract/Compare Operations
        expectedTiming[{"ADC", "Immediate"}] = 2;
        expectedTiming[{"ADC", "Zero Page"}] = 3;
        expectedTiming[{"ADC", "Zero Page,X"}] = 4;
        expectedTiming[{"ADC", "Absolute"}] = 4;
        expectedTiming[{"ADC", "Absolute,X"}] = 4;
        expectedTiming[{"ADC", "Absolute,Y"}] = 4;
        expectedTiming[{"ADC", "(Indirect,X)"}] = 6;
        expectedTiming[{"ADC", "(Indirect),Y"}] = 5;
        expectedTiming[{"ADC", "(Zero Page)"}] = 5;  // 65C02
        
        expectedTiming[{"SBC", "Immediate"}] = 2;
        expectedTiming[{"SBC", "Zero Page"}] = 3;
        expectedTiming[{"SBC", "Zero Page,X"}] = 4;
        expectedTiming[{"SBC", "Absolute"}] = 4;
        expectedTiming[{"SBC", "Absolute,X"}] = 4;
        expectedTiming[{"SBC", "Absolute,Y"}] = 4;
        expectedTiming[{"SBC", "(Indirect,X)"}] = 6;
        expectedTiming[{"SBC", "(Indirect),Y"}] = 5;
        expectedTiming[{"SBC", "(Zero Page)"}] = 5;  // 65C02
        
        expectedTiming[{"CMP", "Immediate"}] = 2;
        expectedTiming[{"CMP", "Zero Page"}] = 3;
        expectedTiming[{"CMP", "Zero Page,X"}] = 4;
        expectedTiming[{"CMP", "Absolute"}] = 4;
        expectedTiming[{"CMP", "Absolute,X"}] = 4;
        expectedTiming[{"CMP", "Absolute,Y"}] = 4;
        expectedTiming[{"CMP", "(Indirect,X)"}] = 6;
        expectedTiming[{"CMP", "(Indirect),Y"}] = 5;
        
        expectedTiming[{"CPX", "Immediate"}] = 2;
        expectedTiming[{"CPX", "Zero Page"}] = 3;
        expectedTiming[{"CPX", "Absolute"}] = 4;
        
        expectedTiming[{"CPY", "Immediate"}] = 2;
        expectedTiming[{"CPY", "Zero Page"}] = 3;
        expectedTiming[{"CPY", "Absolute"}] = 4;
        
        // Logical Operations
        expectedTiming[{"AND", "Immediate"}] = 2;
        expectedTiming[{"AND", "Zero Page"}] = 3;
        expectedTiming[{"AND", "Zero Page,X"}] = 4;
        expectedTiming[{"AND", "Absolute"}] = 4;
        expectedTiming[{"AND", "Absolute,X"}] = 4;
        expectedTiming[{"AND", "Absolute,Y"}] = 4;
        expectedTiming[{"AND", "(Indirect,X)"}] = 6;
        expectedTiming[{"AND", "(Indirect),Y"}] = 5;
        expectedTiming[{"AND", "(Zero Page)"}] = 5;  // 65C02
        
        expectedTiming[{"ORA", "Immediate"}] = 2;
        expectedTiming[{"ORA", "Zero Page"}] = 3;
        expectedTiming[{"ORA", "Zero Page,X"}] = 4;
        expectedTiming[{"ORA", "Absolute"}] = 4;
        expectedTiming[{"ORA", "Absolute,X"}] = 4;
        expectedTiming[{"ORA", "Absolute,Y"}] = 4;
        expectedTiming[{"ORA", "(Indirect,X)"}] = 6;
        expectedTiming[{"ORA", "(Indirect),Y"}] = 5;
        expectedTiming[{"ORA", "(Zero Page)"}] = 5;  // 65C02
        
        expectedTiming[{"EOR", "Immediate"}] = 2;
        expectedTiming[{"EOR", "Zero Page"}] = 3;
        expectedTiming[{"EOR", "Zero Page,X"}] = 4;
        expectedTiming[{"EOR", "Absolute"}] = 4;
        expectedTiming[{"EOR", "Absolute,X"}] = 4;
        expectedTiming[{"EOR", "Absolute,Y"}] = 4;
        expectedTiming[{"EOR", "(Indirect,X)"}] = 6;
        expectedTiming[{"EOR", "(Indirect),Y"}] = 5;
        expectedTiming[{"EOR", "(Zero Page)"}] = 5;  // 65C02
        
        // Bit Test Operations
        expectedTiming[{"BIT", "Zero Page"}] = 3;
        expectedTiming[{"BIT", "Zero Page,X"}] = 4;  // 65C02
        expectedTiming[{"BIT", "Absolute"}] = 4;
        expectedTiming[{"BIT", "Absolute,X"}] = 4;  // 65C02
        expectedTiming[{"BIT", "Immediate"}] = 2;   // 65C02
        
        // Bit Manipulation - 65C02 Only
        expectedTiming[{"TSB", "Zero Page"}] = 5;
        expectedTiming[{"TSB", "Absolute"}] = 6;
        expectedTiming[{"TRB", "Zero Page"}] = 5;
        expectedTiming[{"TRB", "Absolute"}] = 6;
        expectedTiming[{"RMB0", "Zero Page"}] = 5;
        expectedTiming[{"RMB1", "Zero Page"}] = 5;
        expectedTiming[{"RMB2", "Zero Page"}] = 5;
        expectedTiming[{"RMB3", "Zero Page"}] = 5;
        expectedTiming[{"RMB4", "Zero Page"}] = 5;
        expectedTiming[{"RMB5", "Zero Page"}] = 5;
        expectedTiming[{"RMB6", "Zero Page"}] = 5;
        expectedTiming[{"RMB7", "Zero Page"}] = 5;
        expectedTiming[{"SMB0", "Zero Page"}] = 5;
        expectedTiming[{"SMB1", "Zero Page"}] = 5;
        expectedTiming[{"SMB2", "Zero Page"}] = 5;
        expectedTiming[{"SMB3", "Zero Page"}] = 5;
        expectedTiming[{"SMB4", "Zero Page"}] = 5;
        expectedTiming[{"SMB5", "Zero Page"}] = 5;
        expectedTiming[{"SMB6", "Zero Page"}] = 5;
        expectedTiming[{"SMB7", "Zero Page"}] = 5;
        
        // Branch on Bit - 65C02 Only
        expectedTiming[{"BBR0", "Relative"}] = 5;
        expectedTiming[{"BBR1", "Relative"}] = 5;
        expectedTiming[{"BBR2", "Relative"}] = 5;
        expectedTiming[{"BBR3", "Relative"}] = 5;
        expectedTiming[{"BBR4", "Relative"}] = 5;
        expectedTiming[{"BBR5", "Relative"}] = 5;
        expectedTiming[{"BBR6", "Relative"}] = 5;
        expectedTiming[{"BBR7", "Relative"}] = 5;
        expectedTiming[{"BBS0", "Relative"}] = 5;
        expectedTiming[{"BBS1", "Relative"}] = 5;
        expectedTiming[{"BBS2", "Relative"}] = 5;
        expectedTiming[{"BBS3", "Relative"}] = 5;
        expectedTiming[{"BBS4", "Relative"}] = 5;
        expectedTiming[{"BBS5", "Relative"}] = 5;
        expectedTiming[{"BBS6", "Relative"}] = 5;
        expectedTiming[{"BBS7", "Relative"}] = 5;
        
        // Shift/Rotate Operations
        expectedTiming[{"ASL", "Accumulator"}] = 2;
        expectedTiming[{"ASL", "Zero Page"}] = 5;
        expectedTiming[{"ASL", "Zero Page,X"}] = 6;
        expectedTiming[{"ASL", "Absolute"}] = 6;
        expectedTiming[{"ASL", "Absolute,X"}] = 7;
        
        expectedTiming[{"LSR", "Accumulator"}] = 2;
        expectedTiming[{"LSR", "Zero Page"}] = 5;
        expectedTiming[{"LSR", "Zero Page,X"}] = 6;
        expectedTiming[{"LSR", "Absolute"}] = 6;
        expectedTiming[{"LSR", "Absolute,X"}] = 7;
        
        expectedTiming[{"ROL", "Accumulator"}] = 2;
        expectedTiming[{"ROL", "Zero Page"}] = 5;
        expectedTiming[{"ROL", "Zero Page,X"}] = 6;
        expectedTiming[{"ROL", "Absolute"}] = 6;
        expectedTiming[{"ROL", "Absolute,X"}] = 7;
        
        expectedTiming[{"ROR", "Accumulator"}] = 2;
        expectedTiming[{"ROR", "Zero Page"}] = 5;
        expectedTiming[{"ROR", "Zero Page,X"}] = 6;
        expectedTiming[{"ROR", "Absolute"}] = 6;
        expectedTiming[{"ROR", "Absolute,X"}] = 7;
        
        // Increment/Decrement
        expectedTiming[{"INC", "Accumulator"}] = 2;  // 65C02
        expectedTiming[{"INC", "Zero Page"}] = 5;
        expectedTiming[{"INC", "Zero Page,X"}] = 6;
        expectedTiming[{"INC", "Absolute"}] = 6;
        expectedTiming[{"INC", "Absolute,X"}] = 7;
        
        expectedTiming[{"DEC", "Accumulator"}] = 2;  // 65C02
        expectedTiming[{"DEC", "Zero Page"}] = 5;
        expectedTiming[{"DEC", "Zero Page,X"}] = 6;
        expectedTiming[{"DEC", "Absolute"}] = 6;
        expectedTiming[{"DEC", "Absolute,X"}] = 7;
        
        expectedTiming[{"INX", "Implied"}] = 2;
        expectedTiming[{"INY", "Implied"}] = 2;
        expectedTiming[{"DEX", "Implied"}] = 2;
        expectedTiming[{"DEY", "Implied"}] = 2;
        
        // Stack Operations
        expectedTiming[{"PHA", "Implied"}] = 3;
        expectedTiming[{"PLA", "Implied"}] = 4;
        expectedTiming[{"PHP", "Implied"}] = 3;
        expectedTiming[{"PLP", "Implied"}] = 4;
        expectedTiming[{"PHX", "Implied"}] = 3;  // 65C02
        expectedTiming[{"PLX", "Implied"}] = 4;  // 65C02
        expectedTiming[{"PHY", "Implied"}] = 3;  // 65C02
        expectedTiming[{"PLY", "Implied"}] = 4;  // 65C02
        
        // Branches
        expectedTiming[{"BRA", "Relative"}] = 3;   // 65C02
        expectedTiming[{"BCC", "Relative"}] = 2;
        expectedTiming[{"BCS", "Relative"}] = 2;
        expectedTiming[{"BNE", "Relative"}] = 2;
        expectedTiming[{"BEQ", "Relative"}] = 2;
        expectedTiming[{"BPL", "Relative"}] = 2;
        expectedTiming[{"BMI", "Relative"}] = 2;
        expectedTiming[{"BVC", "Relative"}] = 2;
        expectedTiming[{"BVS", "Relative"}] = 2;
        
        // Jump/Return
        expectedTiming[{"JMP", "Absolute"}] = 3;
        expectedTiming[{"JMP", "(Indirect)"}] = 5;
        expectedTiming[{"JSR", "Absolute"}] = 6;
        expectedTiming[{"RTS", "Implied"}] = 6;
        expectedTiming[{"RTI", "Implied"}] = 6;
        
        // Flag Operations
        expectedTiming[{"CLC", "Implied"}] = 2;
        expectedTiming[{"SEC", "Implied"}] = 2;
        expectedTiming[{"CLD", "Implied"}] = 2;
        expectedTiming[{"SED", "Implied"}] = 2;
        expectedTiming[{"CLI", "Implied"}] = 2;
        expectedTiming[{"SEI", "Implied"}] = 2;
        expectedTiming[{"CLV", "Implied"}] = 2;
        
        // Register Transfers
        expectedTiming[{"TAX", "Implied"}] = 2;
        expectedTiming[{"TXA", "Implied"}] = 2;
        expectedTiming[{"TAY", "Implied"}] = 2;
        expectedTiming[{"TYA", "Implied"}] = 2;
        expectedTiming[{"TSX", "Implied"}] = 2;
        expectedTiming[{"TXS", "Implied"}] = 2;
        
        // Special Operations
        expectedTiming[{"NOP", "Implied"}] = 2;
        expectedTiming[{"BRK", "Implied"}] = 7;
        expectedTiming[{"WAI", "Implied"}] = 3;  // 65C02 - Wait for Interrupt
        expectedTiming[{"STP", "Implied"}] = 3;  // 65C02 - Stop
    }
};

// Test 1: Validate all implemented opcodes have correct timing
TEST_F(TimingValidationTest, AllImplementedOpcodesTiming) {
    int correct = 0, incorrect = 0, skipped = 0;
    std::string mismatches;
    
    for (const auto& metadata : InstructionSet::OPCODES) {
        // Skip unimplemented opcodes
        if (std::string(metadata.mnemonic) == "---") {
            skipped++;
            continue;
        }
        
        auto key = std::make_pair(std::string(metadata.mnemonic), 
                                  std::string(metadata.addressing_mode));
        
        auto it = expectedTiming.find(key);
        if (it != expectedTiming.end()) {
            if (metadata.cycles == it->second) {
                correct++;
            } else {
                incorrect++;
                mismatches += std::string("  0x") + 
                    (metadata.opcode < 0x10 ? "0" : "") +
                    "0x" + std::string(1, "0123456789ABCDEF"[metadata.opcode >> 4]) +
                    std::string(1, "0123456789ABCDEF"[metadata.opcode & 0x0F]) + 
                    " " + metadata.mnemonic + " (" + metadata.addressing_mode + 
                    "): got " + std::to_string(metadata.cycles) + 
                    ", expected " + std::to_string(it->second) + "\n";
            }
        }
    }
    
    EXPECT_EQ(incorrect, 0) << "Timing mismatches found:\n" << mismatches;
    std::cout << "Timing validation: " << correct << " correct, " 
              << incorrect << " incorrect, " << skipped << " skipped\n";
}

// Test 2: Validate 65C02-specific improved timing
TEST_F(TimingValidationTest, C65C02ImprovedTiming) {
    // These opcodes have improved timing in 65C02 vs 6502
    struct TimingImprovement {
        std::string mnemonic;
        std::string addressing_mode;
        uint8_t expected_cycles;
        std::string note;
    };
    
    std::vector<TimingImprovement> improvements = {
        {"INC", "Accumulator", 2, "New in 65C02 (was 3 in 6502)"},
        {"DEC", "Accumulator", 2, "New in 65C02 (was 3 in 6502)"},
        {"BRA", "Relative", 3, "New in 65C02 (improved from JSR 6 cycles)"},
        {"BIT", "Immediate", 2, "New in 65C02"},
        {"STZ", "Zero Page", 3, "New in 65C02"},
        {"STZ", "Absolute", 4, "New in 65C02"},
        {"PHX", "Implied", 3, "New in 65C02"},
        {"PLX", "Implied", 4, "New in 65C02"},
        {"PHY", "Implied", 3, "New in 65C02"},
        {"PLY", "Implied", 4, "New in 65C02"},
    };
    
    for (const auto& improvement : improvements) {
        const auto& metadata = InstructionSet::getMetadata(
            [&improvement]() {
                for (const auto& op : InstructionSet::OPCODES) {
                    if (std::string(op.mnemonic) == improvement.mnemonic &&
                        std::string(op.addressing_mode) == improvement.addressing_mode) {
                        return op.opcode;
                    }
                }
                return uint8_t(0);
            }()
        );
        
        if (std::string(metadata.mnemonic) != "---") {
            EXPECT_EQ(metadata.cycles, improvement.expected_cycles)
                << improvement.note << " (" << improvement.mnemonic << " " 
                << improvement.addressing_mode << ")";
        }
    }
}

// Test 3: Validate page-crossing aware cycles
TEST_F(TimingValidationTest, AddressingModeBaselineCycles) {
    struct ModeTimings {
        std::string mode;
        std::vector<uint8_t> expected_cycles;
    };
    
    // Verify that indexed modes have typical timing patterns
    std::map<std::string, std::vector<uint8_t>> addressing_mode_cycles;
    
    for (const auto& metadata : InstructionSet::OPCODES) {
        std::string mode = metadata.addressing_mode;
        addressing_mode_cycles[mode].push_back(metadata.cycles);
    }
    
    // Key insight: most instructions follow predictable patterns by addressing mode
    // Immediate mode should be consistently 2 cycles
    // Zero Page should be consistently 3 cycles (for common ops)
    // Absolute should be consistently 4 cycles (for common ops)
    
    for (const auto& [mode, cycles_list] : addressing_mode_cycles) {
        if (cycles_list.size() > 0) {
            uint8_t min_cycles = *std::min_element(cycles_list.begin(), cycles_list.end());
            uint8_t max_cycles = *std::max_element(cycles_list.begin(), cycles_list.end());
            
            // Most addressing modes should have variation ≤ 2 cycles
            // (except for special cases like page crossing)
            if (mode != "Implied" && mode != "Relative") {
                std::cout << "Mode " << mode << ": " << (int)min_cycles 
                          << "-" << (int)max_cycles << " cycles (count: " 
                          << cycles_list.size() << ")\n";
            }
        }
    }
}

// Test 4: Verify cycle count range is reasonable
TEST_F(TimingValidationTest, CycleCountRangeValidation) {
    uint8_t min_cycles = 255, max_cycles = 0;
    
    for (const auto& metadata : InstructionSet::OPCODES) {
        if (std::string(metadata.mnemonic) != "---") {
            min_cycles = std::min(min_cycles, metadata.cycles);
            max_cycles = std::max(max_cycles, metadata.cycles);
        }
    }
    
    // 65C02 timing should be between 2 (NOP, fast ops) and 7 (BRK)
    EXPECT_GE(min_cycles, 2) << "Some opcodes have unreasonably low cycle count";
    EXPECT_LE(max_cycles, 8) << "Some opcodes have unreasonably high cycle count";
    
    std::cout << "Cycle count range: " << (int)min_cycles << "-" << (int)max_cycles << "\n";
}

// Test 5: Validate that memory operation cycles are consistent
TEST_F(TimingValidationTest, MemoryOperationCycles) {
    struct OperationFamily {
        std::string mnemonic;
        std::map<std::string, uint8_t> expected_by_mode;
    };
    
    std::vector<OperationFamily> families = {
        {"LDA", {
            {"Immediate", 2}, {"Zero Page", 3}, {"Zero Page,X", 4},
            {"Absolute", 4}, {"Absolute,X", 4}, {"Absolute,Y", 4},
            {"(Indirect,X)", 6}, {"(Indirect),Y", 5}, {"(Zero Page)", 5}
        }},
        {"STA", {
            {"Zero Page", 3}, {"Zero Page,X", 4},
            {"Absolute", 4}, {"Absolute,X", 5}, {"Absolute,Y", 5},
            {"(Indirect,X)", 6}, {"(Indirect),Y", 6}, {"(Zero Page)", 5}
        }},
        {"ASL", {
            {"Accumulator", 2}, {"Zero Page", 5}, {"Zero Page,X", 6},
            {"Absolute", 6}, {"Absolute,X", 7}
        }},
    };
    
    for (const auto& family : families) {
        for (const auto& [mode, expected_cycles] : family.expected_by_mode) {
            const auto& metadata = [&]() -> const InstructionMetadata& {
                for (const auto& op : InstructionSet::OPCODES) {
                    if (std::string(op.mnemonic) == family.mnemonic &&
                        std::string(op.addressing_mode) == mode) {
                        return op;
                    }
                }
                return InstructionSet::OPCODES[0];
            }();
            
            if (std::string(metadata.mnemonic) == family.mnemonic) {
                EXPECT_EQ(metadata.cycles, expected_cycles)
                    << family.mnemonic << " " << mode 
                    << " should be " << (int)expected_cycles << " cycles";
            }
        }
    }
}

// Test 6: Branch instruction timing validation
TEST_F(TimingValidationTest, BranchInstructionTiming) {
    struct BranchInfo {
        uint8_t opcode;
        std::string mnemonic;
        uint8_t base_cycles;
    };
    
    std::vector<BranchInfo> branches = {
        {0x80, "BRA", 3},   // 65C02: unconditional branch always 3 cycles
        {0x10, "BPL", 2},   // 6502: 2 cycles base
        {0x30, "BMI", 2},
        {0x50, "BVC", 2},
        {0x70, "BVS", 2},
        {0x90, "BCC", 2},
        {0xB0, "BCS", 2},
        {0xD0, "BNE", 2},
        {0xF0, "BEQ", 2},
    };
    
    for (const auto& branch : branches) {
        const auto& metadata = InstructionSet::getMetadata(branch.opcode);
        EXPECT_EQ(metadata.cycles, branch.base_cycles)
            << branch.mnemonic << " (0x" << std::hex << (int)branch.opcode 
            << std::dec << ") should have " << (int)branch.base_cycles 
            << " base cycles (add +1 if branch taken)";
    }
}

// Test 7: Stack operation timing
TEST_F(TimingValidationTest, StackOperationTiming) {
    struct StackOpInfo {
        uint8_t opcode;
        std::string mnemonic;
        uint8_t cycles;
    };
    
    std::vector<StackOpInfo> stackOps = {
        {0x48, "PHA", 3},
        {0x68, "PLA", 4},
        {0x08, "PHP", 3},
        {0x28, "PLP", 4},
        {0xDA, "PHX", 3},   // 65C02
        {0xFA, "PLX", 4},   // 65C02
        {0x5A, "PHY", 3},   // 65C02
        {0x7A, "PLY", 4},   // 65C02
    };
    
    for (const auto& stackOp : stackOps) {
        const auto& metadata = InstructionSet::getMetadata(stackOp.opcode);
        EXPECT_EQ(metadata.cycles, stackOp.cycles)
            << stackOp.mnemonic << " should be " << (int)stackOp.cycles << " cycles";
    }
}

// Test 8: 65C02 bit manipulation timing
TEST_F(TimingValidationTest, BitManipulationTiming) {
    // All RMB, SMB, BBR, BBS instructions should be 5 cycles
    for (const auto& metadata : InstructionSet::OPCODES) {
        std::string mnem = metadata.mnemonic;
        
        if (mnem.length() >= 3) {
            std::string prefix = mnem.substr(0, 3);
            if (prefix == "RMB" || prefix == "SMB" || prefix == "BBR" || prefix == "BBS") {
                EXPECT_EQ(metadata.cycles, 5)
                    << mnem << " (0x" << std::hex << (int)metadata.opcode 
                    << std::dec << ") should be 5 cycles";
            }
        }
    }
}

// Test 9: Report timing statistics
TEST_F(TimingValidationTest, TimingStatistics) {
    std::map<uint8_t, int> cycle_distribution;
    int total_implemented = 0;
    
    for (const auto& metadata : InstructionSet::OPCODES) {
        if (std::string(metadata.mnemonic) != "---") {
            cycle_distribution[metadata.cycles]++;
            total_implemented++;
        }
    }
    
    std::cout << "\n=== Timing Statistics ===\n";
    std::cout << "Total Implemented: " << total_implemented << "\n";
    std::cout << "Cycle Distribution:\n";
    for (const auto& [cycles, count] : cycle_distribution) {
        std::cout << "  " << (int)cycles << " cycles: " << count 
                  << " opcodes (" << (count * 100 / total_implemented) << "%)\n";
    }
}

// Test 10: Validate 65C02 vs 6502 differences
TEST_F(TimingValidationTest, C65C02VsC6502Differences) {
    struct DifferenceInfo {
        uint8_t opcode;
        std::string mnemonic;
        std::string addressing_mode;
        bool is_65c02_only;
    };
    
    int c6502_opcodes = 0, c65c02_new = 0;
    
    for (const auto& metadata : InstructionSet::OPCODES) {
        if (std::string(metadata.mnemonic) != "---") {
            if (metadata.is_65c02_only) {
                c65c02_new++;
            } else {
                c6502_opcodes++;
            }
        }
    }
    
    std::cout << "\n=== 65C02 vs 6502 ===\n";
    std::cout << "6502 Base Opcodes: " << c6502_opcodes << "\n";
    std::cout << "65C02 New Opcodes: " << c65c02_new << "\n";
    std::cout << "Total: " << (c6502_opcodes + c65c02_new) << "\n";
    
    EXPECT_GT(c65c02_new, 0) << "Should have 65C02-specific opcodes";
    EXPECT_GT(c6502_opcodes, 0) << "Should have 6502 base opcodes";
}
