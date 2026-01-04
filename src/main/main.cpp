#include "cpu/system_map.hpp"
#include "cpu.hpp"
#include "logger.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <atomic>
#include <csignal>

static std::atomic<bool> g_running{true};

// Prototipo de función para cargar un archivo binario en un buffer
bool loadBinary(const char* filename, uint8_t* dest, size_t maxSize) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    file.read(reinterpret_cast<char*>(dest), maxSize);
    return file.gcount() > 0;
}

int main(int argc, char* argv[]) {
    // Configurar logger al inicio
    auto& logger = util::Logger::GetInstance();
    logger.SetLogFile("build/logs/cpu.log", 10 * 1024 * 1024, 5);
    logger.SetLevel(util::LogLevel::INFO);
    
    SystemMap sys;
    CPU cpu;

    std::string romPath;
    bool instructionDump = false;
    bool rawInput = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--instr-dump" || arg == "--instruction-dump") {
            instructionDump = true;
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: emulator [ROM_PATH] [--instr-dump]\n";
            return 0;
        } else if (arg == "--raw-input") {
            rawInput = true;
        } else if (romPath.empty()) {
            romPath = arg;
        } else {
            std::cerr << "Argumento desconocido ignorado: " << arg << "\n";
        }
    }

    if (instructionDump) {
        cpu.setInstructionDumpEnabled(true);
        std::cout << "Instruction dump logging enabled\n";
    }

    if (!romPath.empty()) {
        std::ifstream romfile(romPath, std::ios::binary);
        if (romfile) {
            uint8_t rombuf[32 * 1024] = {0};
            romfile.read(reinterpret_cast<char*>(rombuf), sizeof(rombuf));
            sys.loadROM(rombuf, romfile.gcount());
            std::cout << "ROM cargada: " << romfile.gcount() << " bytes\n";
        } else {
            std::cerr << "No se pudo abrir la ROM: " << romPath << "\n";
        }
    }

    sys.clearRAM();

    // Inicializar CPU (Wozmon monitor siempre arranca en 0xFE00 según el vector en $FFFC/$FFFD)
    uint16_t resetVec = 0xFE00;
    cpu.PC = resetVec;
    cpu.SP = 0xFD;
    // Set interrupt-disable flag on reset
    cpu.I = 1;

    // Leave stdin in blocking mode by default. If rawInput is requested,
    // we'll switch the terminal to raw mode before starting the input thread.

    // use global atomic so signal handler can stop the loop

    // Save terminal state for restoration if we enable raw mode
    struct termios origTermios{};
    bool termSaved = false;

    if (rawInput) {
        // Check if stdin is a TTY and we can set raw mode
        if (!isatty(STDIN_FILENO)) {
            std::cerr << "[WARN] --raw-input solicitado pero stdin no es un terminal interactivo. Usando modo estándar.\n";
            rawInput = false;
        } else if (tcgetattr(STDIN_FILENO, &origTermios) == 0) {
            termSaved = true;
            struct termios raw = origTermios;
            raw.c_lflag &= ~(ECHO | ICANON);
            raw.c_cc[VMIN] = 1;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        } else {
            std::cerr << "[WARN] No se pudo activar raw mode en este terminal. Usando modo estándar.\n";
            rawInput = false;
        }
    }

    // Thread to read stdin in background and fill keyboard buffer
    std::thread inputThread([&sys, rawInput, termSaved, origTermios]() mutable {
        if (rawInput) {
            char c;
            std::string lineBuf;
            while (g_running.load()) {
                ssize_t n = read(STDIN_FILENO, &c, 1);
                if (n <= 0) continue;
                // If user pressed Ctrl+C (ASCII 3), stop the emulator
                if (c == 0x03) {
                    g_running.store(false);
                    break;
                }
                // Handle backspace/delete locally (do not notify emulator yet)
                if (c == 0x7f || c == 0x08) {
                    if (!lineBuf.empty()) {
                        lineBuf.pop_back();
                    }
                    continue;
                }
                // Enter key: push full line characters then CR (no local echo)
                if (c == '\r' || c == '\n') {
                    for (char ch : lineBuf) sys.getC64IO().pushInput(ch);
                    sys.getC64IO().pushInput('\r');
                    lineBuf.clear();
                    continue;
                }
                // Regular character: store locally; send only on Enter
                lineBuf.push_back(c);
            }
        } else {
            std::string line;
            while (g_running.load()) {
                        if (std::getline(std::cin, line)) {
                            // Trim whitespace
                            auto trim = [](std::string s) {
                                size_t a = s.find_first_not_of(" \t\r\n");
                                if (a == std::string::npos) return std::string();
                                size_t b = s.find_last_not_of(" \t\r\n");
                                return s.substr(a, b - a + 1);
                            };
                            std::string t = trim(line);
                            // Recognize local exit commands
                            if (t == ".quit" || t == ".exit" || t == ":q" || t == "BYE" || t == "bye") {
                                g_running.store(false);
                                break;
                            }
                            for (char c : line) sys.getC64IO().pushInput(c);
                            sys.getC64IO().pushInput('\r');
                        } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
        }
    });

    // Setup signal handler to stop cleanly (works in both modes)
    std::signal(SIGINT, [](int){
        g_running.store(false);
        // Also close std::cin to unblock getline in standard mode
        fclose(stdin);
    });


    // Bucle principal: ejecuta instrucciones y gestiona salida
    uint64_t instrCount = 0;
    while (g_running.load()) {
        // Ejecutar instrucciones
        cpu.Execute(100, sys);
        instrCount += 100;

        // Simular reloj (ajustar para velocidad realista)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Signal shutdown: stop input thread and restore terminal if needed
    g_running.store(false);
    if (inputThread.joinable()) inputThread.join();

    if (rawInput && termSaved) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
    }

    return 0;
}
