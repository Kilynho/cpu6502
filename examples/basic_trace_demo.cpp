/**
 * BASIC Execution Tracer Demo
 * 
 * This program executes the MS BASIC ROM with detailed instruction-level tracing.
 * It helps identify where execution stalls or loops indefinitely.
 * 
 * Features:
 * - Instruction-level logging (PC, opcode, registers)
 * - Configurable trace interval (every N instructions)
 * - Maximum instruction limit to prevent infinite loops
 * - Display output capture
 * - Automatic detection of READY prompt
 * 
 * Usage:
 *   ./basic_trace_demo [max_instructions] [trace_interval]
 * 
 * Examples:
 *   ./basic_trace_demo              # Default: 100K instructions, trace every 1000
 *   ./basic_trace_demo 50000        # Run max 50K, trace every 1000
 *   ./basic_trace_demo 50000 100    # Run max 50K, trace every 100
 */

#include "cpu/cpu.hpp"
#include "mem/mem.hpp"
#include <devices/pia.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdlib>

// Configuration
struct TracerConfig {
    uint32_t maxInstructions = 100000;  // Maximum instructions before stopping
    uint32_t traceInterval = 1000;      // Print trace every N instructions
    bool verboseTrace = false;          // Print every instruction (warning: slow!)
    bool detectReadyPrompt = true;      // Stop when "READY" appears
    std::string traceFile = "basic_trace.log";
};

// Execution statistics
struct ExecutionStats {
    uint32_t instructionsExecuted = 0;
    uint32_t cyclesConsumed = 0;
    std::chrono::milliseconds executionTime{0};
    bool reachedReadyPrompt = false;
    bool hitInstructionLimit = false;
    Word finalPC = 0;
    Byte finalA = 0, finalX = 0, finalY = 0;
    Byte finalSP = 0;
};

class BASICTracer {
private:
    CPU& cpu;
    Mem& memory;
    PIA& pia;
    TracerConfig config;
    ExecutionStats stats;
    std::ofstream traceLog;
    std::vector<std::string> displayOutput;
    
    // Buffer management (simulating IRQ handler)
    static constexpr Word INPUT_BUFFER_START = 0x0300;  // Location in kilynho.cfg
    static constexpr Byte ZP_READ_PTR = 0x00;
    static constexpr Byte ZP_WRITE_PTR = 0x01;
    
public:
    BASICTracer(CPU& c, Mem& m, PIA& p, const TracerConfig& cfg)
        : cpu(c), memory(m), pia(p), config(cfg) {
        
        // Initialize buffer pointers in Zero Page
        memory[ZP_READ_PTR] = 0x00;
        memory[ZP_WRITE_PTR] = 0x00;
        
        // Open trace log file
        traceLog.open(config.traceFile);
        if (!traceLog.is_open()) {
            std::cerr << "Warning: Could not open trace log file: " 
                      << config.traceFile << std::endl;
        }
    }
    
    ~BASICTracer() {
        if (traceLog.is_open()) {
            traceLog.close();
        }
    }
    
    void logInstruction(uint32_t instructionNum) {
        // Read current opcode
        Byte opcode = memory[cpu.PC];
        
        // Format trace line
        std::stringstream ss;
        ss << std::dec << std::setw(8) << instructionNum << ": "
           << "PC=" << std::hex << std::setw(4) << std::setfill('0') << cpu.PC
           << " OP=" << std::setw(2) << (int)opcode
           << " A=" << std::setw(2) << (int)cpu.A
           << " X=" << std::setw(2) << (int)cpu.X
           << " Y=" << std::setw(2) << (int)cpu.Y
           << " SP=" << std::setw(2) << (int)cpu.SP
           << " Flags=" << (cpu.N ? 'N' : '-')
                         << (cpu.V ? 'V' : '-')
                         << '-'  // unused
                         << (cpu.B ? 'B' : '-')
                         << (cpu.D ? 'D' : '-')
                         << (cpu.I ? 'I' : '-')
                         << (cpu.Z ? 'Z' : '-')
                         << (cpu.C ? 'C' : '-');
        
        std::string traceLine = ss.str();
        
        // Log to file
        if (traceLog.is_open()) {
            traceLog << traceLine << std::endl;
        }
        
        // Print to console if verbose or at interval
        if (config.verboseTrace || (instructionNum % config.traceInterval == 0)) {
            std::cout << traceLine << std::endl;
        }
    }
    
    bool checkForReadyPrompt() {
        if (!config.detectReadyPrompt) {
            return false;
        }
        
        // Get recent display output from PIA
        std::string recentOutput = pia.getDisplayOutput();
        
        // Check if "READY" appears (case insensitive)
        std::string upperOutput = recentOutput;
        for (char& c : upperOutput) {
            c = std::toupper(c);
        }
        
        if (upperOutput.find("READY") != std::string::npos) {
            std::cout << "\n✅ READY prompt detected!" << std::endl;
            std::cout << "Display output:\n" << recentOutput << std::endl;
            return true;
        }
        
        return false;
    }
    
    void printProgress(uint32_t instructionNum) {
        if (instructionNum % 10000 == 0) {
            std::cout << "Progress: " << instructionNum << " instructions executed..."
                      << " PC=0x" << std::hex << cpu.PC << std::dec << std::endl;
        }
    }
    
    // Simulate IRQ handler: transfer PIA data to INPUT_BUFFER
    void simulateACIAtoBufferTransfer() {
        // In the PIA-based system, the keyboard data is provided through Port A
        // We don't need to actively poll the PIA like we did with ACIA
        // The PIA handles this through the emulator's IODevice interface
    }
    
    ExecutionStats run() {
        std::cout << "Starting BASIC execution with tracing..." << std::endl;
        std::cout << "Configuration:" << std::endl;
        std::cout << "  Max Instructions: " << config.maxInstructions << std::endl;
        std::cout << "  Trace Interval: " << config.traceInterval << std::endl;
        std::cout << "  Trace File: " << config.traceFile << std::endl;
        std::cout << std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        uint32_t instructionNum = 0;
        Word lastPC = 0xFFFF;
        uint32_t samePCCount = 0;
        const uint32_t MAX_SAME_PC = 1000; // Detect infinite loops
        
        // Reset CPU to start execution
        cpu.Reset(memory);
        
        std::cout << "CPU Reset complete. Starting execution from PC=0x" 
                  << std::hex << cpu.PC << std::dec << std::endl;
        std::cout << std::endl;
        
        // Main execution loop
        while (instructionNum < config.maxInstructions) {
            Word currentPC = cpu.PC;
            
            // Detect infinite loop (PC stuck at same address)
            if (currentPC == lastPC) {
                samePCCount++;
                if (samePCCount >= MAX_SAME_PC) {
                    std::cerr << "\n⚠️  INFINITE LOOP DETECTED at PC=0x" 
                              << std::hex << currentPC << std::dec << std::endl;
                    std::cerr << "PC has not changed for " << MAX_SAME_PC << " iterations" << std::endl;
                    Byte opcode = memory[currentPC];
                    std::cerr << "Opcode at this location: 0x" << std::hex 
                              << std::setw(2) << std::setfill('0') << (int)opcode << std::dec << std::endl;
                    break;
                }
            } else {
                samePCCount = 0;
                lastPC = currentPC;
            }
            
            // Log current instruction state before executing
            logInstruction(instructionNum);
            
            // Execute exactly one instruction
            try {
                Word pcBefore = cpu.PC;
                cpu.ExecuteSingleInstruction(memory);
                
                // Check if PC advanced (instruction executed)
                if (cpu.PC == pcBefore) {
                    Byte opcode = memory[pcBefore];
                    std::cerr << "\n⚠️  WARNING: PC did not advance! PC=0x" 
                              << std::hex << std::setw(4) << std::setfill('0') << pcBefore 
                              << " opcode=0x" 
                              << std::setw(2) << std::setfill('0') << (int)opcode << std::dec << std::endl;
                    // This indicates an infinite loop or missing instruction handler
                    break;
                }
                
                instructionNum++;
                stats.cyclesConsumed += 3;  // Approximate (instructions vary from 2-7 cycles)
                
                // Simulate IRQ handler every 10 instructions (transfer ACIA data to buffer)
                if (instructionNum % 10 == 0) {
                    simulateACIAtoBufferTransfer();
                }
                
            } catch (const std::exception& e) {
                std::cerr << "\n❌ Exception during execution: " << e.what() << std::endl;
                break;
            }
            
            // Check for READY prompt every 100 instructions
            if (instructionNum % 100 == 0 && checkForReadyPrompt()) {
                stats.reachedReadyPrompt = true;
                break;
            }
            
            // Print progress
            printProgress(instructionNum);
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        
        // Capture final state
        stats.instructionsExecuted = instructionNum;
        stats.finalPC = cpu.PC;
        stats.finalA = cpu.A;
        stats.finalX = cpu.X;
        stats.finalY = cpu.Y;
        stats.finalSP = cpu.SP;
        stats.executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);
        
        if (instructionNum >= config.maxInstructions) {
            stats.hitInstructionLimit = true;
        }
        
        return stats;
    }
    
    void printStatistics() const {
        std::cout << "\n════════════════════════════════════════════════════════════" << std::endl;
        std::cout << "EXECUTION STATISTICS" << std::endl;
        std::cout << "════════════════════════════════════════════════════════════" << std::endl;
        
        std::cout << "Instructions Executed: " << stats.instructionsExecuted << std::endl;
        std::cout << "Cycles Consumed: ~" << stats.cyclesConsumed << std::endl;
        std::cout << "Execution Time: " << stats.executionTime.count() << " ms" << std::endl;
        
        if (stats.executionTime.count() > 0) {
            double instructionsPerMs = (double)stats.instructionsExecuted / stats.executionTime.count();
            std::cout << "Performance: " << std::fixed << std::setprecision(0) 
                      << instructionsPerMs << " instructions/ms" << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << "Final CPU State:" << std::endl;
        std::cout << "  PC = 0x" << std::hex << std::setw(4) << std::setfill('0') 
                  << stats.finalPC << std::endl;
        std::cout << "  A  = 0x" << std::setw(2) << (int)stats.finalA << std::endl;
        std::cout << "  X  = 0x" << std::setw(2) << (int)stats.finalX << std::endl;
        std::cout << "  Y  = 0x" << std::setw(2) << (int)stats.finalY << std::endl;
        std::cout << "  SP = 0x" << std::setw(2) << (int)stats.finalSP << std::dec << std::endl;
        
        std::cout << std::endl;
        std::cout << "Termination Reason:" << std::endl;
        if (stats.reachedReadyPrompt) {
            std::cout << "  ✅ READY prompt detected" << std::endl;
        } else if (stats.hitInstructionLimit) {
            std::cout << "  ⚠️  Instruction limit reached (" << config.maxInstructions << ")" << std::endl;
        } else {
            std::cout << "  ❌ Exception or error" << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << "Display Output:" << std::endl;
        std::cout << "───────────────────────────────────────────────────────────" << std::endl;
        std::cout << pia.getDisplayOutput() << std::endl;
        std::cout << "───────────────────────────────────────────────────────────" << std::endl;
        
        std::cout << std::endl;
        std::cout << "Trace log saved to: " << config.traceFile << std::endl;
        std::cout << "════════════════════════════════════════════════════════════" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
    std::cout << "           BASIC EXECUTION TRACER - Apple 1 Emulator           " << std::endl;
    std::cout << "═══════════════════════════════════════════════════════════════" << std::endl;
    std::cout << std::endl;
    
    // Parse command line arguments
    TracerConfig config;
    
    if (argc > 1) {
        config.maxInstructions = std::stoul(argv[1]);
    }
    
    if (argc > 2) {
        config.traceInterval = std::stoul(argv[2]);
    }
    
    if (argc > 3) {
        config.verboseTrace = (std::string(argv[3]) == "verbose");
    }
    
    // Initialize CPU and Memory
    CPU cpu;
    Mem memory;
    
    // Initialize PIA (6821) for keyboard/display at 0xD010-0xD013
    auto pia = std::make_shared<PIA>();
    cpu.registerIODevice(pia);
    
    // Pre-load input responses for BASIC prompts
    // BASIC kilynho expects input via Port A of the PIA
    std::cout << "Pre-loading input responses for BASIC prompts..." << std::endl;
    pia->pushKeyboardCharacter('\r');  // CR for Memory Size prompt (accept default)
    pia->pushKeyboardCharacter('\r');  // CR for Terminal Width prompt (accept default) 
    pia->pushKeyboardCharacter('1');   // First BASIC command
    pia->pushKeyboardCharacter('\r');  // Execute command
    std::cout << "  ✅ Responses queued (CR, CR, '1', CR)" << std::endl;
    std::cout << std::endl;
    
    // Load ROM file (single source: rom.bin, overridable via WOZMON_BIN)
    std::cout << "Loading ROM file..." << std::endl;
    std::string romPath = "../src/wozmon/rom.bin";
    if (const char* envRom = std::getenv("WOZMON_BIN")) {
        if (*envRom) romPath = envRom;
    }

    std::ifstream rom(romPath, std::ios::binary);
    if (rom.is_open()) {
        std::vector<uint8_t> romData((std::istreambuf_iterator<char>(rom)),
                                      std::istreambuf_iterator<char>());
        rom.close();
        
        // Load ROM at 0x8000 (size ~32KB)
        for (size_t i = 0; i < romData.size() && i < 0x6000; i++) {
            memory[0x8000 + i] = romData[i];
        }
        std::cout << "  ✅ Loaded ROM from " << romPath << " (" << romData.size() << " bytes)" << std::endl;
    } else {
        std::cerr << "  ⚠️  Could not load ROM file at " << romPath << " (continuing anyway)" << std::endl;
    }
    
    // Set reset vector to BASIC coldstart at 0x9F06
    memory[0xFFFC] = 0x06;      // Low byte of 0x9F06
    memory[0xFFFD] = 0x9F;      // High byte
    
    // Reset CPU after loading ROM to execute coldstart
    cpu.Reset(memory);
    
    // Verify RESET vector and PC
    Word resetVector = memory[0xFFFC] | (memory[0xFFFD] << 8);
    std::cout << "  RESET vector: 0x" << std::hex << resetVector << std::dec << std::endl;
    std::cout << "  CPU PC after Reset(): 0x" << std::hex << cpu.PC << std::dec << std::endl;
    
    std::cout << std::endl;
    
    // Create tracer and run with PIA for I/O
    BASICTracer tracer(cpu, memory, *pia, config);
    
    ExecutionStats stats = tracer.run();
    
    // Print results
    tracer.printStatistics();
    
    // Return exit code based on result
    if (stats.reachedReadyPrompt) {
        std::cout << "\n✅ SUCCESS: BASIC READY prompt reached!" << std::endl;
        return 0;
    } else if (stats.hitInstructionLimit) {
        std::cout << "\n⚠️  WARNING: Instruction limit reached without READY prompt" << std::endl;
        std::cout << "This may indicate an infinite loop or very slow execution." << std::endl;
        std::cout << "Check the trace log for details: " << config.traceFile << std::endl;
        return 1;
    } else {
        std::cout << "\n❌ ERROR: Execution terminated abnormally" << std::endl;
        return 2;
    }
}
