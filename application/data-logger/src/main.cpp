#include "main.hpp"

app::command_line_options command_line_options;

std::queue<std::vector<double>> dataQueue;
std::mutex queueMutex;
std::condition_variable queueCV;
bool isRunning = true;
double g_startTimeSeconds = 0.0; // Глобальное время старта для потока записи

// --- 1. ПОТОК СВЕРХБЫСТРОЙ ЗАПИСИ В БИНАРНЫЙ ФАЙЛ ---
void DataProcessingThread()
{
    // Создаем или перезаписываем бинарный файл в папке запуска приложения
    std::ofstream outFile(command_line_options.get_output_file_path(), std::ios::binary | std::ios::out);

    int channelCount = command_line_options.get_channel_count();
    double samplingRate = command_line_options.get_sampling_rate();

    if (!outFile.is_open()) {
        std::printf("[File Thread] Critical error: Failed to create file for writing!\n");
        return;
    }

    std::printf("[Writer Thread] Binary file daq_data_250khz.bin opened successfully.\n");

    // --- ЗАПИСЬ ЗАГОЛОВКА ФАЙЛА ---
    // Магическое число для идентификации формата
    const uint32_t magic = 0x50434931; // "PCI1"
    const uint32_t version = 3; // Версия 3: добавлена поддержка нескольких каналов
    const double samplingRateLocal = command_line_options.get_sampling_rate();
    const uint32_t channelCountLocal = channelCount;

    // Получаем абсолютное время старта с высокой точностью
    const auto startTimePoint = std::chrono::system_clock::now();
    g_startTimeSeconds = std::chrono::duration<double>(startTimePoint.time_since_epoch()).count();
    double endTimeSeconds = 0.0; // Будет заполнено при завершении

    outFile.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    outFile.write(reinterpret_cast<const char*>(&version), sizeof(version));
    outFile.write(reinterpret_cast<const char*>(&samplingRateLocal), sizeof(samplingRateLocal));
    outFile.write(reinterpret_cast<const char*>(&channelCountLocal), sizeof(channelCountLocal));
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

    std::printf("[Writer Thread] File header written successfully. Channels: %u, Start time: %.6f s since epoch.\n", channelCountLocal, g_startTimeSeconds);

    // Счетчик записанных кадров для вычисления времени
    uint64_t totalFramesWritten = 0;

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
            // localBuffer содержит данные в порядке: [ch0, ch1, ..., ch15, ch0, ch1, ...]
            // Количество кадров = localBuffer.size() / channelCount
            size_t framesInBuffer = localBuffer.size() / channelCount;

            std::vector<double> timedData;
            // Каждый кадр: время + значения всех каналов
            timedData.reserve(framesInBuffer * (1 + channelCount));

            const double timeStep = 1.0 / samplingRate; // Шаг между кадрами (4 мкс при 250 кГц)
            for (size_t frameIdx = 0; frameIdx < framesInBuffer; ++frameIdx) {
                // Абсолютное время: время старта + смещение от начала сбора
                double currentTime = g_startTimeSeconds + (totalFramesWritten + frameIdx) * timeStep;
                timedData.push_back(currentTime);

                // Добавляем все каналы этого кадра
                for (size_t ch = 0; ch < channelCount; ++ch) {
                    timedData.push_back(localBuffer[frameIdx * channelCount + ch]);
                }
            }

            std::streamsize bytesToWrite = timedData.size() * sizeof(double);
            outFile.write(reinterpret_cast<const char*>(timedData.data()), bytesToWrite);

            totalFramesWritten += framesInBuffer;

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

    std::printf("[Writer Thread] All data flushed to disk successfully. Total frames: %llu (channels: %u). File closed.\n", totalFramesWritten, channelCount);
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
int main(int argc, char* argv[])
{
    auto state = command_line_options.parse(argc, argv);

    switch (state) {
        case app::command_line_options::state::success:            
            break;
        case app::command_line_options::state::version:
            std::cout << "v" << command_line_options.get_version() << std::endl;
            return EXIT_SUCCESS;
        case app::command_line_options::state::help:
            std::cout << "Help: " << command_line_options.get_help() << std::endl;
            return EXIT_SUCCESS;
        default:
            std::cout << command_line_options.get_error_message() << std::endl;
            return EXIT_FAILURE;
    }

    std::printf("========================================================\n");
    std::printf("Data Logger Configuration:\n");
    std::printf("  Device:        %s\n", command_line_options.get_device_description().c_str());
    std::printf("  Channels:      %d\n", command_line_options.get_channel_count());
    std::printf("  Sampling rate: %.0f Hz\n", command_line_options.get_sampling_rate());
    std::printf("  Buffer size:   %d samples per channel\n", command_line_options.get_samples_per_channel());
    std::printf("  Output file:   %s\n", command_line_options.get_output_file_path().c_str());
    std::printf("  Demo mode:     %s\n", command_line_options.is_use_demo_device() ? "ON" : "OFF");
    std::printf("========================================================\n\n");

    // Создаем экземпляр контроллера буферизированного ввода
    Automation::BDaq::BufferedAiCtrl* aiCtrl = Automation::BDaq::BufferedAiCtrl::Create();
    std::wstring deviceDesc = std::wstring(
        command_line_options.get_device_description().begin(), 
        command_line_options.get_device_description().end()
    );
    Automation::BDaq::DeviceInformation devInfo(deviceDesc.c_str());

    // 1. Привязка к физическому слоту платы PCI-1716
    if (BioFailed(aiCtrl->setSelectedDevice(devInfo))) {
        std::printf("Initialization error: Device not found in system!\n");
        std::printf("Check device name and BoardID in Advantech Navigator utility.\n");
        aiCtrl->Dispose();

        return EXIT_FAILURE;
    }

    // 2. Параметризация АЦП-сканирования (Настройки для 250 кГц)
    aiCtrl->getScanChannel()->setChannelStart(0);
    aiCtrl->getScanChannel()->setChannelCount(command_line_options.get_channel_count());
    aiCtrl->getScanChannel()->setSamples(command_line_options.get_samples_per_channel());
    aiCtrl->getConvertClock()->setRate(command_line_options.get_sampling_rate());

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