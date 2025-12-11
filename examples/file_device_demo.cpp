#include "cpu.hpp"
#include "mem.hpp"
#include "devices/file_device.hpp"
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>

/**
 * Ejemplo de uso de FileDevice para cargar y guardar binarios
 * 
 * Este programa demuestra cómo:
 * 1. Crear y usar un FileDevice
 * 2. Cargar un binario desde un archivo usando la API directa
 * 3. Ejecutar el código cargado
 * 4. Guardar datos de memoria a un archivo
 * 5. Usar los registros mapeados en memoria para operaciones de E/S
 */

// Función auxiliar para crear un programa de ejemplo
void createSampleProgram(const std::string& filename) {
    std::vector<uint8_t> program = {
        0xA9, 0x48,        // LDA #$48 ('H')
        0x8D, 0x00, 0x02,  // STA $0200
        0xA9, 0x65,        // LDA #$65 ('e')
        0x8D, 0x01, 0x02,  // STA $0201
        0xA9, 0x6C,        // LDA #$6C ('l')
        0x8D, 0x02, 0x02,  // STA $0202
        0x8D, 0x03, 0x02,  // STA $0203
        0xA9, 0x6F,        // LDA #$6F ('o')
        0x8D, 0x04, 0x02,  // STA $0204
        0x00               // BRK
    };
    
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(program.data()), program.size());
        file.close();
        std::cout << "Programa de ejemplo creado: " << filename << "\n";
    } else {
        std::cerr << "Error al crear el archivo de ejemplo\n";
    }
}

// Función para mostrar el contenido de memoria como texto
void displayMemoryAsText(const Mem& mem, uint16_t start, uint16_t length) {
    std::cout << "Contenido de memoria en 0x" << std::hex << start << ": ";
    for (uint16_t i = 0; i < length; ++i) {
        char c = static_cast<char>(mem[start + i]);
        if (c >= 32 && c <= 126) {  // Caracteres imprimibles
            std::cout << c;
        } else {
            std::cout << '.';
        }
    }
    std::cout << std::dec << "\n";
}

int main() {
    std::cout << "=== Demo de FileDevice para CPU 6502 ===\n\n";
    
    // Configuración
    Mem mem;
    CPU cpu;
    auto fileDevice = std::make_shared<FileDevice>(&mem);
    
    cpu.Reset(mem);
    cpu.registerIODevice(fileDevice);
    
    const std::string programFile = "/tmp/sample_program.bin";
    const std::string outputFile = "/tmp/output_data.bin";
    
    // ===== PARTE 1: Crear y cargar un programa =====
    std::cout << "PARTE 1: Cargar y ejecutar un programa\n";
    std::cout << "---------------------------------------\n";
    
    // Crear un programa de ejemplo
    createSampleProgram(programFile);
    
    // Método 1: Cargar usando la API directa
    std::cout << "\nCargando programa usando API directa...\n";
    if (fileDevice->loadBinary(programFile, 0x8000)) {
        std::cout << "Programa cargado exitosamente en 0x8000\n";
    } else {
        std::cerr << "Error al cargar el programa\n";
        return 1;
    }
    
    // Ejecutar el programa
    std::cout << "\nEjecutando programa...\n";
    cpu.PC = 0x8000;
    cpu.Execute(100, mem);
    
    // Mostrar el resultado
    displayMemoryAsText(mem, 0x0200, 5);
    
    // ===== PARTE 2: Guardar datos de memoria =====
    std::cout << "\nPARTE 2: Guardar datos de memoria a un archivo\n";
    std::cout << "-----------------------------------------------\n";
    
    // Guardar los datos usando la API directa
    std::cout << "\nGuardando datos usando API directa...\n";
    if (fileDevice->saveBinary(outputFile, 0x0200, 5)) {
        std::cout << "Datos guardados exitosamente en " << outputFile << "\n";
    } else {
        std::cerr << "Error al guardar datos\n";
    }
    
    // ===== PARTE 3: Uso de registros mapeados en memoria =====
    std::cout << "\nPARTE 3: Uso de registros mapeados en memoria\n";
    std::cout << "----------------------------------------------\n";
    
    // Limpiar memoria para demostración
    for (uint16_t i = 0; i < 5; ++i) {
        mem[0x9000 + i] = 0;
    }
    
    // Configurar nombre de archivo en el buffer (0xFE10-0xFE4F)
    std::cout << "\nConfigurando nombre de archivo en registros...\n";
    const std::string filename = outputFile;
    for (size_t i = 0; i < filename.length() && i < 64; ++i) {
        fileDevice->write(0xFE10 + i, filename[i]);
    }
    fileDevice->write(0xFE10 + filename.length(), 0);  // Null terminator
    
    // Configurar dirección de inicio (0x9000)
    fileDevice->write(0xFE01, 0x00);  // Byte bajo
    fileDevice->write(0xFE02, 0x90);  // Byte alto
    
    // Configurar longitud (5 bytes)
    fileDevice->write(0xFE03, 0x05);  // Byte bajo
    fileDevice->write(0xFE04, 0x00);  // Byte alto
    
    // Ejecutar operación LOAD (escribir 1 en registro de control)
    std::cout << "Ejecutando operación LOAD mediante registro de control...\n";
    fileDevice->write(0xFE00, 1);  // 1 = LOAD
    
    // Verificar estado
    uint8_t status = fileDevice->read(0xFE05);
    if (status == 0) {
        std::cout << "Operación LOAD exitosa\n";
        displayMemoryAsText(mem, 0x9000, 5);
    } else {
        std::cout << "Error en operación LOAD (status = " << static_cast<int>(status) << ")\n";
    }
    
    // ===== PARTE 4: Demostración de SAVE usando registros =====
    std::cout << "\nPARTE 4: Guardar usando registros mapeados\n";
    std::cout << "-------------------------------------------\n";
    
    // Escribir datos nuevos en memoria
    const char* message = "6502!";
    for (int i = 0; i < 5; ++i) {
        mem[0xA000 + i] = message[i];
    }
    
    std::cout << "\nDatos en memoria a guardar: ";
    displayMemoryAsText(mem, 0xA000, 5);
    
    // Configurar registros para guardar
    const std::string saveFile = "/tmp/saved_message.bin";
    for (size_t i = 0; i < saveFile.length() && i < 64; ++i) {
        fileDevice->write(0xFE10 + i, saveFile[i]);
    }
    fileDevice->write(0xFE10 + saveFile.length(), 0);
    
    // Configurar dirección de inicio (0xA000)
    fileDevice->write(0xFE01, 0x00);
    fileDevice->write(0xFE02, 0xA0);
    
    // Configurar longitud (5 bytes)
    fileDevice->write(0xFE03, 0x05);
    fileDevice->write(0xFE04, 0x00);
    
    // Ejecutar operación SAVE (escribir 2 en registro de control)
    std::cout << "Ejecutando operación SAVE mediante registro de control...\n";
    fileDevice->write(0xFE00, 2);  // 2 = SAVE
    
    // Verificar estado
    status = fileDevice->read(0xFE05);
    if (status == 0) {
        std::cout << "Operación SAVE exitosa\n";
        std::cout << "Archivo guardado: " << saveFile << "\n";
    } else {
        std::cout << "Error en operación SAVE (status = " << static_cast<int>(status) << ")\n";
    }
    
    // ===== RESUMEN =====
    std::cout << "\n=== RESUMEN ===\n";
    std::cout << "FileDevice permite:\n";
    std::cout << "  1. Cargar binarios desde archivos del host\n";
    std::cout << "  2. Guardar bloques de memoria a archivos\n";
    std::cout << "  3. Control mediante API directa o registros mapeados\n";
    std::cout << "\nRegistros mapeados:\n";
    std::cout << "  0xFE00:      Control (0=nada, 1=cargar, 2=guardar)\n";
    std::cout << "  0xFE01-0xFE02: Dirección de inicio (little-endian)\n";
    std::cout << "  0xFE03-0xFE04: Longitud (little-endian)\n";
    std::cout << "  0xFE05:      Estado (0=éxito, 1=error)\n";
    std::cout << "  0xFE10-0xFE4F: Nombre de archivo (max 64 bytes)\n";
    
    std::cout << "\n=== Fin del demo ===\n";
    
    return 0;
}
