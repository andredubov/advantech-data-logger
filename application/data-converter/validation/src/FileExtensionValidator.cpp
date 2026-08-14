#include "FileExtensionValidator.hpp"
#include <boost/filesystem.hpp>
#include <algorithm>
#include <cctype>

namespace app {
namespace validation {

FileExtensionValidator::FileExtensionValidator(std::shared_ptr<app::utils::ILogger> logger, const std::vector<std::string>& allowedExtensions)
    : m_allowedExtensions(allowedExtensions)
    , m_errorMessage()
    , m_caseSensitive(false)
    , m_logger(logger)
{
}

std::string FileExtensionValidator::getFileExtension(const std::string& filePath) const
{
    boost::filesystem::path path(filePath);
    std::string ext = path.extension().string();
    
    if (!m_caseSensitive) {
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return std::tolower(c); });
    }
    
    return ext;
}

bool FileExtensionValidator::isValid(const std::string& filePath)
{
    m_errorMessage.clear();
    
    if (m_allowedExtensions.empty()) {
        m_errorMessage = "No allowed extensions defined.";
        return false;
    }
    
    std::string ext = getFileExtension(filePath);
    
    for (const auto& allowed : m_allowedExtensions) {
        std::string allowedExt = allowed;
        if (!m_caseSensitive) {
            std::transform(allowedExt.begin(), allowedExt.end(), allowedExt.begin(),
                [](unsigned char c) { return std::tolower(c); });
        }
        
        if (ext == allowedExt) {
            return true;
        }
    }
    
    m_errorMessage = "File extension '" + ext + "' is not allowed. Allowed extensions: ";
    for (size_t i = 0; i < m_allowedExtensions.size(); ++i) {
        if (i > 0) m_errorMessage += ", ";
        m_errorMessage += m_allowedExtensions[i];
    }
    
    return false;
}

std::string FileExtensionValidator::getErrorMessage() const
{
    return m_errorMessage;
}

} // namespace validation
} // namespace app
