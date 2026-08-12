#ifndef ADVANTECHDEVICE_HPP
#define ADVANTECHDEVICE_HPP

#include "IDataAcquisitionDevice.hpp"
#include "bdaqctrl.h"
#include "compatibility.h"
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>

namespace app {

/**
 * @brief Implementation of IDataAcquisitionDevice for Advantech PCI-1716
 */
class AdvantechDevice : public IDataAcquisitionDevice {
public:
    AdvantechDevice();
    ~AdvantechDevice() override;

    bool initialize(const std::string& deviceDescription) override;
    bool configure(int startChannel, int channelCount, int samplesPerChannel, double samplingRate) override;
    bool start() override;
    void stop() override;
    void dispose() override;
    void setDataReadyCallback(DataReadyCallback callback) override;
    bool isRunning() const override { return m_isRunning.load(); }

private:
    Automation::BDaq::BufferedAiCtrl* m_aiCtrl;
    DataReadyCallback m_callback;
    std::atomic<bool> m_isRunning;
    std::string m_deviceDescription;
    int m_startChannel;
    int m_channelCount;
    int m_samplesPerChannel;
    double m_samplingRate;

    static void BDAQCALL OnDataReadyEvent(void* sender, Automation::BDaq::BfdAiEventArgs* args, void* userParam);
    void handleDataReady(Automation::BDaq::BufferedAiCtrl* aiCtrl, Automation::BDaq::int32 count);
};

} // namespace app

#endif // ADVANTECHDEVICE_HPP
