#include <gtest/gtest.h>
#include "cpu.hpp"
#include "mem.hpp"
#include "debugger.hpp"

TEST(DebuggerBasic, BreakpointStopsExecution) {
    Mem mem;
    CPU cpu;
    cpu.Reset(mem);

    // Program: LDX #$01; DEX; BNE -2 (loop) at 0x8000
    mem[Mem::RESET_VECTOR] = 0x00;
    mem[Mem::RESET_VECTOR + 1] = 0x80;
    mem[0x8000] = 0xA2; // LDX #imm
    mem[0x8001] = 0x03; // value
    mem[0x8002] = 0xCA; // DEX
    mem[0x8003] = 0xD0; // BNE
    mem[0x8004] = 0xFD; // -3 to 0x8001

    Debugger dbg;
    dbg.attach(&cpu, &mem);
    cpu.setDebugger(&dbg);

    dbg.addBreakpoint(0x8003); // at BNE opcode

    cpu.Execute(20, mem);

    ASSERT_TRUE(dbg.hitBreakpoint());
    ASSERT_EQ(dbg.lastBreakpoint(), 0x8003);
}

TEST(DebuggerBasic, WatchpointTriggersOnWrite) {
    Mem mem;
    CPU cpu;
    cpu.Reset(mem);

    // Program: LDA #$10; STA $00
    mem[Mem::RESET_VECTOR] = 0x00;
    mem[Mem::RESET_VECTOR + 1] = 0x80;
    mem[0x8000] = 0xA9; // LDA #imm
    mem[0x8001] = 0x10;
    mem[0x8002] = 0x85; // STA zp
    mem[0x8003] = 0x00; // addr

    Debugger dbg;
    dbg.attach(&cpu, &mem);
    cpu.setDebugger(&dbg);

    dbg.addWatchpoint(0x00);

    cpu.Execute(10, mem);

    ASSERT_TRUE(dbg.hitBreakpoint());
    ASSERT_EQ(dbg.lastBreakpoint(), 0x00);
}

TEST(DebuggerBasic, InspectCPUState) {
    Mem mem;
    CPU cpu;
    cpu.Reset(mem);
    cpu.A = 0x42;
    cpu.X = 0x10;
    cpu.Y = 0x20;

    Debugger dbg;
    dbg.attach(&cpu, &mem);

    auto st = dbg.inspectCPU();
    ASSERT_EQ(st.a, 0x42);
    ASSERT_EQ(st.x, 0x10);
    ASSERT_EQ(st.y, 0x20);
}
