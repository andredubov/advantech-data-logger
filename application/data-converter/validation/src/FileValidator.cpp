#include "FileValidator.hpp"

namespace app {
namespace validation {

FileValidator::FileValidator(std::shared_ptr<app::utils::ILogger> logger)
    : m_validators()
    , m_errorMessage()
    , m_logger(logger)
{
}

void FileValidator::addValidator(std::shared_ptr<IValidator> validator)
{
    if (validator) {
        m_validators.push_back(validator);
    }
}

bool FileValidator::isValid(const std::string& filePath)
{
    m_errorMessage.clear();

    for (const auto& validator : m_validators) {
        if (!validator->isValid(filePath)) {
            m_errorMessage = validator->getErrorMessage();
            return false;
        }
    }

    return true;
}

std::string FileValidator::getErrorMessage() const
{
    return m_errorMessage;
}

} // namespace validation
} // namespace app
