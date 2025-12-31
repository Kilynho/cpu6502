#include "cpu.hpp"
#include "system_map.hpp"
#include "tcp_serial.hpp"
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

/**
 * Ejemplo de uso de TcpSerial para comunicación serial sobre TCP
 * 
 * Este programa demuestra cómo:
 * 1. Crear y usar un TcpSerial
 * 2. Escuchar conexiones TCP entrantes (modo servidor)
 * 3. Enviar y recibir datos usando la API directa
 * 4. Usar los registros mapeados en memoria para E/S serial
 * 5. Integrar con código 6502 para comunicación bidireccional
 */

// Variable global para controlar el bucle principal
volatile bool running = true;

void signalHandler(int signum) {
    std::cout << "\n\nSeñal de interrupción recibida. Cerrando...\n";
    running = false;
}

// Programa 6502 que implementa un echo server simple
// TODO: Refactor to use SystemMap for bus-based memory access
void loadEchoProgram(SystemMap& bus, uint16_t startAddr) {
    uint16_t addr = startAddr;

    // LOOP:
    //   LDA $FA01        ; Leer registro de estado
    bus.write(addr++, 0xAD);  // LDA absolute
    bus.write(addr++, 0x01);
    bus.write(addr++, 0xFA);

    //   AND #$01         ; Verificar bit RDR (dato disponible)
    bus.write(addr++, 0x29);  // AND immediate
    bus.write(addr++, 0x01);

    //   BEQ LOOP         ; Si no hay dato, seguir esperando
    bus.write(addr++, 0xF0);  // BEQ
    bus.write(addr++, 0xF8);  // -8 (volver a LOOP)

    //   LDA $FA00        ; Leer byte recibido
    bus.write(addr++, 0xAD);  // LDA absolute
    bus.write(addr++, 0x00);
    bus.write(addr++, 0xFA);

    //   STA $FA00        ; Enviar byte (echo)
    bus.write(addr++, 0x8D);  // STA absolute
    bus.write(addr++, 0x00);
    bus.write(addr++, 0xFA);

    //   JMP LOOP         ; Volver al inicio
    bus.write(addr++, 0x4C);  // JMP absolute
    bus.write(addr++, (startAddr & 0xFF));
    bus.write(addr++, ((startAddr >> 8) & 0xFF));
}

// Modo 1: Echo server usando API directa (C++)
void runDirectAPIMode(std::shared_ptr<TcpSerial> tcpSerial) {
    std::cout << "\n=== Modo 1: Echo Server usando API directa de C++ ===\n";
    std::cout << "Escuchando en puerto 12345...\n";
    std::cout << "Conéctate con: nc localhost 12345\n";
    std::cout << "Presiona Ctrl+C para salir\n\n";
    
    if (!tcpSerial->listen(12345)) {
        std::cerr << "Error al iniciar servidor\n";
        return;
    }
    
    std::cout << "Esperando conexión...\n";
    
    while (running) {
        // Polling para datos disponibles
        if (tcpSerial->dataAvailable()) {
            uint8_t byte = tcpSerial->receiveByte();
            std::cout << "Recibido: " << static_cast<char>(byte) << " (0x" 
                     << std::hex << static_cast<int>(byte) << std::dec << ")\n";
            
            // Echo: enviar de vuelta
            if (tcpSerial->transmitByte(byte)) {
                std::cout << "Enviado: " << static_cast<char>(byte) << "\n";
            }
        }
        
        if (!tcpSerial->isConnected()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

// Modo 2: Echo server usando código 6502 y registros mapeados
void run6502Mode(SystemMap& bus, CPU& cpu, std::shared_ptr<TcpSerial> tcpSerial) {
    std::cout << "\n=== Modo 2: Echo Server usando código 6502 ===\n";
    std::cout << "Cargando programa 6502...\n";
    // Cargar programa echo en memoria (TODO: update to use bus)
    loadEchoProgram(bus, 0x8000);
    std::cout << "Programa cargado en 0x8000\n";
    // Configurar puerto usando registros mapeados
    tcpSerial->write(0xFA04, 0x39);  // Puerto 12345 & 0xFF
    tcpSerial->write(0xFA05, 0x30);  // Puerto 12345 >> 8
    // Activar modo escucha
    tcpSerial->write(0xFA06, 2);  // LISTEN
    std::cout << "Servidor escuchando en puerto 12345...\n";
    std::cout << "Conéctate con: nc localhost 12345\n";
    std::cout << "El código 6502 hará echo de todo lo que reciba\n";
    std::cout << "Presiona Ctrl+C para salir\n\n";
    // Configurar CPU
    cpu.PC = 0x8000;
    while (running) {
        // Ejecutar algunas instrucciones del programa 6502
        cpu.Execute(100, bus);
        // Dar un pequeño delay para no consumir 100% CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Modo 3: Demostración interactiva
void runInteractiveMode(std::shared_ptr<TcpSerial> tcpSerial) {
    std::cout << "\n=== Modo 3: Demostración interactiva ===\n";
    std::cout << "1. Configurando dispositivo...\n";
    
    // Configurar usando registros mapeados
    const uint16_t port = 12345;
    tcpSerial->write(0xFA04, port & 0xFF);
    tcpSerial->write(0xFA05, (port >> 8) & 0xFF);
    
    std::cout << "   Puerto configurado: " << port << "\n";
    
    // Mostrar estado inicial
    uint8_t status = tcpSerial->read(0xFA01);
    std::cout << "   Estado inicial: 0x" << std::hex << static_cast<int>(status) << std::dec << "\n";
    
    std::cout << "\n2. Iniciando servidor...\n";
    tcpSerial->write(0xFA06, 2);  // LISTEN
    std::cout << "   " << tcpSerial->getConnectionInfo() << "\n";
    
    std::cout << "\n3. Esperando conexión...\n";
    std::cout << "   Usa otro terminal: nc localhost 12345\n\n";
    
    // Esperar conexión
    while (running && !tcpSerial->isConnected()) {
        tcpSerial->read(0xFA01);  // Trigger polling
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (tcpSerial->isConnected()) {
        std::cout << "4. ¡Cliente conectado!\n";
        std::cout << "   " << tcpSerial->getConnectionInfo() << "\n";
        
        // Enviar mensaje de bienvenida
        const char* welcome = "Bienvenido al servidor serial TCP 6502!\n";
        std::cout << "\n5. Enviando mensaje de bienvenida...\n";
        for (size_t i = 0; welcome[i] != '\0'; ++i) {
            tcpSerial->write(0xFA00, welcome[i]);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        std::cout << "\n6. Modo echo activo (5 segundos)...\n";
        auto startTime = std::chrono::steady_clock::now();
        
        while (running) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
            if (elapsed.count() >= 5) break;
            
            // Leer estado
            status = tcpSerial->read(0xFA01);
            
            // Si hay dato disponible (bit 0)
            if (status & 0x01) {
                uint8_t byte = tcpSerial->read(0xFA00);
                std::cout << "   Recibido: '" << static_cast<char>(byte) << "'\n";
                
                // Echo
                tcpSerial->write(0xFA00, byte);
                std::cout << "   Enviado: '" << static_cast<char>(byte) << "'\n";
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        std::cout << "\n7. Desconectando...\n";
        tcpSerial->write(0xFA06, 0);  // DISCONNECT
        std::cout << "   " << tcpSerial->getConnectionInfo() << "\n";
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== Demo de TcpSerial para CPU 6502 ===\n";
    
    // Configurar manejador de señales
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Configuración
    SystemMap bus;
    CPU cpu;
    auto tcpSerial = std::make_shared<TcpSerial>();
    cpu.registerIODevice(tcpSerial);
    tcpSerial->initialize();
    cpu.PC = 0x8000;
    cpu.SP = 0xFD;
    
    // Determinar modo de operación
    int mode = 3;  // Por defecto: interactivo
    
    if (argc > 1) {
        mode = std::atoi(argv[1]);
    }
    
    std::cout << "\nModos disponibles:\n";
    std::cout << "  1 - Echo server usando API directa de C++\n";
    std::cout << "  2 - Echo server usando código 6502\n";
    std::cout << "  3 - Demostración interactiva (por defecto)\n";
    std::cout << "\nUso: " << argv[0] << " [modo]\n";
    std::cout << "Ejecutando modo " << mode << "...\n";
    
    switch (mode) {
        case 1:
            runDirectAPIMode(tcpSerial);
            break;
        case 2:
            run6502Mode(bus, cpu, tcpSerial);
            break;
        case 3:
        default:
            runInteractiveMode(tcpSerial);
            break;
    }
    
    // ===== RESUMEN =====
    std::cout << "\n=== RESUMEN ===\n";
    std::cout << "TcpSerial permite:\n";
    std::cout << "  1. Simular puerto serial ACIA 6551 sobre TCP\n";
    std::cout << "  2. Conectar como cliente o servidor\n";
    std::cout << "  3. Control mediante API directa o registros mapeados\n";
    std::cout << "  4. Integración completa con código 6502\n";
    std::cout << "\nRegistros mapeados (compatibles con ACIA 6551):\n";
    std::cout << "  0xFA00:      Datos (lectura/escritura)\n";
    std::cout << "  0xFA01:      Estado (RDR, TXE, IRQ)\n";
    std::cout << "  0xFA02:      Comando (configuración)\n";
    std::cout << "  0xFA03:      Control (baudios, etc.)\n";
    std::cout << "\nExtensiones TCP:\n";
    std::cout << "  0xFA04-0xFA05: Puerto TCP (little-endian)\n";
    std::cout << "  0xFA06:      Control de conexión (0=desconectar, 1=conectar, 2=escuchar)\n";
    std::cout << "  0xFA10-0xFA4F: Buffer para dirección IP/hostname (max 64 bytes)\n";
    
    std::cout << "\n=== Fin del demo ===\n";
    
    tcpSerial->cleanup();
    return 0;
}
