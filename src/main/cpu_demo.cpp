#include "cpu.hpp"
#include "mem.hpp"
#include "util/logger.hpp"
#include <cstdio>
#include <unistd.h>
#include <cstring>

int main(int argc, char* argv[]) {
    bool infiniteCycles = false;
  
    // Analizar los argumentos de la línea de comandos
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "infinite") == 0) {
            infiniteCycles = true;
        }
    }

    Mem mem; // Crear una instancia de la memoria
    CPU cpu; // Crear una instancia de la CPU

    // Reiniciar la CPU y la memoria
    cpu.Reset(mem);
    
    // Escribir un programa de prueba en la memoria ROM
    mem[0x0040] = 0x55;                     // Valor a cargar en el acumulador
    mem[0x0050] = 0x77;                     // Valor a cargar en el acumulador
    mem[0x8000] = CPU::INS_LDX_IM.opcode;   // Instrucción LDX Immediate
    mem[0x8001] = 0x0f;                     // Valor a cargar en el registro X
    mem[0x8002] = CPU::INS_LDA_IM.opcode;   // Instrucción LDA Immediate
    mem[0x8003] = 0x84;                     // Valor a cargar en el acumulador
    mem[0x8004] = CPU::INS_LDA_ZP.opcode;   // Instrucción LDA Zero Page
    mem[0x8005] = 0x40;                     // Dirección de la página cero para cargar en el acumulador
    mem[0x8006] = CPU::INS_LDA_ZPX.opcode;  // Instrucción LDA Zero Page X
    mem[0x8007] = 0x41;                     // Dirección de la página cero para cargar en el acumulador con desplazamiento X
    mem[0x8008] = CPU::INS_STA_ZP.opcode;   // Instrucción STA Zero Page
    mem[0x8009] = 0x42;                     // Dirección de página cero para almacenar el acumulador
    mem[0x800A] = CPU::INS_JSR.opcode;      // Instrucción JSR (Jump to Subroutine)
    mem[0x800B] = 0x00;                     // Dirección baja de la subrutina
    mem[0x800C] = 0x80;                     // Dirección alta de la subrutina
    mem[0x8100] = CPU::INS_RTS.opcode;      // Instrucción RTS (Return from Subroutine)
    mem[0x800D] = CPU::INS_LDA_IM.opcode;   // Instrucción LDA Immediate
    mem[0x800E] = 0x99;                     // Valor a cargar en el acumulador
    mem[0x800F] = CPU::INS_JSR.opcode;      // Instrucción JSR (Jump to Subroutine)
    mem[0x8010] = 0x00;                     // Dirección baja de la subrutina
    mem[0x8011] = 0x80;                     // Dirección alta de la subrutina

    // Configurar el vector de reset para apuntar a la dirección de inicio del programa
    mem[Mem::RESET_VECTOR] = 0x00;          // Dirección baja del vector de reset
    mem[Mem::RESET_VECTOR + 1] = 0x80;      // Dirección alta del vector de reset

    // Determinar el número de ciclos a ejecutar
    u32 cycles;
    if (infiniteCycles) {
        cycles = -1; // Ciclos infinitos
    } else {
        cycles = cpu.CalculateCycles(mem); // Calcular automáticamente el número de ciclos necesarios
    }

    // Ejecutar el programa de prueba
    cpu.Execute(cycles, mem);

    return 0;
}