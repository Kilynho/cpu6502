#include "file_device.hpp"
#include "mem.hpp"
#include <fstream>
#include <iostream>
#include <cstring>

FileDevice::FileDevice(Mem* memory) 
    : mem(memory), controlReg(0), startAddress(0), length(0), status(0) {
    filenameBuffer.resize(FILENAME_END - FILENAME_START + 1, 0);
}

bool FileDevice::handlesRead(uint16_t address) const {
    return (address >= CONTROL_ADDR && address <= STATUS_ADDR) ||
           (address >= FILENAME_START && address <= FILENAME_END);
}

bool FileDevice::handlesWrite(uint16_t address) const {
    return (address >= CONTROL_ADDR && address <= STATUS_ADDR) ||
           (address >= FILENAME_START && address <= FILENAME_END);
}

uint8_t FileDevice::read(uint16_t address) {
    if (address == CONTROL_ADDR) {
        return controlReg;
    } else if (address == START_ADDR_LO) {
        return startAddress & 0xFF;
    } else if (address == START_ADDR_HI) {
        return (startAddress >> 8) & 0xFF;
    } else if (address == LENGTH_LO) {
        return length & 0xFF;
    } else if (address == LENGTH_HI) {
        return (length >> 8) & 0xFF;
    } else if (address == STATUS_ADDR) {
        return status;
    } else if (address >= FILENAME_START && address <= FILENAME_END) {
        uint16_t index = address - FILENAME_START;
        return filenameBuffer[index];
    }
    return 0;
}

void FileDevice::write(uint16_t address, uint8_t value) {
    if (address == CONTROL_ADDR) {
        controlReg = value;
        // Cuando se escribe en control, ejecutar la operación
        if (controlReg != 0) {
            executeOperation();
            // Limpiar el registro de control después de la operación
            controlReg = 0;
        }
    } else if (address == START_ADDR_LO) {
        startAddress = (startAddress & 0xFF00) | value;
    } else if (address == START_ADDR_HI) {
        startAddress = (startAddress & 0x00FF) | (static_cast<uint16_t>(value) << 8);
    } else if (address == LENGTH_LO) {
        length = (length & 0xFF00) | value;
    } else if (address == LENGTH_HI) {
        length = (length & 0x00FF) | (static_cast<uint16_t>(value) << 8);
    } else if (address == STATUS_ADDR) {
        status = value;
    } else if (address >= FILENAME_START && address <= FILENAME_END) {
        updateFilename(address, value);
    }
}

void FileDevice::updateFilename(uint16_t address, uint8_t value) {
    uint16_t index = address - FILENAME_START;
    filenameBuffer[index] = value;
}

std::string FileDevice::getFilenameFromBuffer() const {
    std::string filename;
    for (uint8_t byte : filenameBuffer) {
        if (byte == 0) break;  // Null terminator
        filename += static_cast<char>(byte);
    }
    return filename;
}

void FileDevice::executeOperation() {
    lastFilename = getFilenameFromBuffer();
    
    if (lastFilename.empty()) {
        status = 1;  // Error: nombre de archivo vacío
        return;
    }
    
    Operation op = static_cast<Operation>(controlReg);
    
    switch (op) {
        case Operation::LOAD:
            status = loadBinary(lastFilename, startAddress) ? 0 : 1;
            break;
        case Operation::SAVE:
            status = saveBinary(lastFilename, startAddress, length) ? 0 : 1;
            break;
        default:
            status = 1;  // Error: operación desconocida
            break;
    }
}

bool FileDevice::loadBinary(const std::string& filename, uint16_t startAddr) {
    if (!mem) {
        std::cerr << "FileDevice: Memoria no inicializada\n";
        return false;
    }
    
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "FileDevice: No se pudo abrir el archivo: " << filename << "\n";
        return false;
    }
    
    // Obtener el tamaño del archivo
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Verificar que no se salga del espacio de memoria (64KB)
    if (startAddr + fileSize > 0x10000) {
        std::cerr << "FileDevice: El archivo es demasiado grande para cargar en 0x" 
                  << std::hex << startAddr << "\n";
        file.close();
        return false;
    }
    
    // Leer el archivo en un buffer temporal
    std::vector<uint8_t> buffer(fileSize);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        std::cerr << "FileDevice: Error al leer el archivo: " << filename << "\n";
        file.close();
        return false;
    }
    
    file.close();
    
    // Copiar el buffer a la memoria
    for (size_t i = 0; i < buffer.size(); ++i) {
        (*mem)[startAddr + i] = buffer[i];
    }
    
    std::cout << "FileDevice: Cargados " << fileSize << " bytes desde '" 
              << filename << "' a 0x" << std::hex << startAddr << "\n";
    
    return true;
}

bool FileDevice::saveBinary(const std::string& filename, uint16_t startAddr, uint16_t len) {
    if (!mem) {
        std::cerr << "FileDevice: Memoria no inicializada\n";
        return false;
    }
    
    // Verificar que no se salga del espacio de memoria
    // Usar comparación segura para evitar overflow
    if (len > 0x10000 || startAddr > 0x10000 - len) {
        std::cerr << "FileDevice: Rango de memoria inválido\n";
        return false;
    }
    
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "FileDevice: No se pudo crear el archivo: " << filename << "\n";
        return false;
    }
    
    // Copiar datos de memoria a un buffer temporal
    std::vector<uint8_t> buffer(len);
    for (uint16_t i = 0; i < len; ++i) {
        buffer[i] = (*mem)[startAddr + i];
    }
    
    // Escribir el buffer al archivo
    if (!file.write(reinterpret_cast<const char*>(buffer.data()), len)) {
        std::cerr << "FileDevice: Error al escribir el archivo: " << filename << "\n";
        file.close();
        return false;
    }
    
    file.close();
    
    std::cout << "FileDevice: Guardados " << len << " bytes desde 0x" 
              << std::hex << startAddr << " a '" << filename << "'\n";
    
    return true;
}

bool FileDevice::fileExists(const std::string& filename) const {
    std::ifstream file(filename);
    return file.good();
}
