#include "main.hpp"

/**
 * @brief Главная точка входа приложения с использованием SOLID-архитектуры
 * 
 * Архитектура:
 * - ConsoleLogger    — логирование в консоль
 * - BinaryFileWriter — запись в бинарный файл
 * - AdvantechDevice  — управление железом PCI-1716
 * - DataProcessingEngine — обработка данных (очередь + поток записи)
 * - AcquisitionManager   — оркестрация всего процесса
 */
int main(int argc, char* argv[]) {
    // 1. Парсинг аргументов командной строки
    auto options = std::make_shared<app::CommandLineOptions>();
    auto state = options->parse(argc, argv);

    switch (state) {
        case app::CommandLineOptions::state::success:
            break;
        case app::CommandLineOptions::state::version:
            std::cout << "v" << options->get_version() << std::endl;
            return EXIT_SUCCESS;
        case app::CommandLineOptions::state::help:
            std::cout << "Help: " << options->get_help() << std::endl;
            return EXIT_SUCCESS;
        default:
            std::cout << options->get_error_message() << std::endl;
            return EXIT_FAILURE;
    }

    // 2. Сборка зависимостей (Dependency Injection)
    auto logger = std::make_shared<app::Logger>();
    auto writer = std::make_shared<app::BinaryFileWriter>(logger);
    auto device = std::make_shared<app::AdvantechDevice>(logger);
    auto engine = std::make_shared<app::DataProcessingEngine>(writer, logger);

    // 3. Оркестрация процесса
    app::AcquisitionManager manager(device, engine, writer, logger, options);

    // 4. Инициализация
    if (!manager.initialize()) {
        logger->error("Initialization failed");
        return EXIT_FAILURE;
    }

    // 5. Запуск сбора данных
    if (!manager.startAcquisition()) {
        logger->error("Failed to start acquisition");
        return EXIT_FAILURE;
    }

    // 6. Ожидание остановки пользователем
    manager.waitForStop();

    // 7. Остановка и освобождение ресурсов
    manager.stopAcquisition();
    manager.shutdown();

    return EXIT_SUCCESS;
}
