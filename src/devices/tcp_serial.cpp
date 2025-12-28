#include "tcp_serial.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

TcpSerial::TcpSerial() 
    : dataReg(0), statusReg(STATUS_TXE), commandReg(0), controlReg(0),
      tcpPort(0), connControl(0), socketFd(-1), clientFd(-1),
      initialized(false), connected(false), listening(false) {
    addressBuffer.resize(ADDR_BUFFER_SIZE, 0);
    static_assert(ADDR_BUFFER_SIZE == 64, "Address buffer must be 64 bytes as documented");
}

TcpSerial::~TcpSerial() {
    cleanup();
}

bool TcpSerial::initialize() {
    if (initialized) {
        return true;
    }
    
    // Inicializar registros
    statusReg = STATUS_TXE;  // Transmisor listo
    dataReg = 0;
    commandReg = 0;
    controlReg = 0;
    
    initialized = true;
    std::cout << "TcpSerial: Dispositivo inicializado\n";
    return true;
}

void TcpSerial::cleanup() {
    disconnect();
    initialized = false;
    std::cout << "TcpSerial: Dispositivo limpiado\n";
}

bool TcpSerial::handlesRead(uint16_t address) const {
    return (address >= DATA_REG && address <= CONN_CONTROL) ||
           (address >= ADDR_BUFFER_START && address <= ADDR_BUFFER_END);
}

bool TcpSerial::handlesWrite(uint16_t address) const {
    return (address >= DATA_REG && address <= CONN_CONTROL) ||
           (address >= ADDR_BUFFER_START && address <= ADDR_BUFFER_END);
}

uint8_t TcpSerial::read(uint16_t address) {
    // Leer datos del socket si están disponibles
    pollSocket();
    
    if (address == DATA_REG) {
        // Leer byte del buffer de recepción
        if (!receiveBuffer.empty()) {
            dataReg = receiveBuffer.front();
            receiveBuffer.pop();
            updateStatus();
        }
        return dataReg;
    } else if (address == STATUS_REG) {
        updateStatus();
        return statusReg;
    } else if (address == COMMAND_REG) {
        return commandReg;
    } else if (address == CONTROL_REG) {
        return controlReg;
    } else if (address == PORT_LO) {
        return tcpPort & 0xFF;
    } else if (address == PORT_HI) {
        return (tcpPort >> 8) & 0xFF;
    } else if (address == CONN_CONTROL) {
        return connControl;
    } else if (address >= ADDR_BUFFER_START && address <= ADDR_BUFFER_END) {
        uint16_t index = address - ADDR_BUFFER_START;
        return addressBuffer[index];
    }
    return 0;
}

void TcpSerial::write(uint16_t address, uint8_t value) {
    if (address == DATA_REG) {
        // Escribir byte para transmitir
        dataReg = value;
        transmitBuffer.push(value);
        flushTransmitBuffer();
        updateStatus();
    } else if (address == COMMAND_REG) {
        commandReg = value;
    } else if (address == CONTROL_REG) {
        controlReg = value;
    } else if (address == PORT_LO) {
        tcpPort = (tcpPort & 0xFF00) | value;
    } else if (address == PORT_HI) {
        tcpPort = (tcpPort & 0x00FF) | (static_cast<uint16_t>(value) << 8);
    } else if (address == CONN_CONTROL) {
        connControl = value;
        // Ejecutar operación de conexión (incluyendo disconnect que es 0)
        executeConnOperation();
    } else if (address >= ADDR_BUFFER_START && address <= ADDR_BUFFER_END) {
        updateAddressBuffer(address, value);
    }
}

void TcpSerial::updateAddressBuffer(uint16_t address, uint8_t value) {
    uint16_t index = address - ADDR_BUFFER_START;
    addressBuffer[index] = value;
}

std::string TcpSerial::getAddressFromBuffer() const {
    std::string address;
    for (uint8_t byte : addressBuffer) {
        if (byte == 0) break;  // Null terminator
        address += static_cast<char>(byte);
    }
    return address;
}

void TcpSerial::executeConnOperation() {
    ConnOperation op = static_cast<ConnOperation>(connControl);
    
    switch (op) {
        case ConnOperation::DISCONNECT:
            disconnect();
            break;
        case ConnOperation::CONNECT: {
            std::string addr = getAddressFromBuffer();
            if (!addr.empty()) {
                // Formato: "hostname:port" o solo usar tcpPort
                size_t colonPos = addr.find(':');
                std::string host;
                uint16_t port;
                
                if (colonPos != std::string::npos) {
                    host = addr.substr(0, colonPos);
                    port = std::stoi(addr.substr(colonPos + 1));
                } else {
                    host = addr;
                    port = tcpPort;
                }
                
                std::string fullAddr = host + ":" + std::to_string(port);
                connect(fullAddr);
            }
            break;
        }
        case ConnOperation::LISTEN:
            listen(tcpPort);
            break;
        default:
            std::cerr << "TcpSerial: Operación de conexión desconocida\n";
            break;
    }
    
    // Resetear registro de control después de ejecutar
    connControl = 0;
}

bool TcpSerial::connect(const std::string& address) {
    if (connected) {
        disconnect();
    }
    
    // Parse address (formato: "hostname:port")
    size_t colonPos = address.find(':');
    if (colonPos == std::string::npos) {
        std::cerr << "TcpSerial: Formato de dirección inválido (use hostname:port)\n";
        return false;
    }
    
    std::string hostname = address.substr(0, colonPos);
    uint16_t port = std::stoi(address.substr(colonPos + 1));
    
    // Crear socket
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        std::cerr << "TcpSerial: Error al crear socket: " << strerror(errno) << "\n";
        return false;
    }
    
    // Configurar socket como no bloqueante
    int flags = fcntl(socketFd, F_GETFL, 0);
    if (flags < 0) {
        std::cerr << "TcpSerial: Error al obtener flags del socket: " << strerror(errno) << "\n";
        close(socketFd);
        socketFd = -1;
        return false;
    }
    if (fcntl(socketFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "TcpSerial: Error al configurar socket no bloqueante: " << strerror(errno) << "\n";
        close(socketFd);
        socketFd = -1;
        return false;
    }
    
    // Resolver hostname
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string portStr = std::to_string(port);
    if (getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &result) != 0) {
        std::cerr << "TcpSerial: Error al resolver hostname: " << hostname << "\n";
        close(socketFd);
        socketFd = -1;
        return false;
    }
    
    // Intentar conectar
    int connectResult = ::connect(socketFd, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);
    
    if (connectResult < 0 && errno != EINPROGRESS) {
        std::cerr << "TcpSerial: Error al conectar: " << strerror(errno) << "\n";
        close(socketFd);
        socketFd = -1;
        return false;
    }
    
    connected = true;
    currentAddress = address;
    updateStatus();
    
    std::cout << "TcpSerial: Conectado a " << address << "\n";
    return true;
}

bool TcpSerial::listen(uint16_t port) {
    if (listening || connected) {
        disconnect();
    }
    
    // Crear socket servidor
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        std::cerr << "TcpSerial: Error al crear socket servidor: " << strerror(errno) << "\n";
        return false;
    }
    
    // Permitir reutilizar la dirección
    int opt = 1;
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Configurar dirección
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    // Bind
    if (bind(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "TcpSerial: Error en bind: " << strerror(errno) << "\n";
        close(socketFd);
        socketFd = -1;
        return false;
    }
    
    // Listen
    if (::listen(socketFd, 1) < 0) {
        std::cerr << "TcpSerial: Error en listen: " << strerror(errno) << "\n";
        close(socketFd);
        socketFd = -1;
        return false;
    }
    
    // Configurar socket como no bloqueante
    int flags = fcntl(socketFd, F_GETFL, 0);
    if (flags < 0) {
        std::cerr << "TcpSerial: Error al obtener flags del socket: " << strerror(errno) << "\n";
        close(socketFd);
        socketFd = -1;
        return false;
    }
    if (fcntl(socketFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "TcpSerial: Error al configurar socket no bloqueante: " << strerror(errno) << "\n";
        close(socketFd);
        socketFd = -1;
        return false;
    }
    
    listening = true;
    tcpPort = port;
    
    std::cout << "TcpSerial: Escuchando en puerto " << port << "\n";
    return true;
}

bool TcpSerial::acceptConnection() const {
    if (!listening || socketFd < 0) {
        return false;
    }
    
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    clientFd = accept(socketFd, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientFd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "TcpSerial: Error en accept: " << strerror(errno) << "\n";
        }
        return false;
    }
    
    // Configurar cliente como no bloqueante
    int flags = fcntl(clientFd, F_GETFL, 0);
    if (flags >= 0) {
        if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) < 0) {
            std::cerr << "TcpSerial: Error al configurar cliente no bloqueante: " << strerror(errno) << "\n";
            // No es fatal, continuar de todos modos
        }
    } else {
        std::cerr << "TcpSerial: Error al obtener flags del cliente: " << strerror(errno) << "\n";
        // No es fatal, continuar de todos modos
    }
    
    connected = true;
    listening = false;
    
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    currentAddress = std::string(clientIP) + ":" + std::to_string(ntohs(clientAddr.sin_port));
    
    std::cout << "TcpSerial: Cliente conectado desde " << currentAddress << "\n";
    
    updateStatus();
    return true;
}

void TcpSerial::disconnect() {
    closeSocket();
    
    // Limpiar buffers
    while (!receiveBuffer.empty()) receiveBuffer.pop();
    while (!transmitBuffer.empty()) transmitBuffer.pop();
    
    connected = false;
    listening = false;
    currentAddress.clear();
    
    updateStatus();
    std::cout << "TcpSerial: Desconectado\n";
}

void TcpSerial::closeSocket() {
    if (clientFd >= 0) {
        close(clientFd);
        clientFd = -1;
    }
    if (socketFd >= 0) {
        close(socketFd);
        socketFd = -1;
    }
}

bool TcpSerial::dataAvailable() const {
    pollSocket();  // Poll to get latest data
    return !receiveBuffer.empty();
}

uint8_t TcpSerial::receiveByte() {
    pollSocket();
    
    if (!receiveBuffer.empty()) {
        uint8_t byte = receiveBuffer.front();
        receiveBuffer.pop();
        return byte;
    }
    return 0;
}

bool TcpSerial::transmitByte(uint8_t data) {
    if (!connected) {
        return false;
    }
    
    int fd = (clientFd >= 0) ? clientFd : socketFd;
    if (fd < 0) {
        return false;
    }
    
    ssize_t sent = send(fd, &data, 1, 0);
    if (sent < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "TcpSerial: Error al enviar: " << strerror(errno) << "\n";
            return false;
        }
        // Si está bloqueado, guardar en buffer
        transmitBuffer.push(data);
        return true;
    }
    
    return sent == 1;
}

bool TcpSerial::isConnected() const {
    return connected;
}

void TcpSerial::pollSocket() const {
    if (listening && !connected) {
        // Intentar aceptar conexión pendiente
        acceptConnection();
        return;
    }
    
    if (!connected) {
        return;
    }
    
    int fd = (clientFd >= 0) ? clientFd : socketFd;
    if (fd < 0) {
        return;
    }
    
    // Leer datos disponibles
    uint8_t buffer[256];
    ssize_t received = recv(fd, buffer, sizeof(buffer), 0);
    
    if (received > 0) {
        // Agregar bytes al buffer de recepción
        for (ssize_t i = 0; i < received; ++i) {
            receiveBuffer.push(buffer[i]);
        }
        updateStatus();
    } else if (received == 0) {
        // Conexión cerrada por el otro extremo - marcar como desconectado
        std::cout << "TcpSerial: Conexión cerrada por el cliente\n";
        connected = false;
        updateStatus();
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        // Error real - marcar como desconectado
        std::cerr << "TcpSerial: Error al recibir: " << strerror(errno) << "\n";
        connected = false;
        updateStatus();
    }
}

void TcpSerial::flushTransmitBuffer() {
    if (!connected || transmitBuffer.empty()) {
        return;
    }
    
    int fd = (clientFd >= 0) ? clientFd : socketFd;
    if (fd < 0) {
        return;
    }
    
    // Intentar enviar bytes del buffer
    while (!transmitBuffer.empty()) {
        uint8_t byte = transmitBuffer.front();
        ssize_t sent = send(fd, &byte, 1, 0);
        
        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Socket lleno, dejar el resto en buffer
                break;
            }
            std::cerr << "TcpSerial: Error al enviar desde buffer: " << strerror(errno) << "\n";
            break;
        }
        
        if (sent == 1) {
            transmitBuffer.pop();
        }
    }
    
    updateStatus();
}

void TcpSerial::updateStatus() const {
    statusReg = 0;
    
    // Bit 0: Dato recibido disponible
    if (!receiveBuffer.empty()) {
        statusReg |= STATUS_RDR;
    }
    
    // Bit 1: Transmisor vacío (listo cuando el buffer de transmisión está vacío)
    if (transmitBuffer.empty()) {
        statusReg |= STATUS_TXE;
    }
    
    // Bit 7: IRQ (activar si hay datos recibidos)
    if (!receiveBuffer.empty()) {
        statusReg |= STATUS_IRQ;
    }
}

std::string TcpSerial::getConnectionInfo() const {
    if (connected) {
        return "Connected to: " + currentAddress;
    } else if (listening) {
        return "Listening on port: " + std::to_string(tcpPort);
    } else {
        return "Not connected";
    }
}
