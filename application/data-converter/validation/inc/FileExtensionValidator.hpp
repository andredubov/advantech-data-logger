#ifndef FILE_EXTENSION_VALIDATOR_HPP
#define FILE_EXTENSION_VALIDATOR_HPP

#include "IValidator.hpp"
#include <string>
#include <vector>

namespace app {
namespace validation {

// Валидатор расширения файла
class FileExtensionValidator : public IValidator {
public:
    explicit FileExtensionValidator(const std::vector<std::string>& allowedExtensions);
    ~FileExtensionValidator() override = default;

    // IValidator implementation
    bool isValid(const std::string& filePath) override;
    std::string getErrorMessage() const override;

private:
    std::vector<std::string> m_allowedExtensions;
    std::string m_errorMessage;
    bool m_caseSensitive;

    std::string getFileExtension(const std::string& filePath) const;
};

} // namespace validation
} // namespace app

#endif // FILE_EXTENSION_VALIDATOR_HPP