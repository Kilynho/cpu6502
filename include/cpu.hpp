#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <cstdio>
#include <bitset>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#include "mem.hpp"
#include "io_device.hpp"
#include "interrupt_controller.hpp"

class Debugger;

// Public API for CPU 6502 Emulator
// This header provides the main interface for using the CPU emulator

// Type definitions for clarity
using Byte = uint8_t;  // A byte (8 bits)
using Word = uint16_t; // A word (16 bits)
using u32 = uint32_t;  // A 32-bit integer

// Structure representing an instruction with its opcode, cycles, bytes, and name
struct Instruction {
    uint8_t opcode;
    uint8_t cycles;
    uint8_t bytes;
    std::string name;
};

// Class representing the system CPU
class CPU {
public:
    // Instruction definitions with their opcodes, cycles, bytes, and names
    static const Instruction INS_LDA_IM; // Instrucción LDA Immediate
    static const Instruction INS_LDA_ZP; // Instrucción LDA Zero Page
    static const Instruction INS_LDA_ZPX; // Instrucción LDA Zero Page,X
    static const Instruction INS_LDX_IM; // Instrucción LDX Immediate
    static const Instruction INS_STA_ZP; // Instrucción STA Zero Page
    static const Instruction INS_JSR;   // Instrucción JSR (Jump to Subroutine)
    static const Instruction INS_RTS;   // Instrucción RTS (Return from Subroutine)
    static const Instruction INS_LDA_ABS; // Instrucción LDA Absolute
    static const Instruction INS_LDA_ABSX; // Instrucción LDA Absolute,X
    static const Instruction INS_LDA_ABSY; // Instrucción LDA Absolute,Y

    // Public methods
    void Reset(Mem& memory); // Resets the CPU and memory
    void Execute(u32 Cycles, Mem& memory); // Executes instructions
    void ExecuteSingleInstruction(Mem& memory); // Executes exactly one instruction
    void PrintCPUState() const; // Prints the CPU state
    u32 CalculateCycles(const Mem& mem) const; // Calculates the cycles needed to run the test program
    Word FetchWordFromMemory(const Mem& memory, Word address) const; // Gets a word from memory
    void LogMemoryAccess(Word address, Byte data, bool isWrite) const; // Logs memory access (DEBUG)
    void LogInstruction(Word pc, Byte opcode) const; // Logs one line per instruction (INFO)
    void RotateLogIfNeeded() const; // Rotates log files if size exceeds limit
    void AssignCyclesAndBytes(Word &pc, u32 &cycles, Byte opcode) const; // Assigns cycles and bytes according to the opcode
    void PushPCToStack(u32& cycles, Mem& memory); // Saves the program counter to the stack
    void PullPCFromStack(u32& cycles, Mem& memory); // Recupera el contador de programa de la pila
    Word PopWordFromStack(u32& cycles, Mem& memory); // Recupera el contador de programa de la pila
    Byte FetchByte(u32& Cycles, Mem& memory); // Obtiene un byte de la memoria
    Word FetchWord(u32& Cycles, Mem& memory); // Obtiene una palabra de la memoria
    Byte ReadByte(u32& Cycles, Byte Address, Mem& memory); // Lee un byte de la memoria
    Word ReadWord(u32& Cycles, Word Address, Mem& memory); // Lee una palabra de la memoria
    void WriteByte(u32& Cycles, Byte Address, Byte Value, Mem& memory); // Escribe un byte en la memoria
    void WriteWord(u32& Cycles, Word Address, Word Value, Mem& memory); // Escribe una palabra en la memoria
    void LDASetStatus(); // Sets the status for the LDA instruction
    void LDXSetStatus(); // Sets the status for the LDX instruction
    void LDYSetStatus(); // Sets the status for the LDY instruction
    std::string ByteToBinaryString(Byte byte) const; // Converts a byte to a binary string
    std::string WordToBinaryString(Word word) const; // Converts a word to a binary string
    Word SPToAddress() const; // Converts the stack pointer to a memory address
    
    // Helper functions for new instruction system (public for instruction handlers)
    void UpdateZeroAndNegativeFlags(Byte value); // Updates the Z and N flags
    void UpdateCarryFlag(bool carry); // Updates the C flag
    void UpdateOverflowFlag(bool overflow); // Updates the V flag
   
    // CPU registers
     Word PC;    // Program Counter
    Byte SP;    // Stack Pointer (Puntero de Pila)
    Byte A, X, Y; // Registros A, X, Y
    Byte C : 1; // Carry Flag
    Byte Z : 1; // Zero Flag
    Byte I : 1; // Interrupt Disable
    Byte D : 1; // Decimal Mode
    Byte B : 1; // Break Command
    Byte V : 1; // Overflow Flag
    Byte N : 1; // Negative Flag
    
    mutable std::ofstream logFile; // CPU log file

    CPU();  // CPU constructor
    // --- IODevice integration ---
    void registerIODevice(std::shared_ptr<IODevice> device);
    void unregisterIODevice(std::shared_ptr<IODevice> device);
    
    // --- Interrupt Controller integration ---
    void setInterruptController(InterruptController* controller);
    InterruptController* getInterruptController() const;

    // --- Debugger integration ---
    void setDebugger(Debugger* debuggerInstance);
    Debugger* getDebugger() const;
    
    // --- Interrupt handling ---
    void serviceIRQ(Mem& memory);
    void serviceNMI(Mem& memory);
    void checkAndHandleInterrupts(Mem& memory);
    
    // Methods for memory access with IODevice support
    Byte ReadMemory(Word address, Mem& memory);
    void WriteMemory(Word address, Byte value, Mem& memory);
    
    ~CPU(); // CPU destructor
    
private:
    std::vector<std::shared_ptr<IODevice>> ioDevices; // Registered I/O devices
    InterruptController* interruptController; // Interrupt controller (not owned)
    Debugger* debugger; // Attached debugger (not owned)

    // Auxiliary methods for IO
    IODevice* findIODeviceForRead(uint16_t address) const;
    IODevice* findIODeviceForWrite(uint16_t address) const;
};

#endif // CPU_HPP