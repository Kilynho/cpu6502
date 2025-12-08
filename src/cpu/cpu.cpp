#include "cpu.hpp"
#include "mem.hpp"
#include "util/logger.hpp"
#include <bitset>
#include <fstream>
#include <iomanip>
#include <sstream>

// Definición de las instrucciones con sus opcodes, ciclos, bytes y nombres
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
    Word pc = Mem::ROM_START; // Inicio del programa en la memoria ROM

    while (true) { // Bucle infinito
        Byte opcode = mem[pc];
        AssignCyclesAndBytes(pc, cycles, opcode); // Asignar ciclos y bytes según el opcode
        // Detener si se alcanza el final de la memoria
        if (pc == Mem::ROM_END) {   // Si se alcanza el final de la memoria
            break;                // Salir del bucle
        }
    }
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

    // Si se encontró una instrucción válida, asignar ciclos y bytes
    if (instruction) {
        cycles += instruction->cycles;
        pc += instruction->bytes;
    } else {
        pc++;
    }
}

Byte CPU::FetchByte(u32& Cycles, Mem& memory) {
    Byte Data = memory[PC]; // Obtener el byte de la memoria en la dirección del contador de programa
    LogMemoryAccess(PC, Data, false); // Registrar el acceso de lectura a la memoria
    PC++; // Incrementar el contador de programa
    Cycles--; // Decrementar los ciclos restantes
    return Data; // Devolver el byte obtenido
}

Word CPU::FetchWord(u32& Cycles, Mem& memory) {
    Word Data = memory[PC]; // Obtener el byte bajo de la palabra
    LogMemoryAccess(PC, Data, false); // Registrar el acceso de lectura a la memoria
    PC++; // Incrementar el contador de programa
    Data |= (memory[PC] << 8); // Obtener el byte alto de la palabra y combinarlo con el byte bajo
    LogMemoryAccess(PC, memory[PC], false); // Registrar el acceso de lectura a la memoria
    PC++; // Incrementar el contador de programa
    Cycles -= 2; // Decrementar los ciclos restantes
    return Data; // Devolver la palabra obtenida
}

Word CPU::FetchWordFromMemory(const Mem& memory, Word address) const {
    LogMemoryAccess(address, memory[address], false); // Registrar el acceso de lectura a la memoria
    LogMemoryAccess(address + 1, memory[address + 1], false); // Registrar el acceso de lectura a la memoria
    return (memory[address] | (memory[address + 1] << 8));
} 

Byte CPU::ReadByte(u32& Cycles, Byte Address, Mem& memory) {
    Byte Data = memory[Address]; // Leer el byte de la memoria en la dirección especificada
    LogMemoryAccess(Address, Data, false); // Registrar el acceso de lectura a la memoria
    Cycles--; // Decrementar los ciclos restantes
    return Data; // Devolver el byte leído
}

Word CPU::ReadWord(u32& Cycles, Word Address, Mem& memory) {
    Word Data = memory[Address]; // Leer el byte bajo de la palabra
    LogMemoryAccess(Address, Data, false); // Registrar el acceso de lectura a la memoria
    Address++; // Incrementar la dirección
    Data |= (memory[Address] << 8); // Leer el byte alto de la palabra y combinarlo con el byte bajo
    LogMemoryAccess(Address, memory[Address], false); // Registrar el acceso de lectura a la memoria
    Cycles--; // Decrementar los ciclos restantes
    return Data; // Devolver la palabra leída
}

void CPU::WriteByte(u32& Cycles, Byte Address, Byte Data, Mem& memory) {
    memory[Address] = Data; // Escribir el byte en la memoria en la dirección especificada
    LogMemoryAccess(Address, Data, true); // Registrar el acceso de escritura a la memoria
    Cycles--; // Decrementar los ciclos restantes
}

void CPU::WriteWord(u32& Cycles, Word Address, Word Data, Mem& memory) {
    memory[Address] = Data & 0x00FF; // Escribir el byte bajo de la palabra en la memoria
    LogMemoryAccess(Address, Data & 0x00FF, true); // Registrar el acceso de escritura a la memoria
    Cycles--; // Decrementar los ciclos restantes
    memory[Address + 1] = (Data & 0xFF00) >> 8; // Escribir el byte alto de la palabra en la memoria
    LogMemoryAccess(Address + 1, (Data & 0xFF00) >> 8, true); // Registrar el acceso de escritura a la memoria
    Cycles--; // Decrementar los ciclos restantes
}

Word CPU::SPToAddress() const {
    return 0x0100 + SP; // Devolver la dirección de la pila
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
    PC = PopWordFromStack(cycles, memory); // Recuperar el contador de programa de la pila
    PC++; // Incrementar el contador de programa
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

void CPU::LDXSetStatus() {
    Z = (X == 0); // Establecer el flag de cero si el registro X es cero
    N = (X & 0b10000000) > 0; // Establecer el flag de negativo si el bit más significativo del registro X es 1
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
    A = X = Y = 0;
    C = Z = I = D = B = V = N = 0;
}

CPU::CPU() : PC(0), SP(0), A(0), X(0), Y(0), C(0), Z(0), I(0), D(0), B(0), V(0), N(0) {
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
        Byte Ins = FetchByte(Cycles, memory); // Obtener el opcode de la instrucción
        switch (Ins) {
            case 0xA9: { // LDA Immediate
                Byte Value = FetchByte(Cycles, memory); // Obtener el valor inmediato
                A = Value; // Cargar el valor en el acumulador
                LDASetStatus(); // Establecer los flags de estado
                //Cycles -= INS_LDA_IM.cycles; // Restar los ciclos consumidos
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
                A = memory[Address]; // Leer el valor de la memoria y cargarlo en el acumulador
                LogMemoryAccess(Address, A, false); // Registrar el acceso de lectura a la memoria
                Cycles--; // Decrementar los ciclos restantes
                LDASetStatus(); // Establecer los flags de estado
            } break;
            case 0xBD: { // LDA Absolute,X
                Word Address = FetchWord(Cycles, memory); // Obtener la dirección absoluta
                Address += X; // Sumar el valor del registro X
                A = memory[Address]; // Leer el valor de la memoria y cargarlo en el acumulador
                LogMemoryAccess(Address, A, false); // Registrar el acceso de lectura a la memoria
                Cycles--; // Decrementar los ciclos restantes
                LDASetStatus(); // Establecer los flags de estado
            } break;
            case 0xB9: { // LDA Absolute,Y
                Word Address = FetchWord(Cycles, memory); // Obtener la dirección absoluta
                Address += Y; // Sumar el valor del registro Y
                A = memory[Address]; // Leer el valor de la memoria y cargarlo en el acumulador
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
            case 0x85: {  // STA Store Accumulator in Memory
                Byte Address = FetchByte(Cycles, memory); // Obtener la dirección de memoria
                WriteByte(Cycles, Address, A, memory); // Escribir el valor del acumulador en la memoria
            } break;
            case 0xA2: { // LDX Immediate
                Byte Value = FetchByte(Cycles, memory); // Obtener el valor inmediato
                X = Value; // Cargar el valor en el registro X
                LDXSetStatus(); // Establecer los flags de estado
            } break;
            case 0x20: { // JSR (Jump to Subroutine)
                Word SubAddr = FetchWord(Cycles, memory); // Obtener la dirección de la subrutina
                PushPCToStack(Cycles, memory); // Guardar el contador de programa en la pila
                PC = SubAddr; // Saltar a la subrutina
                Cycles--; // Ciclo adicional para el salto
            } break;
            default: {
                // printf("Instruction not handled %d\n", Ins); // Imprimir un mensaje si la instrucción no es manejada
            } break;
        }
    }
}