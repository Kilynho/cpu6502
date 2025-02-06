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

TEST_F(M6502Test1, TestSTA)
{
    cpu.A = 0x42;
    mem[0x8000] = CPU::INS_STA_IM.opcode;
    mem[0x8001] = 0x40;

    cpu.Execute(3, mem);

    EXPECT_EQ(mem[0x40], 0x42);
}

TEST_F(M6502Test1, TestLDA_IM)
{
    mem[0x8000] = CPU::INS_LDA_IM.opcode;
    mem[0x8001] = 0x84;

    cpu.Execute(2, mem);

    EXPECT_EQ(cpu.A, 0x84);
}

TEST_F(M6502Test1, TestLDA_ZP)
{
    mem[0x0040] = 0x55;
    mem[0x8000] = CPU::INS_LDA_ZP.opcode;
    mem[0x8001] = 0x40;

    cpu.Execute(3, mem);

    EXPECT_EQ(cpu.A, 0x55);
}

TEST_F(M6502Test1, TestLDA_ZPX)
{
    cpu.X = 0x01;
    mem[0x0041] = 0x77;
    mem[0x8000] = CPU::INS_LDA_ZPX.opcode;
    mem[0x8001] = 0x40;

    cpu.Execute(4, mem);

    EXPECT_EQ(cpu.A, 0x77);
}

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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}