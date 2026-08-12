#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <iostream>
#include <mutex>

namespace app {
namespace utils {

// Уровни логирования
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

// Простой потокобезопасный логгер
class Logger {
public:
    static Logger& getInstance();

    void setLevel(LogLevel level);
    void setOutput(std::ostream& output);

    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

private:
    Logger();
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(LogLevel level, const std::string& message);
    std::string levelToString(LogLevel level) const;

    LogLevel m_level;
    std::ostream* m_output;
    std::mutex m_mutex;
};

} // namespace utils
} // namespace app

#endif // LOGGER_HPP