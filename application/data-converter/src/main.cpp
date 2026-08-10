#include "main.hpp"
#include <cstdint>

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

    // --- ЧТЕНИЕ ЗАГОЛОВКА ---
    uint32_t magic = 0;
    uint32_t version = 0;
    double samplingRate = 0.0;
    double startTimeSeconds = 0.0;

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

    double endTimeSeconds = 0.0;
    if (version == 2) {
        // Читаем время старта и окончания для версии 2
        binFile.read(reinterpret_cast<char*>(&startTimeSeconds), sizeof(startTimeSeconds));
        binFile.read(reinterpret_cast<char*>(&endTimeSeconds), sizeof(endTimeSeconds));
        if (!binFile) {
            std::printf("Error: Failed to read start/end time from header!\n");
            binFile.close();
            return EXIT_FAILURE;
        }
    } else if (version != 1) {
        std::printf("Error: Unsupported file version: %u\n", version);
        binFile.close();
        return EXIT_FAILURE;
    }

    std::printf("File opened successfully.\n");
    std::printf("Format version: %u\n", version);
    std::printf("Sampling rate: %.0f Hz\n", samplingRate);
    if (version == 2) {
        std::printf("Start time (absolute): %.6f s since epoch\n", startTimeSeconds);
    }

    // Определяем количество точек (каждая точка = пара double: time + voltage)
    std::streamsize dataStartPos = binFile.tellg();
    std::streamsize dataSize = fileSize - dataStartPos;
    std::size_t totalSamples = dataSize / (2 * sizeof(double));

    std::printf("Binary data size: %lld bytes.\n", dataSize);
    std::printf("Number of samples: %zu\n", totalSamples);

    if (totalSamples == 0) {
        std::printf("Warning: No data samples found in file.\n");
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
    if (version == 2) {
        csvFile << "Absolute Time(s);Voltage(V)\n";
    } else {
        csvFile << "Time(s);Voltage(V)\n";
    }
    csvFile << std::fixed << std::setprecision(6);

    std::printf("Conversion started, please wait...\n");

    // 3. Чтение и конвертация парами (time, voltage)
    const std::size_t chunkSize = 25000; // Количество точек за раз
    std::vector<double> buffer(chunkSize * 2); // Два double на точку
    std::size_t samplesProcessed = 0;

    while (samplesProcessed < totalSamples)
    {
        std::size_t toRead = std::min(chunkSize, totalSamples - samplesProcessed);
        std::streamsize bytesToRead = toRead * 2 * sizeof(double);

        binFile.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);

        for (std::size_t i = 0; i < toRead; ++i)
        {
            double currentTime = buffer[i * 2];
            double voltage = buffer[i * 2 + 1];

            // В версии 2 время уже абсолютное, ничего не добавляем
            // В версии 1 время было относительным (начиналось с 0)

            csvFile << currentTime << ";" << std::showpos << voltage << std::noshowpos << "\n";
        }

        samplesProcessed += toRead;

        int progress = static_cast<int>((static_cast<double>(samplesProcessed) / totalSamples) * 100);
        std::printf("\rProgress: %d%% (%zu/%zu)", progress, samplesProcessed, totalSamples);
    }

    // 4. Закрываем файлы
    binFile.close();
    csvFile.close();

    std::printf("\nConversion completed successfully! File saved as: %s\n", csvFileName.c_str());

    return EXIT_SUCCESS;
}