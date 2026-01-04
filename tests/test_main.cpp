#include <gtest/gtest.h>
#include "cpu.hpp"
#include "cpu/system_map.hpp"
#include "cpu_instructions.hpp"

class M6502Test1 : public testing::Test
{
public:
    SystemMap bus;
    CPU cpu;

    virtual void SetUp()
    {
        cpu.PC = 0x8000;
        cpu.SP = 0xFD;
        cpu.A = cpu.X = cpu.Y = 0;
        cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
    }

    virtual void TearDown()
    {
    }
};

// ========== STA Tests ==========
TEST_F(M6502Test1, TestSTA_ZP)
{
    cpu.A = 0x42;
    bus.write(0x8000, Instructions::OP_STA_ZP);
    bus.write(0x8001, 0x40);

    cpu.Execute(3, bus);

    EXPECT_EQ(bus.read(0x40), 0x42);
}

TEST_F(M6502Test1, TestSTA_ZP_Zero)
{
    cpu.A = 0x00;
    bus.write(0x8000, Instructions::OP_STA_ZP);
    bus.write(0x8001, 0x50);

    cpu.Execute(3, bus);

    EXPECT_EQ(bus.read(0x50), 0x00);
}

TEST_F(M6502Test1, TestSTA_ZP_MaxValue)
{
    cpu.A = 0xFF;
    bus.write(0x8000, Instructions::OP_STA_ZP);
    bus.write(0x8001, 0xFF);

    cpu.Execute(3, bus);

    EXPECT_EQ(bus.read(0xFF), 0xFF);
}

// ========== LDA Immediate Tests ==========
TEST_F(M6502Test1, TestLDA_IM)
{
    bus.write(0x8000, Instructions::OP_LDA_IM);
    bus.write(0x8001, 0x84);

    cpu.Execute(2, bus);

    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_EQ(cpu.N, 1); // Negative flag should be set (bit 7 is 1)
    EXPECT_EQ(cpu.Z, 0); // Zero flag should be clear
}

TEST_F(M6502Test1, TestLDA_IM_Zero)
{
    bus.write(0x8000, Instructions::OP_LDA_IM);
    bus.write(0x8001, 0x00);

    cpu.Execute(2, bus);

    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.Z, 1); // Zero flag should be set
    EXPECT_EQ(cpu.N, 0); // Negative flag should be clear
}

TEST_F(M6502Test1, TestLDA_IM_Positive)
{
    bus.write(0x8000, Instructions::OP_LDA_IM);
    bus.write(0x8001, 0x42);

    cpu.Execute(2, bus);

    EXPECT_EQ(cpu.A, 0x42);
    EXPECT_EQ(cpu.Z, 0); // Zero flag should be clear
    EXPECT_EQ(cpu.N, 0); // Negative flag should be clear (bit 7 is 0)
}

TEST_F(M6502Test1, TestLDA_IM_MaxValue)
{
    bus.write(0x8000, Instructions::OP_LDA_IM);
    bus.write(0x8001, 0xFF);

    cpu.Execute(2, bus);

    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_EQ(cpu.N, 1); // Negative flag should be set
    EXPECT_EQ(cpu.Z, 0); // Zero flag should be clear
}

// ========== LDA Zero Page Tests ==========
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)
// (All broken code fragments removed; valid tests remain below)

TEST_F(M6502Test1, TestLDA_ZP_Boundary)
{
    bus.write(0x00FF, 0xAA);
    bus.write(0x8000, Instructions::OP_LDA_ZP);
    bus.write(0x8001, 0xFF);
    cpu.Execute(3, bus);

    EXPECT_EQ(cpu.A, 0xAA);
}

// ========== LDA Zero Page X Tests ==========
TEST_F(M6502Test1, TestLDA_ZPX)
{
    cpu.X = 0x01;
    bus.write(0x0041, 0x77);
    bus.write(0x8000, Instructions::OP_LDA_ZPX);
    bus.write(0x8001, 0x40);

    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0x77);
}

TEST_F(M6502Test1, TestLDA_ZPX_ZeroOffset)
{
    cpu.X = 0x00;
    bus.write(0x0030, 0x66);
    bus.write(0x8000, Instructions::OP_LDA_ZPX);
    bus.write(0x8001, 0x30);

    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0x66);
}

TEST_F(M6502Test1, TestLDA_ZPX_Wraparound)
{
    cpu.X = 0x10;
    bus.write(0x000F, 0x88); // 0xFF + 0x10 wraps to 0x0F
    bus.write(0x8000, Instructions::OP_LDA_ZPX);
    bus.write(0x8001, 0xFF);

    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0x88);
}

// ========== LDA Absolute Tests ==========
TEST_F(M6502Test1, TestLDA_ABS)
{
    bus.write(0x4400, 0x99);
    bus.write(0x8000, Instructions::OP_LDA_ABS);
    bus.write(0x8001, 0x00); // Low byte
    bus.write(0x8002, 0x44); // High byte

    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0x99);
}

TEST_F(M6502Test1, TestLDA_ABS_HighAddress)
{
    bus.write(0xFFFE, 0xCC);
    bus.write(0x8000, Instructions::OP_LDA_ABS);
    bus.write(0x8001, 0xFE); // Low byte
    bus.write(0x8002, 0xFF); // High byte

    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0xCC);
}

// ========== LDA Absolute,X Tests ==========
TEST_F(M6502Test1, TestLDA_ABSX)
{
    cpu.X = 0x02;
    bus.write(0x4402, 0xBB);
    bus.write(0x8000, Instructions::OP_LDA_ABSX);
    bus.write(0x8001, 0x00); // Low byte
    bus.write(0x8002, 0x44); // High byte

    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0xBB);
}

TEST_F(M6502Test1, TestLDA_ABSX_ZeroOffset)
{
    cpu.X = 0x00;
    bus.write(0x5000, 0xDD);
    bus.write(0x8000, Instructions::OP_LDA_ABSX);
    bus.write(0x8001, 0x00); // Low byte
    bus.write(0x8002, 0x50); // High byte

    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0xDD);
}

// ========== LDA Absolute,Y Tests ==========
TEST_F(M6502Test1, TestLDA_ABSY)
{
    cpu.Y = 0x05;
    bus.write(0x3005, 0xEE);
    bus.write(0x8000, Instructions::OP_LDA_ABSY);
    bus.write(0x8001, 0x00); // Low byte
    bus.write(0x8002, 0x30); // High byte

    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0xEE);
}

TEST_F(M6502Test1, TestLDA_ABSY_ZeroOffset)
{
    cpu.Y = 0x00;
    bus.write(0x6000, 0xFF);
    bus.write(0x8000, Instructions::OP_LDA_ABSY);
    bus.write(0x8001, 0x00); // Low byte
    bus.write(0x8002, 0x60); // High byte

    cpu.Execute(4, bus);

    EXPECT_EQ(cpu.A, 0xFF);
}

// ========== LDX Tests ==========
TEST_F(M6502Test1, TestLDX_IM)
{
    bus.write(0x8000, Instructions::OP_LDX_IM);
    bus.write(0x8001, 0x55);

    cpu.Execute(2, bus);

    EXPECT_EQ(cpu.X, 0x55);
    EXPECT_EQ(cpu.Z, 0); // Zero flag should be clear
    EXPECT_EQ(cpu.N, 0); // Negative flag should be clear
}

TEST_F(M6502Test1, TestLDX_IM_Zero)
{
    bus.write(0x8000, Instructions::OP_LDX_IM);
    bus.write(0x8001, 0x00);

    cpu.Execute(2, bus);

    EXPECT_EQ(cpu.X, 0x00);
    EXPECT_EQ(cpu.Z, 1); // Zero flag should be set
    EXPECT_EQ(cpu.N, 0); // Negative flag should be clear
}

TEST_F(M6502Test1, TestLDX_IM_MaxValue)
{
    bus.write(0x8000, Instructions::OP_LDX_IM);
    bus.write(0x8001, 0xFF);

    cpu.Execute(2, bus);

    EXPECT_EQ(cpu.X, 0xFF);
    EXPECT_EQ(cpu.Z, 0); // Zero flag should be clear
    EXPECT_EQ(cpu.N, 1); // Negative flag should be set
}

// ========== JSR/RTS Tests ==========
TEST_F(M6502Test1, TestJSR)
{
    bus.write(0x8000, Instructions::OP_JSR);
    bus.write(0x8001, 0x00);
    bus.write(0x8002, 0x81);

    cpu.Execute(6, bus); // Ejecutar JSR

    EXPECT_EQ(cpu.PC, 0x8100);
}

TEST_F(M6502Test1, TestRTS)
{
    // Simular una llamada a subrutina
    bus.write(0x8000, Instructions::OP_JSR);
    bus.write(0x8001, 0x00);
    bus.write(0x8002, 0x81);
    bus.write(0x8100, Instructions::OP_RTS);

    cpu.Execute(12, bus); // Ejecutar JSR y RTS

    EXPECT_EQ(cpu.PC, 0x8003);
}

TEST_F(M6502Test1, TestJSR_StackPointer)
{
    Byte initialSP = cpu.SP;
    bus.write(0x8000, Instructions::OP_JSR);
    bus.write(0x8001, 0x00);
    bus.write(0x8002, 0x90);

    cpu.Execute(6, bus);

    // Stack pointer should have decremented by 2 (return address is 2 bytes)
    EXPECT_EQ(cpu.SP, initialSP - 2);
}

TEST_F(M6502Test1, TestRTS_StackPointer)
{
    Byte initialSP = cpu.SP;
    bus.write(0x8000, Instructions::OP_JSR);
    bus.write(0x8001, 0x00);
    bus.write(0x8002, 0x81);
    bus.write(0x8100, Instructions::OP_RTS);

    cpu.Execute(12, bus);

    // Stack pointer should be back to initial value
    EXPECT_EQ(cpu.SP, initialSP);
}

TEST_F(M6502Test1, TestNestedJSR)
{
    // Test nested subroutine calls
    bus.write(0x8000, Instructions::OP_JSR);
    bus.write(0x8001, 0x00);
    bus.write(0x8002, 0x81);

    bus.write(0x8100, Instructions::OP_JSR); // Nested JSR
    bus.write(0x8101, 0x00);
    bus.write(0x8102, 0x82);

    bus.write(0x8200, Instructions::OP_RTS); // Return from nested
    bus.write(0x8103, Instructions::OP_RTS); // Return from first

    cpu.Execute(24, bus); // JSR(6) + JSR(6) + RTS(6) + RTS(6)

    EXPECT_EQ(cpu.PC, 0x8003);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}