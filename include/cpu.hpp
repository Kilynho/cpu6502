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
#include "cpu/system_map.hpp"
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
    void Execute(u32 Cycles, SystemMap& bus); // Ejecuta instrucciones usando el bus central
    void PrintCPUState() const;
    // Métodos migrados a SystemMap& bus
    Word PopWordFromStack(u32& cycles, SystemMap& bus); // Recupera el contador de programa de la pila (migrado)
    Byte FetchByte(u32& Cycles, SystemMap& bus); // Obtiene un byte de la memoria (migrado)
    Word FetchWord(u32& Cycles, SystemMap& bus); // Obtiene una palabra de la memoria (migrado)
    Byte ReadByte(u32& Cycles, Byte Address, SystemMap& bus); // Lee un byte de la memoria (migrado)
    Word ReadWord(u32& Cycles, Word Address, SystemMap& bus); // Lee una palabra de la memoria (migrado)
    void WriteByte(u32& Cycles, Byte Address, Byte Value, SystemMap& bus); // Escribe un byte en la memoria (migrado)
    void WriteWord(u32& Cycles, Word Address, Word Value, SystemMap& bus); // Escribe una palabra en la memoria (migrado)
    Byte ReadMemory(Word address, SystemMap& bus); // Lee memoria (migrado)
    void WriteMemory(Word address, Byte value, SystemMap& bus); // Escribe memoria (migrado)
    // Métodos aún no migrados
    // TODO: void PushPCToStack(u32& cycles, SystemMap& bus);
    // TODO: void PullPCFromStack(u32& cycles, SystemMap& bus);
    void LDASetStatus(); // Sets the status for the LDA instruction
    void LDXSetStatus(); // Sets the status for the LDX instruction
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
    
    mutable std::ofstream logFile; // CPU log file (rotating)
    mutable size_t logFileSize = 0; // Current log file size in bytes
    mutable int logFileIndex = 0;   // Current log file index (0-4)
    static constexpr size_t LOG_FILE_MAX_SIZE = 100 * 1024 * 1024; // 100MB
    static constexpr int LOG_FILE_COUNT = 5;
    mutable std::string logFileName = "cpu_log.txt";
    void rotateLogFile() const;
    bool instructionDumpEnabled = false; // guard for INSTR_DUMP logging

    CPU();  // CPU constructor
    // --- IODevice integration ---
    void registerIODevice(std::shared_ptr<IODevice> device);
    void unregisterIODevice(std::shared_ptr<IODevice> device);
    
    // --- Interrupt Controller integration ---
    void setInterruptController(InterruptController* controller);
    InterruptController* getInterruptController() const;
    void checkAndHandleInterrupts(SystemMap& bus);

    // --- Debugger integration ---
    void setDebugger(Debugger* debuggerInstance);
    Debugger* getDebugger() const;
    
    ~CPU(); // CPU destructor
    void LogMemoryAccess(Word address, Byte data, bool isWrite) const;
    void setInstructionDumpEnabled(bool enabled);
    bool isInstructionDumpEnabled() const;
    
private:
    std::vector<std::shared_ptr<IODevice>> ioDevices; // Registered I/O devices
    InterruptController* interruptController; // Interrupt controller (not owned)
    Debugger* debugger; // Attached debugger (not owned)
    SystemMap* attachedBus = nullptr; // Last bus used to sync IO devices (not owned)

    // Auxiliary methods for IO
    IODevice* findIODeviceForRead(uint16_t address) const;
    IODevice* findIODeviceForWrite(uint16_t address) const;
    void syncIODeviceRegistrations(SystemMap& bus);
    void AssignCyclesAndBytes(Word &pc, u32 &cycles, Byte opcode) const;
};

#endif // CPU_HPP