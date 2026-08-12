#include "DataConverter.hpp"
#include <iostream>
#include <algorithm>

namespace app {
namespace core {

DataConverter::DataConverter(
    std::shared_ptr<IDataReader> reader,
    std::shared_ptr<IDataWriter> writer
)
    : m_reader(reader)
    , m_writer(writer)
    , m_chunkSize(25000)
    , m_progress(0)
    , m_useProgress(true)
{
}

void DataConverter::setChunkSize(size_t chunkSize)
{
    m_chunkSize = chunkSize;
}

bool DataConverter::convert(const std::string& inputFile, const std::string& outputFile)
{
    m_progress = 0;
    
    // 1. Открываем входной файл
    if (!m_reader->open(inputFile)) {
        std::cerr << "Error: Failed to open input file: " << inputFile << std::endl;
        return false;
    }
    
    // 2. Читаем заголовок
    DataHeader header;
    if (!m_reader->readHeader(header)) {
        std::cerr << "Error: Failed to read header from input file." << std::endl;
        return false;
    }
    
    // 3. Открываем выходной файл
    if (!m_writer->open(outputFile)) {
        std::cerr << "Error: Failed to open output file: " << outputFile << std::endl;
        return false;
    }
    
    // 4. Записываем заголовок в выходной файл
    if (!m_writer->writeHeader(header)) {
        std::cerr << "Error: Failed to write header to output file." << std::endl;
        return false;
    }
    
    // 5. Обрабатываем кадры
    size_t totalFrames = m_reader->getTotalFrames();
    if (totalFrames == 0) {
        std::cout << "Warning: No data frames found in file." << std::endl;
        m_writer->finalize();
        return true;
    }
    
    std::cout << "Total frames: " << totalFrames << std::endl;
    std::cout << "Conversion started, please wait..." << std::endl;
    
    if (!processFrames(totalFrames)) {
        std::cerr << "Error: Failed to process frames." << std::endl;
        return false;
    }
    
    // 6. Завершаем запись
    if (!m_writer->finalize()) {
        std::cerr << "Error: Failed to finalize output file." << std::endl;
        return false;
    }
    
    std::cout << "\nConversion completed successfully!" << std::endl;
    return true;
}

bool DataConverter::processFrames(size_t totalFrames)
{
    size_t framesProcessed = 0;
    std::vector<DataFrame> frames;
    frames.reserve(m_chunkSize);
    
    while (framesProcessed < totalFrames) {
        size_t toRead = std::min(m_chunkSize, totalFrames - framesProcessed);
        frames.resize(toRead);
        
        if (!m_reader->readFrames(frames, toRead)) {
            return false;
        }
        
        if (!m_writer->writeFrames(frames)) {
            return false;
        }
        
        framesProcessed += toRead;
        reportProgress(framesProcessed, totalFrames);
    }
    
    return true;
}

void DataConverter::reportProgress(size_t processed, size_t total)
{
    if (!m_useProgress) {
        return;
    }
    
    int progress = static_cast<int>((static_cast<double>(processed) / total) * 100);
    if (progress != m_progress) {
        m_progress = progress;
        std::cout << "\rProgress: " << progress << "% (" << processed << "/" << total << ")" << std::flush;
    }
}

int DataConverter::getProgress() const
{
    return m_progress;
}

} // namespace core
} // namespace app
