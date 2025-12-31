#include "cpu_addressing.hpp"
#include "cpu.hpp"

namespace Addressing {

Word Immediate(CPU& cpu, u32& cycles, SystemMap& bus) {
    Word address = cpu.PC;
    cpu.PC++;
    return address;
}

Word ZeroPage(CPU& cpu, u32& cycles, SystemMap& bus) {
    Byte zpAddress = cpu.FetchByte(cycles, bus);
    return zpAddress;
}

Word ZeroPageX(CPU& cpu, u32& cycles, SystemMap& bus) {
    Byte zpAddress = cpu.FetchByte(cycles, bus);
    zpAddress += cpu.X;
    cycles--; // Additional cycle for adding X
    return zpAddress; // Wraps around in zero page (0x00FF + 1 = 0x0000)
}

Word ZeroPageY(CPU& cpu, u32& cycles, SystemMap& bus) {
    Byte zpAddress = cpu.FetchByte(cycles, bus);
    zpAddress += cpu.Y;
    cycles--; // Additional cycle for adding Y
    return zpAddress; // Wraps around in zero page
}

Word Absolute(CPU& cpu, u32& cycles, SystemMap& bus) {
    Word address = cpu.FetchWord(cycles, bus);
    return address;
}

Word AbsoluteX(CPU& cpu, u32& cycles, SystemMap& bus, bool pageCrossPenalty) {
    Word address = cpu.FetchWord(cycles, bus);
    Word effectiveAddress = address + cpu.X;
    
    if (pageCrossPenalty && PagesCross(address, effectiveAddress)) {
        cycles--; // Page boundary crossed
    }
    
    return effectiveAddress;
}

Word AbsoluteY(CPU& cpu, u32& cycles, SystemMap& bus, bool pageCrossPenalty) {
    Word address = cpu.FetchWord(cycles, bus);
    Word effectiveAddress = address + cpu.Y;
    
    if (pageCrossPenalty && PagesCross(address, effectiveAddress)) {
        cycles--; // Page boundary crossed
    }
    
    return effectiveAddress;
}

Word IndirectX(CPU& cpu, u32& cycles, SystemMap& bus) {
    Byte zpAddress = cpu.FetchByte(cycles, bus);
    zpAddress += cpu.X;
    cycles--; // Additional cycle for adding X
    
    // Read address from zero page
    Byte lowByte = bus.read(zpAddress);
    Byte highByte = bus.read(static_cast<Byte>(zpAddress + 1)); // Wraps in zero page
    cycles -= 2;
    
    return (highByte << 8) | lowByte;
}

Word IndirectY(CPU& cpu, u32& cycles, SystemMap& bus, bool pageCrossPenalty) {
    Byte zpAddress = cpu.FetchByte(cycles, bus);
    
    // Read address from zero page
    Byte lowByte = bus.read(zpAddress);
    Byte highByte = bus.read(static_cast<Byte>(zpAddress + 1)); // Wraps in zero page
    cycles -= 2;
    
    Word address = (highByte << 8) | lowByte;
    Word effectiveAddress = address + cpu.Y;
    
    if (pageCrossPenalty && PagesCross(address, effectiveAddress)) {
        cycles--; // Page boundary crossed
    }
    
    return effectiveAddress;
}

Word Indirect(CPU& cpu, u32& cycles, SystemMap& bus) {
    Word indirectAddress = cpu.FetchWord(cycles, bus);
    
    // Read the actual address from the indirect address
    // Note: 6502 bug - if low byte is 0xFF, high byte is read from xx00 instead of (xx+1)00
    Byte lowByte = bus.read(indirectAddress);
    Byte highByte;
    
    if ((indirectAddress & 0x00FF) == 0xFF) {
        // Simulate the 6502 page boundary bug
        highByte = bus.read(indirectAddress & 0xFF00);
    } else {
        highByte = bus.read(indirectAddress + 1);
    }
    
    cycles -= 2;
    return (highByte << 8) | lowByte;
}

bool PagesCross(Word addr1, Word addr2) {
    return (addr1 & 0xFF00) != (addr2 & 0xFF00);
}

} // namespace Addressing
