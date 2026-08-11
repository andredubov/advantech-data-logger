#include "main.hpp"

int main()
{
    const std::string binaryFileName = "daq_data_250khz.bin";
    const std::string csvFileName = "daq_data.csv";

    // 1. Открываем бинарный файл для чтения
    std::ifstream binFile(binaryFileName, std::ios::binary | std::ios::ate);
    if (!binFile.is_open()) {
        std::printf("Error: Failed to open binary file %s\n", binaryFileName.c_str());

        return EXIT_FAILURE;
    }

    std::streamsize fileSize = binFile.tellg();
    binFile.seekg(0, std::ios::beg);

    // --- ЧТЕНИЕ ЗАГОЛОВКА (только версия 3) ---
    uint32_t magic = 0;
    uint32_t version = 0;
    double samplingRate = 0.0;
    uint32_t channelCount = 0;
    double startTimeSeconds = 0.0;
    double endTimeSeconds = 0.0;

    binFile.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    binFile.read(reinterpret_cast<char*>(&version), sizeof(version));
    binFile.read(reinterpret_cast<char*>(&samplingRate), sizeof(samplingRate));

    if (!binFile) {
        std::printf("Error: Failed to read file header!\n");
        binFile.close();

        return EXIT_FAILURE;
    }

    if (magic != 0x50434931) {
        std::printf("Error: Invalid file format (magic number mismatch). Expected 0x%X, got 0x%X\n", 0x50434931, magic);
        binFile.close();

        return EXIT_FAILURE;
    }

    // Поддерживается только версия 3
    if (version != 3) {
        std::printf("Error: Unsupported file version: %u. Only version 3 is supported.\n", version);
        binFile.close();

        return EXIT_FAILURE;
    }

    // Версия 3: channelCount + время старта + время окончания
    binFile.read(reinterpret_cast<char*>(&channelCount), sizeof(channelCount));
    binFile.read(reinterpret_cast<char*>(&startTimeSeconds), sizeof(startTimeSeconds));
    binFile.read(reinterpret_cast<char*>(&endTimeSeconds), sizeof(endTimeSeconds));
    if (!binFile) {
        std::printf("Error: Failed to read channel count and start/end time from header!\n");
        binFile.close();

        return EXIT_FAILURE;
    }

    std::printf("File opened successfully.\n");
    std::printf("Format version: %u\n", version);
    std::printf("Sampling rate: %.0f Hz\n", samplingRate);
    std::printf("Channel count: %u\n", channelCount);
    
    // Преобразование времени в формат DD.MM.YYYY HH:MM:SS,ms
    auto formatTime = [](double seconds) -> std::string {
        time_t rawTime = static_cast<time_t>(seconds);
        struct tm timeInfo;
        localtime_s(&timeInfo, &rawTime);
        
        int milliseconds = static_cast<int>((seconds - rawTime) * 1000);
        
        char buffer[64];
        strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", &timeInfo);
        
        char result[80];
        snprintf(result, sizeof(result), "%s,%03d", buffer, milliseconds);
        
        return std::string(result);
    };
    
    std::printf("Start time: %s\n", formatTime(startTimeSeconds).c_str());
    std::printf("End time:   %s\n", formatTime(endTimeSeconds).c_str());

    // Определяем количество кадров (версия 3: время + channelCount каналов)
    std::streamsize dataStartPos = binFile.tellg();
    std::streamsize dataSize = fileSize - dataStartPos;
    std::size_t valuesPerFrame = 1 + channelCount; // (time, ch0...chN)
    std::size_t totalFrames = dataSize / (valuesPerFrame * sizeof(double));

    std::printf("Binary data size: %lld bytes.\n", dataSize);
    std::printf("Values per frame: %zu\n", valuesPerFrame);
    std::printf("Number of frames: %zu\n", totalFrames);

    if (0 == totalFrames) {
        std::printf("Warning: No data frames found in file.\n");
        binFile.close();

        return EXIT_SUCCESS;
    }

    // 2. Открываем CSV файл для записи
    std::ofstream csvFile(csvFileName);
    if (!csvFile.is_open()) {
        std::printf("Error: Failed to create CSV file %s\n", csvFileName.c_str());
        binFile.close();

        return EXIT_FAILURE;
    }

    // Используем стандартную локаль "C" для предсказуемого формата чисел (точка как разделитель)
    csvFile.imbue(std::locale("C"));
    
    // Многоканальный формат: время + каналы (версия 3)
    csvFile << "Absolute Time(s)";
    for (uint32_t ch = 0; ch < channelCount; ++ch) {
        csvFile << ";Channel " << ch;
    }
    csvFile << "\n";
    csvFile << std::fixed << std::setprecision(6);

    std::printf("Conversion started, please wait...\n");

    // 3. Чтение и конвертация кадров
    const std::size_t chunkSize = 25000; // Количество кадров за раз
    std::vector<double> buffer(chunkSize * valuesPerFrame);
    std::size_t framesProcessed = 0;

    while (framesProcessed < totalFrames)
    {
        std::size_t toRead = std::min(chunkSize, totalFrames - framesProcessed);
        std::streamsize bytesToRead = toRead * valuesPerFrame * sizeof(double);

        binFile.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);

        for (std::size_t i = 0; i < toRead; ++i)
        {
            double currentTime = buffer[i * valuesPerFrame];
            csvFile << currentTime;

            for (std::size_t v = 1; v < valuesPerFrame; ++v) {
                double value = buffer[i * valuesPerFrame + v];
                csvFile << ";" << std::showpos << value << std::noshowpos;
            }
            csvFile << "\n";
        }

        framesProcessed += toRead;

        int progress = static_cast<int>((static_cast<double>(framesProcessed) / totalFrames) * 100);
        std::printf("\rProgress: %d%% (%zu/%zu)", progress, framesProcessed, totalFrames);
    }

    // 4. Закрываем файлы
    binFile.close();
    csvFile.close();

    std::printf("\nConversion completed successfully! File saved as: %s\n", csvFileName.c_str());

    return EXIT_SUCCESS;
}