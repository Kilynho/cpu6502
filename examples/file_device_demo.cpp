#include "cpu/cpu.hpp"
#include "mem/mem.hpp"
#include "devices/file_device.hpp"
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>

/**
 * Example usage of FileDevice for loading and saving binaries
 *
 * This program demonstrates how to:
 * 1. Create and use a FileDevice
 * 2. Load a binary from a file using the direct API
 * 3. Execute the loaded code
 * 4. Save memory data to a file
 * 5. Use memory-mapped registers for I/O operations
 */

// Helper function to create a sample program
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
        std::cout << "Sample program created: " << filename << "\n";
    } else {
        std::cerr << "Error creating sample file\n";
    }
}

// Function to display memory content as text
void displayMemoryAsText(const Mem& mem, uint16_t start, uint16_t length) {
    std::cout << "Memory content at 0x" << std::hex << start << ": ";
    for (uint16_t i = 0; i < length; ++i) {
        char c = static_cast<char>(mem[start + i]);
        if (c >= 32 && c <= 126) {  // Printable characters
            std::cout << c;
        } else {
            std::cout << '.';
        }
    }
    std::cout << std::dec << "\n";
}

int main() {
    std::cout << "=== FileDevice Demo for 6502 CPU ===\n\n";
    
    // Setup
    Mem mem;
    CPU cpu;
    auto fileDevice = std::make_shared<FileDevice>(&mem);
    
    cpu.Reset(mem);
    cpu.registerIODevice(fileDevice);
    
    const std::string programFile = "/tmp/sample_program.bin";
    const std::string outputFile = "/tmp/output_data.bin";
    
    // ===== PART 1: Create and load a program =====
    std::cout << "PART 1: Load and execute a program\n";
    std::cout << "-----------------------------------\n";
    
    // Create a sample program
    createSampleProgram(programFile);
    
    // Method 1: Load using the direct API
    std::cout << "\nLoading program using direct API...\n";
    if (fileDevice->loadBinary(programFile, 0x8000)) {
        std::cout << "Program successfully loaded at 0x8000\n";
    } else {
        std::cerr << "Error loading program\n";
        return 1;
    }
    
    // Execute the program
    std::cout << "\nExecuting program...\n";
    cpu.PC = 0x8000;
    cpu.Execute(100, mem);
    
    // Display the result
    displayMemoryAsText(mem, 0x0200, 5);
    
    // ===== PART 2: Save memory data to a file =====
    std::cout << "\nPART 2: Save memory data to a file\n";
    std::cout << "-----------------------------------\n";
    
    // Save data using the direct API
    std::cout << "\nSaving data using direct API...\n";
    if (fileDevice->saveBinary(outputFile, 0x0200, 5)) {
        std::cout << "Data successfully saved to " << outputFile << "\n";
    } else {
        std::cerr << "Error saving data\n";
    }
    
    // ===== PART 3: Using memory-mapped registers =====
    std::cout << "\nPART 3: Using memory-mapped registers\n";
    std::cout << "-------------------------------------\n";
    
    // Clear memory for demonstration
    for (uint16_t i = 0; i < 5; ++i) {
        mem[0x9000 + i] = 0;
    }
    
    // Set up file name in buffer (0xFE10-0xFE4F)
    std::cout << "\nSetting up file name in registers...\n";
    const std::string filename = outputFile;
    for (size_t i = 0; i < filename.length() && i < 64; ++i) {
        fileDevice->write(0xFE10 + i, filename[i]);
    }
    fileDevice->write(0xFE10 + filename.length(), 0);  // Null terminator
    
    // Set up start address (0x9000)
    fileDevice->write(0xFE01, 0x00);  // Low byte
    fileDevice->write(0xFE02, 0x90);  // High byte
    
    // Set up length (5 bytes)
    fileDevice->write(0xFE03, 0x05);  // Low byte
    fileDevice->write(0xFE04, 0x00);  // High byte
    
    // Execute LOAD operation (write 1 to control register)
    std::cout << "Executing LOAD operation via control register...\n";
    fileDevice->write(0xFE00, 1);  // 1 = LOAD
    
    // Check status
    uint8_t status = fileDevice->read(0xFE05);
    if (status == 0) {
        std::cout << "LOAD operation successful\n";
        displayMemoryAsText(mem, 0x9000, 5);
    } else {
        std::cout << "Error in LOAD operation (status = " << static_cast<int>(status) << ")\n";
    }
    
    // ===== PART 4: Demonstration of SAVE using registers =====
    std::cout << "\nPART 4: Save using memory-mapped registers\n";
    std::cout << "------------------------------------------\n";
    
    // Write new data to memory
    const char* message = "6502!";
    for (int i = 0; i < 5; ++i) {
        mem[0xA000 + i] = message[i];
    }
    
    std::cout << "\nData in memory to be saved: ";
    displayMemoryAsText(mem, 0xA000, 5);
    
    // Set up registers for saving
    const std::string saveFile = "/tmp/saved_message.bin";
    for (size_t i = 0; i < saveFile.length() && i < 64; ++i) {
        fileDevice->write(0xFE10 + i, saveFile[i]);
    }
    fileDevice->write(0xFE10 + saveFile.length(), 0);
    
    // Set up start address (0xA000)
    fileDevice->write(0xFE01, 0x00);
    fileDevice->write(0xFE02, 0xA0);
    
    // Set up length (5 bytes)
    fileDevice->write(0xFE03, 0x05);
    fileDevice->write(0xFE04, 0x00);
    
    // Execute SAVE operation (write 2 to control register)
    std::cout << "Executing SAVE operation via control register...\n";
    fileDevice->write(0xFE00, 2);  // 2 = SAVE
    
    // Check status
    status = fileDevice->read(0xFE05);
    if (status == 0) {
        std::cout << "SAVE operation successful\n";
        std::cout << "File saved: " << saveFile << "\n";
    } else {
        std::cout << "Error in SAVE operation (status = " << static_cast<int>(status) << ")\n";
    }
    
    // ===== SUMMARY =====
    std::cout << "\n=== SUMMARY ===\n";
    std::cout << "FileDevice allows:\n";
    std::cout << "  1. Load binaries from host files\n";
    std::cout << "  2. Save memory blocks to files\n";
    std::cout << "  3. Control via direct API or memory-mapped registers\n";
    std::cout << "\nMemory-mapped registers:\n";
    std::cout << "  0xFE00:      Control (0=nothing, 1=load, 2=save)\n";
    std::cout << "  0xFE01-0xFE02: Start address (little-endian)\n";
    std::cout << "  0xFE03-0xFE04: Length (little-endian)\n";
    std::cout << "  0xFE05:      Status (0=success, 1=error)\n";
    std::cout << "  0xFE10-0xFE4F: File name (max 64 bytes)\n";
    
    std::cout << "\n=== End of demo ===\n";
    
    return 0;
}
