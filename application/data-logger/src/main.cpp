#include "main.hpp"

// --- НАСТРОЙКИ СБОРА ДАННЫХ ---
// const wchar_t* deviceDescription = L"PCI-1716,BID#0";
const wchar_t* deviceDescription = L"DemoDevice,BID#0";
const Automation::BDaq::int32 startChannel = 0;
const Automation::BDaq::int32 channelCount = 1;
const double samplingRate = 250000.0; // Максимальная частота 250 кГц
const Automation::BDaq::int32 samplesPerChannel = 25000; // Забираем данные пачками по 25 000 точек (10 раз в сек)

// --- ПОТОКОБЕЗОПАСНАЯ ОЧЕРЕДЬ ---
std::queue<std::vector<double>> dataQueue;
std::mutex queueMutex;
std::condition_variable queueCV;
bool isRunning = true;

// --- 1. ПОТОК СВЕРХБЫСТРОЙ ЗАПИСИ В БИНАРНЫЙ ФАЙЛ ---
void DataProcessingThread()
{
    // Создаем или перезаписываем бинарный файл в папке запуска приложения
    std::ofstream outFile("daq_data_250khz.bin", std::ios::binary | std::ios::out);

    if (!outFile.is_open()) {
        std::printf("[File Thread] Critical error: Failed to create file for writing!\n");
        return;
    }

    std::printf("[Writer Thread] Binary file daq_data_250khz.bin opened successfully.\n");

    while (true)
    {
        std::vector<double> localBuffer;

        // Потокобезопасное извлечение пачки данных из очереди
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [] { return !dataQueue.empty() || !isRunning; });

            // Если сбор остановлен главным потоком и очередь пуста — завершаем работу
            if (!isRunning && dataQueue.empty()) {
                break; 
            }

            localBuffer = std::move(dataQueue.front());
            dataQueue.pop();
        }

        // Высокоскоростное сохранение "сырого" буфера double на диск
        if (!localBuffer.empty()) 
        {
            std::streamsize bytesToWrite = localBuffer.size() * sizeof(double);
            outFile.write(reinterpret_cast<const char*>(localBuffer.data()), bytesToWrite);

            // Проверка на случай переполнения накопителя ПК
            if (!outFile) {
                std::printf("[Writer Thread] Critical error: Physical disk write failure!\n");
            }
        }
    }

    outFile.close();

    std::printf("[Writer Thread] All data flushed to disk successfully. File closed.\n");
}

// --- 2. ФУНКЦИЯ ОБРАТНОГО ВЫЗОВА (Callback от драйвера) ---
// Вызывается автоматически в изолированном высокоприоритетном потоке Advantech
void BDAQCALL OnDataReadyEvent(void* sender, Automation::BDaq::BfdAiEventArgs* args, void* userParam)
{
    Automation::BDaq::BufferedAiCtrl* aiCtrl = (Automation::BDaq::BufferedAiCtrl*)sender;

    // Получаем точное количество отсчетов, готовых к считыванию из FIFO платы
    Automation::BDaq::int32 count = args->Count;

    // Выделяем память под временный вектор
    std::vector<double> rawData(count);

    // Забираем данные из FIFO-буфера драйвера в наш вектор (C++ API принимает 2 аргумента)
    Automation::BDaq::ErrorCode ret = aiCtrl->GetData(count, rawData.data());

    if (Automation::BDaq::Success == ret)
    {
        // Молниеносно перемещаем данные в очередь и будим рабочий поток записи
        std::lock_guard<std::mutex> lock(queueMutex);
        dataQueue.push(std::move(rawData));
        queueCV.notify_one();
    }
}

// --- 3. ГЛАВНЫЙ ПОТОК ПРИЛОЖЕНИЯ ---
int main()
{
    // Создаем экземпляр контроллера буферизированного ввода
    Automation::BDaq::BufferedAiCtrl* aiCtrl = Automation::BDaq::BufferedAiCtrl::Create();
    Automation::BDaq::DeviceInformation devInfo(deviceDescription);

    // 1. Привязка к физическому слоту платы PCI-1716
    if (BioFailed(aiCtrl->setSelectedDevice(devInfo))) {
        std::printf("Initialization error: Device not found in system!\n");
        std::printf("Check device name and BoardID in Advantech Navigator utility.\n");
        aiCtrl->Dispose();

        return EXIT_FAILURE;
    }

    // 2. Параметризация АЦП-сканирования (Настройки для 250 кГц)
    aiCtrl->getScanChannel()->setChannelStart(startChannel);
    aiCtrl->getScanChannel()->setChannelCount(channelCount);
    aiCtrl->getScanChannel()->setSamples(samplesPerChannel);
    aiCtrl->getConvertClock()->setRate(samplingRate);

    // Задаем циклический буфер в ОЗУ ПК на 1 000 000 отсчетов для защиты от микрозависаний ОС.
    // Драйвер Advantech автоматически настроит шину PCI на DMA-передачу в эту область.
    // aiCtrl->getBuffer()->setLength(1000000);

    // 3. Подключаем функцию обработки прерываний буфера
    aiCtrl->addDataReadyHandler(OnDataReadyEvent, nullptr);

    // Запуск параллельного потока записи на диск
    isRunning = true;
    std::thread fileThread(DataProcessingThread);

    // Программный старт железного сбора данных на плате
    Automation::BDaq::ErrorCode ret = aiCtrl->Prepare();
    if (Automation::BDaq::Success == ret) {
        ret = aiCtrl->Start();
    }

    if (BioFailed(ret)) {
        std::printf("Critical error: Failed to start ADC. Error code: %d\n", ret);
        isRunning = false;
        queueCV.notify_one();
        fileThread.join();
        aiCtrl->Dispose();

        return EXIT_FAILURE;
    }

    std::printf("\n========================================================\n");
    std::printf("Data acquisition from PCI-1716 at 250 kHz STARTED.\n");
    std::printf("Data is continuously written to binary file...\n");
    std::printf("Press ENTER to stop the program safely.\n");
    std::printf("========================================================\n\n");
    
    // Ожидаем действия от пользователя в консоли
    std::cin.get(); 

    // Процедура корректной остановки оборудования и потоков
    std::printf("Stopping data acquisition on device...\n");
    aiCtrl->Stop();

    // Сигнализируем файловому потоку, что новых данных больше не будет
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        isRunning = false;
    }
    queueCV.notify_one();

    // Синхронизируем закрытие потока записи
    if (fileThread.joinable()) {
        fileThread.join();
    }

    // Освобождаем дескриптор платы в системе
    aiCtrl->Dispose();

    std::printf("Program finished successfully. All resources released.\n");

    return EXIT_SUCCESS;
}