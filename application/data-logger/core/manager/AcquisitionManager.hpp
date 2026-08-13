#ifndef ACQUISITIONMANAGER_HPP
#define ACQUISITIONMANAGER_HPP

#include <string>
#include <memory>
#include "IDataAcquisitionDevice.hpp"
#include "IDataWriter.hpp"
#include "ILogger.hpp"
#include "DataProcessingEngine.hpp"
#include "CommandLineOptions.hpp"

namespace app {

/**
 * @brief Orchestrates the entire data acquisition process
 */
class AcquisitionManager {
public:
    AcquisitionManager(
        IDataAcquisitionDevice* device,
        DataProcessingEngine* engine,
        IDataWriter* writer,
        ILogger* logger,
        const command_line_options* options
    );
    ~AcquisitionManager();

    /**
     * @brief Initialize the device and writer
     * @return true if successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Start data acquisition and processing
     * @return true if successful, false otherwise
     */
    bool startAcquisition();

    /**
     * @brief Wait for user input to stop
     */
    void waitForStop();

    /**
     * @brief Stop acquisition and cleanup
     */
    void stopAcquisition();

    /**
     * @brief Shutdown all resources
     */
    void shutdown();

private:
    IDataAcquisitionDevice* m_device;
    DataProcessingEngine* m_engine;
    IDataWriter* m_writer;
    ILogger* m_logger;
    const command_line_options* m_options;

    bool m_initialized;
    bool m_acquisitionStarted;
    bool m_shutdownCalled;

    void printConfiguration() const;
    void setupDeviceCallback();
};

} // namespace app

#endif // ACQUISITIONMANAGER_HPP
