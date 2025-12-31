#pragma once
#include "audio_device.hpp"
#include <SDL2/SDL.h>
#include <atomic>
#include <mutex>

/**
 * @brief Implementación básica de un dispositivo de audio para el emulador 6502
 * 
 * BasicAudio simula un generador de tonos simple que puede producir ondas
 * cuadradas a diferentes frecuencias, similar a los chips de sonido de las
 * computadoras clásicas de 8 bits (Apple II, Commodore 64, etc.)
 * 
 * Direcciones mapeadas en memoria:
 * - 0xFB00: Frecuencia baja (LSB) - byte bajo de la frecuencia en Hz
 * - 0xFB01: Frecuencia alta (MSB) - byte alto de la frecuencia en Hz
 * - 0xFB02: Duración baja (LSB) - byte bajo de la duración en ms
 * - 0xFB03: Duración alta (MSB) - byte alto de la duración en ms
 * - 0xFB04: Volumen (0-255)
 * - 0xFB05: Control:
 *   * Bit 0: Play (1=reproducir tono, 0=detener)
 *   * Bit 1: Status (lectura: 1=reproduciéndose, 0=silencio)
 * 
 * Ejemplo de uso desde 6502:
 * - Configurar frecuencia (440 Hz = La musical):
 *   LDA #$B8    ; 440 & 0xFF
 *   STA $FB00
 *   LDA #$01    ; 440 >> 8
 *   STA $FB01
 * - Configurar duración (500 ms):
 *   LDA #$F4    ; 500 & 0xFF
 *   STA $FB02
 *   LDA #$01    ; 500 >> 8
 *   STA $FB03
 * - Configurar volumen (128 = medio):
 *   LDA #$80
 *   STA $FB04
 * - Reproducir:
 *   LDA #$01
 *   STA $FB05
 */
class BasicAudio : public AudioDevice {
public:
    BasicAudio();
    ~BasicAudio() override;
    
    // Implementación de IODevice
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    
    // Implementación de AudioDevice
    bool initialize() override;
    void playTone(uint16_t frequency, uint16_t duration, uint8_t volume) override;
    void stop() override;
    bool isPlaying() const override;
    void cleanup() override;
    
private:
    // Direcciones de memoria mapeada
    static constexpr uint16_t FREQ_LOW_ADDR = 0xFB00;   // Frecuencia baja
    static constexpr uint16_t FREQ_HIGH_ADDR = 0xFB01;  // Frecuencia alta
    static constexpr uint16_t DUR_LOW_ADDR = 0xFB02;    // Duración baja
    static constexpr uint16_t DUR_HIGH_ADDR = 0xFB03;   // Duración alta
    static constexpr uint16_t VOLUME_ADDR = 0xFB04;     // Volumen
    static constexpr uint16_t CONTROL_ADDR = 0xFB05;    // Control
    
    // Bits del registro de control
    static constexpr uint8_t CTRL_PLAY = 0x01;          // Bit 0: Play
    static constexpr uint8_t CTRL_STATUS = 0x02;        // Bit 1: Status (lectura)
    
    // Parámetros de audio
    static constexpr int SAMPLE_RATE = 44100;           // Frecuencia de muestreo
    static constexpr int CHANNELS = 1;                  // Mono
    static constexpr int SAMPLES = 4096;                // Tamaño del buffer
    
    // Registros internos
    uint8_t frequencyLow;       // Byte bajo de frecuencia
    uint8_t frequencyHigh;      // Byte alto de frecuencia
    uint8_t durationLow;        // Byte bajo de duración
    uint8_t durationHigh;       // Byte alto de duración
    uint8_t volume;             // Volumen (0-255)
    uint8_t control;            // Registro de control
    
    // Estado de SDL
    SDL_AudioDeviceID deviceId;
    SDL_AudioSpec audioSpec;
    bool initialized;
    
    // Estado de reproducción
    std::atomic<bool> playing;
    std::atomic<uint32_t> samplesPlayed;
    std::atomic<uint32_t> totalSamples;
    std::atomic<uint16_t> currentFrequency;
    std::atomic<uint8_t> currentVolume;
    
    std::mutex audioMutex;
    
    // Callback de SDL para generar audio
    static void audioCallback(void* userdata, uint8_t* stream, int len);
    
    // Métodos auxiliares
    uint16_t getFrequency() const;
    uint16_t getDuration() const;
    void triggerPlay();
};
