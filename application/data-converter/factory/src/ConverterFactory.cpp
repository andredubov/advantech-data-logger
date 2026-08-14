#include "ConverterFactory.hpp"
#include "BinaryReader.hpp"
#include "CsvWriter.hpp"
#include "TimeFormatter.hpp"
#include "DataConverter.hpp"
#include "FileValidator.hpp"
#include "FileExtensionValidator.hpp"
#include <vector>

namespace app {
namespace factory {

ConverterFactory::ConverterFactory(std::shared_ptr<app::utils::ILogger> logger) 
    : m_logger(logger)
{

}

std::shared_ptr<core::DataConverter> ConverterFactory::createDefaultConverter()
{
    auto timeFormatter = createTimeFormatter();
    auto reader = createBinaryReader(timeFormatter);
    auto writer = createCsvWriter(timeFormatter);
    return createConverter(reader, writer);
}

std::shared_ptr<core::DataConverter> ConverterFactory::createConverter(
    std::shared_ptr<core::IDataReader> reader,
    std::shared_ptr<core::IDataWriter> writer)
{
    return std::make_shared<core::DataConverter>(m_logger, reader, writer);
}

std::shared_ptr<core::IDataReader> ConverterFactory::createBinaryReader(
    std::shared_ptr<core::ITimeFormatter> timeFormatter)
{
    if (!timeFormatter) {
        timeFormatter = createTimeFormatter();
    }
    return std::make_shared<core::BinaryReader>(m_logger, timeFormatter);
}

std::shared_ptr<core::IDataWriter> ConverterFactory::createCsvWriter(
    std::shared_ptr<core::ITimeFormatter> timeFormatter)
{
    if (!timeFormatter) {
        timeFormatter = createTimeFormatter();
    }
    return std::make_shared<core::CsvWriter>(m_logger, timeFormatter);
}

std::shared_ptr<core::ITimeFormatter> ConverterFactory::createTimeFormatter()
{
    return std::make_shared<core::TimeFormatter>();
}

std::shared_ptr<validation::IValidator> ConverterFactory::createInputFileValidator()
{
    auto fileValidator = std::make_shared<validation::FileValidator>(m_logger);
    
    // Добавляем проверку расширения .bin
    auto extValidator = std::make_shared<validation::FileExtensionValidator>(
        m_logger,
        std::vector<std::string>{".bin"}
    );
    fileValidator->addValidator(extValidator);
    
    return fileValidator;
}

std::shared_ptr<validation::IValidator> ConverterFactory::createOutputFileValidator()
{
    auto fileValidator = std::make_shared<validation::FileValidator>(m_logger);
    
    // Добавляем проверку расширения .csv
    auto extValidator = std::make_shared<validation::FileExtensionValidator>(
        m_logger,
        std::vector<std::string>{".csv"}
    );
    fileValidator->addValidator(extValidator);
    
    return fileValidator;
}

bool ConverterFactory::validateInputFile(const std::string& filePath, std::string& errorMessage)
{
    auto validator = createInputFileValidator();
    if (!validator->isValid(filePath)) {
        errorMessage = validator->getErrorMessage();
        return false;
    }
    return true;
}

bool ConverterFactory::validateOutputFile(const std::string& filePath, std::string& errorMessage)
{
    auto validator = createOutputFileValidator();
    if (!validator->isValid(filePath)) {
        errorMessage = validator->getErrorMessage();
        return false;
    }
    return true;
}

bool ConverterFactory::isFileExtensionValid(const std::string& filePath, const std::vector<std::string>& extensions)
{
    auto validator = std::make_shared<validation::FileExtensionValidator>(m_logger, extensions);
    return validator->isValid(filePath);
}

} // namespace factory
} // namespace app
