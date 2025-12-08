#include "util/logger.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>

namespace util {

Logger::Logger() : currentLevel(LogLevel::INFO) {
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
        
        std::cout << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
                  << "[" << LevelToString(level) << "] "
                  << message << std::endl;
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
