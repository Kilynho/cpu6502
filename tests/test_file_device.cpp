#include <gtest/gtest.h>
#include "cpu.hpp"
#include "cpu/system_map.hpp"
#include "file_device.hpp"
#include <memory>
#include <fstream>
#include <vector>
#include <cstdio>

class FileDeviceTest : public testing::Test {
public:
    SystemMap bus;
    CPU cpu;
    std::shared_ptr<FileDevice> fileDevice;
    
    // Archivos de prueba
    const std::string testFile = "/tmp/test_binary.bin";
    const std::string testOutputFile = "/tmp/test_output.bin";

    virtual void SetUp() {
        cpu.PC = 0x8000;
        cpu.SP = 0xFD;
        cpu.A = cpu.X = cpu.Y = 0;
        cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
        fileDevice = std::make_shared<FileDevice>(&bus);
        cpu.registerIODevice(fileDevice);
        
        // Limpiar archivos de prueba anteriores
        std::remove(testFile.c_str());
        std::remove(testOutputFile.c_str());
    }

    virtual void TearDown() {
        cpu.unregisterIODevice(fileDevice);
        
        // Limpiar archivos de prueba
        std::remove(testFile.c_str());
        std::remove(testOutputFile.c_str());
    }
    
    // Función auxiliar para crear un archivo binario de prueba
    void createTestFile(const std::string& filename, const std::vector<uint8_t>& data) {
        std::ofstream file(filename, std::ios::binary);
        ASSERT_TRUE(file.is_open());
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
    }
    
    // Función auxiliar para leer un archivo binario
    std::vector<uint8_t> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) return {};
        
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        std::vector<uint8_t> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        file.close();
        
        return buffer;
    }
    
    // Función auxiliar para escribir un nombre de archivo en el buffer del dispositivo
    void writeFilename(const std::string& filename) {
        for (size_t i = 0; i < filename.length() && i < 64; ++i) {
            fileDevice->write(0xFE10 + i, filename[i]);
        }
        // Null terminator
        if (filename.length() < 64) {
            fileDevice->write(0xFE10 + filename.length(), 0);
        }
    }
};

// Test: Crear FileDevice correctamente
TEST_F(FileDeviceTest, CreateDevice) {
    EXPECT_NE(fileDevice, nullptr);
    EXPECT_EQ(fileDevice->getStatus(), 0);
}

// Test: Verificar que maneja las direcciones correctas para lectura
TEST_F(FileDeviceTest, HandlesReadAddresses) {
    EXPECT_TRUE(fileDevice->handlesRead(0xFE00));  // Control
    EXPECT_TRUE(fileDevice->handlesRead(0xFE01));  // Start addr lo
    EXPECT_TRUE(fileDevice->handlesRead(0xFE02));  // Start addr hi
    EXPECT_TRUE(fileDevice->handlesRead(0xFE03));  // Length lo
    EXPECT_TRUE(fileDevice->handlesRead(0xFE04));  // Length hi
    EXPECT_TRUE(fileDevice->handlesRead(0xFE05));  // Status
    EXPECT_TRUE(fileDevice->handlesRead(0xFE10));  // Filename start
    EXPECT_TRUE(fileDevice->handlesRead(0xFE4F));  // Filename end
    EXPECT_FALSE(fileDevice->handlesRead(0xFE50)); // Fuera de rango
    EXPECT_FALSE(fileDevice->handlesRead(0xFDFF)); // Fuera de rango
}

// Test: Verificar que maneja las direcciones correctas para escritura
TEST_F(FileDeviceTest, HandlesWriteAddresses) {
    EXPECT_TRUE(fileDevice->handlesWrite(0xFE00));
    EXPECT_TRUE(fileDevice->handlesWrite(0xFE05));
    EXPECT_TRUE(fileDevice->handlesWrite(0xFE10));
    EXPECT_FALSE(fileDevice->handlesWrite(0xFE50));
}

// Test: Escribir y leer registros del dispositivo
TEST_F(FileDeviceTest, ReadWriteRegisters) {
    // Escribir dirección de inicio
    fileDevice->write(0xFE01, 0x00);  // Lo byte
    fileDevice->write(0xFE02, 0x80);  // Hi byte
    EXPECT_EQ(fileDevice->read(0xFE01), 0x00);
    EXPECT_EQ(fileDevice->read(0xFE02), 0x80);
    
    // Escribir longitud
    fileDevice->write(0xFE03, 0x10);  // Lo byte
    fileDevice->write(0xFE04, 0x00);  // Hi byte
    EXPECT_EQ(fileDevice->read(0xFE03), 0x10);
    EXPECT_EQ(fileDevice->read(0xFE04), 0x00);
}

// Test: Cargar archivo binario usando la API directa
TEST_F(FileDeviceTest, LoadBinaryDirect) {
    // Crear archivo de prueba
    std::vector<uint8_t> testData = {0xA9, 0x42, 0x8D, 0x00, 0x02};
    createTestFile(testFile, testData);
    
    // Cargar el archivo
    EXPECT_TRUE(fileDevice->loadBinary(testFile, 0x8000));
    
    // Verificar que los datos se cargaron correctamente
    for (size_t i = 0; i < testData.size(); ++i) {
        EXPECT_EQ(bus.read(0x8000 + i), testData[i]);
    }
}

// Test: Guardar archivo binario usando la API directa
TEST_F(FileDeviceTest, SaveBinaryDirect) {
    // Escribir datos en memoria
    std::vector<uint8_t> testData = {0xEA, 0xEA, 0xEA, 0x4C, 0x00, 0x80};
    for (size_t i = 0; i < testData.size(); ++i) {
        bus.write(0x9000 + i, testData[i]);
    }
    
    // Guardar el archivo
    EXPECT_TRUE(fileDevice->saveBinary(testOutputFile, 0x9000, testData.size()));
    
    // Verificar que el archivo se creó y tiene el contenido correcto
    std::vector<uint8_t> fileContent = readFile(testOutputFile);
    EXPECT_EQ(fileContent.size(), testData.size());
    for (size_t i = 0; i < testData.size(); ++i) {
        EXPECT_EQ(fileContent[i], testData[i]);
    }
}

// Test: Verificar integridad después de cargar y guardar
TEST_F(FileDeviceTest, LoadSaveIntegrity) {
    // Crear datos de prueba
    std::vector<uint8_t> originalData;
    for (int i = 0; i < 256; ++i) {
        originalData.push_back(static_cast<uint8_t>(i));
    }
    
    // Guardar datos originales
    createTestFile(testFile, originalData);
    
    // Cargar en memoria
    EXPECT_TRUE(fileDevice->loadBinary(testFile, 0x8000));
    
    // Guardar desde memoria
    EXPECT_TRUE(fileDevice->saveBinary(testOutputFile, 0x8000, originalData.size()));
    
    // Leer el archivo guardado
    std::vector<uint8_t> savedData = readFile(testOutputFile);
    
    // Verificar integridad
    EXPECT_EQ(savedData.size(), originalData.size());
    for (size_t i = 0; i < originalData.size(); ++i) {
        EXPECT_EQ(savedData[i], originalData[i]);
    }
}

// Test: Cargar archivo usando registros mapeados en memoria
TEST_F(FileDeviceTest, LoadUsingMemoryMappedRegisters) {
    // Crear archivo de prueba
    std::vector<uint8_t> testData = {0x01, 0x02, 0x03, 0x04, 0x05};
    createTestFile(testFile, testData);
    
    writeFilename(testFile);
    fileDevice->write(0xFE01, 0x00);  // Start addr = 0x8000
    fileDevice->write(0xFE02, 0x80);
    // Ejecutar operación LOAD (escribir 1 en control)
    fileDevice->write(0xFE00, 1);
    // Verificar estado exitoso
    EXPECT_EQ(fileDevice->read(0xFE05), 0);
    // Verificar datos cargados
    for (size_t i = 0; i < testData.size(); ++i) {
        EXPECT_EQ(bus.read(0x8000 + i), testData[i]);
    }
}

// Test: Guardar archivo usando registros mapeados en memoria
TEST_F(FileDeviceTest, SaveUsingMemoryMappedRegisters) {
    // Escribir datos en memoria
    std::vector<uint8_t> testData = {0xAA, 0xBB, 0xCC, 0xDD};
    for (size_t i = 0; i < testData.size(); ++i) {
        bus.write(0x8500 + i, testData[i]);
    }
    
    // Configurar registros
    writeFilename(testOutputFile);
    fileDevice->write(0xFE01, 0x00);  // Start addr = 0x8500
    fileDevice->write(0xFE02, 0x85);
    fileDevice->write(0xFE03, 0x04);  // Length = 4
    fileDevice->write(0xFE04, 0x00);
    
    // Ejecutar operación SAVE (escribir 2 en control)
    fileDevice->write(0xFE00, 2);
    
    // Verificar estado exitoso
    EXPECT_EQ(fileDevice->read(0xFE05), 0);
    
    // Verificar archivo guardado
    std::vector<uint8_t> savedData = readFile(testOutputFile);
    EXPECT_EQ(savedData.size(), testData.size());
    for (size_t i = 0; i < testData.size(); ++i) {
        EXPECT_EQ(savedData[i], testData[i]);
    }
}

// Test: Error al intentar cargar archivo inexistente
TEST_F(FileDeviceTest, LoadNonexistentFileError) {
    EXPECT_FALSE(fileDevice->loadBinary("/tmp/does_not_exist.bin", 0x8000));
}

// Test: Verificar fileExists
TEST_F(FileDeviceTest, FileExists) {
    EXPECT_FALSE(fileDevice->fileExists(testFile));
    
    // Crear archivo
    std::vector<uint8_t> data = {0x00};
    createTestFile(testFile, data);
    
    EXPECT_TRUE(fileDevice->fileExists(testFile));
}

// Test: Nombre de archivo vacío produce error
TEST_F(FileDeviceTest, EmptyFilenameError) {
    // No escribir nombre de archivo
    fileDevice->write(0xFE01, 0x00);
    // Intentar cargar
    fileDevice->write(0xFE00, 1);
    // Verificar error
    EXPECT_EQ(fileDevice->read(0xFE05), 1);
}

// Test: Leer y escribir nombre de archivo en buffer
TEST_F(FileDeviceTest, FilenameBuffer) {
    std::string filename = "test.bin";
    writeFilename(filename);
    
    // Leer de vuelta el nombre
    for (size_t i = 0; i < filename.length(); ++i) {
        EXPECT_EQ(fileDevice->read(0xFE10 + i), filename[i]);
    }
    
    EXPECT_EQ(fileDevice->getLastFilename(), "");  // No se ha ejecutado operación aún
}

// Test: Operación desconocida produce error
TEST_F(FileDeviceTest, UnknownOperationError) {
    writeFilename(testFile);
    fileDevice->write(0xFE00, 99);  // Operación inválida
    
    EXPECT_EQ(fileDevice->read(0xFE05), 1);  // Error
}

// Test: Cargar archivo grande (1KB)
TEST_F(FileDeviceTest, LoadLargeFile) {
    // Crear archivo de 1KB
    std::vector<uint8_t> largeData(1024);
    for (size_t i = 0; i < largeData.size(); ++i) {
        largeData[i] = static_cast<uint8_t>(i & 0xFF);
    }
    createTestFile(testFile, largeData);
    
    // Cargar archivo
    EXPECT_TRUE(fileDevice->loadBinary(testFile, 0x8000));
    
    // Verificar algunos bytes
    EXPECT_EQ(bus.read(0x8000), 0x00);
    EXPECT_EQ(bus.read(0x8100), 0x00);
    EXPECT_EQ(bus.read(0x83FF), 0xFF);
}

// Test: Integración con CPU - programa que carga y ejecuta código
TEST_F(FileDeviceTest, CPUIntegration) {
    // Crear un pequeño programa: LDA #$42, STA $0200, BRK
    std::vector<uint8_t> program = {0xA9, 0x42, 0x8D, 0x00, 0x02, 0x00};
    createTestFile(testFile, program);
    
    // Cargar programa usando FileDevice
    EXPECT_TRUE(fileDevice->loadBinary(testFile, 0x8000));
    
    // Ejecutar el programa
    cpu.PC = 0x8000;
    cpu.Execute(10, bus);
    
    // Verificar que el programa se ejecutó correctamente
    EXPECT_EQ(bus.read(0x0200), 0x42);
}
