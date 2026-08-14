#ifndef I_DATA_READER_HPP
#define I_DATA_READER_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace app {
namespace core {

// Структура заголовка данных
struct DataHeader {
    uint32_t magic;
    uint32_t version;
    double samplingRate;
    uint32_t startChannel;
    uint32_t endChannel;
    uint32_t channelCount;
    double startTimeSeconds;
    double endTimeSeconds;
};

// Структура кадра данных
struct DataFrame {
    double time;
    std::vector<double> channels;
};

// Интерфейс для чтения данных
class IDataReader {
public:
    virtual ~IDataReader() = default;

    // Открыть файл
    virtual bool open(const std::string& filePath) = 0;
    
    // Прочитать заголовок
    virtual bool readHeader(DataHeader& header) = 0;
    
    // Прочитать кадры данных (по блокам)
    virtual bool readFrames(std::vector<DataFrame>& frames, std::size_t maxFrames) = 0;
    
    // Получить общее количество кадров
    virtual size_t getTotalFrames() const = 0;
    
    // Проверить, открыт ли файл
    virtual bool isOpen() const = 0;
    
    // Закрыть файл
    virtual void close() = 0;
};

} // namespace core
} // namespace app

#endif // I_DATA_READER_HPP