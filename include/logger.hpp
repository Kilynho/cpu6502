#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>

namespace util {

// Niveles de log
enum class LogLevel {
    NONE = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4
};

// Clase Logger singleton
class Logger {
public:
    // Obtener instancia del logger
    static Logger& GetInstance();
    
    // Establecer el nivel de log
    void SetLevel(LogLevel level);
    
    // Obtener el nivel de log actual
    LogLevel GetLevel() const;
    
    // Configurar logging a archivo con rotación
    void SetLogFile(const std::string& basePath, size_t maxFileSize = 10 * 1024 * 1024, size_t maxFiles = 5);
    
    // Métodos de log
    void Error(const std::string& message);
    void Warn(const std::string& message);
    void Info(const std::string& message);
    void Debug(const std::string& message);
    
    // Método genérico de log
    void Log(LogLevel level, const std::string& message);
    
private:
    Logger();
    ~Logger();
    
    // Deshabilitar copia y asignación
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    LogLevel currentLevel;
    std::unique_ptr<std::ofstream> logFile;
    std::string logBasePath;
    size_t maxFileSize;
    size_t maxFiles;
    size_t currentFileSize;
    
    // Rotar archivos de log
    void RotateLogFiles();
    
    // Convertir nivel a string
    const char* LevelToString(LogLevel level) const;
};

// Funciones globales para facilitar el uso
inline void LogSetLevel(LogLevel level) {
    Logger::GetInstance().SetLevel(level);
}

inline void LogError(const std::string& message) {
    Logger::GetInstance().Error(message);
}

inline void LogWarn(const std::string& message) {
    Logger::GetInstance().Warn(message);
}

inline void LogInfo(const std::string& message) {
    Logger::GetInstance().Info(message);
}

inline void LogDebug(const std::string& message) {
    Logger::GetInstance().Debug(message);
}

// Macros para logging con información de archivo y línea (opcional)
#define LOG_ERROR(msg) do { \
    std::ostringstream oss; \
    oss << msg; \
    util::LogError(oss.str()); \
} while(0)

#define LOG_WARN(msg) do { \
    std::ostringstream oss; \
    oss << msg; \
    util::LogWarn(oss.str()); \
} while(0)

#define LOG_INFO(msg) do { \
    std::ostringstream oss; \
    oss << msg; \
    util::LogInfo(oss.str()); \
} while(0)

#define LOG_DEBUG(msg) do { \
    std::ostringstream oss; \
    oss << msg; \
    util::LogDebug(oss.str()); \
} while(0)

} // namespace util

#endif // LOGGER_HPP
