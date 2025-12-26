// wozmon_serial_demo.cpp
// Demo: WOZMON interactivo por TCP/Serial
// Permite conectar por telnet/nc y usar WOZMON como terminal serie

#include "cpu/cpu.hpp"
#include "mem/mem.hpp"
#include "devices/pia.hpp"
#include "devices/tcp_serial.hpp"
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

constexpr uint16_t ROM_START = 0x8000;
constexpr uint16_t RESET_VECTOR = 0xFFFC;
constexpr uint16_t WOZMON_ENTRY = 0xFFFA; // O el entrypoint real de WOZMON

// Cargar binario WOZMON (o usar array embebido si no existe)
bool loadBinaryFromFile(const std::string& filename, Mem& mem, uint16_t startAddr) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (startAddr + i >= 0x10000) break;
        mem[startAddr + i] = buffer[i];
    }
    return true;
}

int main(int argc, char* argv[]) {
    Mem mem;
    CPU cpu;
    auto pia = std::make_shared<PIA>();
    mem.Initialize();
    cpu.registerIODevice(pia);

    // --- TCP server setup (mínimo, sin clase TcpSerial) ---
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(3039);
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, 1) < 0) { perror("listen"); return 1; }
    std::cout << "[WOZMON Serial Demo] Esperando conexión TCP en puerto 3039...\n";
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) { perror("accept"); return 1; }
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    std::cout << "Conectado. Usa telnet/nc para interactuar. (Ctrl+C para salir)\n";

    // --- Cargar WOZMON ---
    std::string romPath = "../src/wozmon/rom.bin";
    if (argc > 1 && argv[1][0] != '\0') romPath = argv[1];
    else if (const char* envRom = std::getenv("WOZMON_BIN")) if (*envRom != '\0') romPath = envRom;
    if (!loadBinaryFromFile(romPath, mem, ROM_START)) {
        std::cerr << "No se pudo cargar WOZMON. Asegúrate de tener rom.bin o embebe el código.\n";
        return 1;
    }
    // Forzar vector de reset a $FE00 (WOZMON)
    mem[0xFFFC] = 0x00; // Low byte
    mem[0xFFFD] = 0xFE; // High byte
    cpu.Reset(mem);

    // --- Bucle principal: reenvía entrada/salida entre TCP y PIA ---
    char buf[256];
    while (true) {
        // Leer bytes entrantes del cliente TCP y ponerlos en el buffer de teclado de la PIA
        ssize_t n = read(client_fd, buf, sizeof(buf));
        if (n > 0) {
            for (ssize_t i = 0; i < n; ++i) {
                pia->pushKeyboardCharacter(buf[i]);
            }
        }
        // Ejecutar un lote de instrucciones
        cpu.Execute(10000, mem);
        // Enviar salida generada por WOZMON al cliente TCP
        std::string out = pia->getDisplayOutput();
        if (!out.empty()) {
            write(client_fd, out.data(), out.size());
            pia->clearDisplayOutput();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    close(client_fd);
    close(server_fd);
    return 0;
}
