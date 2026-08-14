#ifndef FILE_VALIDATOR_HPP
#define FILE_VALIDATOR_HPP

#include "IValidator.hpp"
#include "ILogger.hpp"
#include <memory>
#include <vector>

namespace app {
namespace validation {

// Композитный валидатор для файлов
class FileValidator : public IValidator {
public:
    FileValidator(std::shared_ptr<app::utils::ILogger> logger);
    ~FileValidator() override = default;

    // Добавить валидатор в цепочку
    void addValidator(std::shared_ptr<IValidator> validator);

    // IValidator implementation
    bool isValid(const std::string& filePath) override;
    std::string getErrorMessage() const override;

private:
    std::vector<std::shared_ptr<IValidator>> m_validators;
    std::string m_errorMessage;
    std::shared_ptr<app::utils::ILogger> m_logger;
};

} // namespace validation
} // namespace app

#endif // FILE_VALIDATOR_HPP