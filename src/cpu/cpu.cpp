#include "io_device.hpp"
#include <algorithm>
#include "cpu.hpp"
#include "mem.hpp"
#include "logger.hpp"
#include "debugger.hpp"
#include <bitset>
#include <fstream>
#include <iomanip>
#include <sstream>

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
    memory.Initialize(); // Initialize memory
    memory.Data[Mem::RESET_VECTOR] = 0x00; // Set the low byte of the reset vector address
    memory.Data[Mem::RESET_VECTOR + 1] = 0x80; // Set the high byte of the reset vector address
    memory.Data[Mem::STACK_END] = 0xff; // Set the low byte of the stack end address
    memory.Data[Mem::STACK_END + 1] = 0x00; // Set the high byte of the stack end address
    PC = FetchWordFromMemory(memory, Mem::RESET_VECTOR); // Start the program counter at the reset vector address (little-endian)
    SP = FetchWordFromMemory(memory, Mem::STACK_END); // Start the stack pointer at the stack end address (little-endian)
    A = X = Y = 0;
    C = Z = I = D = B = V = N = 0;
}

CPU::CPU() : PC(0), SP(0), A(0), X(0), Y(0), C(0), Z(0), I(0), D(0), B(0), V(0), N(0), interruptController(nullptr), debugger(nullptr) {
    logFile.open("cpu_log.txt", std::ios_base::app);
}

CPU::~CPU() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void CPU::LogMemoryAccess(Word address, Byte data, bool isWrite) const { // Loguear el acceso a la memoria
    std::ostringstream oss;  // Crear un flujo de salida de cadena
    oss << std::bitset<16>(address) << "  "     // Escribir la dirección de memoria en binario  
        << std::bitset<8>(data) << "  "     // Escribir el byte de datos en binario
        << std::hex << std::setw(4) << std::setfill('0') << address << "  "     // Escribir la dirección de memoria en hexadecimal    
        << (isWrite ? "W" : "r") << "  "            // Escribir "W" si es una escritura o "r" si es una lectura
        << std::hex << std::setw(2) << static_cast<int>(data) << "  ";      // Escribir el byte de datos en hexadecimal

    std::ofstream logFile("cpu_log.txt", std::ios_base::app); // Abrir el archivo de log
    logFile << oss.str(); // Escribir el acceso a la memoria en el archivo de log

     oss << std::hex << std::setw(4) << PC << "  " << std::setw(2) << SP << "  "    // Escribir el contador de programa y el puntero de pila en hexadecimal   
        << std::setw(2) << static_cast<int>(A) << " "                               // Escribir los registros A, X e Y en decimal         
        << std::setw(2) << static_cast<int>(X) << " "                            // con un ancho de campo de 2 caracteres
        << std::setw(2) << static_cast<int>(Y) << " "                         // y rellenando con ceros a la izquierda
        << static_cast<int>(C) << static_cast<int>(Z)                      // Escribir los flags de estado en decimal
        << static_cast<int>(I) << static_cast<int>(D)           // con un ancho de campo de 1 caracter
        << static_cast<int>(B) << static_cast<int>(V)       // y rellenando con ceros a la izquierda
        << static_cast<int>(N);                  // Escribir el estado de la CPU en el archivo de log
    std::string state = oss.str();  // Convertir el flujo de salida en una cadena
 
    logFile << state << "\n";  // Escribir el estado de la CPU en el archivo de log
} 

void CPU::Execute(u32 Cycles, Mem& memory) {
    while (Cycles > 0) {
        Word currentPC = PC;
        if (debugger && debugger->shouldBreak(currentPC)) {
            debugger->notifyBreakpoint(currentPC);
            return;
        }
        Byte Ins = FetchByte(Cycles, memory); // Obtener el opcode de la instrucción
        if (debugger) debugger->traceInstruction(currentPC, Ins);
        switch (Ins) {
            case 0x00: { // BRK (Force Interrupt)
                // Simula el comportamiento básico de BRK: detener la ejecución
                util::LogInfo("BRK ejecutado: Deteniendo la CPU");
                Cycles = 0;
            } break;
            case 0xA9: { // LDA Immediate
                Byte Value = FetchByte(Cycles, memory); // Obtener el valor inmediato
                A = Value; // Cargar el valor en el acumulador
                LDASetStatus(); // Establecer los flags de estado
            } break;
            case 0xA5: { // LDA Zero Page
                Byte Value = FetchByte(Cycles, memory); // Obtener la dirección de la página cero
                A = ReadByte(Cycles, Value, memory); // Leer el valor de la memoria y cargarlo en el acumulador
                LDASetStatus(); // Establecer los flags de estado
            } break;
            case 0xB5: { // LDA Zero Page,X
                Byte Value = FetchByte(Cycles, memory); // Obtener la dirección de la página cero
                Value += X; // Sumar el valor del registro X
                Cycles--; // Ciclo adicional para el cálculo de la dirección
                A = ReadByte(Cycles, Value, memory); // Leer el valor de la memoria y cargarlo en el acumulador
                LDASetStatus(); // Establecer los flags de estado
            } break;
            case 0xAD: { // LDA Absolute
                Word Address = FetchWord(Cycles, memory); // Obtener la dirección absoluta
                A = ReadMemory(Address, memory); // Leer el valor de la memoria y cargarlo en el acumulador
                LogMemoryAccess(Address, A, false); // Registrar el acceso de lectura a la memoria
                Cycles--; // Decrementar los ciclos restantes
                LDASetStatus(); // Establecer los flags de estado
            } break;
            case 0xBD: { // LDA Absolute,X
                Word Address = FetchWord(Cycles, memory); // Obtener la dirección absoluta
                Address += X; // Sumar el valor del registro X
                A = ReadMemory(Address, memory); // Leer el valor de la memoria y cargarlo en el acumulador
                LogMemoryAccess(Address, A, false); // Registrar el acceso de lectura a la memoria
                Cycles--; // Decrementar los ciclos restantes
                LDASetStatus(); // Establecer los flags de estado
            } break;
            case 0xB9: { // LDA Absolute,Y
                Word Address = FetchWord(Cycles, memory); // Obtener la dirección absoluta
                Address += Y; // Sumar el valor del registro Y
                A = ReadMemory(Address, memory); // Leer el valor de la memoria y cargarlo en el acumulador
                LogMemoryAccess(Address, A, false); // Registrar el acceso de lectura a la memoria
                Cycles--; // Decrementar los ciclos restantes
                LDASetStatus(); // Establecer los flags de estado
            } break;
            case 0x60: { // RTS (Return from Subroutine)
                Cycles--; // Ciclo interno
                SP++; // Incrementar el puntero de pila
                Cycles--; // Ciclo para leer byte bajo
                Word LowByte = memory[0x0100 + SP]; // Leer el byte bajo de la dirección de retorno
                LogMemoryAccess(0x0100 + SP, LowByte, false); // Registrar el acceso de lectura a la memoria
                SP++; // Incrementar el puntero de pila
                Cycles--; // Ciclo para leer byte alto
                Word HighByte = memory[0x0100 + SP]; // Leer el byte alto de la dirección de retorno
                LogMemoryAccess(0x0100 + SP, HighByte, false); // Registrar el acceso de lectura a la memoria
                PC = (HighByte << 8) | LowByte; // Combinar los bytes alto y bajo para obtener la dirección de retorno
                Cycles--; // Ciclo para incrementar PC
                PC++; // Incrementar el contador de programa
                Cycles--; // Ciclo adicional
            } break;
            case 0x85: {  // STA Store Accumulator in Memory (Zero Page)
                Byte Address = FetchByte(Cycles, memory); // Obtener la dirección de memoria
                WriteByte(Cycles, Address, A, memory); // Escribir el valor del acumulador en la memoria
            } break;
            case 0x8D: { // STA Absolute
                Word Address = FetchWord(Cycles, memory); // Obtener la dirección absoluta
                WriteMemory(Address, A, memory); // Escribir el acumulador en la memoria
                LogMemoryAccess(Address, A, true); // Registrar el acceso de escritura
                Cycles--; // Decrementar los ciclos restantes
            } break;
            case 0xA2: { // LDX Immediate
                Byte Value = FetchByte(Cycles, memory); // Obtener el valor inmediato
                X = Value; // Cargar el valor en el registro X
                LDXSetStatus(); // Establecer los flags de estado
            } break;
            case 0xCA: { // DEX (Decrement X)
                X--;
                Cycles--;
                LDXSetStatus();
            } break;
            case 0xD0: { // BNE (Branch if Not Equal)
                Byte offset = FetchByte(Cycles, memory); // Leer el offset
                if (!Z) {
                    // Offset es signed, pero en 6502 es un byte
                    int8_t rel = static_cast<int8_t>(offset);
                    PC += rel;
                    Cycles--;
                }
            } break;
            case 0x20: { // JSR (Jump to Subroutine)
                Word SubAddr = FetchWord(Cycles, memory); // Obtener la dirección de la subrutina
                PushPCToStack(Cycles, memory); // Guardar el contador de programa en la pila
                PC = SubAddr; // Saltar a la subrutina
                Cycles--; // Ciclo adicional para el salto
            } break;
            default: {
                util::LogWarn("Instrucción no manejada: 0x" + std::to_string(Ins));
            } break;
        }
    }
}
// --- Integración del Controlador de Interrupciones ---

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
