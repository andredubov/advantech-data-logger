#include "AdvantechDevice.hpp"
#include <cstdio>

namespace app {

AdvantechDevice::AdvantechDevice()
    : m_aiCtrl(nullptr)
    , m_callback(nullptr)
    , m_isRunning(false)
    , m_deviceDescription()
    , m_startChannel(0)
    , m_channelCount(1)
    , m_samplesPerChannel(25000)
    , m_samplingRate(250000.0)
{
    m_aiCtrl = Automation::BDaq::BufferedAiCtrl::Create();
}

AdvantechDevice::~AdvantechDevice() {
    if (m_aiCtrl) {
        m_aiCtrl->Dispose();
        m_aiCtrl = nullptr;
    }
}

bool AdvantechDevice::initialize(const std::string& deviceDescription) {
    m_deviceDescription = deviceDescription;
    std::wstring deviceDesc = std::wstring(deviceDescription.begin(), deviceDescription.end());
    Automation::BDaq::DeviceInformation devInfo(deviceDesc.c_str());

    if (BioFailed(m_aiCtrl->setSelectedDevice(devInfo))) {
        std::printf("Initialization error: Device not found in system!\n");
        std::printf("Check device name and BoardID in Advantech Navigator utility.\n");
        return false;
    }
    return true;
}

bool AdvantechDevice::configure(int startChannel, int channelCount, int samplesPerChannel, double samplingRate) {
    m_startChannel = startChannel;
    m_channelCount = channelCount;
    m_samplesPerChannel = samplesPerChannel;
    m_samplingRate = samplingRate;

    m_aiCtrl->getScanChannel()->setChannelStart(startChannel);
    m_aiCtrl->getScanChannel()->setChannelCount(channelCount);
    m_aiCtrl->getScanChannel()->setSamples(samplesPerChannel);
    m_aiCtrl->getConvertClock()->setRate(samplingRate);
    return true;
}

bool AdvantechDevice::start() {
    if (!m_aiCtrl) {
        std::printf("Error: Device not initialized.\n");
        return false;
    }

    // Регистрируем обработчик данных
    m_aiCtrl->addDataReadyHandler(OnDataReadyEvent, this);

    Automation::BDaq::ErrorCode ret = m_aiCtrl->Prepare();
    if (Automation::BDaq::Success == ret) {
        ret = m_aiCtrl->Start();
    }

    if (BioFailed(ret)) {
        std::printf("Critical error: Failed to start ADC. Error code: %d\n", ret);
        m_isRunning = false;
        return false;
    }

    m_isRunning = true;
    return true;
}

void AdvantechDevice::stop() {
    if (m_aiCtrl && m_isRunning) {
        m_aiCtrl->Stop();
        m_isRunning = false;
    }
}

void AdvantechDevice::dispose() {
    if (m_aiCtrl) {
        m_aiCtrl->Dispose();
        m_aiCtrl = nullptr;
    }
    m_isRunning = false;
}

void AdvantechDevice::setDataReadyCallback(DataReadyCallback callback) {
    m_callback = callback;
}

void BDAQCALL AdvantechDevice::OnDataReadyEvent(void* sender, Automation::BDaq::BfdAiEventArgs* args, void* userParam) {
    AdvantechDevice* device = static_cast<AdvantechDevice*>(userParam);
    if (device) {
        Automation::BDaq::BufferedAiCtrl* aiCtrl = static_cast<Automation::BDaq::BufferedAiCtrl*>(sender);
        device->handleDataReady(aiCtrl, args->Count);
    }
}

void AdvantechDevice::handleDataReady(Automation::BDaq::BufferedAiCtrl* aiCtrl, Automation::BDaq::int32 count) {
    std::vector<double> rawData(count);
    Automation::BDaq::ErrorCode ret = aiCtrl->GetData(count, rawData.data());

    if (Automation::BDaq::Success == ret && m_callback) {
        m_callback(rawData);
    }
}

} // namespace app
