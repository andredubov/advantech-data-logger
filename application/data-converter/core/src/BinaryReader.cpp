#include "BinaryReader.hpp"
#include "Logger.hpp"
#include <iostream>
#include <algorithm>

namespace app {
namespace core {

BinaryReader::BinaryReader(std::shared_ptr<app::utils::ILogger> logger, std::shared_ptr<ITimeFormatter> timeFormatter)
    : m_file()
    , m_filePath()
    , m_header()
    , m_totalFrames(0)
    , m_framesRead(0)
    , m_dataStartPos(0)
    , m_valuesPerFrame(0)
    , m_headerRead(false)
    , m_logger(logger)
    , m_timeFormatter(timeFormatter)
{
}

BinaryReader::~BinaryReader()
{
    close();
}

bool BinaryReader::open(const std::string& filePath)
{
    close();
    
    m_filePath = filePath;
    m_file.open(filePath, std::ios::binary | std::ios::ate);
    
    if (!m_file.is_open()) {
        m_logger->error("Failed to open binary file %s", filePath);
        return false;
    }
    
    return true;
}

bool BinaryReader::readHeader(DataHeader& header)
{
    if (!m_file.is_open()) {
        m_logger->error("File not open for reading header.");
        return false;
    }
    
    // Перемещаемся в начало файла
    m_file.seekg(0, std::ios::beg);
    
    // Читаем заголовок
    if (!readFileHeader(header)) {
        return false;
    }
    
    // Проверяем магическое число
    if (header.magic != 0x50434931) {
        m_logger->error("Invalid file format (magic number mismatch). Expected 0x50434931, got 0x%X", header.magic);
        return false;
    }
    
    // Поддерживается только версия 3
    if (header.version != 3) {
        m_logger->error("Unsupported file version: %u. Only version 3 is supported.", header.version);
        return false;
    }
    
    // Сохраняем заголовок и парсим его
    m_header = header;
    if (!parseHeader(header)) {
        return false;
    }
    
    m_headerRead = true;
    return true;
}

bool BinaryReader::readFileHeader(DataHeader& header)
{
    // Читаем магическое число и версию
    m_file.read(reinterpret_cast<char*>(&header.magic), sizeof(header.magic));
    m_file.read(reinterpret_cast<char*>(&header.version), sizeof(header.version));
    m_file.read(reinterpret_cast<char*>(&header.samplingRate), sizeof(header.samplingRate));

    if (!m_file) {
        m_logger->error("Failed to read file header!");
        return false;
    }

    // Версия 3: channelCount + время старта + время окончания
    m_file.read(reinterpret_cast<char*>(&header.startChannel), sizeof(header.startChannel));
    m_file.read(reinterpret_cast<char*>(&header.endChannel), sizeof(header.endChannel));
    m_file.read(reinterpret_cast<char*>(&header.channelCount), sizeof(header.channelCount));
    m_file.read(reinterpret_cast<char*>(&header.startTimeSeconds), sizeof(header.startTimeSeconds));
    m_file.read(reinterpret_cast<char*>(&header.endTimeSeconds), sizeof(header.endTimeSeconds));

    m_logger->info("Header: magic=0x%X, version=%u, samplingRate=%.0f, startChannel=%u, endChannel=%u, channelCount=%u, startTime=%s, endTime=%s",
        header.magic, 
        header.version, 
        header.samplingRate, 
        header.startChannel, 
        header.endChannel, 
        header.channelCount, 
        m_timeFormatter->formatTime(header.startTimeSeconds).c_str(), 
        m_timeFormatter->formatTime(header.endTimeSeconds).c_str()
    );
    
    if (!m_file) {
        m_logger->error("Failed to read channel count and start/end time from header!");
        return false;
    }
    
    m_dataStartPos = m_file.tellg();
    
    return true;
}

bool BinaryReader::parseHeader(const DataHeader& header)
{
    // Вычисляем количество кадров
    // Определяем размер файла
    m_file.seekg(0, std::ios::end);
    std::streamsize fileSize = m_file.tellg();
    m_file.seekg(m_dataStartPos, std::ios::beg);
    
    std::streamsize dataSize = fileSize - m_dataStartPos;
    m_valuesPerFrame = 1 + header.channelCount; // (time, ch0...chN)
    m_totalFrames = dataSize / (m_valuesPerFrame * sizeof(double));

    m_logger->debug(
        "parseHeader: fileSize=%lld, dataStartPos=%lld, dataSize=%lld, m_valuesPerFrame=%zu, m_totalFrames=%zu",
        static_cast<long long>(fileSize),
        static_cast<long long>(m_dataStartPos),
        static_cast<long long>(dataSize),
        m_valuesPerFrame,
        m_totalFrames
    );

    return true;
}

bool BinaryReader::readFrames(std::vector<DataFrame>& frames, std::size_t maxFrames)
{
    m_logger->debug("readFrames called: maxFrames=%zu, m_totalFrames=%zu", maxFrames, m_totalFrames);
    
    if (!m_headerRead) {
        m_logger->error("Header not read before reading frames.");
        return false;
    }
    
    if (frames.size() < maxFrames) {
        frames.resize(maxFrames);
    }
    
    // Определяем размер буфера для чтения
    std::size_t framesToRead = std::min(maxFrames, m_totalFrames - m_framesRead);
    m_logger->info("framesToRead=%zu", framesToRead);
    if (framesToRead == 0) {
        return true;
    }
    
    // Читаем данные в буфер
    std::vector<double> buffer(framesToRead * m_valuesPerFrame);
    std::streamsize bytesToRead = framesToRead * m_valuesPerFrame * sizeof(double);
    
    m_file.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);
    
    if (!m_file) {
        m_logger->error("Failed to read frame data.");
        return false;
    }
    
    // Заполняем кадры
    for (std::size_t i = 0; i < framesToRead; ++i) {
        frames[i].time = buffer[i * m_valuesPerFrame];
        frames[i].channels.resize(m_header.channelCount);
        
        for (std::size_t ch = 0; ch < m_header.channelCount; ++ch) {
            frames[i].channels[ch] = buffer[i * m_valuesPerFrame + 1 + ch];
        }
    }
    
    m_framesRead += framesToRead;
    
    // Отладочный вывод первого кадра
    if (framesToRead > 0) {
        m_logger->debug("[BinaryReader] First frame time read: %.6f", frames[0].time);
    }
    
    return true;
}

std::size_t BinaryReader::getTotalFrames() const
{
    return m_totalFrames;
}

bool BinaryReader::isOpen() const
{
    return m_file.is_open();
}

void BinaryReader::close()
{
    if (m_file.is_open()) {
        m_file.close();
    }
    m_headerRead = false;
    m_totalFrames = 0;
    m_dataStartPos = 0;
    m_valuesPerFrame = 0;
}

} // namespace core
} // namespace app