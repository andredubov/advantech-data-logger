#ifndef DATAPROCESSINGENGINE_HPP
#define DATAPROCESSINGENGINE_HPP

#include "IDataWriter.hpp"
#include "ILogger.hpp"
#include "DataFrame.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <vector>
#include <cstdint>
#include <memory>

namespace app {

/**
 * @brief Engine that processes data from queue and writes to disk
 */
class DataProcessingEngine {
public:
    DataProcessingEngine(std::shared_ptr<app::IDataWriter> writer, std::shared_ptr<app::ILogger> logger);
    ~DataProcessingEngine();

    /**
     * @brief Start the processing engine
     * @param samplingRate Sampling rate in Hz for time calculations
     * @param channelCount Number of channels
     */
    void start(double samplingRate, int startChannel, int endChannel);

    /**
     * @brief Stop the processing engine and wait for all data to be written
     */
    void stop();

    /**
     * @brief Push data frame into the processing queue
     * @param frame DataFrame containing timestamp and voltages
     */
    void pushDataFrame(DataFrame&& frame);

    /**
     * @brief Check if the engine is running
     */
    bool isRunning() const { return m_isRunning.load(); }

    /**
     * @brief Get the total number of frames written
     */
    uint64_t getTotalFramesWritten() const;

private:
    std::shared_ptr<app::IDataWriter> m_writer;
    std::shared_ptr<app::ILogger> m_logger;

    std::queue<DataFrame> m_dataQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    std::atomic<bool> m_isRunning;
    std::thread m_writerThread;

    double m_samplingRate;
    int m_startChannel;
    int m_endChannel;
    int m_channelCount;
    double m_startTime;

    void writerThreadFunction();
};

} // namespace app

#endif // DATAPROCESSINGENGINE_HPP
