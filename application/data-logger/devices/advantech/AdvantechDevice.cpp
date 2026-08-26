#include "AdvantechDevice.hpp"
#include <cstdio>

namespace app {

AdvantechDevice::AdvantechDevice(std::shared_ptr<app::ILogger> logger)
    : m_aiCtrl(nullptr)
    , m_callback(nullptr)
    , m_isRunning(false)
    , m_deviceDescription()
    , m_startChannel(0)
    , m_channelCount(1)
    , m_samplesPerChannel(25000)
    , m_samplingRate(250000.0)
    , m_logger(logger)
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
        m_logger->error("Initialization error: Device not found in system!");
        m_logger->error("Check device name and BoardID in Advantech Navigator utility.");
        return false;
    }
    return true;
}

bool AdvantechDevice::configure(int startChannel, int channelCount, int samplesPerChannel, double samplingRate, const std::string& inputMode, const std::string& inputRange) {
    m_startChannel = startChannel;
    m_channelCount = channelCount;
    m_samplesPerChannel = samplesPerChannel;
    m_samplingRate = samplingRate;

    m_aiCtrl->getScanChannel()->setChannelStart(startChannel);
    m_aiCtrl->getScanChannel()->setChannelCount(channelCount);
    m_aiCtrl->getScanChannel()->setSamples(samplesPerChannel);
    m_aiCtrl->getConvertClock()->setRate(samplingRate);

    // Определяем режим и диапазон
    Automation::BDaq::ValueRange range = Automation::BDaq::ValueRange::V_0To10;

    if (inputRange == "5V") {
        range = Automation::BDaq::ValueRange::V_0To5;
    } else if (inputRange == "2.5V") {
        range = Automation::BDaq::ValueRange::V_0To2pt5;
    } else if (inputRange == "1.25V") {
        range = Automation::BDaq::ValueRange::V_0To1;
    } else {
        range = Automation::BDaq::ValueRange::V_0To10; // 0..10V default
    }

    // Для биполярного режима используем симметричные диапазоны
    if (inputMode == "bipolar") 
    {
        if (inputRange == "5V") {
            range = Automation::BDaq::ValueRange::V_Neg5To5;
        } else if (inputRange == "2.5V") { 
            range = Automation::BDaq::ValueRange::V_Neg2pt5To2pt5;
        } else if (inputRange == "1.25V") {
            range = Automation::BDaq::ValueRange::V_Neg1pt25To1pt25;
        } else {
            range = Automation::BDaq::ValueRange::V_Neg10To10; // ±10V default
        }
    }

    // Настройка диапазона для каждого канала:
    for (int ch = startChannel; ch < startChannel + channelCount; ++ch) {
        m_aiCtrl->getChannels()->getItem(ch).setValueRange(range);
    }

    return true;
}

bool AdvantechDevice::start() {
    if (!m_aiCtrl) {
        m_logger->error("Error: Device not initialized.");
        return false;
    }

    // Регистрируем обработчик данных
    m_aiCtrl->addDataReadyHandler(OnDataReadyEvent, this);

    Automation::BDaq::ErrorCode ret = m_aiCtrl->Prepare();
    if (Automation::BDaq::Success == ret) {
        ret = m_aiCtrl->Start();
    }

    if (BioFailed(ret)) {
        m_logger->error("Critical error: Failed to start ADC. Error code: %d", ret);
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
