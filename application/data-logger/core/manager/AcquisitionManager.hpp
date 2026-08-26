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
        std::shared_ptr<app::IDataAcquisitionDevice> device,
        std::shared_ptr<app::DataProcessingEngine> engine,
        std::shared_ptr<app::IDataWriter> writer,
        std::shared_ptr<app::ILogger> logger,
        std::shared_ptr<app::CommandLineOptions> options
    );
    virtual ~AcquisitionManager();

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
    std::shared_ptr<app::IDataAcquisitionDevice> m_device;
    std::shared_ptr<app::DataProcessingEngine> m_engine;
    std::shared_ptr<app::IDataWriter> m_writer;
    std::shared_ptr<app::ILogger> m_logger;
    std::shared_ptr<app::CommandLineOptions> m_options;

    bool m_initialized;
    bool m_acquisitionStarted;
    bool m_shutdownCalled;

    void printConfiguration() const;
    void setupDeviceCallback();
};

} // namespace app

#endif // ACQUISITIONMANAGER_HPP
