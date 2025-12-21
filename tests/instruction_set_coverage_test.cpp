#include <gtest/gtest.h>
#include "cpu_instructions.hpp"
#include "cpu.hpp"
#include "mem.hpp"
#include "instruction_set.hpp"
#include <set>
#include <sstream>

// Test suite for 65C02 instruction set coverage and completeness
class InstructionSetCoverageTest : public testing::Test {
protected:
    Mem mem;
    CPU cpu;

    void SetUp() override {
        cpu.Reset(mem);
        Instructions::InitializeInstructionTable();
    }
};

// Verify all 256 opcodes have valid metadata
TEST_F(InstructionSetCoverageTest, AllOpcodeMetadataValid) {
    for (int i = 0; i < 256; i++) {
        uint8_t opcode = static_cast<uint8_t>(i);
        const auto& meta = Instructions::InstructionSet::getMetadata(opcode);
        
        EXPECT_EQ(meta.opcode, opcode) 
            << "Opcode mismatch at index " << std::hex << i;
        EXPECT_NE(meta.mnemonic, nullptr) 
            << "Mnemonic null at 0x" << std::hex << i;
        EXPECT_NE(meta.addressing_mode, nullptr) 
            << "Addressing mode null at 0x" << std::hex << i;
        EXPECT_NE(meta.description, nullptr) 
            << "Description null at 0x" << std::hex << i;
        EXPECT_GT(meta.cycles, 0) 
            << "Invalid cycles at 0x" << std::hex << i;
    }
}

// Verify all 256 opcodes have handlers
TEST_F(InstructionSetCoverageTest, AllOpcodesHaveHandlers) {
    std::vector<uint8_t> unhandledOpcodes;
    
    for (int i = 0; i < 256; i++) {
        uint8_t opcode = static_cast<uint8_t>(i);
        auto handler = Instructions::GetHandler(opcode);
        
        // A handler should be callable
        EXPECT_TRUE(handler != nullptr) 
            << "No handler for opcode 0x" << std::hex << std::setfill('0') 
            << std::setw(2) << (int)opcode;
    }
}

// Verify 65C02-specific opcodes are properly marked
TEST_F(InstructionSetCoverageTest, ValidateC65C02Mapping) {
    // These are the known 65C02-only opcodes
    std::set<uint8_t> expected_65c02 = {
        0x04, 0x0C, 0x07, 0x0F, 0x12, 0x14, 0x17, 0x1A, 0x1C, 0x1F,
        0x27, 0x2F, 0x34, 0x37, 0x3A, 0x3C, 0x3F,
        0x47, 0x4F, 0x52, 0x57, 0x5A, 0x5F,
        0x64, 0x67, 0x6F, 0x72, 0x77, 0x7A, 0x7C, 0x7F,
        0x80, 0x87, 0x89, 0x8F,
        0x92, 0x97, 0x9C, 0x9E, 0x9F,
        0xA7, 0xAF,
        0xB2, 0xB7, 0xBF,
        0xC7, 0xCB, 0xCF,
        0xD2, 0xD7, 0xDA, 0xDB, 0xDF,
        0xE7, 0xEF,
        0xF2, 0xF7, 0xFA
    };
    
    for (uint8_t opcode : expected_65c02) {
        EXPECT_TRUE(Instructions::InstructionSet::is65C02Opcode(opcode))
            << "Opcode 0x" << std::hex << std::setfill('0') 
            << std::setw(2) << (int)opcode 
            << " should be marked as 65C02";
    }
}

// Verify unimplemented opcodes are properly marked
TEST_F(InstructionSetCoverageTest, ValidateUnimplementedOpcodes) {
    std::set<uint8_t> expected_unimplemented = {
        0x02, 0x03, 0x0B, 0x13, 0x1B,
        0x22, 0x23, 0x2B, 0x33, 0x3B,
        0x42, 0x43, 0x44, 0x4B, 0x53, 0x5B, 0x5C,
        0x62, 0x63, 0x6B, 0x73, 0x7B,
        0x82, 0x83, 0x8B, 0x93, 0x9B,
        0xA3, 0xAB, 0xB3, 0xBB,
        0xC2, 0xC3, 0xD3, 0xD4, 0xDC,
        0xE2, 0xE3, 0xEB, 0xF3, 0xF4, 0xFB, 0xFC
    };
    
    for (uint8_t opcode : expected_unimplemented) {
        EXPECT_FALSE(Instructions::InstructionSet::isImplemented(opcode))
            << "Opcode 0x" << std::hex << std::setfill('0') 
            << std::setw(2) << (int)opcode 
            << " should not be marked as implemented";
    }
}

// Count implemented vs. unimplemented opcodes
TEST_F(InstructionSetCoverageTest, OpcodeDistribution) {
    int implemented = 0;
    int unimplemented = 0;
    int c65c02_only = 0;
    
    for (int i = 0; i < 256; i++) {
        uint8_t opcode = static_cast<uint8_t>(i);
        const auto& meta = Instructions::InstructionSet::getMetadata(opcode);
        
        if (Instructions::InstructionSet::isImplemented(opcode)) {
            implemented++;
            if (meta.is_65c02_only) {
                c65c02_only++;
            }
        } else {
            unimplemented++;
        }
    }
    
    std::cout << "\n=== Instruction Set Coverage ===" << std::endl;
    std::cout << "Implemented opcodes: " << implemented << "/256" << std::endl;
    std::cout << "Unimplemented opcodes: " << unimplemented << "/256" << std::endl;
    std::cout << "65C02-only opcodes: " << c65c02_only << std::endl;
    std::cout << "6502 base opcodes: " << (implemented - c65c02_only) << std::endl;
    
    // We expect significant 6502 coverage + 65C02 extensions
    EXPECT_GE(implemented, 210) << "Expected at least 210 implemented opcodes";
    EXPECT_LE(unimplemented, 46) << "Expected at most 46 unimplemented opcodes";
}

// Spot-check some key opcodes
TEST_F(InstructionSetCoverageTest, KeyOpcodesValidation) {
    // LDA Immediate
    EXPECT_TRUE(Instructions::InstructionSet::isImplemented(0xA9));
    EXPECT_FALSE(Instructions::InstructionSet::is65C02Opcode(0xA9));
    
    // INC A (65C02 only)
    EXPECT_TRUE(Instructions::InstructionSet::isImplemented(0x1A));
    EXPECT_TRUE(Instructions::InstructionSet::is65C02Opcode(0x1A));
    
    // DEC A (65C02 only)
    EXPECT_TRUE(Instructions::InstructionSet::isImplemented(0x3A));
    EXPECT_TRUE(Instructions::InstructionSet::is65C02Opcode(0x3A));
    
    // BRA (65C02 only)
    EXPECT_TRUE(Instructions::InstructionSet::isImplemented(0x80));
    EXPECT_TRUE(Instructions::InstructionSet::is65C02Opcode(0x80));
    
    // STZ Absolute (65C02)
    EXPECT_TRUE(Instructions::InstructionSet::isImplemented(0x9C));
    EXPECT_TRUE(Instructions::InstructionSet::is65C02Opcode(0x9C));
    
    // BRK (standard)
    EXPECT_TRUE(Instructions::InstructionSet::isImplemented(0x00));
    EXPECT_FALSE(Instructions::InstructionSet::is65C02Opcode(0x00));
}

// Verify instruction metadata for timing accuracy
TEST_F(InstructionSetCoverageTest, TimingValidation) {
    // Sample checks for known cycle counts
    const auto& lda_imm = Instructions::InstructionSet::getMetadata(0xA9);
    EXPECT_EQ(lda_imm.cycles, 2) << "LDA #imm should be 2 cycles";
    
    const auto& brk = Instructions::InstructionSet::getMetadata(0x00);
    EXPECT_EQ(brk.cycles, 7) << "BRK should be 7 cycles";
    
    const auto& pha = Instructions::InstructionSet::getMetadata(0x48);
    EXPECT_EQ(pha.cycles, 3) << "PHA should be 3 cycles (65C02)";
    
    const auto& pla = Instructions::InstructionSet::getMetadata(0x68);
    EXPECT_EQ(pla.cycles, 4) << "PLA should be 4 cycles (65C02)";
    
    const auto& bra = Instructions::InstructionSet::getMetadata(0x80);
    EXPECT_EQ(bra.cycles, 3) << "BRA should be 3 cycles";
}

// Ensure all addressing modes are recognized
TEST_F(InstructionSetCoverageTest, AddressingModes) {
    std::set<std::string> modes;
    for (int i = 0; i < 256; i++) {
        uint8_t opcode = static_cast<uint8_t>(i);
        const auto& meta = Instructions::InstructionSet::getMetadata(opcode);
        modes.insert(meta.addressing_mode);
    }
    
    std::cout << "\n=== Addressing Modes Present ===" << std::endl;
    for (const auto& mode : modes) {
        std::cout << "  - " << mode << std::endl;
    }
    
    // Verify key modes exist
    EXPECT_NE(modes.find("(Indirect)"), modes.end());
    EXPECT_NE(modes.find("Accumulator"), modes.end());
    EXPECT_NE(modes.find("Immediate"), modes.end());
    EXPECT_NE(modes.find("Zero Page"), modes.end());
    EXPECT_NE(modes.find("Absolute"), modes.end());
    EXPECT_NE(modes.find("Relative"), modes.end());
    EXPECT_NE(modes.find("(Indirect,X)"), modes.end());
}
