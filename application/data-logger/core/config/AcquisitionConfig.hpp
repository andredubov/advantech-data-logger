#ifndef ACQUISITION_CONFIG_HPP
#define ACQUISITION_CONFIG_HPP

#include <string>

namespace app {

/**
 * @brief Configuration parameters for data acquisition
 */
struct AcquisitionConfig {
    int startChannel;
    int channelCount;
    int samplesPerChannel;
    double samplingRate;
    std::string inputMode;
    std::string inputRange;

    AcquisitionConfig();
};

} // namespace app

#endif // ACQUISITION_CONFIG_HPP
