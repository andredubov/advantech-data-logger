#include "CsvWriter.hpp"
#include <iostream>
#include <iomanip>

namespace app {
namespace core {

CsvWriter::CsvWriter(std::shared_ptr<ITimeFormatter> timeFormatter)
    : m_file()
    , m_filePath()
    , m_timeFormatter(timeFormatter)
    , m_header()
    , m_headerWritten(false)
    , m_framesWritten(0)
{
}

CsvWriter::~CsvWriter()
{
    close();
}

bool CsvWriter::open(const std::string& filePath)
{
    close();
    
    m_filePath = filePath;
    m_file.open(filePath);
    
    if (!m_file.is_open()) {
        std::cerr << "Error: Failed to open CSV file: " << filePath << std::endl;
        return false;
    }
    
    // Используем стандартную локаль "C" для предсказуемого формата чисел
    m_file.imbue(std::locale("C"));
    m_file << std::fixed << std::setprecision(6);
    
    return true;
}

bool CsvWriter::writeHeader(const DataHeader& header)
{
    if (!m_file.is_open()) {
        std::cerr << "Error: CSV file not open for writing header." << std::endl;
        return false;
    }
    
    m_header = header;
    
    // Записываем заголовок CSV
    m_file << "Absolute Time(s)";
    for (uint32_t ch = 0; ch < header.channelCount; ++ch) {
        m_file << ";Channel " << ch;
    }
    m_file << "\n";
    
    m_headerWritten = true;
    return true;
}

bool CsvWriter::writeFrames(const std::vector<DataFrame>& frames)
{
    if (!m_headerWritten) {
        std::cerr << "Error: Header must be written before frames." << std::endl;
        return false;
    }
    
    if (!m_file.is_open()) {
        std::cerr << "Error: CSV file not open for writing frames." << std::endl;
        return false;
    }
    
    for (const auto& frame : frames) {
        if (!writeFrame(frame)) {
            return false;
        }
        ++m_framesWritten;
    }
    
    return true;
}

bool CsvWriter::writeFrame(const DataFrame& frame)
{
    // Преобразование времени в формат DD.MM.YYYY HH:MM:SS,ms
    std::string timeStr = m_timeFormatter->formatTime(frame.time);
    
    m_file << timeStr;
    
    // Записываем значения каналов
    for (size_t i = 0; i < frame.channels.size(); ++i) {
        m_file << ";" << std::showpos << frame.channels[i] << std::noshowpos;
    }
    m_file << "\n";
    
    return m_file.good();
}

bool CsvWriter::finalize()
{
    if (!m_file.is_open()) {
        return false;
    }
    
    m_file.flush();
    return m_file.good();
}

bool CsvWriter::isOpen() const
{
    return m_file.is_open();
}

void CsvWriter::close()
{
    if (m_file.is_open()) {
        m_file.flush();
        m_file.close();
    }
    m_headerWritten = false;
    m_framesWritten = 0;
}

} // namespace core
} // namespace app
