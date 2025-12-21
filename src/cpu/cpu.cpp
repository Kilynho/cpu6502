#include "io_device.hpp"
#include <algorithm>
#include "cpu.hpp"
#include "mem.hpp"
#include "util/logger.hpp"
#include "debugger.hpp"
#include "cpu_instructions.hpp"
#include <bitset>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <sys/stat.h>
#include <cstdio>
#include <string>

// Definition of instructions with their opcodes, cycles, bytes, and names
const Instruction CPU::INS_LDA_IM = {0xA9, 2, 2, "LDA_IM"}; // LDA Immediate
const Instruction CPU::INS_LDA_ZP = {0xA5, 3, 2, "LDA_ZP"}; // LDA Zero Page
const Instruction CPU::INS_LDA_ZPX = {0xB5, 4, 2, "LDA_ZPX"}; // LDA Zero Page,X
const Instruction CPU::INS_LDX_IM = {0xA2, 2, 2, "LDX_IM"}; // LDX Immediate
const Instruction CPU::INS_STA_ZP = {0x85, 3, 2, "STA_ZP"}; // STA Zero Page
const Instruction CPU::INS_JSR = {0x20, 6, 3, "JSR"};   // JSR (Jump to Subroutine)
const Instruction CPU::INS_RTS = {0x60, 6, 1, "RTS"};   // RTS (Return from Subroutine)
const Instruction CPU::INS_LDA_ABS = {0xAD, 4, 3, "LDA_ABS"}; // LDA Absolute
const Instruction CPU::INS_LDA_ABSX = {0xBD, 4, 3, "LDA_ABSX"}; // LDA Absolute,X
const Instruction CPU::INS_LDA_ABSY = {0xB9, 4, 3, "LDA_ABSY"}; // LDA Absolute,Y

u32 CPU::CalculateCycles(const Mem& mem) const {
    u32 cycles = 0;
    Word pc = Mem::ROM_START; // Start of the program in ROM memory

    while (true) { // Infinite loop
        Byte opcode = mem[pc];
        AssignCyclesAndBytes(pc, cycles, opcode); // Assign cycles and bytes according to the opcode
        // Stop if the end of memory is reached
        if (pc == Mem::ROM_END) { break; }
    }
    return cycles;
}

void CPU::AssignCyclesAndBytes(Word &pc, u32 &cycles, Byte opcode) const {
    const Instruction* instruction = nullptr;

    // Assign the corresponding instruction according to the opcode
    if (opcode == INS_JSR.opcode) {
        instruction = &INS_JSR;
    } else if (opcode == INS_LDA_IM.opcode) {
        instruction = &INS_LDA_IM;
    } else if (opcode == INS_LDA_ZP.opcode) {
        instruction = &INS_LDA_ZP;
    } else if (opcode == INS_LDA_ZPX.opcode) {
        instruction = &INS_LDA_ZPX;
    } else if (opcode == INS_LDX_IM.opcode) {
        instruction = &INS_LDX_IM;
    } else if (opcode == INS_STA_ZP.opcode) {
        instruction = &INS_STA_ZP;
    } else if (opcode == INS_RTS.opcode) {
        instruction = &INS_RTS;
    } else if (opcode == INS_LDA_ABS.opcode) {
        instruction = &INS_LDA_ABS;
    } else if (opcode == INS_LDA_ABSX.opcode) {
        instruction = &INS_LDA_ABSX;
    } else if (opcode == INS_LDA_ABSY.opcode) {
        instruction = &INS_LDA_ABSY;
    }

    // If a valid instruction was found, assign cycles and bytes
    if (instruction) {
        cycles += instruction->cycles;
        pc += instruction->bytes;
    } else {
        pc++;
    }
}

Byte CPU::FetchByte(u32& Cycles, Mem& memory) {
    Byte Data = memory[PC]; // Get the byte from memory at the program counter address
    if (debugger) debugger->notifyMemoryAccess(PC, Data, false);
    LogMemoryAccess(PC, Data, false); // Log the memory read access
    PC++; // Increment the program counter
    Cycles--; // Decrement remaining cycles
    return Data; // Return the obtained byte
}

Word CPU::FetchWord(u32& Cycles, Mem& memory) {
    Word Data = memory[PC]; // Get the low byte of the word
    if (debugger) debugger->notifyMemoryAccess(PC, Data, false);
    LogMemoryAccess(PC, Data, false); // Log the memory read access
    PC++; // Increment the program counter
    Data |= (memory[PC] << 8); // Get the high byte of the word and combine it with the low byte
    if (debugger) debugger->notifyMemoryAccess(PC, memory[PC], false);
    LogMemoryAccess(PC, memory[PC], false); // Log the memory read access
    PC++; // Increment the program counter
    Cycles -= 2; // Decrement remaining cycles
    return Data; // Return the obtained word
}

Word CPU::FetchWordFromMemory(const Mem& memory, Word address) const {
    LogMemoryAccess(address, memory[address], false); // Log the memory read access
    LogMemoryAccess(address + 1, memory[address + 1], false); // Log the memory read access
    // Definition of instructions with their opcodes, cycles, bytes, and names
    return (memory[address] | (memory[address + 1] << 8));
} 


Byte CPU::ReadByte(u32& Cycles, Byte Address, Mem& memory) {
    // Check IODevices first
    if (IODevice* io = findIODeviceForRead(Address)) {
        Byte Data = io->read(Address);
        if (debugger) debugger->notifyMemoryAccess(Address, Data, false);
        LogMemoryAccess(Address, Data, false);
        Cycles--;
        return Data;
    }
    Byte Data = memory[Address];
    if (debugger) debugger->notifyMemoryAccess(Address, Data, false);
    LogMemoryAccess(Address, Data, false);
    Cycles--;
    return Data;
}

Word CPU::ReadWord(u32& Cycles, Word Address, Mem& memory) {
    Word Data = memory[Address]; // Read the low byte of the word
    if (debugger) debugger->notifyMemoryAccess(Address, Data, false);
    LogMemoryAccess(Address, Data, false); // Log the memory read access
    Address++; // Increment the address
    Data |= (memory[Address] << 8); // Read the high byte of the word and combine it with the low byte
    if (debugger) debugger->notifyMemoryAccess(Address, memory[Address], false);
    LogMemoryAccess(Address, memory[Address], false); // Log the memory read access
    Cycles--; // Decrement remaining cycles
    return Data; // Return the read word
}


void CPU::WriteByte(u32& Cycles, Byte Address, Byte Data, Mem& memory) {
    // Check IODevices first
    if (IODevice* io = findIODeviceForWrite(Address)) {
        io->write(Address, Data);
        if (debugger) debugger->notifyMemoryAccess(Address, Data, true);
        LogMemoryAccess(Address, Data, true);
        Cycles--;
        return;
    }
    memory[Address] = Data;
    if (debugger) debugger->notifyMemoryAccess(Address, Data, true);
    LogMemoryAccess(Address, Data, true);
    Cycles--;
}
// --- IODevice integration methods ---
void CPU::registerIODevice(std::shared_ptr<IODevice> device) {
    ioDevices.push_back(device);
}

void CPU::unregisterIODevice(std::shared_ptr<IODevice> device) {
    ioDevices.erase(std::remove(ioDevices.begin(), ioDevices.end(), device), ioDevices.end());
}

IODevice* CPU::findIODeviceForRead(uint16_t address) const {
    for (const auto& dev : ioDevices) {
        if (dev && dev->handlesRead(address)) return dev.get();
    }
    return nullptr;
}

IODevice* CPU::findIODeviceForWrite(uint16_t address) const {
    for (const auto& dev : ioDevices) {
        if (dev && dev->handlesWrite(address)) return dev.get();
    }
    return nullptr;
}

// Memory access methods with IODevice support
Byte CPU::ReadMemory(Word address, Mem& memory) {
    if (IODevice* io = findIODeviceForRead(address)) {
        return io->read(address);
    }
    if (debugger) debugger->notifyMemoryAccess(address, memory[address], false);
    return memory[address];
}

void CPU::WriteMemory(Word address, Byte value, Mem& memory) {
    if (IODevice* io = findIODeviceForWrite(address)) {
        io->write(address, value);
        return;
    }
    memory[address] = value;
    if (debugger) debugger->notifyMemoryAccess(address, value, true);
}

void CPU::WriteWord(u32& Cycles, Word Address, Word Data, Mem& memory) {
    memory[Address] = Data & 0x00FF; // Write the low byte of the word to memory
    if (debugger) debugger->notifyMemoryAccess(Address, Data & 0x00FF, true);
    LogMemoryAccess(Address, Data & 0x00FF, true); // Log the memory write access
    Cycles--; // Decrement remaining cycles
    memory[Address + 1] = (Data & 0xFF00) >> 8; // Write the high byte of the word to memory
    if (debugger) debugger->notifyMemoryAccess(Address + 1, (Data & 0xFF00) >> 8, true);
    LogMemoryAccess(Address + 1, (Data & 0xFF00) >> 8, true); // Log the memory write access
    Cycles--; // Decrement remaining cycles
}

Word CPU::SPToAddress() const {
    return 0x0100 + SP; // Return the stack address
}

void CPU::PushPCToStack(u32& Cycles, Mem& memory) {
    Word returnAddr = PC - 1;
    // Push high byte first
    memory[SPToAddress()] = returnAddr >> 8;
    LogMemoryAccess(SPToAddress(), returnAddr >> 8, true);
    Cycles--;
    SP--;
    // Push low byte
    memory[SPToAddress()] = returnAddr & 0xFF;
    LogMemoryAccess(SPToAddress(), returnAddr & 0xFF, true);
    Cycles--;
    SP--;
}

void CPU::PullPCFromStack(u32 &cycles, Mem &memory)
{
    PC = PopWordFromStack(cycles, memory); // Restore the program counter from the stack
    PC++; // Increment the program counter
}

Word CPU::PopWordFromStack(u32& Cycles, Mem& memory) {
    Word ValueFromStack = ReadWord(Cycles, SPToAddress(), memory); // Restore the program counter from the stack
    SP += 2; // Increment the stack pointer
    return ValueFromStack; // Return the restored program counter
}

void CPU::LDASetStatus() {
    Z = (A == 0); // Set the zero flag if the accumulator is zero
    N = (A & 0b10000000) > 0; // Set the negative flag if the most significant bit of the accumulator is 1
}

void CPU::LDXSetStatus() {
    Z = (X == 0); // Set the zero flag if the X register is zero
    N = (X & 0b10000000) > 0; // Set the negative flag if the most significant bit of the X register is 1
}

void CPU::LDYSetStatus() {
    Z = (Y == 0); // Set the zero flag if the Y register is zero
    N = (Y & 0b10000000) > 0; // Set the negative flag if the most significant bit of the Y register is 1
}

void CPU::UpdateZeroAndNegativeFlags(Byte value) {
    Z = (value == 0);
    N = (value & 0x80) != 0;
}

void CPU::UpdateCarryFlag(bool carry) {
    C = carry ? 1 : 0;
}

void CPU::UpdateOverflowFlag(bool overflow) {
    V = overflow ? 1 : 0;
}

void CPU::Reset(Mem& memory) {
    // Clear the log file
    std::ofstream logFile("cpu_log.txt", std::ios_base::trunc);
    logFile.close();
    // memory.Initialize(); // Removed: Initialize memory should be done separately
    // memory.Data[Mem::RESET_VECTOR] = 0x00; // Set the low byte of the reset vector address
    // memory.Data[Mem::RESET_VECTOR + 1] = 0x80; // Set the high byte of the reset vector address
    memory.Data[Mem::STACK_END] = 0xff; // Set the low byte of the stack end address
    memory.Data[Mem::STACK_END + 1] = 0x00; // Set the high byte of the stack end address
    PC = FetchWordFromMemory(memory, Mem::RESET_VECTOR); // Start the program counter at the reset vector address (little-endian)
    SP = FetchWordFromMemory(memory, Mem::STACK_END); // Start the stack pointer at the stack end address (little-endian)
    A = X = Y = 0;
    C = Z = I = D = B = V = N = 0;
}

CPU::CPU() : PC(0), SP(0), A(0), X(0), Y(0), C(0), Z(0), I(0), D(0), B(0), V(0), N(0), interruptController(nullptr), debugger(nullptr) {
    logFile.open("cpu_log.txt", std::ios_base::app);
    
    // Initialize the instruction table on first CPU construction
    static bool instructionTableInitialized = false;
    if (!instructionTableInitialized) {
        Instructions::InitializeInstructionTable();
        instructionTableInitialized = true;
    }
}

CPU::~CPU() {
    if (logFile.is_open()) {
        logFile.close();
    }
}


void CPU::LogMemoryAccess(Word address, Byte data, bool isWrite) const {
    // Only log if CPU_LOG_LEVEL=DEBUG (default: INFO)
    const char* logLevel = std::getenv("CPU_LOG_LEVEL");
    if (!logLevel || std::string(logLevel) != "DEBUG") return;
    RotateLogIfNeeded();
    std::ostringstream oss;
    oss << std::bitset<16>(address) << "  "
        << std::bitset<8>(data) << "  "
        << std::hex << std::setw(4) << std::setfill('0') << address << "  "
        << (isWrite ? "W" : "r") << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data) << "  "
        << std::hex << std::setw(4) << std::setfill('0') << PC << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(SP) << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(A) << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(X) << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(Y) << "  "
        << std::dec << static_cast<int>(C) << " "
        << static_cast<int>(Z) << " "
        << static_cast<int>(I) << " "
        << static_cast<int>(D) << " "
        << static_cast<int>(B) << " "
        << static_cast<int>(V) << " "
        << static_cast<int>(N);
    std::ofstream logFile("cpu_log.txt", std::ios_base::app);
    logFile << oss.str() << "\n";
}

void CPU::LogInstruction(Word pc, Byte opcode) const {
    // Only log if CPU_LOG_LEVEL!=DEBUG (default: INFO)
    const char* logLevel = std::getenv("CPU_LOG_LEVEL");
    if (logLevel && std::string(logLevel) == "DEBUG") return;
    RotateLogIfNeeded();
    std::ostringstream oss;
    oss << std::hex << std::setw(4) << std::setfill('0') << pc << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode) << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(A) << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(X) << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(Y) << "  "
        << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(SP) << "  "
        << std::dec << static_cast<int>(C) << " "
        << static_cast<int>(Z) << " "
        << static_cast<int>(I) << " "
        << static_cast<int>(D) << " "
        << static_cast<int>(B) << " "
        << static_cast<int>(V) << " "
        << static_cast<int>(N);
    std::ofstream logFile("cpu_log.txt", std::ios_base::app);
    logFile << oss.str() << "\n";
}

void CPU::Execute(u32 Cycles, Mem& memory) {
    u32 instructionCount = 0;
    const u32 MAX_INSTRUCTIONS = 100000; // Safety limit
    const char* disableGuardEnv = std::getenv("CPU_DISABLE_GUARD");
    const bool guardEnabled = (disableGuardEnv == nullptr || disableGuardEnv[0] == '\0');
    
    // Debug logging
    const char* debugExecuteEnv = std::getenv("CPU_DEBUG_EXECUTE");
    const bool debugEnabled = (debugExecuteEnv != nullptr && debugExecuteEnv[0] != '\0');

    while (Cycles > 0) {
        if (guardEnabled && ++instructionCount > MAX_INSTRUCTIONS) {
            std::stringstream ss;
            ss << "Execution limit reached (" << MAX_INSTRUCTIONS << " instructions) at PC=0x" 
               << std::hex << std::setw(4) << std::setfill('0') << PC;
            util::LogWarn(ss.str());
            return;
        }
        
        if (debugEnabled && instructionCount % 1000 == 0) {
            std::cerr << "DEBUG: Execute loop iteration " << instructionCount 
                      << ", Cycles=" << Cycles << ", PC=0x" << std::hex << PC << std::dec << std::endl;
        }
        
        Word currentPC = PC;
        
        // Check for debugger breakpoints
        if (debugger && debugger->shouldBreak(currentPC)) {
            debugger->notifyBreakpoint(currentPC);
            return;
        }
        
        // Fetch the opcode
        Byte opcode = FetchByte(Cycles, memory);

        // Log instruction (INFO mode)
        LogInstruction(currentPC, opcode);

        // Trace instruction for debugger
        if (debugger) debugger->traceInstruction(currentPC, opcode);

        // Get the instruction handler from the table
        InstrHandler handler = Instructions::GetHandler(opcode);

        // Execute the instruction
        if (handler) {
            handler(*this, Cycles, memory);
        } else {
            // This shouldn't happen if the table is properly initialized
            std::stringstream ss;
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode);
            ss << " at PC=" << std::hex << std::setw(4) << std::setfill('0') << currentPC;
            util::LogWarn("Unhandled opcode: 0x" + ss.str());
            Cycles = 0; // Stop execution on unhandled opcode
            return;
        }
    }
}

// Execute exactly one instruction (for tracing and debugging)
void CPU::ExecuteSingleInstruction(Mem& memory) {
    Word currentPC = PC;
    
    // Check for debugger breakpoints
    if (debugger && debugger->shouldBreak(currentPC)) {
        debugger->notifyBreakpoint(currentPC);
        return;
    }
    
    // Fetch the opcode
    u32 cycles = 10; // Give enough cycles for any instruction (max is 7)
    Byte opcode = FetchByte(cycles, memory);
    
    // Trace instruction for debugger
    if (debugger) debugger->traceInstruction(currentPC, opcode);
    
    // Get the instruction handler from the table
    InstrHandler handler = Instructions::GetHandler(opcode);
    
    // Execute the instruction
    if (handler) {
        handler(*this, cycles, memory);
    } else {
        // This shouldn't happen if the table is properly initialized
        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode);
        ss << " at PC=" << std::hex << std::setw(4) << std::setfill('0') << currentPC;
        util::LogWarn("Unhandled opcode: 0x" + ss.str());
    }
}
// --- Interrupt Controller Integration ---

void CPU::setInterruptController(InterruptController* controller) {
    interruptController = controller;
}

InterruptController* CPU::getInterruptController() const {
    return interruptController;
}

void CPU::setDebugger(Debugger* debuggerInstance) {
    debugger = debuggerInstance;
}

Debugger* CPU::getDebugger() const {
    return debugger;
}

void CPU::serviceIRQ(Mem& memory) {
    // Save PC to the stack (high byte first, then low byte)
    memory[0x0100 + SP] = static_cast<Byte>((PC >> 8) & 0xFF);
    SP--;
    memory[0x0100 + SP] = static_cast<Byte>(PC & 0xFF);
    SP--;
    // Save the status register (P) to the stack
    Byte status = 0;
    status |= (C ? 0x01 : 0);
    status |= (Z ? 0x02 : 0);
    status |= (I ? 0x04 : 0);
    status |= (D ? 0x08 : 0);
    status |= (B ? 0x10 : 0);
    status |= 0x20;  // Bit 5 is always set to 1
    status |= (V ? 0x40 : 0);
    status |= (N ? 0x80 : 0);
    memory[0x0100 + SP] = status;
    SP--;
    // Set the I flag (Interrupt Disable)
    I = 1;
    // Load the IRQ vector into PC
    PC = memory[Mem::IRQ_VECTOR] | (memory[Mem::IRQ_VECTOR + 1] << 8);
}

void CPU::serviceNMI(Mem& memory) {
    // Save PC to the stack (high byte first, then low byte)
    memory[0x0100 + SP] = static_cast<Byte>((PC >> 8) & 0xFF);
    SP--;
    memory[0x0100 + SP] = static_cast<Byte>(PC & 0xFF);
    SP--;
    // Save the status register (P) to the stack
    Byte status = 0;
    status |= (C ? 0x01 : 0);
    status |= (Z ? 0x02 : 0);
    status |= (I ? 0x04 : 0);
    status |= (D ? 0x08 : 0);
    status |= (B ? 0x10 : 0);
    status |= 0x20;  // Bit 5 is always set to 1
    status |= (V ? 0x40 : 0);
    status |= (N ? 0x80 : 0);
    memory[0x0100 + SP] = status;
    SP--;
    // Set the I flag (Interrupt Disable)
    I = 1;
    // Load the NMI vector into PC
    PC = memory[Mem::NMI_VECTOR] | (memory[Mem::NMI_VECTOR + 1] << 8);
}

void CPU::checkAndHandleInterrupts(Mem& memory) {
    if (!interruptController) {
        return;
    }
    
    // NMI has priority over IRQ
    if (interruptController->hasNMI()) {
        serviceNMI(memory);
        interruptController->acknowledgeNMI();
        return;
    }
    
    // IRQ is only handled if the I flag is clear
    if (interruptController->hasIRQ() && !I) {
        serviceIRQ(memory);
        interruptController->acknowledgeIRQ();
    }
}

// Log rotation parameters
constexpr size_t LOGS_MAX_SIZE = 20 * 1024 * 1024; // 20 MB
constexpr int LOGS_MAX_FILES = 5;

// --- Log rotation implementation (must be after all CPU methods) ---
void CPU::RotateLogIfNeeded() const {
    const char* base = "cpu_log.txt";
    struct stat st;
    // Open the file and check its size atomically
    FILE* file = fopen(base, "r");
    if (file) {
        if (fstat(fileno(file), &st) == 0 && static_cast<size_t>(st.st_size) >= LOGS_MAX_SIZE) {
            fclose(file);
            // Remove the oldest log if it exists
            std::string oldest = std::string(base) + "." + std::to_string(LOGS_MAX_FILES - 1);
            std::remove(oldest.c_str());
            // Shift logs: .3->.4, .2->.3, ...
            for (int i = LOGS_MAX_FILES - 2; i >= 1; --i) {
                std::string from = std::string(base) + "." + std::to_string(i);
                std::string to = std::string(base) + "." + std::to_string(i + 1);
                std::rename(from.c_str(), to.c_str());
            }
            // cpu_log.txt -> cpu_log.txt.1
            std::string to = std::string(base) + ".1";
            std::rename(base, to.c_str());
        } else {
            fclose(file);
        }
    }
}
