#include "mem.hpp"

// Inicializa la memoria estableciendo todos los bytes a 0
void Mem::Initialize() {
    for (auto& byte : Data) {
        byte = 0;
    }
}

// Operador de acceso de solo lectura para la memoria
// Devuelve el byte en la dirección especificada
Byte Mem::operator[](Word Address) const {
    return Data[Address];
}

// Operador de acceso de lectura/escritura para la memoria
// Devuelve una referencia al byte en la dirección especificada
Byte& Mem::operator[](Word Address) {
    return Data[Address];
}