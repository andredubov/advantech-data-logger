#ifndef CONVERTER_FACTORY_HPP
#define CONVERTER_FACTORY_HPP

#include <memory>
#include <string>
#include <vector>
#include "ILogger.hpp"
#include "ITimeFormatter.hpp"

namespace app {
namespace core {
    class DataConverter;
    class IDataReader;
    class IDataWriter;
    class ITimeFormatter;
}

namespace validation {
    class IValidator;
}

namespace utils {
    class ILogger;
}

namespace factory {

// Фабрика для создания компонентов конвертера
class ConverterFactory {
public:
    ConverterFactory(std::shared_ptr<app::utils::ILogger> logger);
    ~ConverterFactory() = default;

    // Создать стандартный конвертер
    std::shared_ptr<core::DataConverter> createDefaultConverter();

    // Создать конвертер с кастомными компонентами
    std::shared_ptr<core::DataConverter> createConverter(
        std::shared_ptr<core::IDataReader> reader,
        std::shared_ptr<core::IDataWriter> writer
    );

    // Создать бинарный читатель
    std::shared_ptr<core::IDataReader> createBinaryReader(
        std::shared_ptr<core::ITimeFormatter> timeFormatter = nullptr
    );

    // Создать CSV-записыватель с кастомным форматтером времени
    std::shared_ptr<core::IDataWriter> createCsvWriter(
        std::shared_ptr<core::ITimeFormatter> timeFormatter = nullptr
    );

    // Создать форматтер времени
    std::shared_ptr<core::ITimeFormatter> createTimeFormatter();

    // Создать валидатор для входного файла (.bin)
    std::shared_ptr<validation::IValidator> createInputFileValidator();

    // Создать валидатор для выходного файла (.csv)
    std::shared_ptr<validation::IValidator> createOutputFileValidator();

    // Проверить валидность файла
    bool validateInputFile(const std::string& filePath, std::string& errorMessage);
    bool validateOutputFile(const std::string& filePath, std::string& errorMessage);

private:
    std::shared_ptr<app::utils::ILogger> m_logger;

    bool isFileExtensionValid(const std::string& filePath, const std::vector<std::string>& extensions);
};

} // namespace factory
} // namespace app

#endif // CONVERTER_FACTORY_HPP