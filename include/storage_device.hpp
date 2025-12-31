#pragma once
#include <string>
#include "io_device.hpp"

/**
 * @brief Interfaz base para dispositivos de almacenamiento
 * 
 * Esta interfaz define las operaciones básicas de almacenamiento que deben
 * implementar los dispositivos de almacenamiento (disco, archivo, etc.)
 */
class StorageDevice : public IODevice {
public:
    virtual ~StorageDevice() = default;
    
    /**
     * @brief Carga un binario desde el almacenamiento a memoria
     * @param filename Nombre del archivo a cargar
     * @param startAddress Dirección de memoria donde cargar el binario
     * @return true si se cargó exitosamente, false en caso contrario
     */
    virtual bool loadBinary(const std::string& filename, uint16_t startAddress) = 0;
    
    /**
     * @brief Guarda un bloque de memoria al almacenamiento
     * @param filename Nombre del archivo donde guardar
     * @param startAddress Dirección de inicio del bloque de memoria
     * @param length Longitud del bloque a guardar
     * @return true si se guardó exitosamente, false en caso contrario
     */
    virtual bool saveBinary(const std::string& filename, uint16_t startAddress, uint16_t length) = 0;
    
    /**
     * @brief Verifica si un archivo existe
     * @param filename Nombre del archivo a verificar
     * @return true si el archivo existe, false en caso contrario
     */
    virtual bool fileExists(const std::string& filename) const = 0;
};
