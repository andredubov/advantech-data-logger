#ifndef I_VALIDATOR_HPP
#define I_VALIDATOR_HPP

#include <string>

namespace app {
namespace validation {

// Интерфейс для валидации
class IValidator {
public:
    virtual ~IValidator() = default;

    // Проверить, соответствует ли значение критериям валидации
    virtual bool isValid(const std::string& value) = 0;
    
    // Получить сообщение об ошибке
    virtual std::string getErrorMessage() const = 0;
};

} // namespace validation
} // namespace app

#endif // I_VALIDATOR_HPP