#pragma once
#include "io_device.hpp"
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Interfaz base para dispositivos de video
 * 
 * Esta interfaz define las operaciones básicas que deben implementar
 * los dispositivos de video (pantalla de texto, framebuffer gráfico, etc.)
 */
class VideoDevice : public IODevice {
public:
    virtual ~VideoDevice() = default;
    
    /**
     * @brief Refresca la pantalla con el contenido actual del buffer de video
     */
    virtual void refresh() = 0;
    
    /**
     * @brief Limpia la pantalla
     */
    virtual void clear() = 0;
    
    /**
     * @brief Obtiene el buffer de video actual como string (para debugging/testing)
     * @return String con el contenido del buffer de video
     */
    virtual std::string getBuffer() const = 0;
    
    /**
     * @brief Obtiene el ancho de la pantalla en caracteres/píxeles
     * @return Ancho de la pantalla
     */
    virtual uint16_t getWidth() const = 0;
    
    /**
     * @brief Obtiene el alto de la pantalla en caracteres/píxeles
     * @return Alto de la pantalla
     */
    virtual uint16_t getHeight() const = 0;
};
