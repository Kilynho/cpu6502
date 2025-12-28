#include <gtest/gtest.h>
#include "cpu.hpp"
#include "mem.hpp"
#include "basic_audio.hpp"
#include <memory>
#include <thread>
#include <chrono>

class BasicAudioTest : public testing::Test {
public:
    Mem mem;
    CPU cpu;
    std::shared_ptr<BasicAudio> audio;

    virtual void SetUp() {
        cpu.Reset(mem);
        audio = std::make_shared<BasicAudio>();
        ASSERT_TRUE(audio->initialize()) << "Failed to initialize audio device";
        cpu.registerIODevice(audio);
    }

    virtual void TearDown() {
        cpu.unregisterIODevice(audio);
        audio->cleanup();
    }
};

// Test: Inicialización del dispositivo de audio
TEST_F(BasicAudioTest, Initialization) {
    // Verificar que el dispositivo se inicializó correctamente
    EXPECT_FALSE(audio->isPlaying());
}

// Test: Lectura y escritura de registros de frecuencia
TEST_F(BasicAudioTest, FrequencyRegisters) {
    // Escribir frecuencia 440 Hz (La musical) directamente en los registros
    audio->write(0xFB00, 184);  // 440 & 0xFF
    audio->write(0xFB01, 1);    // 440 >> 8
    
    // Leer de vuelta los registros
    uint8_t lowByte = audio->read(0xFB00);
    uint8_t highByte = audio->read(0xFB01);
    
    EXPECT_EQ(lowByte, 184);
    EXPECT_EQ(highByte, 1);
}

// Test: Lectura y escritura de registros de duración
TEST_F(BasicAudioTest, DurationRegisters) {
    // Escribir duración 500 ms directamente en los registros
    audio->write(0xFB02, 244);  // 500 & 0xFF
    audio->write(0xFB03, 1);    // 500 >> 8
    
    // Leer de vuelta los registros
    uint8_t lowByte = audio->read(0xFB02);
    uint8_t highByte = audio->read(0xFB03);
    
    EXPECT_EQ(lowByte, 244);
    EXPECT_EQ(highByte, 1);
}

// Test: Control de volumen
TEST_F(BasicAudioTest, VolumeControl) {
    // Escribir volumen 200 directamente en el registro
    audio->write(0xFB04, 200);
    
    // Leer de vuelta el registro
    uint8_t volume = audio->read(0xFB04);
    
    EXPECT_EQ(volume, 200);
}

// Test: Generación de tono básico
TEST_F(BasicAudioTest, BasicToneGeneration) {
    // Configurar un tono de 440 Hz durante 100 ms usando la API directa
    audio->playTone(440, 100, 128);
    
    // Verificar que está reproduciéndose
    EXPECT_TRUE(audio->isPlaying());
    
    // Esperar a que termine (más del tiempo de duración)
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // En un entorno sin dispositivo de audio real, el estado puede variar
    // Solo verificamos que no crashee
}

// Test: Detener reproducción
TEST_F(BasicAudioTest, StopPlayback) {
    // Iniciar un tono largo
    audio->playTone(440, 1000, 128);
    
    EXPECT_TRUE(audio->isPlaying());
    
    // Detener
    audio->stop();
    
    EXPECT_FALSE(audio->isPlaying());
}

// Test: Estado de reproducción en registro de control
TEST_F(BasicAudioTest, PlaybackStatus) {
    // Iniciar un tono usando la API directa
    audio->playTone(440, 100, 128);
    
    // Leer el registro de control mientras está reproduciéndose
    uint8_t controlReg = audio->read(0xFB05);
    
    // El bit 1 (status) debe estar activo si hay audio reproduciéndose
    EXPECT_TRUE((controlReg & 0x02) != 0);
    
    // Detener
    audio->stop();
    
    // Leer de nuevo el registro de control
    controlReg = audio->read(0xFB05);
    
    // El bit 1 (status) debe estar inactivo
    EXPECT_TRUE((controlReg & 0x02) == 0);
}
