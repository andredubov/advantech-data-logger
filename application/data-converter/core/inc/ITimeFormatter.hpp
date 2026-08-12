#ifndef I_TIME_FORMATTER_HPP
#define I_TIME_FORMATTER_HPP

#include <string>

namespace app {
namespace core {

// Интерфейс для форматирования времени
class ITimeFormatter {
public:
    virtual ~ITimeFormatter() = default;

    // Преобразовать время в секундах в строку формата DD.MM.YYYY HH:MM:SS,ms
    virtual std::string formatTime(double seconds) const = 0;
};

} // namespace core
} // namespace app

#endif // I_TIME_FORMATTER_HPP