#include "AcquisitionConfig.hpp"

namespace app {

AcquisitionConfig::AcquisitionConfig()
    : startChannel(0)
    , channelCount(1)
    , samplesPerChannel(25000)
    , samplingRate(250000.0)
    , inputMode("bipolar")
    , inputRange("10V")
{
}

} // namespace app
