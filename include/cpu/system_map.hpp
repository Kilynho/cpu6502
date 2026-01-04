#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "c64_io.hpp"
#include "io_device.hpp"

// Forward declarations for stubs
template<typename T> class DeviceStub;

class SystemMap {
public:
    SystemMap();
    ~SystemMap();

    // Acceso a memoria
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);

    // Registro de dispositivos externos (ej. TextScreen)
    void registerIODevice(std::shared_ptr<IODevice> device);
    void unregisterIODevice(std::shared_ptr<IODevice> device);

    // Acceso directo a dispositivos
    C64IO& getC64IO();
    // ...otros getters para audio/serial si se desea...

    // Métodos para cargar ROM, limpiar RAM, etc.
    void loadROM(const uint8_t* data, size_t size);
    void clearRAM();

private:
    uint8_t* ram;   // 256K RAM
    uint8_t* rom;   // 32K ROM
    std::unique_ptr<C64IO> c64io;
    std::unique_ptr<DeviceStub<void>> audio;
    std::unique_ptr<DeviceStub<void>> serial;
    std::vector<std::shared_ptr<IODevice>> externalDevices;
};
