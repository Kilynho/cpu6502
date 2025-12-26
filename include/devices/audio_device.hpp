#pragma once
#include "devices/io_device.hpp"
#include <cstdint>

/**
 * @brief Interfaz base para dispositivos de audio
 * 
 * Esta interfaz define las operaciones básicas que deben implementar
 * los dispositivos de audio (generadores de tonos, chips de sonido, etc.)
 */
class AudioDevice : public IODevice {
public:
    virtual ~AudioDevice() = default;
    
    /**
     * @brief Inicializa el sistema de audio
     * @return true si se inicializó exitosamente, false en caso contrario
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Genera un tono con la frecuencia especificada
     * @param frequency Frecuencia del tono en Hz
     * @param duration Duración del tono en milisegundos
     * @param volume Volumen del tono (0-255)
     */
    virtual void playTone(uint16_t frequency, uint16_t duration, uint8_t volume) = 0;
    
    /**
     * @brief Detiene la reproducción de audio actual
     */
    virtual void stop() = 0;
    
    /**
     * @brief Verifica si hay audio reproduciéndose
     * @return true si hay audio reproduciéndose, false en caso contrario
     */
    virtual bool isPlaying() const = 0;
    
    /**
     * @brief Limpia el sistema de audio y libera recursos
     */
    virtual void cleanup() = 0;
};
