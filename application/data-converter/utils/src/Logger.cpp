#include "Logger.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdarg>
#include <vector>
#include <cstdio>

namespace app {
namespace utils {

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
    : m_level(LogLevel::INFO)
    , m_output(&std::cout)
    , m_mutex()
{
}

void Logger::setLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
}

void Logger::setOutput(std::ostream& output)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_output = &output;
}

void Logger::debug(const std::string& message)
{
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message)
{
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message)
{
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message)
{
    log(LogLevel::ERROR, message);
}

void Logger::log(LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (level < m_level) {
        return;
    }

    // Получаем текущее время
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    struct tm timeInfo;
    ::localtime_s(&timeInfo, &time_t_now);

    std::stringstream ss;
    ss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S")
       << "." << std::setw(3) << std::setfill('0') << ms.count()
       << " [" << levelToString(level) << "] "
       << message;

    (*m_output) << ss.str() << std::endl;
}

std::string Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

void Logger::logFormatted(LogLevel level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    // Определяем необходимый размер буфера
    va_list args_copy;
    va_copy(args_copy, args);
    int size = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);
    
    if (size < 0) {
        va_end(args);
        return;
    }
    
    std::vector<char> buffer(size + 1);
    std::vsnprintf(buffer.data(), buffer.size(), format, args);
    va_end(args);
    
    log(level, std::string(buffer.data()));
}

} // namespace utils
} // namespace app
