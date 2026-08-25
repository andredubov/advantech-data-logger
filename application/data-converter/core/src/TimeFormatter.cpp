#include "TimeFormatter.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace app {
namespace core {

std::string TimeFormatter::formatTime(double seconds) const
{
    time_t rawTime = static_cast<time_t>(seconds);
    struct tm timeInfo;
    ::localtime_s(&timeInfo, &rawTime);
    
    int milliseconds = static_cast<int>((seconds - rawTime) * 1000);
    
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", &timeInfo);
    
    char result[80];
    std::snprintf(result, sizeof(result), "%s,%03d", buffer, milliseconds);
    
    return std::string(result);
}

} // namespace core
} // namespace app
