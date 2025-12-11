#include "devices/basic_audio.hpp"
#include <cmath>
#include <algorithm>
#include <cstring>

BasicAudio::BasicAudio()
    : frequencyLow(0),
      frequencyHigh(0),
      durationLow(0),
      durationHigh(0),
      volume(128),  // Volumen medio por defecto
      control(0),
      deviceId(0),
      initialized(false),
      playing(false),
      samplesPlayed(0),
      totalSamples(0),
      currentFrequency(0),
      currentVolume(128) {
}

BasicAudio::~BasicAudio() {
    cleanup();
}

bool BasicAudio::initialize() {
    if (initialized) {
        return true;
    }
    
    // Inicializar SDL Audio
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        // No hay dispositivo de audio disponible, pero permitir la inicialización
        // en modo "silencioso" para pruebas
        initialized = true;
        deviceId = 0;
        return true;
    }
    
    // Configurar especificación de audio
    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq = SAMPLE_RATE;
    desired.format = AUDIO_S16SYS;  // 16-bit signed audio
    desired.channels = CHANNELS;
    desired.samples = SAMPLES;
    desired.callback = audioCallback;
    desired.userdata = this;
    
    // Abrir dispositivo de audio
    deviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &audioSpec, 0);
    if (deviceId == 0) {
        // No se pudo abrir el dispositivo, pero permitir continuar en modo silencioso
        initialized = true;
        return true;
    }
    
    // Iniciar reproducción (silencio inicialmente)
    SDL_PauseAudioDevice(deviceId, 0);
    
    initialized = true;
    return true;
}

void BasicAudio::cleanup() {
    if (!initialized) {
        return;
    }
    
    stop();
    
    if (deviceId > 0) {
        SDL_CloseAudioDevice(deviceId);
        deviceId = 0;
    }
    
    SDL_Quit();
    initialized = false;
}

bool BasicAudio::handlesRead(uint16_t address) const {
    return (address >= FREQ_LOW_ADDR && address <= CONTROL_ADDR);
}

bool BasicAudio::handlesWrite(uint16_t address) const {
    return (address >= FREQ_LOW_ADDR && address <= CONTROL_ADDR);
}

uint8_t BasicAudio::read(uint16_t address) {
    switch (address) {
        case FREQ_LOW_ADDR:
            return frequencyLow;
        case FREQ_HIGH_ADDR:
            return frequencyHigh;
        case DUR_LOW_ADDR:
            return durationLow;
        case DUR_HIGH_ADDR:
            return durationHigh;
        case VOLUME_ADDR:
            return volume;
        case CONTROL_ADDR:
            // Actualizar bit de estado
            if (isPlaying()) {
                return control | CTRL_STATUS;
            } else {
                return control & ~CTRL_STATUS;
            }
        default:
            return 0;
    }
}

void BasicAudio::write(uint16_t address, uint8_t value) {
    switch (address) {
        case FREQ_LOW_ADDR:
            frequencyLow = value;
            break;
        case FREQ_HIGH_ADDR:
            frequencyHigh = value;
            break;
        case DUR_LOW_ADDR:
            durationLow = value;
            break;
        case DUR_HIGH_ADDR:
            durationHigh = value;
            break;
        case VOLUME_ADDR:
            volume = value;
            break;
        case CONTROL_ADDR:
            control = value;
            // Verificar si se solicita reproducir
            if (value & CTRL_PLAY) {
                triggerPlay();
            } else {
                stop();
            }
            break;
    }
}

void BasicAudio::playTone(uint16_t frequency, uint16_t duration, uint8_t vol) {
    if (!initialized) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(audioMutex);
    
    // Configurar parámetros
    currentFrequency = frequency;
    currentVolume = vol;
    
    // Calcular número total de muestras para la duración
    totalSamples = (static_cast<uint32_t>(duration) * SAMPLE_RATE) / 1000;
    samplesPlayed = 0;
    
    playing = true;
}

void BasicAudio::stop() {
    playing = false;
    samplesPlayed = 0;
    totalSamples = 0;
}

bool BasicAudio::isPlaying() const {
    return playing.load();
}

void BasicAudio::audioCallback(void* userdata, uint8_t* stream, int len) {
    BasicAudio* audio = static_cast<BasicAudio*>(userdata);
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);
    
    // Si no está reproduciéndose, silencio
    if (!audio->playing.load()) {
        std::memset(stream, 0, len);
        return;
    }
    
    uint16_t freq = audio->currentFrequency.load();
    uint8_t vol = audio->currentVolume.load();
    uint32_t played = audio->samplesPlayed.load();
    uint32_t total = audio->totalSamples.load();
    
    // Generar onda cuadrada
    for (int i = 0; i < samples; i++) {
        if (played >= total) {
            // Se acabó la duración, detener
            audio->playing = false;
            buffer[i] = 0;
            continue;
        }
        
        // Generar onda cuadrada
        // La onda cuadrada alterna entre -amplitude y +amplitude
        // a una frecuencia determinada
        double phase = (static_cast<double>(played) / SAMPLE_RATE) * freq;
        double fractionalPart = phase - std::floor(phase);
        
        int16_t amplitude = static_cast<int16_t>((vol / 255.0) * 32767);
        buffer[i] = (fractionalPart < 0.5) ? amplitude : -amplitude;
        
        played++;
    }
    
    audio->samplesPlayed = played;
}

uint16_t BasicAudio::getFrequency() const {
    return static_cast<uint16_t>(frequencyLow) | (static_cast<uint16_t>(frequencyHigh) << 8);
}

uint16_t BasicAudio::getDuration() const {
    return static_cast<uint16_t>(durationLow) | (static_cast<uint16_t>(durationHigh) << 8);
}

void BasicAudio::triggerPlay() {
    uint16_t freq = getFrequency();
    uint16_t dur = getDuration();
    
    // Validar frecuencia razonable (20 Hz - 20 kHz)
    if (freq < 20 || freq > 20000) {
        return;
    }
    
    // Validar duración razonable (1 ms - 10 segundos)
    if (dur < 1 || dur > 10000) {
        return;
    }
    
    playTone(freq, dur, volume);
}
