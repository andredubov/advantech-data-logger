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

ConverterFactory::ConverterFactory() = default;

std::shared_ptr<core::DataConverter> ConverterFactory::createDefaultConverter()
{
    auto reader = createBinaryReader();
    auto writer = createCsvWriter(createTimeFormatter());
    return createConverter(reader, writer);
}

std::shared_ptr<core::DataConverter> ConverterFactory::createConverter(
    std::shared_ptr<core::IDataReader> reader,
    std::shared_ptr<core::IDataWriter> writer)
{
    return std::make_shared<core::DataConverter>(reader, writer);
}

std::shared_ptr<core::IDataReader> ConverterFactory::createBinaryReader()
{
    return std::make_shared<core::BinaryReader>();
}

std::shared_ptr<core::IDataWriter> ConverterFactory::createCsvWriter(
    std::shared_ptr<core::ITimeFormatter> timeFormatter)
{
    if (!timeFormatter) {
        timeFormatter = createTimeFormatter();
    }
    return std::make_shared<core::CsvWriter>(timeFormatter);
}

std::shared_ptr<core::ITimeFormatter> ConverterFactory::createTimeFormatter()
{
    return std::make_shared<core::TimeFormatter>();
}

std::shared_ptr<validation::IValidator> ConverterFactory::createInputFileValidator()
{
    auto fileValidator = std::make_shared<validation::FileValidator>();
    
    // Добавляем проверку расширения .bin
    auto extValidator = std::make_shared<validation::FileExtensionValidator>(
        std::vector<std::string>{".bin"}
    );
    fileValidator->addValidator(extValidator);
    
    return fileValidator;
}

std::shared_ptr<validation::IValidator> ConverterFactory::createOutputFileValidator()
{
    auto fileValidator = std::make_shared<validation::FileValidator>();
    
    // Добавляем проверку расширения .csv
    auto extValidator = std::make_shared<validation::FileExtensionValidator>(
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

bool ConverterFactory::isFileExtensionValid(
    const std::string& filePath,
    const std::vector<std::string>& extensions)
{
    auto validator = std::make_shared<validation::FileExtensionValidator>(extensions);
    return validator->isValid(filePath);
}

} // namespace factory
} // namespace app
