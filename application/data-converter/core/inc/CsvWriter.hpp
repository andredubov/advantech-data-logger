#ifndef CSV_WRITER_HPP
#define CSV_WRITER_HPP

#include "IDataWriter.hpp"
#include "ITimeFormatter.hpp"
#include "ILogger.hpp"
#include <fstream>
#include <memory>

namespace app {
namespace core {

class CsvWriter : public IDataWriter {
public:
    explicit CsvWriter(std::shared_ptr<app::utils::ILogger> logger, std::shared_ptr<ITimeFormatter> timeFormatter);
    ~CsvWriter() override;

    // IDataWriter implementation
    bool open(const std::string& filePath) override;
    bool writeHeader(const DataHeader& header) override;
    bool writeFrames(const std::vector<DataFrame>& frames) override;
    bool finalize() override;
    bool isOpen() const override;
    void close() override;

private:
    std::ofstream m_file;
    std::string m_filePath;
    std::shared_ptr<ITimeFormatter> m_timeFormatter;
    DataHeader m_header;
    bool m_headerWritten;
    size_t m_framesWritten;
    std::shared_ptr<app::utils::ILogger> m_logger;

    bool writeFrame(const DataFrame& frame);
};

} // namespace core
} // namespace app

#endif // CSV_WRITER_HPP