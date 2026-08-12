#ifndef BINARYFILEWRITER_HPP
#define BINARYFILEWRITER_HPP

#include "IDataWriter.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <ctime>

namespace app {

class BinaryFileWriter : public IDataWriter {
public:
    BinaryFileWriter();
    ~BinaryFileWriter() override;

    bool open(const std::string& filePath) override;
    void write(const std::vector<double>& data) override;
    void flush() override;
    void close() override;
    void setMetadata(double samplingRate, int channelCount, double startTime, double endTime) override;
    uint64_t getTotalFramesWritten() const override { return m_totalFramesWritten; }

private:
    std::ofstream m_file;
    std::string m_filePath;
    double m_samplingRate;
    int m_channelCount;
    double m_startTime;
    double m_endTime;
    uint64_t m_totalFramesWritten;
    std::streampos m_endTimePos;

    void writeHeader();
    void writeEndTime();
    std::string formatTime(double seconds) const;
};

} // namespace app

#endif // BINARYFILEWRITER_HPP
