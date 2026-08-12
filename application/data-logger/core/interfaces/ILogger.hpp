#ifndef ILOGGER_HPP
#define ILOGGER_HPP

#include <string>

namespace app {

/**
 * @brief Interface for logging messages
 */
class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void info(const std::string& message) = 0;
    virtual void warning(const std::string& message) = 0;
    virtual void error(const std::string& message) = 0;
};

} // namespace app

#endif // ILOGGER_HPP
