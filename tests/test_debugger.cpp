#include <gtest/gtest.h>
#include "cpu.hpp"
#include "cpu/system_map.hpp"
#include "debugger.hpp"

TEST(DebuggerBasic, BreakpointStopsExecution) {
    SystemMap bus;
    CPU cpu;
    cpu.PC = 0x8000;
    cpu.SP = 0xFD;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;

    // Program: LDX #$01; DEX; BNE -2 (loop) at 0x8000
    bus.write(0x8000, 0xA2); // LDX #imm
    bus.write(0x8001, 0x03); // value
    bus.write(0x8002, 0xCA); // DEX
    bus.write(0x8003, 0xD0); // BNE
    bus.write(0x8004, 0xFD); // -3 to 0x8001

    Debugger dbg;
    dbg.attach(&cpu, &bus);
    cpu.setDebugger(&dbg);

    dbg.addBreakpoint(0x8003); // at BNE opcode

    cpu.Execute(20, bus);

    ASSERT_TRUE(dbg.hitBreakpoint());
    ASSERT_EQ(dbg.lastBreakpoint(), 0x8003);
}

TEST(DebuggerBasic, WatchpointTriggersOnWrite) {
    SystemMap bus;
    CPU cpu;
    cpu.PC = 0x8000;
    cpu.SP = 0xFD;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;

    // Program: LDA #$10; STA $00
    bus.write(0x8000, 0xA9); // LDA #imm
    bus.write(0x8001, 0x10);
    bus.write(0x8002, 0x85); // STA zp
    bus.write(0x8003, 0x00); // addr

    Debugger dbg;
    dbg.attach(&cpu, &bus);
    cpu.setDebugger(&dbg);

    dbg.addWatchpoint(0x00);

    cpu.Execute(10, bus);

    ASSERT_TRUE(dbg.hitBreakpoint());
    ASSERT_EQ(dbg.lastBreakpoint(), 0x00);
}

TEST(DebuggerBasic, InspectCPUState) {
    SystemMap bus;
    CPU cpu;
    cpu.PC = 0x8000;
    cpu.SP = 0xFD;
    cpu.A = 0x42;
    cpu.X = 0x10;
    cpu.Y = 0x20;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;

    Debugger dbg;
    dbg.attach(&cpu, &bus);

    auto st = dbg.inspectCPU();
    ASSERT_EQ(st.a, 0x42);
    ASSERT_EQ(st.x, 0x10);
    ASSERT_EQ(st.y, 0x20);
}
