#include "main.hpp"
#include <chrono>
#include <cstdint>

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
double g_startTimeSeconds = 0.0; // Глобальное время старта для потока записи

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

    // --- ЗАПИСЬ ЗАГОЛОВКА ФАЙЛА ---
    // Магическое число для идентификации формата
    const uint32_t magic = 0x50434931; // "PCI1"
    const uint32_t version = 2; // Увеличиваем версию из-за добавления времени старта
    const double samplingRateLocal = samplingRate;

    // Получаем абсолютное время старта с высокой точностью
    const auto startTimePoint = std::chrono::system_clock::now();
    g_startTimeSeconds = std::chrono::duration<double>(startTimePoint.time_since_epoch()).count();
    double endTimeSeconds = 0.0; // Будет заполнено при завершении

    outFile.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    outFile.write(reinterpret_cast<const char*>(&version), sizeof(version));
    outFile.write(reinterpret_cast<const char*>(&samplingRateLocal), sizeof(samplingRateLocal));
    outFile.write(reinterpret_cast<const char*>(&g_startTimeSeconds), sizeof(g_startTimeSeconds));
    // Запоминаем позицию для записи времени окончания
    std::streampos endTimePos = outFile.tellp();
    // Временно записываем 0, позже перезапишем на фактическое время окончания
    outFile.write(reinterpret_cast<const char*>(&endTimeSeconds), sizeof(endTimeSeconds));

    if (!outFile) {
        std::printf("[Writer Thread] Critical error: Failed to write file header!\n");
        outFile.close();
        return;
    }

    std::printf("[Writer Thread] File header written successfully. Start time: %.6f s since epoch.\n", g_startTimeSeconds);

    // Счетчик записанных точек для вычисления времени
    uint64_t totalSamplesWritten = 0;

    while (true)
    {
        std::vector<double> localBuffer;

        // Потокобезопасное извлечение пачки данных из очереди
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [] {
                return !dataQueue.empty() || !isRunning;
            });

            // Если сбор остановлен главным потоком и очередь пуста — завершаем работу
            if (!isRunning && dataQueue.empty()) {
                break; 
            }

            localBuffer = std::move(dataQueue.front());
            dataQueue.pop();
        }

        // Сохранение данных с временными метками
        if (!localBuffer.empty())
        {
            // Для каждой точки записываем пару (time, voltage)
            std::vector<double> timedData;
            timedData.reserve(localBuffer.size() * 2);

            const double timeStep = 1.0 / samplingRate;
            for (size_t i = 0; i < localBuffer.size(); ++i) {
                // Абсолютное время: время старта + смещение от начала сбора
                double currentTime = g_startTimeSeconds + (totalSamplesWritten + i) * timeStep;
                timedData.push_back(currentTime);
                timedData.push_back(localBuffer[i]);
            }

            std::streamsize bytesToWrite = timedData.size() * sizeof(double);
            outFile.write(reinterpret_cast<const char*>(timedData.data()), bytesToWrite);

            totalSamplesWritten += localBuffer.size();

            // Проверка на случай переполнения накопителя ПК
            if (!outFile) {
                std::printf("[Writer Thread] Critical error: Physical disk write failure!\n");
            }
        }
    }

    // Запись времени окончания в заголовок файла
    const auto endTimePoint = std::chrono::system_clock::now();
    endTimeSeconds = std::chrono::duration<double>(endTimePoint.time_since_epoch()).count();

    outFile.seekp(endTimePos);
    outFile.write(reinterpret_cast<const char*>(&endTimeSeconds), sizeof(endTimeSeconds));
    if (!outFile) {
        std::printf("[Writer Thread] Warning: Failed to write end time to file header!\n");
    }
    outFile.seekp(0, std::ios::end);

    outFile.close();

    std::printf("[Writer Thread] All data flushed to disk successfully. Total samples: %llu. File closed.\n", totalSamplesWritten);
    std::printf("[Writer Thread] End time: %.6f s since epoch.\n", endTimeSeconds);
}

// --- 2. ФУНКЦИЯ ОБРАТНОГО ВЫЗОВА (Callback от драйвера) ---
// Вызывается автоматически в изолированном высокоприоритетном потоке Advantech
void BDAQCALL OnDataReadyEvent(void* sender, Automation::BDaq::BfdAiEventArgs* args, void* userParam)
{
    Automation::BDaq::BufferedAiCtrl* aiCtrl = static_cast<Automation::BDaq::BufferedAiCtrl*>(sender);

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