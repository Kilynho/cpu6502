#include "mem.hpp"

// Initializes memory by setting all bytes to 0
void Mem::Initialize() {
    for (auto& byte : Data) {
        byte = 0;
    }
}

// Read-only access operator for memory
// Returns the byte at the specified address
Byte Mem::operator[](Word Address) const {
    return Data[Address];
}

// Read/write access operator for memory
// Returns a reference to the byte at the specified address
Byte& Mem::operator[](Word Address) {
    return Data[Address];
}