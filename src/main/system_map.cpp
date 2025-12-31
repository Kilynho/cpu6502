#include "system_map.hpp"
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <algorithm>

// Stub genérico para dispositivos no implementados
template<typename T = void>
class DeviceStub {
public:
    uint8_t read(uint16_t) { return 0xFF; }
    void write(uint16_t, uint8_t) {}
};

constexpr size_t RAM_SIZE = 256 * 1024; // 256K
constexpr size_t ROM_SIZE = 32 * 1024;  // 32K
constexpr uint16_t RAM_BASE = 0x0000;
constexpr uint16_t ROM_BASE = 0x8000;
constexpr uint16_t ROM_END  = 0xFFFF;
constexpr uint16_t AUDIO_BASE = 0x6000;
constexpr uint16_t AUDIO_END  = 0x6FFF;
constexpr uint16_t SERIAL_BASE = 0x5000;
constexpr uint16_t SERIAL_END  = 0x5004;

SystemMap::SystemMap() {
    ram = new uint8_t[RAM_SIZE]();
    rom = new uint8_t[ROM_SIZE]();
    c64io = std::make_unique<C64IO>();
    audio = std::make_unique<DeviceStub<>>();
    serial = std::make_unique<DeviceStub<>>();
}

SystemMap::~SystemMap() {
    delete[] ram;
    delete[] rom;
}

uint8_t SystemMap::read(uint16_t address) {
    for (auto& dev : externalDevices) {
        if (dev && dev->handlesRead(address)) {
            return dev->read(address);
        }
    }
    // C64IO (pantalla/teclado) - check before ROM for WOZMON I/O addresses
    if (c64io->handlesRead(address)) {
        return c64io->read(address);
    }
    // RAM $0000-$7FFF (32K visibles, resto para expansión)
    if (address < ROM_BASE) {
        return ram[address % RAM_SIZE];
    }
    // ROM $8000-$FFFF (32K)
    if (address >= ROM_BASE && address <= ROM_END) {
        return rom[address - ROM_BASE];
    }
    // Serial $5000-$5004
    if (address >= SERIAL_BASE && address <= SERIAL_END) {
        return serial->read(address);
    }
    // Audio $6000-$6FFF
    if (address >= AUDIO_BASE && address <= AUDIO_END) {
        return audio->read(address);
    }
    // Por defecto
    return 0xFF;
}

void SystemMap::write(uint16_t address, uint8_t value) {
    for (auto& dev : externalDevices) {
        if (dev && dev->handlesWrite(address)) {
            dev->write(address, value);
            return;
        }
    }
    // C64IO (pantalla/teclado) - check before ROM for WOZMON I/O addresses
    if (c64io->handlesWrite(address)) {
        c64io->write(address, value);
        return;
    }
    // RAM $0000-$7FFF
    if (address < ROM_BASE) {
        ram[address % RAM_SIZE] = value;
        return;
    }
    // ROM $8000-$FFFF (permitir escritura para pruebas/carga manual)
    if (address >= ROM_BASE && address <= ROM_END) {
        rom[address - ROM_BASE] = value;
        return;
    }
    // Serial $5000-$5004
    if (address >= SERIAL_BASE && address <= SERIAL_END) {
        serial->write(address, value);
        return;
    }
    // Audio $6000-$6FFF
    if (address >= AUDIO_BASE && address <= AUDIO_END) {
        audio->write(address, value);
        return;
    }
    // Ignorar otras escrituras
}

C64IO& SystemMap::getC64IO() {
    return *c64io;
}

void SystemMap::loadROM(const uint8_t* data, size_t size) {
    if (size > ROM_SIZE) throw std::runtime_error("ROM demasiado grande");
    std::memcpy(rom, data, size);
}

void SystemMap::clearRAM() {
    std::memset(ram, 0, RAM_SIZE);
}

void SystemMap::registerIODevice(std::shared_ptr<IODevice> device) {
    if (!device) return;
    auto it = std::find(externalDevices.begin(), externalDevices.end(), device);
    if (it == externalDevices.end()) {
        externalDevices.push_back(device);
    }
}

void SystemMap::unregisterIODevice(std::shared_ptr<IODevice> device) {
    if (!device) return;
    auto it = std::remove(externalDevices.begin(), externalDevices.end(), device);
    if (it != externalDevices.end()) {
        externalDevices.erase(it, externalDevices.end());
    }
}
