#ifndef CONSOLELOGGER_HPP
#define CONSOLELOGGER_HPP

#include <iostream>
#include "ILogger.hpp"


namespace app {

class ConsoleLogger : public ILogger {
public:
    void info(const std::string& message) override {
        std::printf("[INFO] %s\n", message.c_str());
    }

    void warning(const std::string& message) override {
        std::printf("[WARNING] %s\n", message.c_str());
    }

    void error(const std::string& message) override {
        std::printf("[ERROR] %s\n", message.c_str());
    }
};

} // namespace app

#endif // CONSOLELOGGER_HPP
