#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <mutex>
#include <string>
#include <cstdarg>
#include <vector>
#include <cstdio>
#include "ILogger.hpp"

namespace app {

// Простой потокобезопасный логгер
class Logger : public ILogger {
public:
    static Logger& getInstance();

    void setLevel(LogLevel level) override;
    void setOutput(std::ostream& output);

    void debug(const std::string& message) override;
    void info(const std::string& message) override;
    void warning(const std::string& message) override;
    void error(const std::string& message) override;

    // Форматированные методы с использованием вариативных шаблонов
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

    Logger();
    ~Logger() = default;
    
private:
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(LogLevel level, const std::string& message);
    std::string levelToString(LogLevel level) const;

    // Реализация форматирования через va_list
    void logFormatted(LogLevel level, const char* format, ...);

    LogLevel m_level;
    std::ostream* m_output;
    std::mutex m_mutex;
};

} // namespace app

#endif // LOGGER_HPP