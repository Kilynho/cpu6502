#pragma once
#include "../storage_device.hpp"
#include <string>
#include <vector>
#include <cstdint>

// Forward declaration
class Mem;

/**
 * @brief Implementación de dispositivo de almacenamiento basado en archivos del host
 * 
 * FileDevice permite cargar y guardar binarios desde/hacia archivos del sistema host.
 * Se mapea a direcciones de memoria específicas para controlar operaciones de E/S.
 * 
 * Direcciones mapeadas:
 * - 0xFE00: Control de operación (0=nada, 1=cargar, 2=guardar)
 * - 0xFE01-0xFE02: Dirección de inicio (little-endian)
 * - 0xFE03-0xFE04: Longitud de datos (little-endian)
 * - 0xFE05: Estado (0=éxito, 1=error)
 * - 0xFE10-0xFE4F: Nombre de archivo (hasta 64 bytes, null-terminated)
 */
class FileDevice : public StorageDevice {
public:
    FileDevice(Mem* memory);
    
    // Implementación de IODevice
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    
    // Implementación de StorageDevice
    bool loadBinary(const std::string& filename, uint16_t startAddress) override;
    bool saveBinary(const std::string& filename, uint16_t startAddress, uint16_t length) override;
    bool fileExists(const std::string& filename) const override;
    
    // Métodos de diagnóstico
    std::string getLastFilename() const { return lastFilename; }
    uint8_t getStatus() const { return status; }
    
private:
    static constexpr uint16_t CONTROL_ADDR = 0xFE00;      // Control de operación
    static constexpr uint16_t START_ADDR_LO = 0xFE01;     // Byte bajo dirección inicio
    static constexpr uint16_t START_ADDR_HI = 0xFE02;     // Byte alto dirección inicio
    static constexpr uint16_t LENGTH_LO = 0xFE03;         // Byte bajo longitud
    static constexpr uint16_t LENGTH_HI = 0xFE04;         // Byte alto longitud
    static constexpr uint16_t STATUS_ADDR = 0xFE05;       // Estado de operación
    static constexpr uint16_t FILENAME_START = 0xFE10;    // Inicio del nombre de archivo
    static constexpr uint16_t FILENAME_END = 0xFE4F;      // Fin del nombre de archivo
    
    enum class Operation : uint8_t {
        NONE = 0,
        LOAD = 1,
        SAVE = 2
    };
    
    Mem* mem;                          // Referencia a la memoria del sistema
    uint8_t controlReg;                // Registro de control
    uint16_t startAddress;             // Dirección de inicio para operación
    uint16_t length;                   // Longitud de datos a transferir
    uint8_t status;                    // Estado de la última operación
    std::vector<uint8_t> filenameBuffer; // Buffer para nombre de archivo
    std::string lastFilename;          // Último nombre de archivo procesado
    
    void executeOperation();           // Ejecuta la operación pendiente
    void updateFilename(uint16_t address, uint8_t value); // Actualiza el buffer de nombre
    std::string getFilenameFromBuffer() const; // Extrae nombre de archivo del buffer
};
