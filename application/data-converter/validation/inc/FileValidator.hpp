#ifndef FILE_VALIDATOR_HPP
#define FILE_VALIDATOR_HPP

#include "IValidator.hpp"
#include <memory>
#include <vector>

namespace app {
namespace validation {

// Композитный валидатор для файлов
class FileValidator : public IValidator {
public:
    FileValidator();
    ~FileValidator() override = default;

    // Добавить валидатор в цепочку
    void addValidator(std::shared_ptr<IValidator> validator);

    // IValidator implementation
    bool isValid(const std::string& filePath) override;
    std::string getErrorMessage() const override;

private:
    std::vector<std::shared_ptr<IValidator>> m_validators;
    std::string m_errorMessage;
};

} // namespace validation
} // namespace app

#endif // FILE_VALIDATOR_HPP