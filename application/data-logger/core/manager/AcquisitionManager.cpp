#include <chrono>
#include <cstdio>
#include <iostream>
#include "AcquisitionManager.hpp"

namespace app {

AcquisitionManager::AcquisitionManager(
    std::shared_ptr<app::IDataAcquisitionDevice> device,
    std::shared_ptr<app::DataProcessingEngine> engine,
    std::shared_ptr<app::IDataWriter> writer,
    std::shared_ptr<app::ILogger> logger,
    std::shared_ptr<app::CommandLineOptions> options)
    : m_device(device)
    , m_engine(engine)
    , m_writer(writer)
    , m_logger(logger)
    , m_options(options)
    , m_initialized(false)
    , m_acquisitionStarted(false)
    , m_shutdownCalled(false)
{

}

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

    std::string inputMode = m_options->get_input_mode();
    std::string inputRange = m_options->get_input_range();
    app::AcquisitionConfig config;
    config.startChannel = startChannel;
    config.channelCount = channelCount;
    config.samplesPerChannel = samplesPerChannel;
    config.samplingRate = samplingRate;
    config.inputMode = inputMode;
    config.inputRange = inputRange;

    if (!m_device->configure(config)) {
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
    int startChannel = m_options->get_start_channel();
    int endChannel = m_options->get_end_channel();
    double samplingRate = m_options->get_sampling_rate();

    // Настройка callback для устройства
    setupDeviceCallback();

    // Запуск движка обработки
    m_engine->start(samplingRate, startChannel, endChannel);

    // Старт сбора данных
    if (!m_device->start()) {
        m_logger->error("Failed to start data acquisition");
        m_engine->stop();
        return false;
    }

    m_acquisitionStarted = true;

    m_logger->info("Data acquisition from %s at %.0f Hz STARTED.",
        m_options->get_device_description().c_str(),
        m_options->get_sampling_rate()
    );
    m_logger->info("Data is continuously written to binary file...");
    m_logger->info("Press ENTER to stop the program safely.");

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

    m_logger->info("Stopping data acquisition on device...");

    // Остановка устройства
    m_device->stop();

    // Остановка движка (закрывает файл)
    m_engine->stop();

    m_acquisitionStarted = false;
}

void AcquisitionManager::shutdown() {
    if (m_shutdownCalled) {
        return;
    }
    m_shutdownCalled = true;

    if (m_acquisitionStarted) {
        stopAcquisition();
    }

    // Освобождение устройства
    if (m_device) {
        m_device->dispose();
    }

    m_initialized = false;
    m_logger->info("Program finished successfully. All resources released.");
}

void AcquisitionManager::setupDeviceCallback() {
    if (!m_device) {
        return;
    }

    // Связываем callback устройства с движком
    m_device->setDataReadyCallback([this](const std::vector<double>& data) {
        // data содержит значения для всех каналов и фреймов
        // Нужно разбить на отдельные фреймы
        int channelCount = m_options->get_channel_count();
        size_t framesCount = data.size() / channelCount;

        for (size_t i = 0; i < framesCount; ++i) {
            std::vector<double> voltages;
            voltages.reserve(channelCount);
            for (int ch = 0; ch < channelCount; ++ch) {
                voltages.push_back(data[i * channelCount + ch]);
            }

            double timestamp = std::chrono::duration<double>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

            app::DataFrame frame(timestamp, std::move(voltages));
            m_engine->pushDataFrame(std::move(frame));
        }
    });
}

void AcquisitionManager::printConfiguration() const {
    m_logger->info("Data Logger Configuration:");
    m_logger->info("  Device:        %s", m_options->get_device_description().c_str());
    m_logger->info("  Channels:      %d-%d (%d channels)", 
        m_options->get_start_channel(),
        m_options->get_end_channel(),
        m_options->get_channel_count()
    );
    m_logger->info("  Sampling rate: %.0f Hz", m_options->get_sampling_rate());
    m_logger->info("  Buffer size:   %d samples per channel", m_options->get_samples_per_channel());
    m_logger->info("  Output file:   %s", m_options->get_output_file_path().c_str());
}

} // namespace app
