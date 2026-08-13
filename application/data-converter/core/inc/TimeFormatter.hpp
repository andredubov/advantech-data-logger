#ifndef TIME_FORMATTER_HPP
#define TIME_FORMATTER_HPP

#include "ITimeFormatter.hpp"
#include <string>

namespace app {
namespace core {

class TimeFormatter : public ITimeFormatter {

public:
    TimeFormatter() = default;
    ~TimeFormatter() override = default;

    // ITimeFormatter implementation
    std::string formatTime(double seconds) const override;
};

} // namespace core
} // namespace app

#endif // TIME_FORMATTER_HPP