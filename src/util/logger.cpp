#include "logger.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

namespace util {

Logger::Logger() : currentLevel(LogLevel::INFO), maxFileSize(10 * 1024 * 1024), maxFiles(5), currentFileSize(0) {
}

Logger::~Logger() {
    if (logFile && logFile->is_open()) {
        logFile->close();
    }
}

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

void Logger::SetLevel(LogLevel level) {
    currentLevel = level;
}

LogLevel Logger::GetLevel() const {
    return currentLevel;
}

void Logger::SetLogFile(const std::string& basePath, size_t maxSize, size_t numFiles) {
    logBasePath = basePath;
    maxFileSize = maxSize;
    maxFiles = numFiles;
    currentFileSize = 0;
    
    // Crear directorio si no existe
    fs::path logPath(basePath);
    fs::path logDir = logPath.parent_path();
    if (!logDir.empty() && !fs::exists(logDir)) {
        fs::create_directories(logDir);
    }
    
    // Abrir el archivo de log
    logFile = std::make_unique<std::ofstream>(basePath, std::ios::app);
    if (logFile->is_open()) {
        // Verificar tamaño actual
        logFile->seekp(0, std::ios::end);
        currentFileSize = logFile->tellp();
    }
}

void Logger::RotateLogFiles() {
    if (!logFile) return;
    
    // Cerrar archivo actual
    logFile->close();
    
    // Rotar archivos existentes
    for (int i = maxFiles - 1; i >= 1; --i) {
        std::string oldName = logBasePath + "." + std::to_string(i);
        std::string newName = logBasePath + "." + std::to_string(i + 1);
        
        if (fs::exists(oldName)) {
            if (i == static_cast<int>(maxFiles) - 1) {
                fs::remove(oldName);  // Eliminar el más antiguo
            } else {
                fs::rename(oldName, newName);
            }
        }
    }
    
    // Renombrar archivo actual
    if (fs::exists(logBasePath)) {
        fs::rename(logBasePath, logBasePath + ".1");
    }
    
    // Abrir nuevo archivo
    logFile = std::make_unique<std::ofstream>(logBasePath, std::ios::app);
    currentFileSize = 0;
}

const char* Logger::LevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::NONE:  return "NONE ";
        default: return "UNKNOWN";
    }
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (level <= currentLevel && currentLevel != LogLevel::NONE) {
        // Obtener timestamp actual
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        
        std::ostringstream oss;
        oss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
            << "[" << LevelToString(level) << "] "
            << message;
        
        std::string logMessage = oss.str();
        
        // Escribir a archivo si está configurado
        if (logFile && logFile->is_open()) {
            *logFile << logMessage << std::endl;
            currentFileSize += logMessage.length() + 1;
            
            // Rotar si es necesario
            if (currentFileSize >= maxFileSize) {
                RotateLogFiles();
            }
        } else {
            // Si no hay archivo, escribir a consola
            std::cout << logMessage << std::endl;
        }
    }
}

void Logger::Error(const std::string& message) {
    Log(LogLevel::ERROR, message);
}

void Logger::Warn(const std::string& message) {
    Log(LogLevel::WARN, message);
}

void Logger::Info(const std::string& message) {
    Log(LogLevel::INFO, message);
}

void Logger::Debug(const std::string& message) {
    Log(LogLevel::DEBUG, message);
}

} // namespace util
