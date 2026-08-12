#include "AcquisitionManager.hpp"
#include <cstdio>
#include <iostream>

namespace app {

AcquisitionManager::AcquisitionManager(
    IDataAcquisitionDevice* device,
    DataProcessingEngine* engine,
    IDataWriter* writer,
    ILogger* logger,
    const command_line_options* options)
    : m_device(device)
    , m_engine(engine)
    , m_writer(writer)
    , m_logger(logger)
    , m_options(options)
    , m_initialized(false)
    , m_acquisitionStarted(false)
{}

AcquisitionManager::~AcquisitionManager() {
    shutdown();
}

bool AcquisitionManager::initialize() {
    if (!m_device || !m_engine || !m_writer || !m_logger || !m_options) {
        m_logger->error("Invalid dependencies in AcquisitionManager");
        return false;
    }

    // Инициализация устройства
    if (!m_device->initialize(m_options->get_device_description())) {
        m_logger->error("Failed to initialize device");
        return false;
    }

    // Настройка устройства
    int channelCount = m_options->get_channel_count();
    int startChannel = m_options->get_start_channel();
    int endChannel = m_options->get_end_channel();
    int samplesPerChannel = m_options->get_samples_per_channel();
    double samplingRate = m_options->get_sampling_rate();

    if (!m_device->configure(startChannel, channelCount, samplesPerChannel, samplingRate)) {
        m_logger->error("Failed to configure device");
        return false;
    }

    // Открываем файл для записи
    if (!m_writer->open(m_options->get_output_file_path())) {
        m_logger->error("Failed to open output file");
        return false;
    }

    m_initialized = true;
    printConfiguration();
    return true;
}

bool AcquisitionManager::startAcquisition() {
    if (!m_initialized) {
        m_logger->error("Manager not initialized");
        return false;
    }

    if (m_acquisitionStarted) {
        m_logger->warning("Acquisition already started");
        return true;
    }

    // Подготовка движка
    int channelCount = m_options->get_channel_count();
    double samplingRate = m_options->get_sampling_rate();

    // Настройка callback для устройства
    setupDeviceCallback();

    // Запуск движка обработки
    m_engine->start(samplingRate, channelCount);

    // Старт сбора данных
    if (!m_device->start()) {
        m_logger->error("Failed to start data acquisition");
        m_engine->stop();
        return false;
    }

    m_acquisitionStarted = true;

    std::printf("\n========================================================\n");
    std::printf("Data acquisition from %s at %.0f Hz STARTED.\n",
        m_options->get_device_description().c_str(),
        m_options->get_sampling_rate()
    );
    std::printf("Data is continuously written to binary file...\n");
    std::printf("Press ENTER to stop the program safely.\n");
    std::printf("========================================================\n\n");

    return true;
}

void AcquisitionManager::waitForStop() {
    if (m_acquisitionStarted) {
        std::cin.get();
    }
}

void AcquisitionManager::stopAcquisition() {
    if (!m_acquisitionStarted) {
        return;
    }

    std::printf("Stopping data acquisition on device...\n");

    // Остановка устройства
    m_device->stop();

    // Остановка движка (закрывает файл)
    m_engine->stop();

    m_acquisitionStarted = false;
}

void AcquisitionManager::shutdown() {
    if (m_acquisitionStarted) {
        stopAcquisition();
    }

    // Освобождение устройства
    if (m_device) {
        m_device->dispose();
    }

    m_initialized = false;
    std::printf("Program finished successfully. All resources released.\n");
}

void AcquisitionManager::setupDeviceCallback() {
    if (!m_device) return;

    // Связываем callback устройства с движком
    m_device->setDataReadyCallback([this](const std::vector<double>& data) {
        // Копируем данные в движок
        std::vector<double> localData = data; // Пришлось скопировать, т.к. callback передаёт const
        m_engine->pushData(std::move(localData));
    });
}

void AcquisitionManager::printConfiguration() const {
    std::printf("========================================================\n");
    std::printf("Data Logger Configuration:\n");
    std::printf("  Device:        %s\n", m_options->get_device_description().c_str());
    std::printf("  Channels:      %d-%d (%d channels)\n",
        m_options->get_start_channel(),
        m_options->get_end_channel(),
        m_options->get_channel_count());
    std::printf("  Sampling rate: %.0f Hz\n", m_options->get_sampling_rate());
    std::printf("  Buffer size:   %d samples per channel\n", m_options->get_samples_per_channel());
    std::printf("  Output file:   %s\n", m_options->get_output_file_path().c_str());
    std::printf("========================================================\n\n");
}

} // namespace app
