#ifndef BINARY_READER_HPP
#define BINARY_READER_HPP

#include "IDataReader.hpp"
#include "ILogger.hpp"
#include "ITimeFormatter.hpp"
#include <fstream>
#include <cstdint>
#include <memory>

namespace app {
namespace core {

class BinaryReader : public IDataReader {
public:
    BinaryReader(std::shared_ptr<app::utils::ILogger> logger, std::shared_ptr<ITimeFormatter> timeFormatter);
    ~BinaryReader() override;

    // IDataReader implementation
    bool open(const std::string& filePath) override;
    bool readHeader(DataHeader& header) override;
    bool readFrames(std::vector<DataFrame>& frames, size_t maxFrames) override;
    std::size_t getTotalFrames() const override;
    bool isOpen() const override;
    void close() override;

private:
    std::ifstream m_file;
    std::string m_filePath;
    DataHeader m_header;
    std::size_t m_totalFrames;
    std::size_t m_framesRead;
    std::streamsize m_dataStartPos;
    std::size_t m_valuesPerFrame;
    bool m_headerRead;
    std::shared_ptr<app::utils::ILogger> m_logger;
    std::shared_ptr<ITimeFormatter> m_timeFormatter;

    bool readFileHeader(DataHeader& header);
    bool parseHeader(const DataHeader& header);
};

} // namespace core
} // namespace app

#endif // BINARY_READER_HPP