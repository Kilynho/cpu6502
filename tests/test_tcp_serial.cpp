#include <gtest/gtest.h>
#include "cpu.hpp"
#include "mem.hpp"
#include "devices/tcp_serial.hpp"
#include <memory>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class TcpSerialTest : public testing::Test {
public:
    Mem mem;
    CPU cpu;
    std::shared_ptr<TcpSerial> tcpSerial;
    
    // Note: Using a fixed high port to minimize conflicts in CI/CD.
    // If port conflicts occur, tests will fail gracefully.
    const uint16_t TEST_PORT = 12345;

    virtual void SetUp() {
        cpu.Reset(mem);
        tcpSerial = std::make_shared<TcpSerial>();
        cpu.registerIODevice(tcpSerial);
        tcpSerial->initialize();
    }

    virtual void TearDown() {
        tcpSerial->disconnect();
        tcpSerial->cleanup();
        cpu.unregisterIODevice(tcpSerial);
    }
    
    // Función auxiliar para escribir una dirección en el buffer
    void writeAddress(const std::string& address) {
        for (size_t i = 0; i < address.length() && i < 64; ++i) {
            tcpSerial->write(0xFA10 + i, address[i]);
        }
        // Null terminator
        if (address.length() < 64) {
            tcpSerial->write(0xFA10 + address.length(), 0);
        }
    }
    
    // Crear un socket cliente para pruebas
    int createTestClient(const std::string& host, uint16_t port) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            return -1;
        }
        
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
        
        if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            close(sockfd);
            return -1;
        }
        
        return sockfd;
    }
};

// Test: Crear TcpSerial correctamente
TEST_F(TcpSerialTest, CreateDevice) {
    EXPECT_NE(tcpSerial, nullptr);
    EXPECT_EQ(tcpSerial->isConnected(), false);
}

// Test: Inicializar dispositivo
TEST_F(TcpSerialTest, Initialize) {
    // Ya inicializado en SetUp, pero probemos de nuevo
    EXPECT_TRUE(tcpSerial->initialize());
    
    // Estado inicial: transmisor listo
    uint8_t status = tcpSerial->read(0xFA01);
    EXPECT_EQ(status & 0x02, 0x02);  // TXE bit
}

// Test: Verificar que maneja las direcciones correctas
TEST_F(TcpSerialTest, HandlesAddresses) {
    EXPECT_TRUE(tcpSerial->handlesRead(0xFA00));   // DATA
    EXPECT_TRUE(tcpSerial->handlesRead(0xFA01));   // STATUS
    EXPECT_TRUE(tcpSerial->handlesRead(0xFA02));   // COMMAND
    EXPECT_TRUE(tcpSerial->handlesRead(0xFA03));   // CONTROL
    EXPECT_TRUE(tcpSerial->handlesRead(0xFA04));   // PORT_LO
    EXPECT_TRUE(tcpSerial->handlesRead(0xFA05));   // PORT_HI
    EXPECT_TRUE(tcpSerial->handlesRead(0xFA06));   // CONN_CONTROL
    EXPECT_TRUE(tcpSerial->handlesRead(0xFA10));   // ADDR_BUFFER_START
    EXPECT_TRUE(tcpSerial->handlesRead(0xFA4F));   // ADDR_BUFFER_END
    EXPECT_FALSE(tcpSerial->handlesRead(0xFA50));  // Fuera de rango
    EXPECT_FALSE(tcpSerial->handlesRead(0xF9FF));  // Fuera de rango
    
    EXPECT_TRUE(tcpSerial->handlesWrite(0xFA00));
    EXPECT_TRUE(tcpSerial->handlesWrite(0xFA06));
    EXPECT_FALSE(tcpSerial->handlesWrite(0xFA50));
}

// Test: Leer y escribir registros
TEST_F(TcpSerialTest, ReadWriteRegisters) {
    // Escribir puerto TCP
    tcpSerial->write(0xFA04, 0x39);  // Lo byte (12345 & 0xFF = 0x39)
    tcpSerial->write(0xFA05, 0x30);  // Hi byte (12345 >> 8 = 0x30)
    EXPECT_EQ(tcpSerial->read(0xFA04), 0x39);
    EXPECT_EQ(tcpSerial->read(0xFA05), 0x30);
    
    // Escribir comando
    tcpSerial->write(0xFA02, 0x42);
    EXPECT_EQ(tcpSerial->read(0xFA02), 0x42);
    
    // Escribir control
    tcpSerial->write(0xFA03, 0x1F);
    EXPECT_EQ(tcpSerial->read(0xFA03), 0x1F);
}

// Test: Modo escucha (servidor)
TEST_F(TcpSerialTest, ListenMode) {
    // Configurar puerto
    tcpSerial->write(0xFA04, TEST_PORT & 0xFF);
    tcpSerial->write(0xFA05, (TEST_PORT >> 8) & 0xFF);
    
    // Activar modo escucha
    tcpSerial->write(0xFA06, 2);  // LISTEN
    
    // Dar tiempo para que el socket se configure
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Verificar que está escuchando (no conectado todavía)
    EXPECT_FALSE(tcpSerial->isConnected());
    
    // Intentar conectar un cliente
    int clientSock = createTestClient("127.0.0.1", TEST_PORT);
    EXPECT_GE(clientSock, 0);
    
    if (clientSock >= 0) {
        // Dar tiempo para que acepte la conexión
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Leer estado para activar acceptConnection
        tcpSerial->read(0xFA01);
        
        // Ahora debería estar conectado
        EXPECT_TRUE(tcpSerial->isConnected());
        
        close(clientSock);
    }
}

// Test: Enviar y recibir datos usando API directa
TEST_F(TcpSerialTest, TransmitReceiveDirect) {
    // Configurar servidor
    EXPECT_TRUE(tcpSerial->listen(TEST_PORT));
    
    // Conectar cliente
    int clientSock = createTestClient("127.0.0.1", TEST_PORT);
    ASSERT_GE(clientSock, 0);
    
    // Dar tiempo para aceptar conexión
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tcpSerial->read(0xFA01);  // Trigger accept
    
    EXPECT_TRUE(tcpSerial->isConnected());
    
    // Enviar datos desde el dispositivo
    const char* message = "Hello";
    for (size_t i = 0; i < strlen(message); ++i) {
        EXPECT_TRUE(tcpSerial->transmitByte(message[i]));
    }
    
    // Dar tiempo para que lleguen los datos
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Leer desde el cliente
    char buffer[64] = {0};
    ssize_t received = recv(clientSock, buffer, sizeof(buffer), 0);
    EXPECT_GT(received, 0);
    EXPECT_STREQ(buffer, message);
    
    // Enviar datos al dispositivo
    const char* response = "World";
    send(clientSock, response, strlen(response), 0);
    
    // Dar tiempo para que lleguen los datos
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Leer desde el dispositivo
    EXPECT_TRUE(tcpSerial->dataAvailable());
    for (size_t i = 0; i < strlen(response); ++i) {
        uint8_t byte = tcpSerial->receiveByte();
        EXPECT_EQ(byte, response[i]);
    }
    
    close(clientSock);
}

// Test: Enviar datos usando registros mapeados
TEST_F(TcpSerialTest, TransmitUsingRegisters) {
    // Configurar servidor
    EXPECT_TRUE(tcpSerial->listen(TEST_PORT));
    
    // Conectar cliente
    int clientSock = createTestClient("127.0.0.1", TEST_PORT);
    ASSERT_GE(clientSock, 0);
    
    // Dar tiempo para aceptar conexión
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tcpSerial->read(0xFA01);  // Trigger accept
    
    // Enviar byte usando registro de datos
    tcpSerial->write(0xFA00, 'A');
    
    // Dar tiempo para que llegue
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Leer desde el cliente
    char buffer[2] = {0};
    ssize_t received = recv(clientSock, buffer, 1, 0);
    EXPECT_EQ(received, 1);
    EXPECT_EQ(buffer[0], 'A');
    
    close(clientSock);
}

// Test: Recibir datos usando registros mapeados
TEST_F(TcpSerialTest, ReceiveUsingRegisters) {
    // Configurar servidor
    EXPECT_TRUE(tcpSerial->listen(TEST_PORT));
    
    // Conectar cliente
    int clientSock = createTestClient("127.0.0.1", TEST_PORT);
    ASSERT_GE(clientSock, 0);
    
    // Dar tiempo para aceptar conexión
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tcpSerial->read(0xFA01);  // Trigger accept
    
    // Enviar byte desde el cliente
    char data = 'X';
    send(clientSock, &data, 1, 0);
    
    // Dar tiempo para que llegue
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Leer estado: debe indicar dato disponible
    uint8_t status = tcpSerial->read(0xFA01);
    EXPECT_EQ(status & 0x01, 0x01);  // RDR bit
    
    // Leer dato
    uint8_t received = tcpSerial->read(0xFA00);
    EXPECT_EQ(received, 'X');
    
    close(clientSock);
}

// Test: Desconectar
TEST_F(TcpSerialTest, Disconnect) {
    EXPECT_TRUE(tcpSerial->listen(TEST_PORT));
    
    int clientSock = createTestClient("127.0.0.1", TEST_PORT);
    ASSERT_GE(clientSock, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tcpSerial->read(0xFA01);  // Trigger accept
    
    EXPECT_TRUE(tcpSerial->isConnected());
    
    // Desconectar usando API
    tcpSerial->disconnect();
    EXPECT_FALSE(tcpSerial->isConnected());
    
    close(clientSock);
}

// Test: Desconectar usando registro de control
TEST_F(TcpSerialTest, DisconnectUsingRegister) {
    EXPECT_TRUE(tcpSerial->listen(TEST_PORT));
    
    int clientSock = createTestClient("127.0.0.1", TEST_PORT);
    ASSERT_GE(clientSock, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tcpSerial->read(0xFA01);  // Trigger accept
    
    EXPECT_TRUE(tcpSerial->isConnected());
    
    // Desconectar usando registro
    tcpSerial->write(0xFA06, 0);  // DISCONNECT
    EXPECT_FALSE(tcpSerial->isConnected());
    
    close(clientSock);
}

// Test: Buffer de dirección
TEST_F(TcpSerialTest, AddressBuffer) {
    std::string address = "192.168.1.1:8080";
    writeAddress(address);
    
    // Leer de vuelta
    for (size_t i = 0; i < address.length(); ++i) {
        EXPECT_EQ(tcpSerial->read(0xFA10 + i), address[i]);
    }
}

// Test: Estado sin conexión
TEST_F(TcpSerialTest, StatusDisconnected) {
    uint8_t status = tcpSerial->read(0xFA01);
    
    // Sin conexión: TXE debe estar activo (buffer listo), pero no RDR ni IRQ
    EXPECT_EQ(status & 0x01, 0x00);  // RDR bit off
    EXPECT_EQ(status & 0x02, 0x02);  // TXE bit on (transmisor listo)
    EXPECT_EQ(status & 0x80, 0x00);  // IRQ bit off
}

// Test: Información de conexión
TEST_F(TcpSerialTest, ConnectionInfo) {
    std::string info = tcpSerial->getConnectionInfo();
    EXPECT_NE(info.find("Not connected"), std::string::npos);
    
    tcpSerial->listen(TEST_PORT);
    info = tcpSerial->getConnectionInfo();
    EXPECT_NE(info.find("Listening"), std::string::npos);
}

// Test: Múltiples bytes en secuencia
TEST_F(TcpSerialTest, MultipleBytes) {
    EXPECT_TRUE(tcpSerial->listen(TEST_PORT));
    
    int clientSock = createTestClient("127.0.0.1", TEST_PORT);
    ASSERT_GE(clientSock, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tcpSerial->read(0xFA01);  // Trigger accept
    
    // Enviar múltiples bytes
    const char* message = "Testing123";
    send(clientSock, message, strlen(message), 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Leer todos los bytes
    for (size_t i = 0; i < strlen(message); ++i) {
        EXPECT_TRUE(tcpSerial->dataAvailable());
        uint8_t byte = tcpSerial->receiveByte();
        EXPECT_EQ(byte, message[i]);
    }
    
    close(clientSock);
}

// Test: Integración con CPU
TEST_F(TcpSerialTest, CPUIntegration) {
    // Configurar servidor en puerto específico
    tcpSerial->write(0xFA04, TEST_PORT & 0xFF);
    tcpSerial->write(0xFA05, (TEST_PORT >> 8) & 0xFF);
    tcpSerial->write(0xFA06, 2);  // LISTEN
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    int clientSock = createTestClient("127.0.0.1", TEST_PORT);
    ASSERT_GE(clientSock, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Crear un pequeño programa que lee del serial y escribe en memoria
    // LDA $FA01 (status), LDA $FA00 (data), STA $0200, BRK
    mem[0x8000] = 0xAD;  // LDA absolute
    mem[0x8001] = 0x01;
    mem[0x8002] = 0xFA;
    mem[0x8003] = 0xAD;  // LDA absolute
    mem[0x8004] = 0x00;
    mem[0x8005] = 0xFA;
    mem[0x8006] = 0x8D;  // STA absolute
    mem[0x8007] = 0x00;
    mem[0x8008] = 0x02;
    mem[0x8009] = 0x00;  // BRK
    
    // Enviar un byte desde el cliente
    char data = 'Z';
    send(clientSock, &data, 1, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Ejecutar programa
    cpu.PC = 0x8000;
    cpu.Execute(20, mem);
    
    // Verificar que el byte se copió a memoria
    EXPECT_EQ(mem[0x0200], 'Z');
    
    close(clientSock);
}
