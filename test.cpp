#include <gtest/gtest.h>
#include "cpu.hpp"
#include "mem.hpp"

class M6502Test1 : public testing::Test
{
public:
    Mem mem;
    CPU cpu;

    virtual void SetUp()
    {
        cpu.Reset(mem);
    }

    virtual void TearDown()
    {
    }
};

// ========== STA Tests ==========
TEST_F(M6502Test1, TestSTA)
{
    cpu.A = 0x42;
    mem[0x8000] = CPU::INS_STA_IM.opcode;
    mem[0x8001] = 0x40;

    cpu.Execute(3, mem);

    EXPECT_EQ(mem[0x40], 0x42);
}

TEST_F(M6502Test1, TestSTA_Zero)
{
    cpu.A = 0x00;
    mem[0x8000] = CPU::INS_STA_IM.opcode;
    mem[0x8001] = 0x50;

    cpu.Execute(3, mem);

    EXPECT_EQ(mem[0x50], 0x00);
}

TEST_F(M6502Test1, TestSTA_MaxValue)
{
    cpu.A = 0xFF;
    mem[0x8000] = CPU::INS_STA_IM.opcode;
    mem[0x8001] = 0xFF;

    cpu.Execute(3, mem);

    EXPECT_EQ(mem[0xFF], 0xFF);
}

// ========== LDA Immediate Tests ==========
TEST_F(M6502Test1, TestLDA_IM)
{
    mem[0x8000] = CPU::INS_LDA_IM.opcode;
    mem[0x8001] = 0x84;

    cpu.Execute(2, mem);

    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_EQ(cpu.N, 1); // Negative flag should be set (bit 7 is 1)
    EXPECT_EQ(cpu.Z, 0); // Zero flag should be clear
}

TEST_F(M6502Test1, TestLDA_IM_Zero)
{
    mem[0x8000] = CPU::INS_LDA_IM.opcode;
    mem[0x8001] = 0x00;

    cpu.Execute(2, mem);

    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.Z, 1); // Zero flag should be set
    EXPECT_EQ(cpu.N, 0); // Negative flag should be clear
}

TEST_F(M6502Test1, TestLDA_IM_Positive)
{
    mem[0x8000] = CPU::INS_LDA_IM.opcode;
    mem[0x8001] = 0x42;

    cpu.Execute(2, mem);

    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.Z, 0); // Zero flag should be clear
    EXPECT_EQ(cpu.N, 0); // Negative flag should be clear (bit 7 is 0)
}

TEST_F(M6502Test1, TestLDA_IM_MaxValue)
{
    mem[0x8000] = CPU::INS_LDA_IM.opcode;
    mem[0x8001] = 0xFF;

    cpu.Execute(2, mem);

    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_EQ(cpu.N, 1); // Negative flag should be set
    EXPECT_EQ(cpu.Z, 0); // Zero flag should be clear
}

// ========== LDA Zero Page Tests ==========
TEST_F(M6502Test1, TestLDA_ZP)
{
    mem[0x0040] = 0x55;
    mem[0x8000] = CPU::INS_LDA_ZP.opcode;
    mem[0x8001] = 0x40;

    cpu.Execute(3, mem);

    EXPECT_EQ(cpu.A, 0x55);
    EXPECT_EQ(cpu.Z, 0);
    EXPECT_EQ(cpu.N, 0);
}

TEST_F(M6502Test1, TestLDA_ZP_Zero)
{
    mem[0x0010] = 0x00;
    mem[0x8000] = CPU::INS_LDA_ZP.opcode;
    mem[0x8001] = 0x10;

    cpu.Execute(3, mem);

    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.Z, 1); // Zero flag should be set
}

TEST_F(M6502Test1, TestLDA_ZP_Boundary)
{
    mem[0x00FF] = 0xAA;
    mem[0x8000] = CPU::INS_LDA_ZP.opcode;
    mem[0x8001] = 0xFF;

    cpu.Execute(3, mem);

    EXPECT_EQ(cpu.A, 0xAA);
}

// ========== LDA Zero Page X Tests ==========
TEST_F(M6502Test1, TestLDA_ZPX)
{
    cpu.X = 0x01;
    mem[0x0041] = 0x77;
    mem[0x8000] = CPU::INS_LDA_ZPX.opcode;
    mem[0x8001] = 0x40;

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0x77);
}

TEST_F(M6502Test1, TestLDA_ZPX_ZeroOffset)
{
    cpu.X = 0x00;
    mem[0x0030] = 0x66;
    mem[0x8000] = CPU::INS_LDA_ZPX.opcode;
    mem[0x8001] = 0x30;

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0x66);
}

TEST_F(M6502Test1, TestLDA_ZPX_Wraparound)
{
    cpu.X = 0x10;
    mem[0x000F] = 0x88; // 0xFF + 0x10 wraps to 0x0F
    mem[0x8000] = CPU::INS_LDA_ZPX.opcode;
    mem[0x8001] = 0xFF;

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0x88);
}

// ========== LDA Absolute Tests ==========
TEST_F(M6502Test1, TestLDA_ABS)
{
    mem[0x4400] = 0x99;
    mem[0x8000] = CPU::INS_LDA_ABS.opcode;
    mem[0x8001] = 0x00; // Low byte
    mem[0x8002] = 0x44; // High byte

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0x99);
}

TEST_F(M6502Test1, TestLDA_ABS_HighAddress)
{
    mem[0xFFFE] = 0xCC;
    mem[0x8000] = CPU::INS_LDA_ABS.opcode;
    mem[0x8001] = 0xFE; // Low byte
    mem[0x8002] = 0xFF; // High byte

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0xCC);
}

// ========== LDA Absolute,X Tests ==========
TEST_F(M6502Test1, TestLDA_ABSX)
{
    cpu.X = 0x02;
    mem[0x4402] = 0xBB;
    mem[0x8000] = CPU::INS_LDA_ABSX.opcode;
    mem[0x8001] = 0x00; // Low byte
    mem[0x8002] = 0x44; // High byte

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0xBB);
}

TEST_F(M6502Test1, TestLDA_ABSX_ZeroOffset)
{
    cpu.X = 0x00;
    mem[0x5000] = 0xDD;
    mem[0x8000] = CPU::INS_LDA_ABSX.opcode;
    mem[0x8001] = 0x00; // Low byte
    mem[0x8002] = 0x50; // High byte

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0xDD);
}

// ========== LDA Absolute,Y Tests ==========
TEST_F(M6502Test1, TestLDA_ABSY)
{
    cpu.Y = 0x05;
    mem[0x3005] = 0xEE;
    mem[0x8000] = CPU::INS_LDA_ABSY.opcode;
    mem[0x8001] = 0x00; // Low byte
    mem[0x8002] = 0x30; // High byte

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0xEE);
}

TEST_F(M6502Test1, TestLDA_ABSY_ZeroOffset)
{
    cpu.Y = 0x00;
    mem[0x6000] = 0xFF;
    mem[0x8000] = CPU::INS_LDA_ABSY.opcode;
    mem[0x8001] = 0x00; // Low byte
    mem[0x8002] = 0x60; // High byte

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0xFF);
}

// ========== LDX Tests ==========
TEST_F(M6502Test1, TestLDX_IM)
{
    mem[0x8000] = CPU::INS_LDX_IM.opcode;
    mem[0x8001] = 0x55;

    cpu.Execute(2, mem);

    EXPECT_EQ(cpu.X, 0x55);
}

TEST_F(M6502Test1, TestLDX_IM_Zero)
{
    mem[0x8000] = CPU::INS_LDX_IM.opcode;
    mem[0x8001] = 0x00;

    cpu.Execute(2, mem);

    EXPECT_EQ(cpu.X, 0x00);
}

TEST_F(M6502Test1, TestLDX_IM_MaxValue)
{
    mem[0x8000] = CPU::INS_LDX_IM.opcode;
    mem[0x8001] = 0xFF;

    cpu.Execute(2, mem);

    EXPECT_EQ(cpu.X, 0xFF);
}

// ========== JSR/RTS Tests ==========
TEST_F(M6502Test1, TestJSR)
{
    mem[0x8000] = CPU::INS_JSR.opcode;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x81;

    cpu.Execute(6, mem); // Ejecutar JSR

    EXPECT_EQ(cpu.PC, 0x8100);
}

TEST_F(M6502Test1, TestRTS)
{
    // Simular una llamada a subrutina
    mem[0x8000] = CPU::INS_JSR.opcode;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x81;
    mem[0x8100] = CPU::INS_RTS.opcode;

    cpu.Execute(12, mem); // Ejecutar JSR y RTS
    
    EXPECT_EQ(cpu.PC, 0x8003);
}

TEST_F(M6502Test1, TestJSR_StackPointer)
{
    Byte initialSP = cpu.SP;
    mem[0x8000] = CPU::INS_JSR.opcode;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x90;

    cpu.Execute(6, mem);

    // Stack pointer should have decremented by 2 (return address is 2 bytes)
    EXPECT_EQ(cpu.SP, initialSP - 2);
}

TEST_F(M6502Test1, TestRTS_StackPointer)
{
    Byte initialSP = cpu.SP;
    mem[0x8000] = CPU::INS_JSR.opcode;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x81;
    mem[0x8100] = CPU::INS_RTS.opcode;

    cpu.Execute(12, mem);

    // Stack pointer should be back to initial value
    EXPECT_EQ(cpu.SP, initialSP);
}

TEST_F(M6502Test1, TestNestedJSR)
{
    // Test nested subroutine calls
    mem[0x8000] = CPU::INS_JSR.opcode;
    mem[0x8001] = 0x00;
    mem[0x8002] = 0x81;
    
    mem[0x8100] = CPU::INS_JSR.opcode; // Nested JSR
    mem[0x8101] = 0x00;
    mem[0x8102] = 0x82;
    
    mem[0x8200] = CPU::INS_RTS.opcode; // Return from nested
    mem[0x8103] = CPU::INS_RTS.opcode; // Return from first

    cpu.Execute(24, mem); // JSR(6) + JSR(6) + RTS(6) + RTS(6)

    EXPECT_EQ(cpu.PC, 0x8003);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}