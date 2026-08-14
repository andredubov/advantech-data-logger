#ifndef I_LOGGER_HPP
#define I_LOGGER_HPP

#include <string>

namespace app {

// Уровни логирования
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

/**
 * @brief Интерфейс для логирования
 */
class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void setLevel(LogLevel level) = 0;
    virtual void debug(const std::string& message) = 0;
    virtual void info(const std::string& message) = 0;
    virtual void warning(const std::string& message) = 0;
    virtual void error(const std::string& message) = 0;

    // Форматированные методы (шаблонные, реализованы в интерфейсе)
    template<typename... Args>
    void debug(const std::string& format, Args... args) {
        logFormatted(LogLevel::DEBUG, format.c_str(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const std::string& format, Args... args) {
        logFormatted(LogLevel::INFO, format.c_str(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warning(const std::string& format, Args... args) {
        logFormatted(LogLevel::WARNING, format.c_str(), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const std::string& format, Args... args) {
        logFormatted(LogLevel::ERROR, format.c_str(), std::forward<Args>(args)...);
    }

protected:
    // Метод для форматированного логирования (реализуется в наследнике)
    virtual void logFormatted(LogLevel level, const char* format, ...) = 0;
};

} // namespace app

#endif // I_LOGGER_HPP