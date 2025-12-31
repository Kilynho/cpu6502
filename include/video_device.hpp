#pragma once
#include "io_device.hpp"
#include <string>
#include <vector>

/**
 * @brief Base interface for video devices
 *
 * This interface defines the basic operations that video devices must implement
 * (text screen, graphic framebuffer, etc.)
 */
class VideoDevice : public IODevice {
public:
    virtual ~VideoDevice() = default;
    
    /**
     * @brief Refreshes the screen with the current video buffer content
     */
    virtual void refresh() = 0;
    
    /**
     * @brief Clears the screen
     */
    virtual void clear() = 0;
    
    /**
     * @brief Gets the current video buffer as a string (for debugging/testing)
     * @return String with the video buffer content
     */
    virtual std::string getBuffer() const = 0;
    
    /**
     * @brief Gets the width of the screen in characters/pixels
     * @return Screen width
     */
    virtual uint16_t getWidth() const = 0;
    
    /**
     * @brief Gets the height of the screen in characters/pixels
     * @return Screen height
     */
    virtual uint16_t getHeight() const = 0;
};
