#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>

class CPU;
class Mem;

class Debugger {
public:
    struct MemoryEvent {
        uint16_t address{0};
        uint8_t value{0};
        bool isWrite{false};
    };

    struct TraceEvent {
        uint16_t address{0};
        uint8_t opcode{0};
    };

    struct CpuState {
        uint16_t pc{0};
        uint8_t sp{0};
        uint8_t a{0};
        uint8_t x{0};
        uint8_t y{0};
        uint8_t c{0};
        uint8_t z{0};
        uint8_t i{0};
        uint8_t d{0};
        uint8_t b{0};
        uint8_t v{0};
        uint8_t n{0};
    };

    Debugger();

    void attach(CPU* cpu, Mem* mem);

    void addBreakpoint(uint16_t address);
    void removeBreakpoint(uint16_t address);
    bool hasBreakpoint(uint16_t address) const;
    void clearBreakpoints();

    void addWatchpoint(uint16_t address);
    void removeWatchpoint(uint16_t address);
    bool hasWatchpoint(uint16_t address) const;
    void clearWatchpoints();

    bool shouldBreak(uint16_t pc) const;
    void notifyBreakpoint(uint16_t pc);

    void traceInstruction(uint16_t pc, uint8_t opcode);
    void notifyMemoryAccess(uint16_t address, uint8_t value, bool isWrite);

    const std::vector<MemoryEvent>& memoryEvents() const;
    const std::vector<TraceEvent>& traceEvents() const;

    uint16_t lastBreakpoint() const;
    bool hitBreakpoint() const;

    CpuState inspectCPU() const;
    uint8_t readMemory(uint16_t address) const;
    void writeMemory(uint16_t address, uint8_t value);

private:
    CPU* cpu_;
    Mem* mem_;
    std::unordered_set<uint16_t> breakpoints_;
    std::unordered_set<uint16_t> watchpoints_;
    std::vector<MemoryEvent> memoryEvents_;
    std::vector<TraceEvent> traceEvents_;
    uint16_t lastBreakpoint_;
    bool hitBreakpoint_;
};
