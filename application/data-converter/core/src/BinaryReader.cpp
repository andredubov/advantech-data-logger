#include "BinaryReader.hpp"
#include <iostream>
#include <algorithm>

namespace app {
namespace core {

BinaryReader::BinaryReader()
    : m_file()
    , m_filePath()
    , m_header()
    , m_totalFrames(0)
    , m_dataStartPos(0)
    , m_valuesPerFrame(0)
    , m_headerRead(false)
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
        std::cerr << "Error: Failed to open binary file: " << filePath << std::endl;
        return false;
    }
    
    return true;
}

bool BinaryReader::readHeader(DataHeader& header)
{
    if (!m_file.is_open()) {
        std::cerr << "Error: File not open for reading header." << std::endl;
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
        std::cerr << "Error: Invalid file format (magic number mismatch). Expected 0x50434931, got 0x" 
                  << std::hex << header.magic << std::dec << std::endl;
        return false;
    }
    
    // Поддерживается только версия 3
    if (header.version != 3) {
        std::cerr << "Error: Unsupported file version: " << header.version 
                  << ". Only version 3 is supported." << std::endl;
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
        std::cerr << "Error: Failed to read file header!" << std::endl;
        return false;
    }
    
    // Версия 3: channelCount + время старта + время окончания
    m_file.read(reinterpret_cast<char*>(&header.channelCount), sizeof(header.channelCount));
    m_file.read(reinterpret_cast<char*>(&header.startTimeSeconds), sizeof(header.startTimeSeconds));
    m_file.read(reinterpret_cast<char*>(&header.endTimeSeconds), sizeof(header.endTimeSeconds));
    
    if (!m_file) {
        std::cerr << "Error: Failed to read channel count and start/end time from header!" << std::endl;
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
    
    return true;
}

bool BinaryReader::readFrames(std::vector<DataFrame>& frames, size_t maxFrames)
{
    if (!m_headerRead) {
        std::cerr << "Error: Header not read before reading frames." << std::endl;
        return false;
    }
    
    if (frames.size() < maxFrames) {
        frames.resize(maxFrames);
    }
    
    // Определяем размер буфера для чтения
    size_t framesToRead = std::min(maxFrames, m_totalFrames - getTotalFrames());
    if (framesToRead == 0) {
        return true;
    }
    
    // Читаем данные в буфер
    std::vector<double> buffer(framesToRead * m_valuesPerFrame);
    std::streamsize bytesToRead = framesToRead * m_valuesPerFrame * sizeof(double);
    
    m_file.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);
    
    if (!m_file) {
        std::cerr << "Error: Failed to read frame data." << std::endl;
        return false;
    }
    
    // Заполняем кадры
    for (size_t i = 0; i < framesToRead; ++i) {
        frames[i].time = buffer[i * m_valuesPerFrame];
        frames[i].channels.resize(m_header.channelCount);
        
        for (size_t ch = 0; ch < m_header.channelCount; ++ch) {
            frames[i].channels[ch] = buffer[i * m_valuesPerFrame + 1 + ch];
        }
    }
    
    return true;
}

size_t BinaryReader::getTotalFrames() const
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