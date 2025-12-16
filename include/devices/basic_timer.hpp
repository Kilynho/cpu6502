#pragma once
#include "../timer_device.hpp"
#include "../interrupt_controller.hpp"
#include <cstdint>
#include <atomic>
#include <mutex>

/**
 * @brief Implementación básica de un temporizador para el emulador 6502
 * 
 * BasicTimer simula un temporizador simple que cuenta ciclos de CPU y puede
 * generar interrupciones IRQ periódicas, similar a los chips temporizadores
 * de las computadoras clásicas de 8 bits (VIA 6522, CIA 6526, etc.)
 * 
 * Direcciones mapeadas en memoria:
 * - 0xFC00: Counter bajo (LSB) - byte bajo del contador (lectura/escritura)
 * - 0xFC01: Counter medio (byte 1) - byte medio del contador (lectura/escritura)
 * - 0xFC02: Counter medio-alto (byte 2) - byte medio-alto del contador (lectura/escritura)
 * - 0xFC03: Counter alto (MSB) - byte alto del contador (lectura/escritura)
 * - 0xFC04: Limite bajo (LSB) - byte bajo del límite para IRQ (lectura/escritura)
 * - 0xFC05: Limite medio (byte 1) - byte medio del límite (lectura/escritura)
 * - 0xFC06: Limite medio-alto (byte 2) - byte medio-alto del límite (lectura/escritura)
 * - 0xFC07: Limite alto (MSB) - byte alto del límite (lectura/escritura)
 * - 0xFC08: Control:
 *   * Bit 0: Enable (1=habilitado, 0=deshabilitado)
 *   * Bit 1: IRQ Enable (1=generar IRQ, 0=no generar)
 *   * Bit 2: IRQ Flag (lectura: 1=IRQ pendiente, 0=sin IRQ; escritura: 1=limpiar IRQ)
 *   * Bit 3: Reset (escritura: 1=reiniciar contador a 0)
 *   * Bit 4: Auto-reload (1=reiniciar automáticamente al alcanzar el límite, 0=detener)
 * - 0xFC09: Status (solo lectura):
 *   * Bit 0: Enabled (1=timer habilitado, 0=deshabilitado)
 *   * Bit 1: IRQ Pending (1=IRQ pendiente, 0=sin IRQ)
 *   * Bit 2: Limit Reached (1=contador alcanzó el límite, 0=no alcanzado)
 * 
 * Ejemplo de uso desde 6502:
 * - Configurar límite (1000000 ciclos):
 *   LDA #$40    ; 1000000 & 0xFF
 *   STA $FC04
 *   LDA #$42    ; (1000000 >> 8) & 0xFF
 *   STA $FC05
 *   LDA #$0F    ; (1000000 >> 16) & 0xFF
 *   STA $FC06
 *   LDA #$00    ; 1000000 >> 24
 *   STA $FC07
 * - Habilitar timer con IRQ:
 *   LDA #$03    ; Enable | IRQ Enable
 *   STA $FC08
 * - Leer contador actual:
 *   LDA $FC00   ; Byte bajo
 *   STA temp
 *   LDA $FC01   ; Byte medio
 *   STA temp+1
 *   LDA $FC02   ; Byte medio-alto
 *   STA temp+2
 *   LDA $FC03   ; Byte alto
 *   STA temp+3
 * - Limpiar IRQ:
 *   LDA #$04    ; IRQ Flag bit
 *   STA $FC08
 */
class BasicTimer : public TimerDevice, public InterruptSource {
public:
    BasicTimer();
    ~BasicTimer() override;
    
    // Implementación de IODevice
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    
    // Implementación de TimerDevice
    bool initialize() override;
    uint32_t getCounter() const override;
    void setCounter(uint32_t value) override;
    void reset() override;
    bool isEnabled() const override;
    void setEnabled(bool enabled) override;
    bool hasIRQ() const override;
    void clearIRQ() override;
    bool hasNMI() const override;
    void clearNMI() override;
    void tick(uint32_t cycles) override;
    void cleanup() override;
    
    /**
     * @brief Obtiene el valor del límite configurado
     * @return Valor del límite
     */
    uint32_t getLimit() const;
    
    /**
     * @brief Establece el valor del límite
     * @param value Nuevo valor del límite
     */
    void setLimit(uint32_t value);
    
    /**
     * @brief Verifica si el IRQ está habilitado
     * @return true si IRQ está habilitado, false en caso contrario
     */
    bool isIRQEnabled() const;
    
    /**
     * @brief Verifica si el auto-reload está habilitado
     * @return true si auto-reload está habilitado, false en caso contrario
     */
    bool isAutoReload() const;
    
private:
    // Direcciones de memoria mapeada
    static constexpr uint16_t COUNTER_LOW_ADDR = 0xFC00;      // Counter byte 0 (LSB)
    static constexpr uint16_t COUNTER_MID1_ADDR = 0xFC01;     // Counter byte 1
    static constexpr uint16_t COUNTER_MID2_ADDR = 0xFC02;     // Counter byte 2
    static constexpr uint16_t COUNTER_HIGH_ADDR = 0xFC03;     // Counter byte 3 (MSB)
    static constexpr uint16_t LIMIT_LOW_ADDR = 0xFC04;        // Limit byte 0 (LSB)
    static constexpr uint16_t LIMIT_MID1_ADDR = 0xFC05;       // Limit byte 1
    static constexpr uint16_t LIMIT_MID2_ADDR = 0xFC06;       // Limit byte 2
    static constexpr uint16_t LIMIT_HIGH_ADDR = 0xFC07;       // Limit byte 3 (MSB)
    static constexpr uint16_t CONTROL_ADDR = 0xFC08;          // Control
    static constexpr uint16_t STATUS_ADDR = 0xFC09;           // Status (read-only)
    
    // Bits del registro de control
    static constexpr uint8_t CTRL_ENABLE = 0x01;              // Bit 0: Enable
    static constexpr uint8_t CTRL_IRQ_ENABLE = 0x02;          // Bit 1: IRQ Enable
    static constexpr uint8_t CTRL_IRQ_FLAG = 0x04;            // Bit 2: IRQ Flag
    static constexpr uint8_t CTRL_RESET = 0x08;               // Bit 3: Reset
    static constexpr uint8_t CTRL_AUTO_RELOAD = 0x10;         // Bit 4: Auto-reload
    
public:
    // Bits del registro de status (public para tests)
    static constexpr uint8_t STATUS_ENABLED = 0x01;           // Bit 0: Enabled
    static constexpr uint8_t STATUS_IRQ_PENDING = 0x02;       // Bit 1: IRQ Pending
    static constexpr uint8_t STATUS_LIMIT_REACHED = 0x04;     // Bit 2: Limit Reached

private:
    // Estado interno
    std::atomic<uint32_t> counter;                            // Contador actual
    std::atomic<uint32_t> limit;                              // Límite para IRQ
    std::atomic<uint8_t> control;                             // Registro de control
    std::atomic<bool> enabled;                                // Timer habilitado
    std::atomic<bool> irqEnabled;                             // IRQ habilitado
    std::atomic<bool> irqPending;                             // IRQ pendiente
    std::atomic<bool> autoReload;                             // Auto-reload habilitado
    std::atomic<bool> limitReached;                           // Límite alcanzado
    bool initialized;                                         // Inicializado
    
    std::mutex timerMutex;                                    // Mutex para operaciones thread-safe
    
    // Métodos auxiliares
    void updateControlFlags(uint8_t value);
    uint8_t getStatusRegister() const;
};
