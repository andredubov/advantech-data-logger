#ifndef IDATAACQUISITIONDEVICE_HPP
#define IDATAACQUISITIONDEVICE_HPP

#include <string>
#include <functional>
#include <vector>
#include "AcquisitionConfig.hpp"

namespace app {

/**
 * @brief Callback type for data ready events
 * @param data Vector of doubles containing the acquired data
 */
using DataReadyCallback = std::function<void(const std::vector<double>& data)>;

/**
 * @brief Interface for data acquisition devices
 */
class IDataAcquisitionDevice {
public:
    virtual ~IDataAcquisitionDevice() = default;

    /**
     * @brief Initialize the device
     * @param deviceDescription Device description string
     * @return true if successful, false otherwise
     */
    virtual bool initialize(const std::string& deviceDescription) = 0;

    /**
     * @brief Configure the device parameters
     * @param config Configuration parameters for acquisition
     * @return true if successful, false otherwise
     */
    virtual bool configure(const AcquisitionConfig& config) = 0;

    /**
     * @brief Start data acquisition
     * @return true if successful, false otherwise
     */
    virtual bool start() = 0;

    /**
     * @brief Stop data acquisition
     */
    virtual void stop() = 0;

    /**
     * @brief Dispose and release device resources
     */
    virtual void dispose() = 0;

    /**
     * @brief Set the callback function for data ready events
     * @param callback Function to call when data is ready
     */
    virtual void setDataReadyCallback(DataReadyCallback callback) = 0;

    /**
     * @brief Check if the device is currently running
     * @return true if running, false otherwise
     */
    virtual bool isRunning() const = 0;
};

} // namespace app

#endif // IDATAACQUISITIONDEVICE_HPP
