#include "BinaryFileWriter.hpp"
#include <cstdio>
#include <cstring>

namespace app {

BinaryFileWriter::BinaryFileWriter(std::shared_ptr<app::ILogger> logger)
    : m_file()
    , m_filePath()
    , m_samplingRate(0.0)
    , m_startChannel(0)
    , m_endChannel(0)
    , m_channelCount(0)
    , m_startTime(0.0)
    , m_endTime(0.0)
    , m_totalFramesWritten(0)
    , m_endTimePos()
    , m_logger(logger)
{}

BinaryFileWriter::~BinaryFileWriter() {
    if (m_file.is_open()) {
        close();
    }
}

bool BinaryFileWriter::open(const std::string& filePath) {
    m_filePath = filePath;
    m_file.open(filePath, std::ios::binary | std::ios::out);
    if (!m_file.is_open()) {
        if (m_logger) {
            m_logger->error("[Writer Thread] Critical error: Failed to create file for writing!");
        }
        return false;
    }
    if (m_logger) {
        m_logger->info("[Writer Thread] Binary file %s opened successfully.", filePath.c_str());
    }
    return true;
}

void BinaryFileWriter::setMetadata(double samplingRate, int startChannel, int endChannel,  double startTime, double endTime) {
    m_samplingRate = samplingRate;
    m_startChannel = startChannel;
    m_endChannel = endChannel;
    m_channelCount = endChannel - startChannel + 1;
    m_startTime = startTime;
    m_endTime = endTime;
    m_logger->info(
        "[Writer Thread] setMetadata called: samplingRate=%.0f, startChannel=%d, endChannel=%d, channelCount=%d, startTime=%.6f",
        samplingRate, 
        startChannel, 
        endChannel, 
        m_channelCount, 
        startTime
    );
    // Файл должен быть открыт до вызова этого метода (AcquisitionManager::initialize())
    if (!m_file.is_open()) {
        if (m_logger) {
            m_logger->error("[Writer Thread] Critical error: File is not open! Cannot write header.");
        }
        return;
    }
    
    writeHeader();
}

void BinaryFileWriter::writeHeader() {
    const uint32_t magic = 0x50434931; // "PCI1"
    const uint32_t version = 3; // Версия 3: добавлена поддержка нескольких каналов
    const double samplingRateLocal = m_samplingRate;
    const uint32_t startChannelLocal = static_cast<uint32_t>(m_startChannel);
    const uint32_t endChannelLocal = static_cast<uint32_t>(m_endChannel);
    const uint32_t channelCountLocal = static_cast<uint32_t>(m_channelCount);    
    double endTimeSeconds = 0.0; // Временно

    m_file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    m_file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    m_file.write(reinterpret_cast<const char*>(&samplingRateLocal), sizeof(samplingRateLocal));
    m_file.write(reinterpret_cast<const char*>(&startChannelLocal), sizeof(startChannelLocal));
    m_file.write(reinterpret_cast<const char*>(&endChannelLocal), sizeof(endChannelLocal));
    m_file.write(reinterpret_cast<const char*>(&channelCountLocal), sizeof(channelCountLocal));
    m_file.write(reinterpret_cast<const char*>(&m_startTime), sizeof(m_startTime));

    // Запоминаем позицию для времени окончания
    m_endTimePos = m_file.tellp();
    m_file.write(reinterpret_cast<const char*>(&endTimeSeconds), sizeof(endTimeSeconds));

    if (!m_file) {
        m_logger->error("[Writer Thread] Critical error: Failed to write file header!");
        m_file.close();
        return;
    }

    m_logger->info(
        "[Writer Thread] File header written successfully. Channels: %u, Start time: %s",
        channelCountLocal,
        formatTime(m_startTime).c_str()
    );
}

void BinaryFileWriter::write(const std::vector<double>& data) {
    if (data.empty() || m_channelCount == 0) {
        return;
    }

    std::size_t framesInBuffer = data.size() / m_channelCount;
    std::vector<double> timedData;
    timedData.reserve(framesInBuffer * (1 + m_channelCount));

    const double timeStep = 1.0 / m_samplingRate;
    for (std::size_t frameIdx = 0; frameIdx < framesInBuffer; ++frameIdx) {
        double currentTime = m_startTime + (m_totalFramesWritten + frameIdx) * timeStep;
        timedData.push_back(currentTime);
        for (std::size_t ch = 0; ch < static_cast<std::size_t>(m_channelCount); ++ch) {
            timedData.push_back(data[frameIdx * m_channelCount + ch]);
        }
    }

    // Отладочный вывод для первого кадра
    if (0 == m_totalFramesWritten) {
        m_logger->debug("[Writer Thread] First frame time: %.6f (startTime=%.6f, timeStep=%.6f)",
            m_startTime + 0 * timeStep, 
            m_startTime, 
            timeStep
        );
    }

    std::streamsize bytesToWrite = timedData.size() * sizeof(double);
    m_file.write(reinterpret_cast<const char*>(timedData.data()), bytesToWrite);
    m_totalFramesWritten += framesInBuffer;

    if (!m_file) {
        m_logger->error("[Writer Thread] Critical error: Physical disk write failure!");
    }
}

void BinaryFileWriter::writeFrame(const DataFrame& frame) {
    if (!frame.isValid() || m_channelCount == 0) {
        return;
    }

    // Проверяем, что количество каналов в кадре соответствует ожидаемому
    if (frame.voltages.size() != static_cast<size_t>(m_channelCount)) {
        m_logger->warning(
            "[Writer Thread] Frame channel count mismatch: expected %d, got %zu",
            m_channelCount, 
            frame.voltages.size()
        );
        return;
    }

    // Записываем временную метку и напряжения для каждого канала
    m_file.write(reinterpret_cast<const char*>(&frame.timestamp), sizeof(frame.timestamp));
    m_file.write(reinterpret_cast<const char*>(frame.voltages.data()), frame.voltages.size() * sizeof(double));

    m_totalFramesWritten++;

    if (!m_file) {
        m_logger->error("[Writer Thread] Critical error: Physical disk write failure for frame!");
    }
}

void BinaryFileWriter::flush() {
    m_file.flush();
}

void BinaryFileWriter::close() {
    // Записываем время окончания
    const auto endTimePoint = std::chrono::system_clock::now();
    m_endTime = std::chrono::duration<double>(endTimePoint.time_since_epoch()).count();

    m_file.seekp(m_endTimePos);
    m_file.write(reinterpret_cast<const char*>(&m_endTime), sizeof(m_endTime));
    if (!m_file) {
        m_logger->warning("[Writer Thread] Warning: Failed to write end time to file header!");
    }
    m_file.seekp(0, std::ios::end);
    m_file.close();

    m_logger->info(
        "[Writer Thread] All data flushed to disk successfully. Total frames: %llu (channels: %d). File closed.",
        m_totalFramesWritten,
        m_channelCount
    );
    m_logger->info("[Writer Thread] End time: %s", formatTime(m_endTime).c_str());

}

void BinaryFileWriter::writeEndTime() {
    // Этот метод вызывается внутри close()
}

std::string BinaryFileWriter::formatTime(double seconds) const {
    time_t rawTime = static_cast<time_t>(seconds);
    struct tm timeInfo;
    ::localtime_s(&timeInfo, &rawTime);

    int microseconds = static_cast<int>((seconds - rawTime) * 1000000);

    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", &timeInfo);

    char result[80];
    std::snprintf(result, sizeof(result), "%s,%06d", buffer, microseconds);

    return std::string(result);
}

} // namespace app
