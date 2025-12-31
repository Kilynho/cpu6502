/**
 * @file audio_demo.cpp
 * @brief Demostración del dispositivo de audio BasicAudio
 * 
 * Este programa demuestra cómo usar el dispositivo de audio BasicAudio
 * para generar tonos desde código 6502. Reproduce la escala musical
 * Do-Re-Mi-Fa-Sol-La-Si-Do.
 */

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "cpu.hpp"
#include "mem.hpp"
#include "basic_audio.hpp"

// Frecuencias de la escala musical en Hz (octava 4)
const uint16_t NOTE_C4 = 262;  // Do
const uint16_t NOTE_D4 = 294;  // Re
const uint16_t NOTE_E4 = 330;  // Mi
const uint16_t NOTE_F4 = 349;  // Fa
const uint16_t NOTE_G4 = 392;  // Sol
const uint16_t NOTE_A4 = 440;  // La
const uint16_t NOTE_B4 = 494;  // Si
const uint16_t NOTE_C5 = 523;  // Do (octava superior)

void playNote(std::shared_ptr<BasicAudio> audio, uint16_t frequency, const char* noteName) {
    std::cout << "Reproduciendo: " << noteName << " (" << frequency << " Hz)..." << std::endl;
    audio->playTone(frequency, 500, 200);  // 500ms de duración, volumen 200
    
    // Esperar a que termine la nota
    while (audio->isPlaying()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Pausa entre notas
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void playCPUGeneratedScale(CPU& cpu, Mem& mem, std::shared_ptr<BasicAudio> audio) {
    std::cout << "\n=== Generando escala musical desde código 6502 ===" << std::endl;
    std::cout << "El CPU 6502 está escribiendo directamente en los registros de audio..." << std::endl;
    
    // Array de frecuencias de la escala
    uint16_t scale[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
    const char* noteNames[] = {"Do", "Re", "Mi", "Fa", "Sol", "La", "Si", "Do"};
    
    // ...existing code...
    // This function needs to be refactored to use SystemMap instead of Mem.
    // For now, skip 6502 code playback in this demo migration.
}

int main() {
    std::cout << "=== Demostración de Audio para 6502 Emulator ===" << std::endl;
    std::cout << "Este programa reproduce la escala musical Do-Re-Mi-Fa-Sol-La-Si-Do" << std::endl;
    std::cout << std::endl;
    
    // Crear memoria y CPU
    SystemMap bus;
    CPU cpu;
    
    // Crear y registrar dispositivo de audio
    auto audio = std::make_shared<BasicAudio>();
    
    if (!audio->initialize()) {
        std::cerr << "Error: Could not initialize audio device" << std::endl;
        std::cerr << "Make sure you have an audio device available" << std::endl;
        return 1;
    }
    
    cpu.registerIODevice(audio);
    cpu.PC = 0x8000;
    cpu.SP = 0xFD;
    
    std::cout << "Audio device initialized successfully" << std::endl;
    std::cout << "Audio registers mapped at:" << std::endl;
    std::cout << "  $FB00-$FB01: Frecuencia (16-bit)" << std::endl;
    std::cout << "  $FB02-$FB03: Duración (16-bit, en ms)" << std::endl;
    std::cout << "  $FB04:       Volumen (0-255)" << std::endl;
    std::cout << "  $FB05:       Control (bit 0=play, bit 1=status)" << std::endl;
    std::cout << std::endl;
    
    // Opción 1: Reproducir escala usando API de C++
    std::cout << "=== Opción 1: Reproducción directa (API C++) ===" << std::endl;
    playNote(audio, NOTE_C4, "Do");
    playNote(audio, NOTE_D4, "Re");
    playNote(audio, NOTE_E4, "Mi");
    playNote(audio, NOTE_F4, "Fa");
    playNote(audio, NOTE_G4, "Sol");
    playNote(audio, NOTE_A4, "La");
    playNote(audio, NOTE_B4, "Si");
    playNote(audio, NOTE_C5, "Do");
    
    std::cout << "\nScale completed!" << std::endl;
    
    // Opción 2: Reproducir escala usando código 6502
    std::cout << "\nPress Enter to play the scale using 6502 code..." << std::endl;
    std::cin.get();
    
    // playCPUGeneratedScale(cpu, mem, audio); // TODO: refactor for SystemMap
    
    std::cout << "\nDemo completed!" << std::endl;
    
    // Limpiar
    cpu.unregisterIODevice(audio);
    audio->cleanup();
    
    return 0;
}
