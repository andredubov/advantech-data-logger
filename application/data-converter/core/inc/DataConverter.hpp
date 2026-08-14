#ifndef DATA_CONVERTER_HPP
#define DATA_CONVERTER_HPP

#include <memory>
#include <string>
#include <vector>
#include "IDataReader.hpp"
#include "IDataWriter.hpp"
#include "ILogger.hpp"

namespace app {
namespace core {

// Класс, управляющий процессом конвертации
class DataConverter {
public:
    DataConverter(
        std::shared_ptr<app::utils::ILogger> logger,
        std::shared_ptr<IDataReader> reader,
        std::shared_ptr<IDataWriter> writer
    );
    ~DataConverter() = default;

    // Установить размер чанка для чтения
    void setChunkSize(size_t chunkSize);

    // Выполнить конвертацию
    bool convert(const std::string& inputFile, const std::string& outputFile);

    // Получить прогресс (0-100)
    int getProgress() const;

private:
    std::shared_ptr<IDataReader> m_reader;
    std::shared_ptr<IDataWriter> m_writer;
    size_t m_chunkSize;
    int m_progress;
    bool m_useProgress;
    std::shared_ptr<app::utils::ILogger> m_logger;

    bool processFrames(size_t totalFrames);
    void reportProgress(size_t processed, size_t total);
};

} // namespace core
} // namespace app

#endif // DATA_CONVERTER_HPP