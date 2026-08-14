#include "DataConverter.hpp"
#include <iostream>
#include <algorithm>

namespace app {
namespace core {

DataConverter::DataConverter(
    std::shared_ptr<app::utils::ILogger> logger,
    std::shared_ptr<IDataReader> reader,
    std::shared_ptr<IDataWriter> writer
)
    : m_reader(reader)
    , m_writer(writer)
    , m_chunkSize(25000)
    , m_progress(0)
    , m_useProgress(true)
    , m_logger(logger)
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
        m_logger->error("Failed to open input file: %s", inputFile.c_str());
        return false;
    }
    
    // 2. Читаем заголовок
    DataHeader header;
    if (!m_reader->readHeader(header)) {
        m_logger->error("Failed to read header from input file.");
        return false;
    }
    
    // 3. Открываем выходной файл
    if (!m_writer->open(outputFile)) {
        m_logger->error("Failed to open output file: %s", outputFile.c_str());
        return false;
    }
    
    // 4. Записываем заголовок в выходной файл
    if (!m_writer->writeHeader(header)) {
        m_logger->error("Failed to write header to output file.");
        return false;
    }
    
    // 5. Обрабатываем кадры
    size_t totalFrames = m_reader->getTotalFrames();
    if (totalFrames == 0) {
        m_logger->warning("No data frames found in file.");
        m_writer->finalize();
        return true;
    }
    
    m_logger->info("Total frames: %zu", totalFrames);
    m_logger->info("Conversion started, please wait...");
    
    if (!processFrames(totalFrames)) {
        m_logger->error("Failed to process frames.");
        return false;
    }
    
    // 6. Завершаем запись
    if (!m_writer->finalize()) {
        m_logger->error("Failed to finalize output file.");
        return false;
    }
    
    m_logger->debug("Conversion completed successfully!");
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
        m_logger->info("\r");
        m_logger->info("Progress: %d%% (%zu/%zu)", progress, processed, total);
    }
}

int DataConverter::getProgress() const
{
    return m_progress;
}

} // namespace core
} // namespace app
