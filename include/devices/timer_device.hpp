#pragma once
#include "devices/io_device.hpp"
#include <cstdint>

/**
 * @brief Base interface for timing devices
 *
 * This interface defines the basic operations that timing devices must implement
 * (timers, RTC, etc.)
 */
class TimerDevice : public IODevice {
public:
    virtual ~TimerDevice() = default;
    
    /**
     * @brief Inicializa el dispositivo de temporización
     * @return true si se inicializó exitosamente, false en caso contrario
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Obtiene el valor actual del contador del timer
     * @return Valor actual del contador
     */
    virtual uint32_t getCounter() const = 0;
    
    /**
     * @brief Establece el valor del contador del timer
     * @param value Nuevo valor del contador
     */
    virtual void setCounter(uint32_t value) = 0;
    
    /**
     * @brief Reinicia el contador del timer a cero
     */
    virtual void reset() = 0;
    
    /**
     * @brief Verifica si el timer está habilitado
     * @return true si está habilitado, false en caso contrario
     */
    virtual bool isEnabled() const = 0;
    
    /**
     * @brief Habilita o deshabilita el timer
     * @param enabled true para habilitar, false para deshabilitar
     */
    virtual void setEnabled(bool enabled) = 0;
    
    /**
     * @brief Verifica si hay una interrupción pendiente
     * @return true si hay IRQ pendiente, false en caso contrario
     */
    virtual bool hasIRQ() const = 0;
    
    /**
     * @brief Limpia la bandera de interrupción
     */
    virtual void clearIRQ() = 0;
    
    /**
     * @brief Actualiza el estado del timer
     * @param cycles Número de ciclos de CPU transcurridos desde la última actualización
     */
    virtual void tick(uint32_t cycles) = 0;
    
    /**
     * @brief Limpia el dispositivo y libera recursos
     */
    virtual void cleanup() = 0;
};
