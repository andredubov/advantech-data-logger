#ifndef I_DATA_WRITER_HPP
#define I_DATA_WRITER_HPP

#include <string>
#include <vector>
#include "IDataReader.hpp"

namespace app {
namespace core {

// Интерфейс для записи данных
class IDataWriter {
public:
    virtual ~IDataWriter() = default;

    // Открыть файл для записи
    virtual bool open(const std::string& filePath) = 0;
    
    // Записать заголовок
    virtual bool writeHeader(const DataHeader& header) = 0;
    
    // Записать кадры данных
    virtual bool writeFrames(const std::vector<DataFrame>& frames) = 0;
    
    // Завершить запись
    virtual bool finalize() = 0;
    
    // Проверить, открыт ли файл
    virtual bool isOpen() const = 0;
    
    // Закрыть файл
    virtual void close() = 0;
};

} // namespace core
} // namespace app

#endif // I_DATA_WRITER_HPP