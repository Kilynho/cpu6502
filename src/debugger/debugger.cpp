#include "debugger.hpp"
#include "cpu.hpp"
#include "system_map.hpp"

Debugger::Debugger()
    : cpu_(nullptr), bus_(nullptr), lastBreakpoint_(0), hitBreakpoint_(false) {}

void Debugger::attach(CPU* cpu, SystemMap* bus) {
    cpu_ = cpu;
    bus_ = bus;
}

void Debugger::addBreakpoint(uint16_t address) {
    breakpoints_.insert(address);
}

void Debugger::removeBreakpoint(uint16_t address) {
    breakpoints_.erase(address);
}

bool Debugger::hasBreakpoint(uint16_t address) const {
    return breakpoints_.find(address) != breakpoints_.end();
}

void Debugger::clearBreakpoints() {
    breakpoints_.clear();
}

void Debugger::addWatchpoint(uint16_t address) {
    watchpoints_.insert(address);
}

void Debugger::removeWatchpoint(uint16_t address) {
    watchpoints_.erase(address);
}

bool Debugger::hasWatchpoint(uint16_t address) const {
    return watchpoints_.find(address) != watchpoints_.end();
}

void Debugger::clearWatchpoints() {
    watchpoints_.clear();
}

bool Debugger::shouldBreak(uint16_t pc) const {
    return hasBreakpoint(pc);
}

void Debugger::notifyBreakpoint(uint16_t pc) {
    hitBreakpoint_ = true;
    lastBreakpoint_ = pc;
}

void Debugger::traceInstruction(uint16_t pc, uint8_t opcode) {
    traceEvents_.push_back({pc, opcode});
}

void Debugger::notifyMemoryAccess(uint16_t address, uint8_t value, bool isWrite) {
    memoryEvents_.push_back({address, value, isWrite});
    if (hasWatchpoint(address)) {
        hitBreakpoint_ = true;
        lastBreakpoint_ = address;
    }
}

const std::vector<Debugger::MemoryEvent>& Debugger::memoryEvents() const {
    return memoryEvents_;
}

const std::vector<Debugger::TraceEvent>& Debugger::traceEvents() const {
    return traceEvents_;
}

uint16_t Debugger::lastBreakpoint() const {
    return lastBreakpoint_;
}

bool Debugger::hitBreakpoint() const {
    return hitBreakpoint_;
}

Debugger::CpuState Debugger::inspectCPU() const {
    CpuState state{};
    if (!cpu_) {
        return state;
    }
    state.pc = cpu_->PC;
    state.sp = cpu_->SP;
    state.a = cpu_->A;
    state.x = cpu_->X;
    state.y = cpu_->Y;
    state.c = cpu_->C;
    state.z = cpu_->Z;
    state.i = cpu_->I;
    state.d = cpu_->D;
    state.b = cpu_->B;
    state.v = cpu_->V;
    state.n = cpu_->N;
    return state;
}

uint8_t Debugger::readMemory(uint16_t address) const {
    if (!bus_) {
        return 0;
    }
    return bus_->read(address);
}

void Debugger::writeMemory(uint16_t address, uint8_t value) {
    if (!bus_) {
        return;
    }
    bus_->write(address, value);
}
