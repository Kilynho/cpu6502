# AudioDevice - Documentación del Dispositivo de Audio

## Descripción General

El `AudioDevice` es una interfaz que define las operaciones básicas para dispositivos de audio en el emulador 6502. La implementación `BasicAudio` proporciona un generador de tonos simple que puede producir ondas cuadradas a diferentes frecuencias, similar a los chips de sonido de las computadoras clásicas de 8 bits (Apple II, Commodore 64, Atari 2600, etc.).

## Características

- **Generación de tonos**: Ondas cuadradas con frecuencias de 20 Hz a 20 kHz
- **Control de duración**: De 1 ms a 10 segundos
- **Control de volumen**: Niveles de 0 a 255
- **Registros mapeados en memoria**: Acceso directo desde código 6502
- **API C++**: Control programático desde el código del emulador
- **Basado en SDL2**: Reproducción de audio en tiempo real

## Arquitectura

### Interfaz AudioDevice

```cpp
class AudioDevice : public IODevice {
public:
    virtual bool initialize() = 0;
    virtual void playTone(uint16_t frequency, uint16_t duration, uint8_t volume) = 0;
    virtual void stop() = 0;
    virtual bool isPlaying() const = 0;
    virtual void cleanup() = 0;
};
```

### Implementación BasicAudio

`BasicAudio` implementa la interfaz `AudioDevice` usando SDL2 Audio para generar ondas cuadradas en tiempo real.

## Mapa de Memoria

Los registros de audio están mapeados en las siguientes direcciones:

| Dirección | Nombre | Descripción |
|-----------|--------|-------------|
| `$FB00` | FREQ_LOW | Byte bajo de la frecuencia (0-255) |
| `$FB01` | FREQ_HIGH | Byte alto de la frecuencia (0-255) |
| `$FB02` | DUR_LOW | Byte bajo de la duración en ms (0-255) |
| `$FB03` | DUR_HIGH | Byte alto de la duración en ms (0-255) |
| `$FB04` | VOLUME | Volumen (0-255) |
| `$FB05` | CONTROL | Control y estado |

### Registro de Control ($FB05)

| Bit | Nombre | Lectura | Escritura |
|-----|--------|---------|-----------|
| 0 | PLAY | - | 1=Reproducir tono, 0=Detener |
| 1 | STATUS | 1=Reproduciéndose, 0=Silencio | - |
| 2-7 | - | Reservado | Reservado |

## Uso desde Código 6502

### Ejemplo 1: Reproducir un Tono de 440 Hz (La)

```assembly
; Configurar frecuencia: 440 Hz
LDA #$B8        ; 440 & 0xFF = 184
STA $FB00       ; Frecuencia baja
LDA #$01        ; 440 >> 8 = 1
STA $FB01       ; Frecuencia alta

; Configurar duración: 500 ms
LDA #$F4        ; 500 & 0xFF = 244
STA $FB02       ; Duración baja
LDA #$01        ; 500 >> 8 = 1
STA $FB03       ; Duración alta

; Configurar volumen: medio
LDA #$80        ; 128 = volumen medio
STA $FB04       ; Volumen

; Reproducir
LDA #$01        ; Bit 0 = 1 (play)
STA $FB05       ; Control
```

### Ejemplo 2: Reproducir la Escala Musical

```assembly
; Datos de la escala musical (Do-Re-Mi-Fa-Sol-La-Si-Do)
; Frecuencias en Hz (octava 4)
SCALE_FREQS:
    .word 262   ; Do (C4)
    .word 294   ; Re (D4)
    .word 330   ; Mi (E4)
    .word 349   ; Fa (F4)
    .word 392   ; Sol (G4)
    .word 440   ; La (A4)
    .word 494   ; Si (B4)
    .word 523   ; Do (C5)

; Reproducir escala
    LDX #$00        ; Índice de nota
PLAY_SCALE:
    ; Cargar frecuencia baja
    LDA SCALE_FREQS,X
    STA $FB00
    
    ; Cargar frecuencia alta
    INX
    LDA SCALE_FREQS,X
    STA $FB01
    INX
    
    ; Configurar duración (500 ms)
    LDA #$F4
    STA $FB02
    LDA #$01
    STA $FB03
    
    ; Configurar volumen
    LDA #$C8        ; 200
    STA $FB04
    
    ; Reproducir
    LDA #$01
    STA $FB05
    
    ; Esperar a que termine
WAIT_PLAY:
    LDA $FB05       ; Leer registro de control
    AND #$02        ; Verificar bit de status
    BNE WAIT_PLAY   ; Si está reproduciéndose, esperar
    
    ; Pausa entre notas (polling delay)
    LDY #$FF
DELAY:
    DEY
    BNE DELAY
    
    ; Siguiente nota
    CPX #$10        ; 8 notas * 2 bytes = 16
    BNE PLAY_SCALE
    
    RTS
```

### Ejemplo 3: Verificar Estado de Reproducción

```assembly
; Iniciar un tono
LDA #$B8
STA $FB00
LDA #$01
STA $FB01
LDA #$88
STA $FB02
LDA #$13
STA $FB03
LDA #$80
STA $FB04
LDA #$01
STA $FB05

; Verificar si está reproduciéndose
CHECK_STATUS:
    LDA $FB05       ; Leer control
    AND #$02        ; Aislar bit de status
    BNE PLAYING     ; Si != 0, está reproduciéndose
    ; No está reproduciéndose
    JMP NOT_PLAYING
PLAYING:
    ; Está reproduciéndose
    ; ...
NOT_PLAYING:
    ; ...
```

## Uso desde C++

### Inicialización

```cpp
#include "devices/basic_audio.hpp"

// Crear dispositivo de audio
auto audio = std::make_shared<BasicAudio>();

// Inicializar
if (!audio->initialize()) {
    std::cerr << "Error: No se pudo inicializar el audio" << std::endl;
    return 1;
}

// Registrar con la CPU
cpu.registerIODevice(audio);
```

### Reproducir un Tono

```cpp
// Reproducir La (440 Hz) durante 500 ms con volumen 200
audio->playTone(440, 500, 200);

// Esperar a que termine
while (audio->isPlaying()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

### Detener Reproducción

```cpp
// Detener el audio actual
audio->stop();
```

### Limpiar

```cpp
// Antes de salir del programa
audio->cleanup();
cpu.unregisterIODevice(audio);
```

## Notas de la Escala Musical

Frecuencias estándar para la octava 4 (temperamento igual):

| Nota | Frecuencia (Hz) | Valor Hex |
|------|----------------|-----------|
| Do (C4) | 262 | $0106 |
| Re (D4) | 294 | $0126 |
| Mi (E4) | 330 | $014A |
| Fa (F4) | 349 | $015D |
| Sol (G4) | 392 | $0188 |
| La (A4) | 440 | $01B8 |
| Si (B4) | 494 | $01EE |
| Do (C5) | 523 | $020B |

## Limitaciones

- **Forma de onda**: Solo se soportan ondas cuadradas (no seno, triangular, etc.)
- **Polifonía**: Solo se puede reproducir un tono a la vez
- **Duración máxima**: 10 segundos por tono
- **Rango de frecuencia**: 20 Hz - 20 kHz
- **Latencia**: Pequeña latencia inherente al sistema de audio SDL2

## Demo

Ejecuta el demo de audio para escuchar la escala musical:

```bash
cd build
./audio_demo
```

El demo reproduce la escala musical completa de dos formas:
1. Usando la API de C++ directamente
2. Generando código 6502 que controla los registros de audio

## Aplicaciones

- **Juegos retro**: Efectos de sonido y música simple
- **Sintetizadores**: Generadores de tonos programables
- **Educación**: Aprender sobre síntesis de audio y programación de sonido
- **Música procedural**: Generar melodías desde código
- **Alarmas y notificaciones**: Tonos de alerta

## Compatibilidad

El dispositivo de audio requiere:
- SDL2 instalado en el sistema
- Dispositivo de audio disponible (altavoces/auriculares)

En entornos sin audio (como CI/CD), el dispositivo se inicializa en modo "silencioso" permitiendo que las pruebas se ejecuten sin errores.

## Referencia de API

### BasicAudio::initialize()
Inicializa el sistema de audio SDL2.
- **Retorna**: `true` si se inicializó correctamente, `false` en caso contrario

### BasicAudio::playTone(frequency, duration, volume)
Reproduce un tono con los parámetros especificados.
- **frequency**: Frecuencia en Hz (20-20000)
- **duration**: Duración en milisegundos (1-10000)
- **volume**: Volumen (0-255)

### BasicAudio::stop()
Detiene la reproducción actual inmediatamente.

### BasicAudio::isPlaying()
Verifica si hay audio reproduciéndose.
- **Retorna**: `true` si está reproduciéndose, `false` en caso contrario

### BasicAudio::cleanup()
Libera recursos de audio y cierra SDL2.

## Ver También

- [Video Device Documentation](video_device.md)
- [File Device Documentation](file_device.md)
- [Architecture Documentation](architecture.md)
