# AudioDevice - Audio Device Documentation

## General Description
`AudioDevice` is an interface that defines the basic operations for audio devices in the 6502 emulator. The `BasicAudio` implementation provides a simple tone generator that can produce square waves at different frequencies, similar to the sound chips of classic 8-bit computers (Apple II, Commodore 64, Atari 2600, etc.).

## Features
- **Tone generation**: Square waves with frequencies from 20 Hz to 20 kHz
- **Duration control**: From 1 ms to 10 seconds
- **Volume control**: Levels from 0 to 255
- **Memory-mapped registers**: Direct access from 6502 code
- **C++ API**: Programmatic control from emulator code
- **SDL2-based**: Real-time audio playback
## Architecture

### AudioDevice Interface

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

### BasicAudio Implementation

`BasicAudio` implements the `AudioDevice` interface using SDL2 Audio to generate square waves in real-time.

## Memory Map

The audio registers are mapped to the following addresses:

| Address | Name | Description |
|-----------|--------|-------------|
| `$FB00` | FREQ_LOW | Low byte of the frequency (0-255) |
| `$FB01` | FREQ_HIGH | High byte of the frequency (0-255) |
| `$FB02` | DUR_LOW | Low byte of the duration in ms (0-255) |
| `$FB03` | DUR_HIGH | High byte of the duration in ms (0-255) |
| `$FB04` | VOLUME | Volume (0-255) |
| `$FB05` | CONTROL | Control and status |

### Control Register ($FB05)

| Bit | Name | Read | Write |
|-----|--------|---------|-----------|
| 0 | PLAY | - | 1=Play tone, 0=Stop |
| 1 | STATUS | 1=Playing, 0=Silence | - |
| 2-7 | - | Reserved | Reserved |

## Usage from 6502 Code

### Example 1: Play a 440 Hz (A) Tone

```assembly
; Set frequency: 440 Hz
LDA #$B8        ; 440 & 0xFF = 184
STA $FB00       ; Low frequency byte
LDA #$01        ; 440 >> 8 = 1
STA $FB01       ; High frequency byte

; Set duration: 500 ms
LDA #$F4        ; 500 & 0xFF = 244
STA $FB02       ; Low duration byte
LDA #$01        ; 500 >> 8 = 1
STA $FB03       ; High duration byte

; Set volume: medium
LDA #$80        ; 128 = medium volume
STA $FB04       ; Volume

; Play
LDA #$01        ; Bit 0 = 1 (play)
STA $FB05       ; Control
```

### Example 2: Play the Musical Scale

```assembly
; Musical scale data (Do-Re-Mi-Fa-Sol-La-Si-Do)
; Frequencies in Hz (octave 4)
SCALE_FREQS:
    .word 262   ; Do (C4)
    .word 294   ; Re (D4)
    .word 330   ; Mi (E4)
    .word 349   ; Fa (F4)
    .word 392   ; Sol (G4)
    .word 440   ; La (A4)
    .word 494   ; Si (B4)
    .word 523   ; Do (C5)

; Play scale
    LDX #$00        ; Note index
PLAY_SCALE:
    ; Load low frequency byte
    LDA SCALE_FREQS,X
    STA $FB00
    
    ; Load high frequency byte
    INX
    LDA SCALE_FREQS,X
    STA $FB01
    INX
    
    ; Set duration (500 ms)
    LDA #$F4
    STA $FB02
    LDA #$01
    STA $FB03
    
    ; Set volume
    LDA #$C8        ; 200
    STA $FB04
    
    ; Play
    LDA #$01
    STA $FB05
    
    ; Wait for it to finish
WAIT_PLAY:
    LDA $FB05       ; Read control register
    AND #$02        ; Check status bit
    BNE WAIT_PLAY   ; If playing, wait
    
    ; Pause between notes (polling delay)
    LDY #$FF
DELAY:
    DEY
    BNE DELAY
    
    ; Next note
    CPX #$10        ; 8 notes * 2 bytes = 16
    BNE PLAY_SCALE
    
    RTS
```

### Example 3: Check Playback Status

```assembly
; Start a tone
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

; Check if it's playing
CHECK_STATUS:
    LDA $FB05       ; Read control
    AND #$02        ; Isolate status bit
    BNE PLAYING     ; If != 0, it's playing
    ; Not playing
    JMP NOT_PLAYING
PLAYING:
    ; Is playing
    ; ...
NOT_PLAYING:
    ; ...
```

## Usage from C++

### Initialization

```cpp
#include "devices/basic_audio.hpp"

// Create audio device
auto audio = std::make_shared<BasicAudio>();

// Initialize
if (!audio->initialize()) {
    std::cerr << "Error: Could not initialize audio" << std::endl;
    return 1;
}

// Register with CPU
cpu.registerIODevice(audio);
```

### Play a Tone

```cpp
// Play La (440 Hz) for 500 ms with volume 200
audio->playTone(440, 500, 200);

// Wait for it to finish
while (audio->isPlaying()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

### Stop Playback

```cpp
// Stop current audio
audio->stop();
```

### Cleanup

```cpp
// Before exiting the program
audio->cleanup();
cpu.unregisterIODevice(audio);
```

## Musical Scale Notes

Standard frequencies for octave 4 (equal temperament):

| Note | Frequency (Hz) | Hex Value |
|------|----------------|-----------|
| Do (C4) | 262 | $0106 |
| Re (D4) | 294 | $0126 |
| Mi (E4) | 330 | $014A |
| Fa (F4) | 349 | $015D |
| Sol (G4) | 392 | $0188 |
| La (A4) | 440 | $01B8 |
| Si (B4) | 494 | $01EE |
| Do (C5) | 523 | $020B |

## Limitations

- **Waveform**: Only square waves are supported (no sine, triangle, etc.)
- **Polyphony**: Only one tone can be played at a time
- **Maximum duration**: 10 seconds per tone
- **Frequency range**: 20 Hz - 20 kHz
- **Latency**: Small inherent latency due to SDL2 audio system

## Demo

Run the audio demo to hear the musical scale:

```bash
cd build
./audio_demo
```

The demo plays the complete musical scale in two ways:
1. Using the C++ API directly
2. Generating 6502 code that controls the audio registers

## Applications

- **Retro games**: Sound effects and simple music
- **Synthesizers**: Programmable tone generators
- **Education**: Learn about audio synthesis and sound programming
- **Procedural music**: Generate melodies from code
- **Alarms and notifications**: Alert tones

## Compatibility

The audio device requires:
- SDL2 installed on the system
- Available audio device (speakers/headphones)

In audio-less environments (like CI/CD), the device initializes in "silent" mode allowing tests to run without errors.

## API Reference

### BasicAudio::initialize()
Initializes the SDL2 audio system.
- **Returns**: `true` if initialized successfully, `false` otherwise

### BasicAudio::playTone(frequency, duration, volume)
Plays a tone with the specified parameters.
- **frequency**: Frequency in Hz (20-20000)
- **duration**: Duration in milliseconds (1-10000)
- **volume**: Volume (0-255)

### BasicAudio::stop()
Immediately stops current playback.

### BasicAudio::isPlaying()
Checks if audio is playing.
- **Returns**: `true` if playing, `false` otherwise

### BasicAudio::cleanup()
Frees audio resources and closes SDL2.

## See Also

- [Video Device Documentation](video_device.md)
- [File Device Documentation](file_device.md)
- [Architecture Documentation](architecture.md)
