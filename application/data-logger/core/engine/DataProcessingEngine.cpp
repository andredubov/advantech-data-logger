#include "DataProcessingEngine.hpp"
#include <chrono>

namespace app {

DataProcessingEngine::DataProcessingEngine(IDataWriter* writer, ILogger* logger)
    : m_writer(writer)
    , m_logger(logger)
    , m_isRunning(false)
    , m_samplingRate(0.0)
    , m_channelCount(0)
    , m_startTime(0.0)
{}

DataProcessingEngine::~DataProcessingEngine() {
    if (m_isRunning) {
        stop();
    }
}

void DataProcessingEngine::start(double samplingRate, int channelCount) {
    m_samplingRate = samplingRate;
    m_channelCount = channelCount;
    m_isRunning = true;

    const auto startTimePoint = std::chrono::system_clock::now();
    m_startTime = std::chrono::duration<double>(startTimePoint.time_since_epoch()).count();

    // Передаём метаданные в writer
    m_writer->setMetadata(m_samplingRate, m_channelCount, m_startTime, 0.0);

    // Запускаем поток записи
    m_writerThread = std::thread(&DataProcessingEngine::writerThreadFunction, this);
}

void DataProcessingEngine::stop() {
    if (!m_isRunning) {
        return;
    }

    // Сигнализируем потоку, что новых данных не будет
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_isRunning = false;
    }
    m_queueCV.notify_one();

    // Ждём завершения потока записи
    if (m_writerThread.joinable()) {
        m_writerThread.join();
    }

    // Закрываем файл через writer
    m_writer->close();
}

void DataProcessingEngine::pushData(std::vector<double>&& data) {
    if (!m_isRunning || data.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_dataQueue.push(std::move(data));
    m_queueCV.notify_one();
}

uint64_t DataProcessingEngine::getTotalFramesWritten() const {
    return m_writer ? m_writer->getTotalFramesWritten() : 0;
}

void DataProcessingEngine::writerThreadFunction() {
    uint64_t totalFramesWritten = 0;

    while (true) {
        std::vector<double> localBuffer;

        // Извлечение данных из очереди
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queueCV.wait(lock, [this] {
                return !m_dataQueue.empty() || !m_isRunning;
            });

            if (!m_isRunning && m_dataQueue.empty()) {
                break;
            }

            localBuffer = std::move(m_dataQueue.front());
            m_dataQueue.pop();
        }

        // Запись данных через writer
        if (!localBuffer.empty()) {
            m_writer->write(localBuffer);
        }
    }

    // После завершения цикла все данные записаны
    m_logger->info("Writer thread finished processing all data.");
}

} // namespace app
