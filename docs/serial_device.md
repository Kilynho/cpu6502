# TcpSerial: Serial Communication over TCP/IP

## Introducción

El sistema `TcpSerial` simula un puerto serial ACIA 6551 utilizando sockets TCP/IP, permitiendo al emulador 6502 comunicarse con programas externos a través de la red. Esta funcionalidad es esencial para:

- Comunicación serial bidireccional entre el emulador y aplicaciones externas
- Simulación de modems y terminales clásicos
- Integración con herramientas de desarrollo modernas (netcat, telnet, etc.)
- Creación de servidores y clientes TCP desde código 6502
- Debugging y monitoreo remoto del emulador

## Arquitectura

### Jerarquía de Clases

```
IODevice (interfaz base)
    └── SerialDevice (interfaz de comunicación serial)
            └── TcpSerial (implementación sobre TCP/IP)
```

### SerialDevice Interface

Interfaz base que define las operaciones de comunicación serial:

```cpp
class SerialDevice : public IODevice {
public:
    // Inicializar el dispositivo
    virtual bool initialize() = 0;
    
    // Conectar a una dirección
    virtual bool connect(const std::string& address) = 0;
    
    // Desconectar
    virtual void disconnect() = 0;
    
    // Verificar si hay datos disponibles
    virtual bool dataAvailable() const = 0;
    
    // Recibir/transmitir bytes
    virtual uint8_t receiveByte() = 0;
    virtual bool transmitByte(uint8_t data) = 0;
    
    // Verificar estado de conexión
    virtual bool isConnected() const = 0;
    
    // Limpiar recursos
    virtual void cleanup() = 0;
};
```

### TcpSerial Implementation

`TcpSerial` implementa `SerialDevice` y proporciona:

1. **API Directa C++**: Control programático completo
2. **Registros ACIA 6551**: Compatibilidad con código 6502 clásico
3. **Extensiones TCP**: Registros adicionales para configuración de red

## Registros Mapeados en Memoria

TcpSerial mapea registros compatibles con ACIA 6551 en el rango `0xFA00-0xFA4F`:

### Registros ACIA Estándar

| Dirección | Nombre | Tipo | Descripción |
|-----------|--------|------|-------------|
| `0xFA00` | DATA | W/R | Registro de datos (transmitir/recibir) |
| `0xFA01` | STATUS | R | Registro de estado |
| `0xFA02` | COMMAND | W/R | Registro de comando |
| `0xFA03` | CONTROL | W/R | Registro de control |

### Extensiones TCP

| Dirección | Nombre | Tipo | Descripción |
|-----------|--------|------|-------------|
| `0xFA04` | PORT_LO | W/R | Byte bajo del puerto TCP |
| `0xFA05` | PORT_HI | W/R | Byte alto del puerto TCP |
| `0xFA06` | CONN_CONTROL | W | Control de conexión |
| `0xFA10-0xFA4F` | ADDR_BUFFER | W/R | Buffer para dirección IP/hostname (64 bytes) |

### Registro de Datos (0xFA00)

- **Lectura**: Obtiene el próximo byte recibido del buffer
- **Escritura**: Envía un byte por la conexión TCP

### Registro de Estado (0xFA01)

Bits del registro de estado:

| Bit | Nombre | Descripción |
|-----|--------|-------------|
| 0 | RDR (Receive Data Ready) | `1` = Dato recibido disponible |
| 1 | TXE (Transmit Empty) | `1` = Buffer de transmisión vacío |
| 7 | IRQ (Interrupt Request) | `1` = Interrupción pendiente (dato recibido) |

### Registro de Control de Conexión (0xFA06)

Valores posibles:

| Valor | Operación | Descripción |
|-------|-----------|-------------|
| 0 | DISCONNECT | Cerrar conexión actual |
| 1 | CONNECT | Conectar como cliente TCP |
| 2 | LISTEN | Escuchar como servidor TCP |

**Nota**: La operación se ejecuta automáticamente al escribir en este registro.

## Modos de Operación

### Modo Cliente

Conectar a un servidor TCP remoto:

```cpp
auto tcpSerial = std::make_shared<TcpSerial>();
tcpSerial->initialize();

// Método 1: API directa
tcpSerial->connect("localhost:8080");

// Método 2: Usando registros (desde código 6502)
// Escribir dirección en buffer
tcpSerial->write(0xFA10, 'l');  // localhost:8080
tcpSerial->write(0xFA11, 'o');
// ... (escribir toda la dirección)
tcpSerial->write(0xFA1F, 0);    // null terminator

// Configurar puerto (8080 = 0x1F90)
tcpSerial->write(0xFA04, 0x90); // Byte bajo
tcpSerial->write(0xFA05, 0x1F); // Byte alto

// Conectar
tcpSerial->write(0xFA06, 1);    // CONNECT
```

### Modo Servidor

Escuchar conexiones entrantes:

```cpp
auto tcpSerial = std::make_shared<TcpSerial>();
tcpSerial->initialize();

// Método 1: API directa
tcpSerial->listen(12345);

// Método 2: Usando registros
// Configurar puerto (12345 = 0x3039)
tcpSerial->write(0xFA04, 0x39);
tcpSerial->write(0xFA05, 0x30);

// Activar modo escucha
tcpSerial->write(0xFA06, 2);    // LISTEN
```

## Ejemplos de Uso

### Ejemplo 1: Echo Server en C++

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "devices/tcp_serial.hpp"

int main() {
    Mem mem;
    CPU cpu;
    auto tcpSerial = std::make_shared<TcpSerial>();
    
    cpu.Reset(mem);
    cpu.registerIODevice(tcpSerial);
    tcpSerial->initialize();
    
    // Escuchar en puerto 12345
    tcpSerial->listen(12345);
    std::cout << "Esperando conexión en puerto 12345...\n";
    
    // Loop de echo
    while (true) {
        if (tcpSerial->dataAvailable()) {
            uint8_t byte = tcpSerial->receiveByte();
            std::cout << "Recibido: " << (char)byte << "\n";
            tcpSerial->transmitByte(byte);  // Echo
        }
    }
    
    return 0;
}
```

### Ejemplo 2: Echo Server en Código 6502

```assembly
; Echo server en código 6502
; Lee un byte del serial, lo envía de vuelta

    ORG $8000

SERIAL_DATA    = $FA00
SERIAL_STATUS  = $FA01
SERIAL_PORT_LO = $FA04
SERIAL_PORT_HI = $FA05
SERIAL_CONN    = $FA06

STATUS_RDR     = %00000001  ; Bit 0: Dato recibido

init:
    ; Configurar puerto 12345 (0x3039)
    LDA #$39
    STA SERIAL_PORT_LO
    LDA #$30
    STA SERIAL_PORT_HI
    
    ; Activar modo escucha
    LDA #$02
    STA SERIAL_CONN

loop:
    ; Leer registro de estado
    LDA SERIAL_STATUS
    AND #STATUS_RDR        ; Verificar bit RDR
    BEQ loop               ; Si no hay dato, seguir esperando
    
    ; Leer byte recibido
    LDA SERIAL_DATA
    
    ; Enviar byte (echo)
    STA SERIAL_DATA
    
    JMP loop               ; Repetir

    END
```

### Ejemplo 3: Cliente Simple

```cpp
auto tcpSerial = std::make_shared<TcpSerial>();
tcpSerial->initialize();
tcpSerial->connect("localhost:8080");

// Enviar mensaje
const char* msg = "Hello from 6502!\n";
for (int i = 0; msg[i]; i++) {
    tcpSerial->transmitByte(msg[i]);
}

// Esperar respuesta
while (!tcpSerial->dataAvailable()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

while (tcpSerial->dataAvailable()) {
    char c = tcpSerial->receiveByte();
    std::cout << c;
}

tcpSerial->disconnect();
```

## Integración con Herramientas Externas

### Netcat (nc)

```bash
# Conectar al emulador como cliente
nc localhost 12345

# Servidor netcat (emulador como cliente)
nc -l 8080
```

### Telnet

```bash
telnet localhost 12345
```

### Python

```python
import socket

# Cliente Python
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 12345))
sock.send(b'Hello from Python!\n')
data = sock.recv(1024)
print(f'Received: {data.decode()}')
sock.close()
```

## Comandos de Demostración

El proyecto incluye un programa de demostración completo:

```bash
# Ejecutar demo interactivo (modo 3)
./build/tcp_serial_demo

# Echo server usando API de C++
./build/tcp_serial_demo 1

# Echo server usando código 6502
./build/tcp_serial_demo 2
```

Conectar desde otro terminal:
```bash
# Con netcat
echo "Hello 6502!" | nc localhost 12345

# Interactivo
nc localhost 12345
```

## Notas Técnicas

### Non-blocking I/O

TcpSerial utiliza sockets no bloqueantes para evitar bloquear la ejecución del emulador. Los datos se leen y escriben de forma asíncrona.

### Buffers Internos

- **Buffer de recepción**: Almacena bytes recibidos del socket
- **Buffer de transmisión**: Almacena bytes pendientes de envío
- El polling ocurre automáticamente durante las operaciones de lectura

### Manejo de Errores

- Conexiones cerradas se detectan automáticamente
- Los errores de socket se reportan pero no detienen el emulador
- El estado de conexión se actualiza en tiempo real

### Compatibilidad ACIA 6551

TcpSerial es compatible con programas escritos para ACIA 6551, pero simplifica algunos aspectos:

- Los registros de baudios y paridad son simulados (no afectan TCP)
- Las interrupciones se simulan mediante el bit IRQ
- El handshaking se maneja automáticamente por TCP

## Limitaciones Conocidas

1. **Un cliente a la vez**: El modo servidor solo acepta una conexión simultánea
2. **Sin control de flujo**: El control de flujo se delega a TCP
3. **Baudios simulados**: La velocidad real depende de la red, no del registro de control

## Casos de Uso

### Desarrollo y Debug

- Monitor remoto del emulador
- Carga de programas por red
- Logging remoto

### Simulación de Hardware Clásico

- Modems de acceso telefónico
- Terminales seriales
- Impresoras con interfaz serial

### Integración Moderna

- APIs REST simples
- Comunicación IoT
- Protocolos personalizados

## Referencias

- **ACIA 6551 Datasheet**: Motorola/MOS Technology
- **Documentación RFC**: TCP/IP Standards
- **Código fuente**: `src/devices/tcp_serial.cpp`
- **Ejemplo completo**: `examples/tcp_serial_demo.cpp`
- **Tests**: `tests/test_tcp_serial.cpp`

## Ver También

- [FileDevice](file_device.md) - Almacenamiento de archivos
- [AudioDevice](audio_device.md) - Generación de audio
- [TextScreen](video_device.md) - Pantalla de texto
