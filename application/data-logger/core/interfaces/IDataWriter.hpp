#ifndef IDATAWRITER_HPP
#define IDATAWRITER_HPP

#include <string>
#include <vector>
#include <cstdint>
#include "DataFrame.hpp"

namespace app {

/**
 * @brief Interface for writing acquired data to a file
 */
class IDataWriter {
public:
    virtual ~IDataWriter() = default;

    /**
     * @brief Open the output file
     * @param filePath Path to the output file
     * @return true if successful, false otherwise
     */
    virtual bool open(const std::string& filePath) = 0;

    /**
     * @brief Write a chunk of data to the file (legacy method)
     * @param data Vector of doubles containing the data to write
     */
    virtual void write(const std::vector<double>& data) = 0;

    /**
     * @brief Write a data frame to the file (new method with timestamp and voltages)
     * @param frame DataFrame containing timestamp and voltage values
     */
    virtual void writeFrame(const DataFrame& frame) = 0;

    /**
     * @brief Flush any buffered data to disk
     */
    virtual void flush() = 0;

    /**
     * @brief Close the file
     */
    virtual void close() = 0;

    /**
     * @brief Set metadata for the file (sampling rate, channel count, timestamps)
     */
    virtual void setMetadata(double samplingRate, int startChannel, int stopChannel, double startTime, double endTime) = 0;

    /**
     * @brief Get the total number of frames written
     */
    virtual uint64_t getTotalFramesWritten() const = 0;
};

} // namespace app

#endif // IDATAWRITER_HPP
