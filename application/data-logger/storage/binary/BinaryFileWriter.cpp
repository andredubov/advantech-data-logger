#include "BinaryFileWriter.hpp"
#include <cstdio>
#include <cstring>

namespace app {

BinaryFileWriter::BinaryFileWriter()
    : m_file()
    , m_filePath()
    , m_samplingRate(0.0)
    , m_channelCount(0)
    , m_startTime(0.0)
    , m_endTime(0.0)
    , m_totalFramesWritten(0)
    , m_endTimePos()
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
        std::printf("[Writer Thread] Critical error: Failed to create file for writing!\n");
        return false;
    }
    std::printf("[Writer Thread] Binary file %s opened successfully.\n", filePath.c_str());
    return true;
}

void BinaryFileWriter::setMetadata(double samplingRate, int channelCount, double startTime, double endTime) {
    m_samplingRate = samplingRate;
    m_channelCount = channelCount;
    m_startTime = startTime;
    m_endTime = endTime;
    
    // Файл должен быть открыт до вызова этого метода (AcquisitionManager::initialize())
    if (!m_file.is_open()) {
        std::printf("[Writer Thread] Critical error: File is not open! Cannot write header.\n");
        return;
    }
    
    writeHeader();
}

void BinaryFileWriter::writeHeader() {
    const uint32_t magic = 0x50434931; // "PCI1"
    const uint32_t version = 3; // Версия 3: добавлена поддержка нескольких каналов
    const double samplingRateLocal = m_samplingRate;
    const uint32_t channelCountLocal = static_cast<uint32_t>(m_channelCount);
    double endTimeSeconds = 0.0; // Временно

    m_file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    m_file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    m_file.write(reinterpret_cast<const char*>(&samplingRateLocal), sizeof(samplingRateLocal));
    m_file.write(reinterpret_cast<const char*>(&channelCountLocal), sizeof(channelCountLocal));
    m_file.write(reinterpret_cast<const char*>(&m_startTime), sizeof(m_startTime));

    // Запоминаем позицию для времени окончания
    m_endTimePos = m_file.tellp();
    m_file.write(reinterpret_cast<const char*>(&endTimeSeconds), sizeof(endTimeSeconds));

    if (!m_file) {
        std::printf("[Writer Thread] Critical error: Failed to write file header!\n");
        m_file.close();
        return;
    }

    std::printf("[Writer Thread] File header written successfully. Channels: %u, Start time: %s\n",
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

    std::streamsize bytesToWrite = timedData.size() * sizeof(double);
    m_file.write(reinterpret_cast<const char*>(timedData.data()), bytesToWrite);
    m_totalFramesWritten += framesInBuffer;

    if (!m_file) {
        std::printf("[Writer Thread] Critical error: Physical disk write failure!\n");
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
        std::printf("[Writer Thread] Warning: Failed to write end time to file header!\n");
    }
    m_file.seekp(0, std::ios::end);
    m_file.close();

    std::printf("[Writer Thread] All data flushed to disk successfully. Total frames: %llu (channels: %d). File closed.\n",
        m_totalFramesWritten,
        m_channelCount
    );
    std::printf("[Writer Thread] End time: %s\n", formatTime(m_endTime).c_str());
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
