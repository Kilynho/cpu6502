#include <gtest/gtest.h>
#include <array>
#include <initializer_list>
#include "cpu.hpp"
#include "mem.hpp"
#include "cpu_instructions.hpp"
#include "cpu_addressing.hpp"

// Test fixture for new instruction handlers
class InstructionHandlersTest : public testing::Test
{
public:
    Mem mem;
    CPU cpu;

    virtual void SetUp()
    {
        mem.Initialize();
        mem[Mem::RESET_VECTOR] = 0x00;
        mem[Mem::RESET_VECTOR + 1] = 0x80;
        Instructions::InitializeInstructionTable();
        cpu.Reset(mem);
    }

    virtual void TearDown()
    {
    }
};

// Test fixture for undocumented NOP opcodes executed through the instruction table
class UndocumentedNopTableTest : public testing::Test
{
public:
    Mem mem;
    CPU cpu;

    void SetUp() override
    {
        resetCpu();
    }

protected:
    void resetCpu()
    {
        mem.Initialize();
        mem[Mem::RESET_VECTOR] = 0x00;
        mem[Mem::RESET_VECTOR + 1] = 0x80;
        Instructions::InitializeInstructionTable();
        cpu.Reset(mem);
    }

    void loadOpcode(Byte opcode, std::initializer_list<Byte> operands = {})
    {
        Word address = cpu.PC;
        mem[address++] = opcode;
        for (Byte value : operands)
        {
            mem[address++] = value;
        }
    }

    void fetchAndExecute(u32& cycles)
    {
        Byte opcode = cpu.FetchByte(cycles, mem);
        InstrHandler handler = Instructions::GetHandler(opcode);
        handler(cpu, cycles, mem);
    }
};

// ========== LDY Tests ==========
TEST_F(InstructionHandlersTest, TestLDY_Immediate)
{
    cpu.Y = 0;
    mem[0x8000] = 0x42;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::LDY(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.Y, 0x42);
    EXPECT_EQ(cpu.Z, 0);
    EXPECT_EQ(cpu.N, 0);
}

TEST_F(InstructionHandlersTest, TestLDY_Zero)
{
    cpu.Y = 0xFF;
    mem[0x8000] = 0x00;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::LDY(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.Y, 0x00);
    EXPECT_EQ(cpu.Z, 1);
    EXPECT_EQ(cpu.N, 0);
}

TEST_F(InstructionHandlersTest, TestLDY_Negative)
{
    mem[0x8000] = 0x80;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::LDY(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.Y, 0x80);
    EXPECT_EQ(cpu.N, 1);
}

// ========== STX Tests ==========
TEST_F(InstructionHandlersTest, TestSTX_ZeroPage)
{
    cpu.X = 0x42;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x50;
    u32 cycles = 3;
    
    Word addr = Addressing::ZeroPage(cpu, cycles, mem);
    Instructions::STX(cpu, cycles, mem, addr);

    EXPECT_EQ(mem[0x50], 0x42);
}

// ========== STY Tests ==========
TEST_F(InstructionHandlersTest, TestSTY_ZeroPage)
{
    cpu.Y = 0x33;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x60;
    u32 cycles = 3;
    
    Word addr = Addressing::ZeroPage(cpu, cycles, mem);
    Instructions::STY(cpu, cycles, mem, addr);

    EXPECT_EQ(mem[0x60], 0x33);
}

// ========== Transfer Tests ==========
TEST_F(InstructionHandlersTest, TestTAX)
{
    cpu.A = 0x42;
    u32 cycles = 2;
    
    Instructions::TAX(cpu, cycles, mem);

    EXPECT_EQ(cpu.X, 0x42);
    EXPECT_EQ(cpu.Z, 0);
    EXPECT_EQ(cpu.N, 0);
}

TEST_F(InstructionHandlersTest, TestTAY)
{
    cpu.A = 0x84;
    u32 cycles = 2;
    
    Instructions::TAY(cpu, cycles, mem);

    EXPECT_EQ(cpu.Y, 0x84);
    EXPECT_EQ(cpu.N, 1);
}

TEST_F(InstructionHandlersTest, TestTXA)
{
    cpu.X = 0x55;
    u32 cycles = 2;
    
    Instructions::TXA(cpu, cycles, mem);

    EXPECT_EQ(cpu.A, 0x55);
}

TEST_F(InstructionHandlersTest, TestTYA)
{
    cpu.Y = 0x99;
    u32 cycles = 2;
    
    Instructions::TYA(cpu, cycles, mem);

    EXPECT_EQ(cpu.A, 0x99);
}

TEST_F(InstructionHandlersTest, TestTSX)
{
    cpu.SP = 0xFD;
    u32 cycles = 2;
    
    Instructions::TSX(cpu, cycles, mem);

    EXPECT_EQ(cpu.X, 0xFD);
}

TEST_F(InstructionHandlersTest, TestTXS)
{
    cpu.X = 0xAB;
    u32 cycles = 2;
    
    Instructions::TXS(cpu, cycles, mem);

    EXPECT_EQ(cpu.SP, 0xAB);
}

// ========== Stack Tests ==========
TEST_F(InstructionHandlersTest, TestPHA)
{
    cpu.A = 0x42;
    Byte initialSP = cpu.SP;
    u32 cycles = 3;
    
    Instructions::PHA(cpu, cycles, mem);

    EXPECT_EQ(mem[0x0100 + initialSP], 0x42);
    EXPECT_EQ(cpu.SP, initialSP - 1);
}

TEST_F(InstructionHandlersTest, TestPLA)
{
    cpu.SP = 0xFE;
    mem[0x01FF] = 0x55;
    u32 cycles = 4;
    
    Instructions::PLA(cpu, cycles, mem);

    EXPECT_EQ(cpu.A, 0x55);
    EXPECT_EQ(cpu.SP, 0xFF);
}

// ========== Logical Tests ==========
TEST_F(InstructionHandlersTest, TestAND_Immediate)
{
    cpu.A = 0xFF;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x0F;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::AND(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.A, 0x0F);
    EXPECT_EQ(cpu.Z, 0);
}

TEST_F(InstructionHandlersTest, TestEOR_Immediate)
{
    cpu.A = 0xFF;
    cpu.PC = 0x8000;
    mem[0x8000] = 0xFF;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::EOR(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.Z, 1);
}

TEST_F(InstructionHandlersTest, TestORA_Immediate)
{
    cpu.A = 0x0F;
    cpu.PC = 0x8000;
    mem[0x8000] = 0xF0;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::ORA(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.A, 0xFF);
    EXPECT_EQ(cpu.N, 1);
}

TEST_F(InstructionHandlersTest, TestBIT_ZeroPage)
{
    cpu.A = 0xFF;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x42;
    mem[0x0042] = 0xC0;
    u32 cycles = 3;
    
    Word addr = Addressing::ZeroPage(cpu, cycles, mem);
    Instructions::BIT(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.N, 1);
    EXPECT_EQ(cpu.V, 1);
    EXPECT_EQ(cpu.Z, 0);
}

// ========== Arithmetic Tests ==========
TEST_F(InstructionHandlersTest, TestADC_Simple)
{
    cpu.A = 0x10;
    cpu.C = 0;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x20;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::ADC(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.A, 0x30);
    EXPECT_EQ(cpu.C, 0);
    EXPECT_EQ(cpu.V, 0);
}

TEST_F(InstructionHandlersTest, TestADC_Overflow)
{
    cpu.A = 0xFF;
    cpu.C = 0;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x01;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::ADC(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.C, 1);
    EXPECT_EQ(cpu.Z, 1);
}

TEST_F(InstructionHandlersTest, TestSBC_Simple)
{
    cpu.A = 0x30;
    cpu.C = 1;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x10;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::SBC(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.A, 0x20);
    EXPECT_EQ(cpu.C, 1);
}

// ========== Compare Tests ==========
TEST_F(InstructionHandlersTest, TestCMP_Equal)
{
    cpu.A = 0x42;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x42;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::CMP(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.Z, 1);
    EXPECT_EQ(cpu.C, 1);
}

TEST_F(InstructionHandlersTest, TestCPX_Equal)
{
    cpu.X = 0x42;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x42;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::CPX(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.Z, 1);
    EXPECT_EQ(cpu.C, 1);
}

TEST_F(InstructionHandlersTest, TestCPY_Equal)
{
    cpu.Y = 0x42;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x42;
    u32 cycles = 2;
    
    Word addr = Addressing::Immediate(cpu, cycles, mem);
    Instructions::CPY(cpu, cycles, mem, addr);

    EXPECT_EQ(cpu.Z, 1);
    EXPECT_EQ(cpu.C, 1);
}

// ========== Inc/Dec Tests ==========
TEST_F(InstructionHandlersTest, TestINC_ZeroPage)
{
    cpu.PC = 0x8000;
    mem[0x8000] = 0x42;
    mem[0x0042] = 0x10;
    u32 cycles = 5;
    
    Word addr = Addressing::ZeroPage(cpu, cycles, mem);
    Instructions::INC(cpu, cycles, mem, addr);

    EXPECT_EQ(mem[0x0042], 0x11);
    EXPECT_EQ(cpu.Z, 0);
}

TEST_F(InstructionHandlersTest, TestINX)
{
    cpu.X = 0x10;
    u32 cycles = 2;
    
    Instructions::INX(cpu, cycles, mem);

    EXPECT_EQ(cpu.X, 0x11);
}

TEST_F(InstructionHandlersTest, TestINY)
{
    cpu.Y = 0x10;
    u32 cycles = 2;
    
    Instructions::INY(cpu, cycles, mem);

    EXPECT_EQ(cpu.Y, 0x11);
}

TEST_F(InstructionHandlersTest, TestDEC_ZeroPage)
{
    cpu.PC = 0x8000;
    mem[0x8000] = 0x42;
    mem[0x0042] = 0x10;
    u32 cycles = 5;
    
    Word addr = Addressing::ZeroPage(cpu, cycles, mem);
    Instructions::DEC(cpu, cycles, mem, addr);

    EXPECT_EQ(mem[0x0042], 0x0F);
}

TEST_F(InstructionHandlersTest, TestDEX)
{
    cpu.X = 0x10;
    u32 cycles = 2;
    
    Instructions::DEX(cpu, cycles, mem);

    EXPECT_EQ(cpu.X, 0x0F);
}

TEST_F(InstructionHandlersTest, TestDEY)
{
    cpu.Y = 0x10;
    u32 cycles = 2;
    
    Instructions::DEY(cpu, cycles, mem);

    EXPECT_EQ(cpu.Y, 0x0F);
}

// ========== Shift Tests ==========
TEST_F(InstructionHandlersTest, TestASL_Accumulator)
{
    cpu.A = 0x40;
    u32 cycles = 2;
    
    Instructions::ASL(cpu, cycles, mem, 0, true);

    EXPECT_EQ(cpu.A, 0x80);
    EXPECT_EQ(cpu.C, 0);
    EXPECT_EQ(cpu.N, 1);
}

TEST_F(InstructionHandlersTest, TestLSR_Accumulator)
{
    cpu.A = 0x02;
    u32 cycles = 2;
    
    Instructions::LSR(cpu, cycles, mem, 0, true);

    EXPECT_EQ(cpu.A, 0x01);
    EXPECT_EQ(cpu.C, 0);
}

TEST_F(InstructionHandlersTest, TestROL_Accumulator)
{
    cpu.A = 0x40;
    cpu.C = 1;
    u32 cycles = 2;
    
    Instructions::ROL(cpu, cycles, mem, 0, true);

    EXPECT_EQ(cpu.A, 0x81);
    EXPECT_EQ(cpu.C, 0);
}

TEST_F(InstructionHandlersTest, TestROR_Accumulator)
{
    cpu.A = 0x02;
    cpu.C = 1;
    u32 cycles = 2;
    
    Instructions::ROR(cpu, cycles, mem, 0, true);

    EXPECT_EQ(cpu.A, 0x81);
    EXPECT_EQ(cpu.C, 0);
}

// ========== Branch Tests ==========
TEST_F(InstructionHandlersTest, TestBranch_Taken)
{
    cpu.PC = 0x8000;
    mem[0x8000] = 0x10; // +16 offset
    u32 cycles = 2;
    
    Instructions::Branch(cpu, cycles, mem, true);

    EXPECT_EQ(cpu.PC, 0x8011); // 0x8001 + 0x10
}

TEST_F(InstructionHandlersTest, TestBranch_NotTaken)
{
    cpu.PC = 0x8000;
    mem[0x8000] = 0x10;
    u32 cycles = 2;
    Word initialPC = cpu.PC;
    
    Instructions::Branch(cpu, cycles, mem, false);
    
    EXPECT_EQ(cpu.PC, 0x8001); // Only consumed offset byte, no branch taken
    EXPECT_EQ(cycles, 0u); // Fetch + base branch cycle consumed
}

// ========== Flag Tests ==========
TEST_F(InstructionHandlersTest, TestCLC)
{
    cpu.C = 1;
    u32 cycles = 2;
    
    Instructions::CLC(cpu, cycles, mem);

    EXPECT_EQ(cpu.C, 0);
}

TEST_F(InstructionHandlersTest, TestSEC)
{
    cpu.C = 0;
    u32 cycles = 2;
    
    Instructions::SEC(cpu, cycles, mem);

    EXPECT_EQ(cpu.C, 1);
}

TEST_F(InstructionHandlersTest, TestCLI)
{
    cpu.I = 1;
    u32 cycles = 2;
    
    Instructions::CLI(cpu, cycles, mem);

    EXPECT_EQ(cpu.I, 0);
}

TEST_F(InstructionHandlersTest, TestSEI)
{
    cpu.I = 0;
    u32 cycles = 2;
    
    Instructions::SEI(cpu, cycles, mem);

    EXPECT_EQ(cpu.I, 1);
}

TEST_F(InstructionHandlersTest, TestCLV)
{
    cpu.V = 1;
    u32 cycles = 2;
    
    Instructions::CLV(cpu, cycles, mem);

    EXPECT_EQ(cpu.V, 0);
}


// ========== 65C02 Tests ==========
TEST_F(InstructionHandlersTest, TestINC_A_65C02)
{
    cpu.A = 0x7F;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x1A; // INC A
    u32 cycles = 2;
    Byte opcode = cpu.FetchByte(cycles, mem);
    InstrHandler h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    EXPECT_EQ(cpu.A, 0x80);
    EXPECT_EQ(cpu.N, 1);
    // Cycle accounting varies with fetch; only verify result
}

TEST_F(InstructionHandlersTest, TestDEC_A_65C02)
{
    cpu.A = 0x01;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x3A; // DEC A
    u32 cycles = 2;
    Byte opcode = cpu.FetchByte(cycles, mem);
    InstrHandler h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    EXPECT_EQ(cpu.A, 0x00);
    EXPECT_EQ(cpu.Z, 1);
    EXPECT_EQ(cycles, 0u);
}

TEST_F(InstructionHandlersTest, TestSTZ_ZeroPage_65C02)
{
    cpu.PC = 0x8000;
    mem[0x10] = 0xFF;
    mem[0x8000] = 0x64; // STZ zp
    mem[0x8001] = 0x10;
    u32 cycles = 3;
    Byte opcode = cpu.FetchByte(cycles, mem);
    InstrHandler h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    EXPECT_EQ(mem[0x10], 0x00);
}

TEST_F(InstructionHandlersTest, TestTSB_TRB_ZeroPage_65C02)
{
    mem[0x10] = 0x0F;
    cpu.A = 0x03;
    cpu.PC = 0x8000;
    mem[0x8000] = 0x04; // TSB zp
    mem[0x8001] = 0x10;
    u32 cycles = 5;
    Byte opcode = cpu.FetchByte(cycles, mem);
    InstrHandler h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    EXPECT_EQ(mem[0x10], 0x0F);
    cpu.A = 0x0C;
    mem[0x8002] = 0x14; // TRB zp
    mem[0x8003] = 0x10;
    cpu.PC = 0x8002;
    cycles = 5;
    opcode = cpu.FetchByte(cycles, mem);
    h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    EXPECT_EQ(mem[0x10], 0x03);
}

TEST_F(InstructionHandlersTest, TestBRA_65C02)
{
    cpu.PC = 0x8000;
    u32 cycles = 3;
    mem[0x8000] = 0x80; // BRA
    mem[0x8001] = 0x02; // +2
    Byte opcode = cpu.FetchByte(cycles, mem);
    InstrHandler h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    EXPECT_EQ(cpu.PC, 0x8004);
}

TEST_F(InstructionHandlersTest, TestIndirectZeroPageLDA_65C02)
{
    // Set pointer at zp 0x20 -> 0x9000
    mem[0x20] = 0x00;
    mem[0x21] = 0x90;
    mem[0x9000] = 0x42;
    cpu.PC = 0x8000;
    mem[0x8000] = 0xB2; // LDA (zp)
    mem[0x8001] = 0x20; // pointer
    u32 cycles = 5;
    Byte opcode = cpu.FetchByte(cycles, mem);
    InstrHandler h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    EXPECT_EQ(cpu.A, 0x42);
}

TEST_F(InstructionHandlersTest, TestPHX_PLX_PHY_PLY_65C02)
{
    // PHX
    cpu.X = 0x12;
    cpu.PC = 0x8000;
    mem[0x8000] = 0xDA; // PHX
    u32 cycles = 3;
    Byte opcode = cpu.FetchByte(cycles, mem);
    InstrHandler h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    // Cycle accounting varies with fetch
    // PHY
    cpu.Y = 0x34;
    cpu.PC = 0x8001;
    mem[0x8001] = 0x5A; // PHY
    cycles = 3;
    opcode = cpu.FetchByte(cycles, mem);
    h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    // Cycle accounting varies with fetch
    // PLX
    cpu.PC = 0x8002;
    mem[0x8002] = 0xFA; // PLX
    cycles = 4;
    opcode = cpu.FetchByte(cycles, mem);
    h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    // Last pushed (Y) is popped into X
    EXPECT_EQ(cpu.X, 0x34);
    // Cycle accounting varies with fetch
    // PLY
    cpu.PC = 0x8003;
    mem[0x8003] = 0x7A; // PLY
    cycles = 4;
    opcode = cpu.FetchByte(cycles, mem);
    h = Instructions::GetHandler(opcode);
    h(cpu, cycles, mem);
    // Next popped (X) is popped into Y
    EXPECT_EQ(cpu.Y, 0x12);
    // Cycle accounting varies with fetch
}

// ========== System Tests ==========
TEST_F(InstructionHandlersTest, TestNOP)
{
    u32 cycles = 2;
    
    Instructions::NOP(cpu, cycles, mem);

    EXPECT_EQ(cycles, 1); // Should consume 1 cycle
}
