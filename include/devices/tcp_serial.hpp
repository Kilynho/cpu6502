#pragma once
#include "../serial_device.hpp"
#include <string>
#include <queue>
#include <cstdint>

/**
 * @brief Implementación de dispositivo serial usando sockets TCP
 * 
 * TcpSerial simula un puerto serial (ACIA 6551) usando comunicación TCP.
 * Permite conectar el emulador 6502 con programas externos vía red.
 * 
 * Direcciones mapeadas (compatible con ACIA 6551):
 * - 0xFA00: Datos (lectura/escritura)
 * - 0xFA01: Estado (solo lectura)
 *   Bit 0: Dato recibido disponible (RDR)
 *   Bit 1: Transmisor vacío (TXE)
 *   Bit 7: Interrupción pendiente (IRQ)
 * - 0xFA02: Comando
 *   Bit 0-1: Control de paridad
 *   Bit 2-3: Control de bits de parada
 *   Bit 4: Echo mode
 * - 0xFA03: Control
 *   Bits 0-3: Velocidad de baudios (simulado)
 *   Bits 5-6: Longitud de palabra
 * - 0xFA04: Dirección/Puerto (byte bajo) - extensión para TCP
 * - 0xFA05: Dirección/Puerto (byte alto) - extensión para TCP
 * - 0xFA06: Control de conexión (0=desconectar, 1=conectar servidor, 2=escuchar)
 * - 0xFA10-0xFA4F: Buffer para dirección IP/hostname (max 64 bytes)
 */
class TcpSerial : public SerialDevice {
public:
    TcpSerial();
    ~TcpSerial() override;
    
    // Implementación de IODevice
    bool handlesRead(uint16_t address) const override;
    bool handlesWrite(uint16_t address) const override;
    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    
    // Implementación de SerialDevice
    bool initialize() override;
    bool connect(const std::string& address) override;
    void disconnect() override;
    bool dataAvailable() const override;
    uint8_t receiveByte() override;
    bool transmitByte(uint8_t data) override;
    bool isConnected() const override;
    void cleanup() override;
    
    // Métodos específicos para TCP
    bool listen(uint16_t port);
    bool acceptConnection() const;
    
    // Métodos de diagnóstico
    std::string getConnectionInfo() const;
    uint8_t getStatus() const { return statusReg; }
    
private:
    // Registros ACIA
    static constexpr uint16_t DATA_REG = 0xFA00;       // Registro de datos
    static constexpr uint16_t STATUS_REG = 0xFA01;     // Registro de estado
    static constexpr uint16_t COMMAND_REG = 0xFA02;    // Registro de comando
    static constexpr uint16_t CONTROL_REG = 0xFA03;    // Registro de control
    
    // Extensiones TCP
    static constexpr uint16_t PORT_LO = 0xFA04;        // Puerto TCP (byte bajo)
    static constexpr uint16_t PORT_HI = 0xFA05;        // Puerto TCP (byte alto)
    static constexpr uint16_t CONN_CONTROL = 0xFA06;   // Control de conexión
    static constexpr uint16_t ADDR_BUFFER_START = 0xFA10; // Inicio buffer dirección
    static constexpr uint16_t ADDR_BUFFER_END = 0xFA4F;   // Fin buffer dirección
    
    // Bits de estado
    static constexpr uint8_t STATUS_RDR = 0x01;   // Dato recibido disponible
    static constexpr uint8_t STATUS_TXE = 0x02;   // Transmisor vacío
    static constexpr uint8_t STATUS_IRQ = 0x80;   // Interrupción pendiente
    
    // Operaciones de conexión
    enum class ConnOperation : uint8_t {
        DISCONNECT = 0,
        CONNECT = 1,
        LISTEN = 2
    };
    
    // Registros del dispositivo
    uint8_t dataReg;           // Último byte leído/escrito
    mutable uint8_t statusReg; // Estado del dispositivo
    uint8_t commandReg;        // Registro de comando
    uint8_t controlReg;        // Registro de control
    uint16_t tcpPort;          // Puerto TCP configurado
    uint8_t connControl;       // Control de conexión
    
    // Buffers
    std::vector<uint8_t> addressBuffer;  // Buffer para dirección IP/hostname
    mutable std::queue<uint8_t> receiveBuffer;   // Buffer de recepción
    std::queue<uint8_t> transmitBuffer;  // Buffer de transmisión
    
    // Estado de conexión
    int socketFd;                   // Descriptor de socket
    mutable int clientFd;           // Descriptor de cliente (para modo servidor)
    bool initialized;               // Sistema inicializado
    mutable bool connected;         // Conexión establecida
    mutable bool listening;         // Modo escucha activado
    mutable std::string currentAddress; // Dirección actual
    
    // Métodos internos
    void updateStatus() const;                      // Actualiza el registro de estado
    void executeConnOperation();                    // Ejecuta operación de conexión
    std::string getAddressFromBuffer() const;       // Obtrae dirección del buffer
    void updateAddressBuffer(uint16_t address, uint8_t value);
    void pollSocket() const;                        // Lee datos del socket si están disponibles
    void flushTransmitBuffer();                     // Envía datos pendientes
    void closeSocket();                             // Cierra socket actual
};
