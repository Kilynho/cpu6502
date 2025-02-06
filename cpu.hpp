#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <cstdio>
#include <bitset>
#include <string>
#include "mem.hpp"

// Definición de tipos para mayor claridad
using Byte = uint8_t;  // Un byte (8 bits)
using Word = uint16_t; // Una palabra (16 bits)
using u32 = uint32_t;  // Un entero de 32 bits

// Definición de tipos para mayor claridad
using Byte = uint8_t;  // Un byte (8 bits)
using Word = uint16_t; // Una palabra (16 bits)

// Estructura que representa una instrucción con su opcode, ciclos, bytes y nombre
struct Instruction {
    uint8_t opcode;
    uint8_t cycles;
    uint8_t bytes;
    std::string name;
};

// Clase que representa la CPU del sistema
class CPU {
public:
    // Definición de instrucciones con sus opcodes, ciclos, bytes y nombres
    static const Instruction INS_LDA_IM; // Instrucción LDA Immediate
    static const Instruction INS_LDA_ZP; // Instrucción LDA Zero Page
    static const Instruction INS_LDA_ZPX; // Instrucción LDA Zero Page,X
    static const Instruction INS_LDX_IM; // Instrucción LDX Immediate
    static const Instruction INS_STA_IM; // Instrucción STA Immediate
    static const Instruction INS_JSR;   // Instrucción JSR (Jump to Subroutine)
    static const Instruction INS_RTS;   // Instrucción RTS (Return from Subroutine)

    // Registros de la CPU
    Word PC;    // Program Counter (Contador de Programa)
    Byte SP;    // Stack Pointer (Puntero de Pila)
    Byte A, X, Y; // Registros A, X, Y
    Byte C : 1; // Carry Flag (Bandera de Acarreo)
    Byte Z : 1; // Zero Flag (Bandera de Cero)
    Byte I : 1; // Interrupt Disable (Deshabilitar Interrupciones)
    Byte D : 1; // Decimal Mode (Modo Decimal)
    Byte B : 1; // Break Command (Comando de Interrupción)
    Byte V : 1; // Overflow Flag (Bandera de Desbordamiento)
    Byte N : 1; // Negative Flag (Bandera de Negativo)

    // Métodos públicos
    void Reset(Mem& memory); // Reinicia la CPU y la memoria
    void Execute(u32 Cycles, Mem& memory); // Ejecuta las instrucciones
    void PrintCPUState() const; // Imprime el estado de la CPU
    u32 CalculateCycles(const Mem& mem) const; // Calcula los ciclos necesarios para ejecutar el programa de prueba
    Word FetchWordFromMemory(const Mem& memory, Word address) const; // Obtiene una palabra de la memoria
    void Log(const std::string& message) const; // Registra un mensaje en el log
    void LogMemoryAccess(Word address, Byte data, bool isWrite) const; // Registra el acceso a la memoria
    void AssignCyclesAndBytes(Word &pc, u32 &cycles, Byte opcode) const; // Asigna ciclos y bytes según el opcode
    void PushPCToStack(u32& cycles, Mem& memory); // Guarda el contador de programa en la pila
    void PullPCFromStack(u32& cycles, Mem& memory); // Recupera el contador de programa de la pila
    Word PopWordFromStack(u32& cycles, Mem& memory); // Recupera el contador de programa de la pila

private:
    // Métodos privados
    Byte FetchByte(u32& Cycles, Mem& memory); // Obtiene un byte de la memoria
    Word FetchWord(u32& Cycles, Mem& memory); // Obtiene una palabra de la memoria
    Byte ReadByte(u32& Cycles, Byte Address, Mem& memory); // Lee un byte de la memoria
    Word ReadWord(u32& Cycles, Word Address, Mem& memory); // Lee una palabra de la memoria
    void WriteByte(u32& Cycles, Byte Address, Byte Value, Mem& memory); // Escribe un byte en la memoria
    void WriteWord(u32& Cycles, Word Address, Word Value, Mem& memory); // Escribe una palabra en la memoria
    void LDASetStatus(); // Establece el estado de la instrucción LDA
    std::string ByteToBinaryString(Byte byte) const; // Convierte un byte a una cadena binaria
    std::string WordToBinaryString(Word word) const; // Convierte una palabra a una cadena binaria
    Word SPToAddress(); // Convierte el puntero de pila en una dirección de memoria
};

#endif // CPU_HPP