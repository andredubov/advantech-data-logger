#ifndef BINARYFILEWRITER_HPP
#define BINARYFILEWRITER_HPP

#include "IDataWriter.hpp"
#include "ILogger.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <ctime>
#include <memory>

namespace app {

class BinaryFileWriter : public IDataWriter {
public:
    BinaryFileWriter(std::shared_ptr<app::ILogger> logger);
    ~BinaryFileWriter() override;

    bool open(const std::string& filePath) override;
    void write(const std::vector<double>& data) override;
    void writeFrame(const DataFrame& frame) override;
    void flush() override;
    void close() override;
    void setMetadata(double samplingRate, int startChannel, int endChannel, double startTime, double endTime) override;

    uint64_t getTotalFramesWritten() const override { 
        return m_totalFramesWritten; 
    }

private:
    std::ofstream m_file;
    std::string m_filePath;
    double m_samplingRate;
    int m_startChannel;
    int m_endChannel;
    int m_channelCount;
    double m_startTime;
    double m_endTime;
    uint64_t m_totalFramesWritten;
    std::streampos m_endTimePos;
    std::shared_ptr<app::ILogger> m_logger;

    void writeHeader();
    void writeEndTime();
    std::string formatTime(double seconds) const;
};

} // namespace app

#endif // BINARYFILEWRITER_HPP