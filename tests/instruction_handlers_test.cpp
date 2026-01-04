#include <gtest/gtest.h>
#include "cpu.hpp"
#include "system_map.hpp"
#include "cpu_instructions.hpp"
#include "cpu_addressing.hpp"

// Test fixture for new instruction handlers
class InstructionHandlersTest : public testing::Test
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
        // Initialize instruction table
        Instructions::InitializeInstructionTable();
    }

    virtual void TearDown()
    {
    }
};

// ========== LDY Tests ==========
TEST_F(InstructionHandlersTest, TestLDY_Immediate)
{
    cpu.Y = 0;
    bus.write(0x8000, 0x42);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::LDY(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.Y, 0x42);
    EXPECT_EQ(cpu.Z, 0);
    EXPECT_EQ(cpu.N, 0);
}

TEST_F(InstructionHandlersTest, TestLDY_Zero)
{
    cpu.Y = 0xFF;
    bus.write(0x8000, 0x00);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::LDY(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.Y, 0x00);
    EXPECT_EQ(cpu.Z, 1);
    EXPECT_EQ(cpu.N, 0);
}

TEST_F(InstructionHandlersTest, TestLDY_Negative)
{
    bus.write(0x8000, 0x80);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::LDY(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.Y, 0x80);
    EXPECT_EQ(cpu.N, 1);
}

// ========== STX Tests ==========
TEST_F(InstructionHandlersTest, TestSTX_ZeroPage)
{
    cpu.X = 0x42;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x50);
    u32 cycles = 3;
    Word addr = Addressing::ZeroPage(cpu, cycles, bus);
    Instructions::STX(cpu, cycles, bus, addr);
    EXPECT_EQ(bus.read(0x50), 0x42);
}

// ========== STY Tests ==========
TEST_F(InstructionHandlersTest, TestSTY_ZeroPage)
{
    cpu.Y = 0x33;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x60);
    u32 cycles = 3;
    Word addr = Addressing::ZeroPage(cpu, cycles, bus);
    Instructions::STY(cpu, cycles, bus, addr);
    EXPECT_EQ(bus.read(0x60), 0x33);
}

// ========== Transfer Tests ==========
TEST_F(InstructionHandlersTest, TestTAX)
{
    cpu.A = 0x42;
    u32 cycles = 2;
    Instructions::TAX(cpu, cycles, bus);
    EXPECT_EQ(cpu.X, 0x42);
    EXPECT_EQ(cpu.Z, 0);
    EXPECT_EQ(cpu.N, 0);
}

TEST_F(InstructionHandlersTest, TestTAY)
{
    cpu.A = 0x84;
    u32 cycles = 2;
    Instructions::TAY(cpu, cycles, bus);
    EXPECT_EQ(cpu.Y, 0x84);
    EXPECT_EQ(cpu.N, 1);
}

TEST_F(InstructionHandlersTest, TestTXA)
{
    cpu.X = 0x55;
    u32 cycles = 2;
    
    Instructions::TXA(cpu, cycles, bus);

    EXPECT_EQ(cpu.A, 0x55);
}

TEST_F(InstructionHandlersTest, TestTYA)
{
    cpu.Y = 0x99;
    u32 cycles = 2;
    
    Instructions::TYA(cpu, cycles, bus);
    EXPECT_EQ(cpu.A, 0x99);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestTSX)
{
    cpu.SP = 0xFD;
    u32 cycles = 2;
    
    Instructions::TSX(cpu, cycles, bus);
    EXPECT_EQ(cpu.X, 0xFD);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestTXS)
{
    cpu.X = 0x55;
    u32 cycles = 2;
    Instructions::TXS(cpu, cycles, bus);
    EXPECT_EQ(cpu.SP, 0x55);
}
// ...existing code...
// ========== Stack Tests ==========
TEST_F(InstructionHandlersTest, TestPHA)
{
    cpu.A = 0x42;
    Byte initialSP = cpu.SP;
    u32 cycles = 3;
    Instructions::PHA(cpu, cycles, bus);
    EXPECT_EQ(bus.read(0x0100 + initialSP), 0x42);
    EXPECT_EQ(cpu.SP, initialSP - 1);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestPLA)
{
    cpu.SP = 0xFE;
    bus.write(0x01FF, 0x55);
    u32 cycles = 4;
    Instructions::PLA(cpu, cycles, bus);
    EXPECT_EQ(cpu.A, 0x55);
    EXPECT_EQ(cpu.SP, 0xFF);
// ...existing code...
}

// ========== Logical Tests ==========
TEST_F(InstructionHandlersTest, TestAND_Immediate)
{
    cpu.A = 0xFF;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x0F);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::AND(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.A, 0x0F);
    EXPECT_EQ(cpu.Z, 0);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestEOR_Immediate)
{
    cpu.A = 0xFF;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0xFF);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::EOR(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.Z, 1);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestORA_Immediate)
{
    cpu.A = 0x0F;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0xF0);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::ORA(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_EQ(cpu.N, 1);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestBIT_ZeroPage)
{
    cpu.A = 0xFF;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x42);
    bus.write(0x0042, 0xC0);
    u32 cycles = 3;
    Word addr = Addressing::ZeroPage(cpu, cycles, bus);
    Instructions::BIT(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.N, 1);
    EXPECT_EQ(cpu.V, 1);
    EXPECT_EQ(cpu.Z, 0);
// ...existing code...
}

// ========== Arithmetic Tests ==========
TEST_F(InstructionHandlersTest, TestADC_Simple)
{
    cpu.A = 0x10;
    cpu.C = 0;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x20);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::ADC(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.A, 0x30);
    EXPECT_EQ(cpu.C, 0);
    EXPECT_EQ(cpu.V, 0);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestADC_Overflow)
{
    cpu.A = 0xFF;
    cpu.C = 0;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x01);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::ADC(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.C, 1);
    EXPECT_EQ(cpu.Z, 1);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestSBC_Simple)
{
    cpu.A = 0x30;
    cpu.C = 1;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x10);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::SBC(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.A, 0x20);
    EXPECT_EQ(cpu.C, 1);
// ...existing code...
}

// ========== Compare Tests ==========
TEST_F(InstructionHandlersTest, TestCMP_Equal)
{
    cpu.A = 0x42;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x42);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::CMP(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.Z, 1);
    EXPECT_EQ(cpu.C, 1);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestCPX_Equal)
{
    cpu.X = 0x42;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x42);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::CPX(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.Z, 1);
    EXPECT_EQ(cpu.C, 1);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestCPY_Equal)
{
    cpu.Y = 0x42;
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x42);
    u32 cycles = 2;
    Word addr = Addressing::Immediate(cpu, cycles, bus);
    Instructions::CPY(cpu, cycles, bus, addr);
    EXPECT_EQ(cpu.Z, 1);
    EXPECT_EQ(cpu.C, 1);
// ...existing code...
}

// ========== Inc/Dec Tests ==========
TEST_F(InstructionHandlersTest, TestINC_ZeroPage)
{
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x42);
    bus.write(0x0042, 0x10);
    u32 cycles = 5;
    Word addr = Addressing::ZeroPage(cpu, cycles, bus);
    Instructions::INC(cpu, cycles, bus, addr);
    EXPECT_EQ(bus.read(0x0042), 0x11);
    EXPECT_EQ(cpu.Z, 0);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestINX)
{
    cpu.X = 0x10;
    u32 cycles = 2;
    Instructions::INX(cpu, cycles, bus);
    EXPECT_EQ(cpu.X, 0x11);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestINY)
{
    cpu.Y = 0x10;
    u32 cycles = 2;
    Instructions::INY(cpu, cycles, bus);
    EXPECT_EQ(cpu.Y, 0x11);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestDEC_ZeroPage)
{
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x42);
    bus.write(0x0042, 0x10);
    u32 cycles = 5;
    Word addr = Addressing::ZeroPage(cpu, cycles, bus);
    Instructions::DEC(cpu, cycles, bus, addr);
    EXPECT_EQ(bus.read(0x0042), 0x0F);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestDEX)
{
    cpu.X = 0x10;
    u32 cycles = 2;
    Instructions::DEX(cpu, cycles, bus);
    EXPECT_EQ(cpu.X, 0x0F);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestDEY)
{
    cpu.Y = 0x10;
    u32 cycles = 2;
    Instructions::DEY(cpu, cycles, bus);
    EXPECT_EQ(cpu.Y, 0x0F);
// ...existing code...
}

// ========== Shift Tests ==========
TEST_F(InstructionHandlersTest, TestASL_Accumulator)
{
    cpu.A = 0x40;
    u32 cycles = 2;
    Instructions::ASL(cpu, cycles, bus, 0, true);
    EXPECT_EQ(cpu.A, 0x80);
    EXPECT_EQ(cpu.C, 0);
    EXPECT_EQ(cpu.N, 1);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestLSR_Accumulator)
{
    cpu.A = 0x02;
    u32 cycles = 2;
    Instructions::LSR(cpu, cycles, bus, 0, true);
    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cpu.C, 0);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestROL_Accumulator)
{
    cpu.A = 0x40;
    cpu.C = 1;
    u32 cycles = 2;
    Instructions::ROL(cpu, cycles, bus, 0, true);
    EXPECT_EQ(cpu.A, 0x81);
    EXPECT_EQ(cpu.C, 0);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestROR_Accumulator)
{
    cpu.A = 0x02;
    cpu.C = 1;
    u32 cycles = 2;
    Instructions::ROR(cpu, cycles, bus, 0, true);
    EXPECT_EQ(cpu.A, 0x81);
    EXPECT_EQ(cpu.C, 0);
// ...existing code...
}

// ========== Branch Tests ==========
TEST_F(InstructionHandlersTest, TestBranch_Taken)
{
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x10); // +16 offset
    u32 cycles = 2;
    Instructions::Branch(cpu, cycles, bus, true);
    EXPECT_EQ(cpu.PC, 0x8011); // 0x8001 + 0x10
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestBranch_NotTaken)
{
    cpu.PC = 0x8000;
    bus.write(0x8000, 0x10);
    u32 cycles = 2;
    Word initialPC = cpu.PC;
    Instructions::Branch(cpu, cycles, bus, false);
    EXPECT_EQ(cpu.PC, 0x8001); // Only consumed offset byte, no branch taken
    EXPECT_EQ(cycles, 1); // Should have consumed 1 cycle for fetch
// ...existing code...
}

// ========== Flag Tests ==========
TEST_F(InstructionHandlersTest, TestCLC)
{
    cpu.C = 1;
    u32 cycles = 2;
    Instructions::CLC(cpu, cycles, bus);
    EXPECT_EQ(cpu.C, 0);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestSEC)
{
    cpu.C = 0;
    u32 cycles = 2;
    Instructions::SEC(cpu, cycles, bus);
    EXPECT_EQ(cpu.C, 1);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestCLI)
{
    cpu.I = 1;
    u32 cycles = 2;
    Instructions::CLI(cpu, cycles, bus);
    EXPECT_EQ(cpu.I, 0);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestSEI)
{
    cpu.I = 0;
    u32 cycles = 2;
    Instructions::SEI(cpu, cycles, bus);
    EXPECT_EQ(cpu.I, 1);
// ...existing code...
}

TEST_F(InstructionHandlersTest, TestCLV)
{
    cpu.V = 1;
    u32 cycles = 2;
    Instructions::CLV(cpu, cycles, bus);
    EXPECT_EQ(cpu.V, 0);
// ...existing code...
}

// ========== System Tests ==========
TEST_F(InstructionHandlersTest, TestNOP)
{
    u32 cycles = 2;
    Instructions::NOP(cpu, cycles, bus);
    EXPECT_EQ(cycles, 1); // Should consume 1 cycle
// ...existing code...
}
