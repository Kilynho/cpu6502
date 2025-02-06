#ifndef MEM_HPP
#define MEM_HPP

#include <array>
#include <cstdint>
#include <cstddef>

// Definición de tipos para mayor claridad
using Byte = uint8_t;  // Un byte (8 bits)
using Word = uint16_t; // Una palabra (16 bits)

// Clase que representa la memoria del sistema
class Mem {
public:
    // Inicializa la memoria estableciendo todos los bytes a 0
    void Initialize();

    // Operador de acceso de solo lectura para la memoria
    // Devuelve el byte en la dirección especificada
    Byte operator[](Word Address) const;

    // Operador de acceso de lectura/escritura para la memoria
    // Devuelve una referencia al byte en la dirección especificada
    Byte& operator[](Word Address);

public:
    static constexpr size_t MEM_SIZE = 65536; // Tamaño total de la memoria (64 KB)
    std::array<Byte, MEM_SIZE> Data; // Array que representa la memoria

    // Definición de las regiones de memoria
    static constexpr Word ZERO_PAGE_START = 0x0000;
    static constexpr Word ZERO_PAGE_END = 0x00FF;
    static constexpr Word STACK_START = 0x0100;
    static constexpr Word STACK_END = 0x01FF;
    static constexpr Word RAM_START = 0x0200;
    static constexpr Word RAM_END = 0x1FFF;
    static constexpr Word ROM_START = 0x8000;
    static constexpr Word ROM_END = 0xFFFF;
    static constexpr Word RESET_VECTOR = 0xFFFC;
    static constexpr Word IRQ_VECTOR = 0xFFFE;  // Dirección del vector de IRQ
    static constexpr Word NMI_VECTOR = 0xFFFA;  // Dirección del vector de NMI
};

#endif // MEM_HPP