#include "main.hpp"

int main(int argc, char* argv[])
{
    // 1. Настройка логгера
    auto logger = std::make_shared<app::utils::Logger>();
    logger->setLevel(app::utils::LogLevel::INFO);

    // 2. Парсинг аргументов командной строки
    app::CommandLineOptions options;
    auto state = options.parse(argc, argv);

    switch (state) {
        case app::CommandLineOptions::state::success:
            break;
        case app::CommandLineOptions::state::version:
            std::cout << "v" << options.get_version() << std::endl;
            return EXIT_SUCCESS;
        case app::CommandLineOptions::state::help:
            std::cout << options.get_help() << std::endl;
            return EXIT_SUCCESS;
        default:
            logger->error(options.get_error_message());
            return EXIT_FAILURE;
    }

    const std::string binaryFileName = options.get_input_file_path();
    const std::string csvFileName = options.get_output_file_path();

    logger->info("Input file: " + binaryFileName);
    logger->info("Output file: " + csvFileName);

    // 3. Создание фабрики и валидация файлов
    app::factory::ConverterFactory factory(logger);
    std::string errorMessage;

    if (!factory.validateInputFile(binaryFileName, errorMessage)) {
        logger->error("Input validation failed: " + errorMessage);
        return EXIT_FAILURE;
    }

    if (!factory.validateOutputFile(csvFileName, errorMessage)) {
        logger->error("Output validation failed: " + errorMessage);
        return EXIT_FAILURE;
    }

    // 4. Создание конвертера
    auto converter = factory.createDefaultConverter();
    converter->setChunkSize(25000);

    // 5. Выполнение конвертации
    logger->info("Starting conversion...");
    bool success = converter->convert(binaryFileName, csvFileName);

    if (!success) {
        logger->error("Conversion failed!");
        return EXIT_FAILURE;
    }

    logger->info("Conversion completed successfully! File saved as: " + csvFileName);

    return EXIT_SUCCESS;
}