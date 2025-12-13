#pragma once
#include "io_device.hpp"
#include <cstdint>
#include <string>

/**
 * @brief Interfaz base para dispositivos seriales
 * 
 * Esta interfaz define las operaciones básicas que deben implementar
 * los dispositivos seriales (UART, ACIA 6551, TCP/IP, etc.)
 */
class SerialDevice : public IODevice {
public:
    virtual ~SerialDevice() = default;
    
    /**
     * @brief Inicializa el dispositivo serial
     * @return true si se inicializó exitosamente, false en caso contrario
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Conecta el dispositivo serial a una dirección/puerto
     * @param address Dirección o puerto a conectar (formato depende de la implementación)
     * @return true si se conectó exitosamente, false en caso contrario
     */
    virtual bool connect(const std::string& address) = 0;
    
    /**
     * @brief Desconecta el dispositivo serial
     */
    virtual void disconnect() = 0;
    
    /**
     * @brief Verifica si hay datos disponibles para leer
     * @return true si hay datos disponibles, false en caso contrario
     */
    virtual bool dataAvailable() const = 0;
    
    /**
     * @brief Lee un byte del dispositivo serial
     * @return Byte leído (0 si no hay datos disponibles)
     */
    virtual uint8_t receiveByte() = 0;
    
    /**
     * @brief Envía un byte por el dispositivo serial
     * @param data Byte a enviar
     * @return true si se envió exitosamente, false en caso contrario
     */
    virtual bool transmitByte(uint8_t data) = 0;
    
    /**
     * @brief Verifica si el dispositivo está conectado
     * @return true si está conectado, false en caso contrario
     */
    virtual bool isConnected() const = 0;
    
    /**
     * @brief Limpia el sistema serial y libera recursos
     */
    virtual void cleanup() = 0;
};
