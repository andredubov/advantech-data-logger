#ifndef BINARY_READER_HPP
#define BINARY_READER_HPP

#include "IDataReader.hpp"
#include <fstream>
#include <cstdint>

namespace app {
namespace core {

class BinaryReader : public IDataReader {
public:
    BinaryReader();
    ~BinaryReader() override;

    // IDataReader implementation
    bool open(const std::string& filePath) override;
    bool readHeader(DataHeader& header) override;
    bool readFrames(std::vector<DataFrame>& frames, size_t maxFrames) override;
    size_t getTotalFrames() const override;
    bool isOpen() const override;
    void close() override;

private:
    std::ifstream m_file;
    std::string m_filePath;
    DataHeader m_header;
    size_t m_totalFrames;
    std::streamsize m_dataStartPos;
    size_t m_valuesPerFrame;
    bool m_headerRead;

    bool readFileHeader(DataHeader& header);
    bool parseHeader(const DataHeader& header);
};

} // namespace core
} // namespace app

#endif // BINARY_READER_HPP