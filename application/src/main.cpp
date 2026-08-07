#include "main.hpp"

using namespace Automation::BDaq;

// Константы для настройки сбора данных
// const wchar_t* deviceDescription = L"PCI-1716,BID#0"; // Имя устройства из Advantech Navigator
const wchar_t* deviceDescription = L"DemoDevice,BID#0"; // Имя устройства из Advantech Navigator
const int32 startChannel = 0;                        // Начальный канал
const int32 channelCount = 1;                        // Количество опрашиваемых каналов
const int32 samplesPerChannel = 1000;                // Размер буфера на один канал
const double samplingRate = 10000.0;                 // Частота дискретизации (Гц)

// Обработчик события заполнения буфера (Callback)
void BDAQCALL OnDataReadyEvent(void* sender, BfdAiEventArgs* args, void* userParam)
{
    BufferedAiCtrl* aiCtrl = (BufferedAiCtrl*)sender;

    // Получаем точное количество сэмплов, подготовленных драйвером
    int32 count = args->Count; 

    // Выделяем память под данные текущей пачки
    double* dataBuffer = new double[count];

    // В C++ SDK метод GetData принимает всего 2 параметра: 
    // 1. Сколько данных забрать (count)
    // 2. Указатель на массив (dataBuffer)
    ErrorCode ret = aiCtrl->GetData(count, dataBuffer);

    if (ret == Success)
    {
        // Output the first sample value to console
        // dataBuffer[0] is the first voltage reading (in Volts)
        std::printf("Read %06d samples. ", count);
        std::printf("Current voltage on CH0: %.3f V\n", dataBuffer[0]);
    }

    // Обязательно освобождаем выделенную память
    delete[] dataBuffer;
}
int main(int argc, char* argv[])
{
    // Инициализация контроллера буферизированного ввода
    BufferedAiCtrl* aiCtrl = BufferedAiCtrl::Create();
    
    // 1. Привязка к физическому устройству
    DeviceInformation devInfo(deviceDescription);
    ErrorCode ret = aiCtrl->setSelectedDevice(devInfo);
    if (BioFailed(ret)) {
        std::printf("Error: Failed to open device. Code: %d\n", ret);
        aiCtrl->Dispose();
        return -1;
    }

    // 2. Настройка параметров сканирования каналов
    aiCtrl->getScanChannel()->setChannelStart(startChannel);
    aiCtrl->getScanChannel()->setChannelCount(channelCount);
    aiCtrl->getScanChannel()->setSamples(samplesPerChannel);
    aiCtrl->getConvertClock()->setRate(samplingRate);

    // 3. Подключение функции обратного вызова (обработчик прерывания буфера)
    aiCtrl->addDataReadyHandler(OnDataReadyEvent, nullptr);

    // 4. Запуск сбора данных
    ret = aiCtrl->Prepare();
    if (ret == Success) {
        ret = aiCtrl->Start();
    }

    if (BioFailed(ret)) {
        std::printf("Error starting data acquisition. Code: %d\n", ret);
        aiCtrl->Dispose();
        return -1;
    }

    std::printf("Data acquisition started. Press ENTER to stop...\n");
    std::cin.get(); 

    // 5. Остановка и освобождение ресурсов
    aiCtrl->Stop();
    aiCtrl->Dispose();

    std::printf("Application finished successfully.\n");

    return 0;
}