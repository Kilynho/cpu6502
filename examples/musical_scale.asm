; ============================================================================
; MUSICAL SCALE EXAMPLE - 6502 Assembly
; ============================================================================
; Este programa reproduce la escala musical Do-Re-Mi-Fa-Sol-La-Si-Do
; usando el dispositivo de audio BasicAudio del emulador CPU6502.
;
; Autor: CPU6502 Project
; Licencia: MIT
; ============================================================================

; Constantes de registros de audio
FREQ_LOW    = $FB00     ; Byte bajo de frecuencia
FREQ_HIGH   = $FB01     ; Byte alto de frecuencia
DUR_LOW     = $FB02     ; Byte bajo de duración (ms)
DUR_HIGH    = $FB03     ; Byte alto de duración (ms)
VOLUME      = $FB04     ; Volumen (0-255)
CONTROL     = $FB05     ; Control (bit 0=play, bit 1=status)

; Constantes de duración
NOTE_DURATION_LOW   = $F4   ; 500 ms (low byte)
NOTE_DURATION_HIGH  = $01   ; 500 ms (high byte)
NOTE_VOLUME         = $C8   ; 200 (volumen medio-alto)

; Punto de entrada del programa
            ORG $8000

START:
            LDX #$00        ; Inicializar índice de nota

PLAY_NEXT_NOTE:
            ; Cargar frecuencia baja desde la tabla
            LDA SCALE_FREQS_LOW,X
            STA FREQ_LOW
            
            ; Cargar frecuencia alta desde la tabla
            LDA SCALE_FREQS_HIGH,X
            STA FREQ_HIGH
            
            ; Configurar duración
            LDA #NOTE_DURATION_LOW
            STA DUR_LOW
            LDA #NOTE_DURATION_HIGH
            STA DUR_HIGH
            
            ; Configurar volumen
            LDA #NOTE_VOLUME
            STA VOLUME
            
            ; Reproducir nota
            LDA #$01        ; Bit 0 = 1 (play)
            STA CONTROL
            
            ; Esperar a que termine la reproducción
WAIT_NOTE:
            LDA CONTROL     ; Leer registro de control
            AND #$02        ; Verificar bit 1 (status)
            BNE WAIT_NOTE   ; Si está reproduciéndose, seguir esperando
            
            ; Pausa breve entre notas (delay loop)
            JSR SHORT_DELAY
            
            ; Siguiente nota
            INX
            CPX #$08        ; ¿Reproducimos 8 notas?
            BNE PLAY_NEXT_NOTE
            
            ; Fin del programa
            BRK

; ============================================================================
; Subrutina: SHORT_DELAY
; Crea una pausa breve entre notas
; ============================================================================
SHORT_DELAY:
            LDY #$FF
DELAY_LOOP:
            DEY
            BNE DELAY_LOOP
            RTS

; ============================================================================
; Tabla de Frecuencias - Escala Musical (Octava 4)
; Valores en Hz, separados en bytes bajos y altos
; ============================================================================

SCALE_FREQS_LOW:
            .byte $06       ; Do (C4) = 262 Hz
            .byte $26       ; Re (D4) = 294 Hz
            .byte $4A       ; Mi (E4) = 330 Hz
            .byte $5D       ; Fa (F4) = 349 Hz
            .byte $88       ; Sol (G4) = 392 Hz
            .byte $B8       ; La (A4) = 440 Hz
            .byte $EE       ; Si (B4) = 494 Hz
            .byte $0B       ; Do (C5) = 523 Hz

SCALE_FREQS_HIGH:
            .byte $01       ; Do (C4) = 262 Hz
            .byte $01       ; Re (D4) = 294 Hz
            .byte $01       ; Mi (E4) = 330 Hz
            .byte $01       ; Fa (F4) = 349 Hz
            .byte $01       ; Sol (G4) = 392 Hz
            .byte $01       ; La (A4) = 440 Hz
            .byte $01       ; Si (B4) = 494 Hz
            .byte $02       ; Do (C5) = 523 Hz

; ============================================================================
; Notas:
; - Este código asume que el dispositivo BasicAudio está registrado
; - Las frecuencias están en Hz (temperamento igual)
; - La duración de cada nota es de 500 ms
; - El volumen está configurado en 200 (de 0-255)
; - Para usar este código, ensámblalo y cárgalo en $8000
; ============================================================================
