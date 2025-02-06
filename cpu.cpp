#include "cpu.hpp"
#include "mem.hpp"
#include <bitset>
#include <fstream>
#include <iomanip>
#include <sstream>

// Definición de las instrucciones con sus opcodes, ciclos, bytes y nombres
const Instruction CPU::INS_LDA_IM = {0xA9, 2, 2, "LDA_IM"}; // LDA Immediate
const Instruction CPU::INS_LDA_ZP = {0xA5, 3, 2, "LDA_ZP"}; // LDA Zero Page
const Instruction CPU::INS_LDA_ZPX = {0xB5, 4, 2, "LDA_ZPX"}; // LDA Zero Page,X
const Instruction CPU::INS_LDX_IM = {0xA2, 2, 2, "LDX_IM"}; // LDX Immediate
const Instruction CPU::INS_STA_IM = {0x85, 3, 2, "STA_IM"}; // STA Immediate
const Instruction CPU::INS_JSR = {0x20, 6, 3, "JSR"};   // JSR (Jump to Subroutine
const Instruction CPU::INS_RTS = {0x60, 6, 1, "RTS"};   // RTS (Return from Subroutine)

u32 CPU::CalculateCycles(const Mem& mem) const {
    u32 cycles = 0;
    Word pc = Mem::ROM_START; // Inicio del programa en la memoria ROM

    while (true) { // Bucle infinito
        Byte opcode = mem[pc];
        AssignCyclesAndBytes(pc, cycles, opcode); // Asignar ciclos y bytes según el opcode
        // Detener si se alcanza el final de la memoria
        if (pc == Mem::ROM_END) {   // Si se alcanza el final de la memoria
            break;                // Salir del bucle
        }
    }
    printf("Total %d cycles\n\n", cycles); // Imprimir el total de ciclos
    return cycles;
}

void CPU::AssignCyclesAndBytes(Word &pc, u32 &cycles, Byte opcode) const {
    const Instruction* instruction = nullptr;

    // Asignar la instrucción correspondiente según el opcode
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
    } else if (opcode == INS_STA_IM.opcode) {
        instruction = &INS_STA_IM;
    } else if (opcode == INS_RTS.opcode) {
        instruction = &INS_RTS;
    }

    // Si se encontró una instrucción válida, asignar ciclos y bytes
    if (instruction) {
        Word printpc = pc;
        u32 printcycles = instruction->cycles;
        cycles += instruction->cycles;
        pc += instruction->bytes;
        printf("Opcode manejado %s (%02X) en la dirección %04X con %d cycles para un total de %d\n", instruction->name, opcode, printpc, printcycles, cycles);
    } else {
        pc++;
        //printf("Opcode no manejado %02X en la dirección %04X\n", opcode, pc);
    }
}

Byte CPU::FetchByte(u32& Cycles, Mem& memory) {
    Byte Data = memory[PC]; // Obtener el byte de la memoria en la dirección del contador de programa
    LogMemoryAccess(PC, Data, false); // Registrar el acceso de lectura a la memoria
    PC++; // Incrementar el contador de programa
    //Cycles--; // Decrementar los ciclos restantes
    return Data; // Devolver el byte obtenido
}

Word CPU::FetchWord(u32& Cycles, Mem& memory) {
    Word Data = memory[PC]; // Obtener el byte bajo de la palabra
    LogMemoryAccess(PC, Data, false); // Registrar el acceso de lectura a la memoria
    PC++; // Incrementar el contador de programa
    Data |= (memory[PC] << 8); // Obtener el byte alto de la palabra y combinarlo con el byte bajo
    LogMemoryAccess(PC, memory[PC], false); // Registrar el acceso de lectura a la memoria
    PC++; // Incrementar el contador de programa
    //Cycles -= 2; // Decrementar los ciclos restantes
    return Data; // Devolver la palabra obtenida
}

Word CPU::FetchWordFromMemory(const Mem& memory, Word address) const {
    return (memory[address] | (memory[address + 1] << 8));
    LogMemoryAccess(address, memory[address], false); // Registrar el acceso de lectura a la memoria
}   

Byte CPU::ReadByte(u32& Cycles, Byte Address, Mem& memory) { //
    Byte Data = memory[Address]; // Leer el byte de la memoria en la dirección especificada
    LogMemoryAccess(Address, Data, false); // Registrar el acceso de lectura a la memoria
    //Cycles--; // Decrementar los ciclos restantes
    return Data; // Devolver el byte leído
}

Word CPU::ReadWord(u32& Cycles, Word Address, Mem& memory) {
    Word Data = memory[Address]; // Leer el byte bajo de la palabra
    LogMemoryAccess(Address, Data, false); // Registrar el acceso de lectura a la memoria
    Address++; // Incrementar la dirección
    Data |= (memory[Address] << 8); // Leer el byte alto de la palabra y combinarlo con el byte bajo
    LogMemoryAccess(Address, memory[Address], false); // Registrar el acceso de lectura a la memoria
    //Cycles--; // Decrementar los ciclos restantes
    return Data; // Devolver la palabra leída
}

void CPU::WriteByte(u32& Cycles, Byte Address, Byte Data, Mem& memory) {
    memory[Address] = Data; // Escribir el byte en la memoria en la dirección especificada
    LogMemoryAccess(Address, Data, true); // Registrar el acceso de escritura a la memoria
    //Cycles--; // Decrementar los ciclos restantes
}

void CPU::WriteWord(u32& Cycles, Word Address, Word Data, Mem& memory) {
    memory[Address] = Data & 0x00FF; // Escribir el byte bajo de la palabra en la memoria
    LogMemoryAccess(Address, Data & 0x00FF, true); // Registrar el acceso de escritura a la memoria
    memory[Address + 1] = (Data & 0xFF00) >> 8; // Escribir el byte alto de la palabra en la memoria
    LogMemoryAccess(Address, (Data & 0xFF00) >> 8, true); // Registrar el acceso de escritura a la memoria
    //Cycles--; // Decrementar los ciclos restantes
}

Word CPU::SPToAddress() {
    SP++; // Incrementar el puntero de pila
    return 0x0100 + SP; // Devolver la dirección de la pila
}

void CPU::PushPCToStack(u32& Cycles, Mem& memory) {
    WriteWord(Cycles, SPToAddress() - 1, PC - 1, memory); // Guardar el contador de programa en la pila
    SP -= 2; // Decrementar el puntero de pila
}

Word CPU::PopWordFromStack(u32& Cycles, Mem& memory) {
    Word ValueFromStack = ReadWord(Cycles, SPToAddress(), memory); // Recuperar el contador de programa de la pila
    SP += 2; // Incrementar el puntero de pila
    return ValueFromStack; // Devolver el contador de programa recuperado
}

void CPU::LDASetStatus() {
    Z = (A == 0); // Establecer el flag de cero si el acumulador es cero
    N = (A & 0b10000000) > 0; // Establecer el flag de negativo si el bit más significativo del acumulador es 1
}

void CPU::Reset(Mem& memory) {
    // Limpiar el fichero de log
    std::ofstream logFile("cpu_log.txt", std::ios_base::trunc);
    logFile.close();
    
    memory.Initialize(); // Inicializar la memoria
    memory.Data[Mem::RESET_VECTOR] = 0x00; // Establecer la dirección baja del vector de reset
    memory.Data[Mem::RESET_VECTOR + 1] = 0x80; // Establecer la dirección alta del vector de reset
    memory.Data[Mem::STACK_END] = 0xff; // Establecer la dirección baja del vector de reset
    memory.Data[Mem::STACK_END + 1] = 0x00; // Establecer la dirección alta del vector de reset
    PC = FetchWordFromMemory(memory, Mem::RESET_VECTOR); // Iniciar el contador de programa en la dirección del vector de reset (little-endian)
    SP = FetchWordFromMemory(memory, Mem::STACK_END); // Iniciar el contador de programa en la dirección del vector de reset (little-endian)
    //printf("PC after reset vector: %04X\n", PC); // Imprimir el valor de PC después de la asignación del vector de reset
    //printf("SP after reset vector: %04X\n", SP); // Imprimir el valor de SP después de la asignación del vector de reset
    A = X = Y = 0;
    C = Z = I = D = B = V = N = 0;
}

void CPU::LogMemoryAccess(Word address, Byte data, bool isWrite) const {
    std::ostringstream oss;
    oss << std::bitset<16>(address) << "  "
        << std::bitset<8>(data) << "  "
        << std::hex << std::setw(4) << std::setfill('0') << address << "  "
        << (isWrite ? "W" : "r") << "  "
        << std::hex << std::setw(2) << static_cast<int>(data) << "  ";

    std::ofstream logFile("cpu_log.txt", std::ios_base::app);
    logFile << oss.str();

    oss << std::setw(4) << PC << "  " << std::setw(2) << SP << "  "
        << std::setw(2) << static_cast<int>(A) << " "
        << std::setw(2) << static_cast<int>(X) << " "
        << std::setw(2) << static_cast<int>(Y) << " "
        << static_cast<int>(C) << static_cast<int>(Z)
        << static_cast<int>(I) << static_cast<int>(D)
        << static_cast<int>(B) << static_cast<int>(V)
        << static_cast<int>(N);
    std::string state = oss.str();
    logFile << state << std::endl;
    //printf("%s\n", state.c_str());
}

void CPU::Execute(u32 Cycles, Mem& memory) {
    while (Cycles > 0 || Cycles == static_cast<u32>(-1)) {
        Byte Ins = FetchByte(Cycles, memory); // Obtener el opcode de la instrucción
        switch (Ins) {
            case 0xA9: { // LDA Immediate
                Byte Value = FetchByte(Cycles, memory); // Obtener el valor inmediato
                A = Value; // Cargar el valor en el acumulador
                LDASetStatus(); // Establecer los flags de estado
                Cycles -= INS_LDA_IM.cycles; // Restar los ciclos consumidos
            } break;
            case 0xA5: { // LDA Zero Page
                Byte Value = FetchByte(Cycles, memory); // Obtener la dirección de la página cero
                A = ReadByte(Cycles, Value, memory); // Leer el valor de la memoria y cargarlo en el acumulador
                LDASetStatus(); // Establecer los flags de estado
                Cycles -= INS_LDA_ZP.cycles; // Restar los ciclos consumidos
            } break;
            case 0xB5: { // LDA Zero Page,X
                Byte Value = FetchByte(Cycles, memory); // Obtener la dirección de la página cero
                Value += X; // Sumar el valor del registro X
                A = ReadByte(Cycles, Value, memory); // Leer el valor de la memoria y cargarlo en el acumulador
                LDASetStatus(); // Establecer los flags de estado
                Cycles -= INS_LDA_ZPX.cycles; // Restar los ciclos consumidos
            } break;
            case 0x20: { // JSR (Jump to Subroutine)
                Word SubAddr = FetchWord(Cycles, memory); // Obtener la dirección de la subrutina
                PushPCToStack(Cycles, memory); // Guardar el contador de programa en la pila
                PC = SubAddr; // Saltar a la subrutina
                Cycles -= INS_JSR.cycles; // Restar los ciclos consumidos
            } break;
            case 0x60: { // RTS (Return from Subroutine)
                Word ReturnAddress = PopWordFromStack(Cycles, memory); // Leer la dirección de retorno de la pila
                PC = ReturnAddress + 1; // Volver a la dirección de retorno
                Cycles -= INS_RTS.cycles; // Restar los ciclos consumidos
            } break;
            case 0xA2: { // LDX Immediate
                Byte Value = FetchByte(Cycles, memory); // Obtener el valor inmediato
                X = Value; // Cargar el valor en el registro X
                LDASetStatus(); // Establecer los flags de estado
                Cycles -= INS_LDX_IM.cycles; // Restar los ciclos consumidos
            } break;
            case 0x85: { // STA Immediate
                Byte Address = FetchByte(Cycles, memory); // Obtener la dirección inmediata
                memory[Address] = A; // Almacenar el valor del acumulador en la memoria
                LogMemoryAccess(A, PC & 0x00FF, true); // Registrar el acceso de escritura a la memoria
                Cycles -= INS_STA_IM.cycles; // Restar los ciclos consumidos
            } break;
            default: {
                //printf("Instruction not handled %d\n", Ins); // Imprimir un mensaje si la instrucción no es manejada
            } break;
        }
    }
}