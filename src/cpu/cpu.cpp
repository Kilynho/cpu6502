#include "cpu.hpp"
#include "io_device.hpp"
#include <algorithm>
#include <memory>
#include "mem.hpp"
#include "logger.hpp"
#include "debugger.hpp"
#include "cpu_instructions.hpp"
#include <bitset>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>

// --- IODevice registration ---
void CPU::registerIODevice(std::shared_ptr<IODevice> device) {
    if (device) {
        // Avoid duplicates
        auto it = std::find(ioDevices.begin(), ioDevices.end(), device);
        if (it == ioDevices.end()) {
            ioDevices.push_back(device);
        }
        // Keep CPU list and last-used bus in sync when possible
        if (attachedBus) {
            attachedBus->registerIODevice(device);
        }
    }
}

void CPU::unregisterIODevice(std::shared_ptr<IODevice> device) {
    if (device) {
        auto it = std::remove(ioDevices.begin(), ioDevices.end(), device);
        if (it != ioDevices.end()) {
            ioDevices.erase(it, ioDevices.end());
        }
        if (attachedBus) {
            attachedBus->unregisterIODevice(device);
        }
    }
}

void CPU::syncIODeviceRegistrations(SystemMap& bus) {
    // Ensure CPU-registered devices are also known to the provided bus
    for (auto& dev : ioDevices) {
        if (dev) {
            bus.registerIODevice(dev);
        }
    }
    attachedBus = &bus;
}

// Helper to subtract cycles without underflowing
static inline void ConsumeCycles(u32 &cycles, u32 amount) {
    if (cycles > amount) cycles -= amount;
    else cycles = 0;
}

// CPU no longer stores opcode Instruction constants here; metadata is provided by Instructions.cpp

void CPU::AssignCyclesAndBytes(Word &pc, u32 &cycles, Byte opcode) const {
    // Query opcode metadata from the Instructions module
    Instructions::OpcodeInfo info = Instructions::GetOpcodeInfo(opcode);
    cycles += info.cycles;
    pc += info.bytes;
}

Byte CPU::FetchByte(u32& Cycles, SystemMap& bus) {
    Byte Data = bus.read(PC);
    if (debugger) debugger->notifyMemoryAccess(PC, Data, false);
    LogMemoryAccess(PC, Data, false);
    PC++;
    ConsumeCycles(Cycles, 1);
    return Data;
}

Word CPU::FetchWord(u32& Cycles, SystemMap& bus) {
    Word Data = bus.read(PC);
    if (debugger) debugger->notifyMemoryAccess(PC, Data, false);
    LogMemoryAccess(PC, Data, false);
    PC++;
    Data |= (bus.read(PC) << 8);
    if (debugger) debugger->notifyMemoryAccess(PC, bus.read(PC), false);
    LogMemoryAccess(PC, bus.read(PC), false);
    PC++;
    ConsumeCycles(Cycles, 2);
    return Data;
}

Byte CPU::ReadByte(u32& Cycles, Byte Address, SystemMap& bus) {
    Byte Data = bus.read(Address);
    if (debugger) debugger->notifyMemoryAccess(Address, Data, false);
    LogMemoryAccess(Address, Data, false);
    ConsumeCycles(Cycles, 1);
    return Data;
}

Word CPU::ReadWord(u32& Cycles, Word Address, SystemMap& bus) {
    Word Data = bus.read(Address);
    if (debugger) debugger->notifyMemoryAccess(Address, Data, false);
    LogMemoryAccess(Address, Data, false);
    Address++;
    Data |= (bus.read(Address) << 8);
    if (debugger) debugger->notifyMemoryAccess(Address, bus.read(Address), false);
    LogMemoryAccess(Address, bus.read(Address), false);
    ConsumeCycles(Cycles, 1);
    return Data;
}

void CPU::WriteByte(u32& Cycles, Byte Address, Byte Data, SystemMap& bus) {
    bus.write(Address, Data);
    if (debugger) debugger->notifyMemoryAccess(Address, Data, true);
    LogMemoryAccess(Address, Data, true);
    ConsumeCycles(Cycles, 1);
}

void CPU::WriteWord(u32& Cycles, Word Address, Word Data, SystemMap& bus) {
    bus.write(Address, Data & 0x00FF);
    if (debugger) debugger->notifyMemoryAccess(Address, Data & 0x00FF, true);
    LogMemoryAccess(Address, Data & 0x00FF, true);
    ConsumeCycles(Cycles, 1);
    bus.write(Address + 1, (Data & 0xFF00) >> 8);
    if (debugger) debugger->notifyMemoryAccess(Address + 1, (Data & 0xFF00) >> 8, true);
    LogMemoryAccess(Address + 1, (Data & 0xFF00) >> 8, true);
    ConsumeCycles(Cycles, 1);
}

Byte CPU::ReadMemory(Word address, SystemMap& bus) {
    Byte Data = bus.read(address);
    if (debugger) debugger->notifyMemoryAccess(address, Data, false);
    return Data;
}

void CPU::WriteMemory(Word address, Byte value, SystemMap& bus) {
    bus.write(address, value);
    if (debugger) debugger->notifyMemoryAccess(address, value, true);
}

Word CPU::PopWordFromStack(u32& Cycles, SystemMap& bus) {
    Word ValueFromStack = ReadWord(Cycles, SPToAddress(), bus);
    SP += 2;
    return ValueFromStack;
}

// TODO: migrar PushPCToStack a SystemMap& bus
// TODO: migrar PullPCFromStack a SystemMap& bus

void CPU::LDASetStatus() {
    Z = (A == 0); // Set the zero flag if the accumulator is zero
    N = (A & 0b10000000) > 0; // Set the negative flag if the most significant bit of the accumulator is 1
}

void CPU::LDXSetStatus() {
    Z = (X == 0); // Set the zero flag if the X register is zero
    N = (X & 0b10000000) > 0; // Set the negative flag if the most significant bit of the X register is 1
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


CPU::CPU() : PC(0), SP(0), A(0), X(0), Y(0), C(0), Z(0), I(0), D(0), B(0), V(0), N(0), interruptController(nullptr), debugger(nullptr) {
    // Calcular ruta del ejecutable y crear carpeta logs al lado
    std::filesystem::path exePath;
    std::error_code ec;
    exePath = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (ec) exePath = std::filesystem::current_path();
    auto exeDir = exePath.has_filename() ? exePath.parent_path() : exePath;
    auto logsDir = exeDir / "logs";
    std::filesystem::create_directories(logsDir, ec);

    // Open the main log file and determine its size (en carpeta logs)
    logFileName = (logsDir / "cpu_log.txt").string();
    logFileIndex = 0;
    logFile.open(logFileName, std::ios_base::app);
    if (logFile.is_open()) {
        logFile.seekp(0, std::ios::end);
        logFileSize = static_cast<size_t>(logFile.tellp());
    } else {
        logFileSize = 0;
    }
    // Ensure the shared instruction table is initialized once
    Instructions::InitializeInstructionTable();
}


CPU::~CPU() {
    if (logFile.is_open()) {
        logFile.close();
    }
}


void CPU::rotateLogFile() const {
    // Close current file if open
    if (logFile.is_open()) {
        logFile.close();
    }
    // Advance index
    logFileIndex = (logFileIndex + 1) % LOG_FILE_COUNT;
    // Mantener la misma carpeta de logs al rotar
    std::filesystem::path baseDir = std::filesystem::path(logFileName).parent_path();
    if (logFileIndex == 0) {
        logFileName = (baseDir / "cpu_memory.log").string();
    } else {
        logFileName = (baseDir / ("cpu_memory." + std::to_string(logFileIndex) + ".log")).string();
    }
    // Open new file (overwrite)
    logFile.open(logFileName, std::ios_base::trunc);
    logFileSize = 0;
}

void CPU::LogMemoryAccess(Word address, Byte data, bool isWrite) const {
    std::ostringstream oss;
    oss << std::bitset<16>(address) << "  "
        << std::bitset<8>(data) << "  "
        << std::hex << std::setw(4) << std::setfill('0') << address << "  "
        << (isWrite ? "W" : "r") << "  "
        << std::hex << std::setw(2) << static_cast<int>(data) << "  ";

    oss << std::hex << std::setw(4) << PC << "  " << std::setw(2) << SP << "  "
        << std::setw(2) << static_cast<int>(A) << " "
        << std::setw(2) << static_cast<int>(X) << " "
        << std::setw(2) << static_cast<int>(Y) << " "
        << static_cast<int>(C) << static_cast<int>(Z)
        << static_cast<int>(I) << static_cast<int>(D)
        << static_cast<int>(B) << static_cast<int>(V)
        << static_cast<int>(N);
    std::string state = oss.str();

    // Check if log file is open, else open
    if (!logFile.is_open()) {
        logFile.open(logFileName, std::ios_base::app);
        if (logFile.is_open()) {
            logFile.seekp(0, std::ios::end);
            logFileSize = static_cast<size_t>(logFile.tellp());
        } else {
            logFileSize = 0;
        }
    }

    // Rotate if needed
    if (logFileSize > LOG_FILE_MAX_SIZE) {
        rotateLogFile();
    }

    logFile << state << "\n";
    logFileSize += state.size() + 1; // +1 for newline
}

void CPU::Execute(u32 Cycles, SystemMap& bus) {
    size_t instrCounter = 0;
    const size_t DUMP_INTERVAL = 10000;
    // Ensure any IO devices registered on the CPU are attached to the active bus
    syncIODeviceRegistrations(bus);
    while (Cycles > 0) {
        Word currentPC = PC;
        if (debugger && debugger->shouldBreak(currentPC)) {
            debugger->notifyBreakpoint(currentPC);
            return;
        }
        Byte Ins = FetchByte(Cycles, bus); // Usar bus
        if (debugger) debugger->traceInstruction(currentPC, Ins);

        // Dispatch via the centralized instruction table
        auto handler = Instructions::GetHandler(Ins);
        if (handler) {
            handler(*this, Cycles, bus); // Pasar bus a los handlers (deben migrarse)
        } else {
            std::ostringstream _oss_unhandled;
            _oss_unhandled << "Instrucción no manejada: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(Ins);
            util::LogWarn(_oss_unhandled.str());
            ConsumeCycles(Cycles, 1);
        }

        // Stop execution if a breakpoint or watchpoint was hit during this instruction
        if (debugger && debugger->hitBreakpoint()) {
            return;
        }

        instrCounter++;
        if (instructionDumpEnabled && (instrCounter % DUMP_INTERVAL) == 0) {
            std::ostringstream oss;
            oss << "INSTR_DUMP count=" << instrCounter
                << " pc=0x" << std::hex << std::setw(4) << std::setfill('0') << currentPC
                << " opcode=0x" << std::hex << std::setw(2) << static_cast<int>(Ins)
                << " A=0x" << std::hex << std::setw(2) << static_cast<int>(A)
                << " X=0x" << std::hex << std::setw(2) << static_cast<int>(X)
                << " Y=0x" << std::hex << std::setw(2) << static_cast<int>(Y)
                << " SP=0x" << std::hex << std::setw(2) << static_cast<int>(SP)
                << " P=" << std::dec << static_cast<int>(C) << static_cast<int>(Z) << static_cast<int>(I)
                << static_cast<int>(D) << static_cast<int>(B) << static_cast<int>(V) << static_cast<int>(N);
            util::LogWarn(oss.str());
        }
    }
}
// TODO: migrar todos los handlers de instrucciones para aceptar SystemMap& bus
// --- Integración del Controlador de Interrupciones ---

void CPU::setInterruptController(InterruptController* controller) {
    interruptController = controller;
}

InterruptController* CPU::getInterruptController() const {
    return interruptController;
}

void CPU::checkAndHandleInterrupts(SystemMap& bus) {
    if (!interruptController) return;

    auto pushStack = [&](Byte value) {
        bus.write(SPToAddress(), value);
        SP--;
    };

    auto packStatus = [&]() -> Byte {
        Byte status = 0;
        status |= (C & 0x1);
        status |= (Z & 0x1) << 1;
        status |= (I & 0x1) << 2;
        status |= (D & 0x1) << 3;
        status |= (B & 0x1) << 4;
        status |= 1 << 5; // Bit 5 always set
        status |= (V & 0x1) << 6;
        status |= (N & 0x1) << 7;
        return status;
    };

    auto serviceInterrupt = [&](uint16_t vectorAddr, bool isNMI) {
        pushStack(static_cast<Byte>((PC >> 8) & 0xFF));
        pushStack(static_cast<Byte>(PC & 0xFF));
        pushStack(packStatus());
        I = 1; // Disable further IRQs
        Byte lo = bus.read(vectorAddr);
        Byte hi = bus.read(vectorAddr + 1);
        PC = static_cast<Word>((hi << 8) | lo);
        if (isNMI) {
            interruptController->acknowledgeNMI();
        } else {
            interruptController->acknowledgeIRQ();
        }
    };

    if (interruptController->hasNMI()) {
        serviceInterrupt(Mem::NMI_VECTOR, true);
        return; // NMI has priority
    }

    if (interruptController->hasIRQ() && !I) {
        serviceInterrupt(Mem::IRQ_VECTOR, false);
    }
}

void CPU::setDebugger(Debugger* debuggerInstance) {
    debugger = debuggerInstance;
}

Debugger* CPU::getDebugger() const {
    return debugger;
}

void CPU::setInstructionDumpEnabled(bool enabled) {
    instructionDumpEnabled = enabled;
}

bool CPU::isInstructionDumpEnabled() const {
    return instructionDumpEnabled;
}

Word CPU::SPToAddress() const {
    return 0x0100 | SP;
}
