#include "main.hpp"

int main()
{
    // Настройки исходных данных (должны строго совпадать с параметрами сбора)
    const double samplingRate = 250000.0; // 250 кГц
    const double timeStep = 1.0 / samplingRate; // Шаг времени между точками (4 микросекунды)

    const std::string binaryFileName = "daq_data_250khz.bin";
    const std::string csvFileName = "daq_data.csv";

    // 1. Открываем бинарный файл для чтения
    std::ifstream binFile(binaryFileName, std::ios::binary | std::ios::ate);
    if (!binFile.is_open()) {
        std::printf("Error: Failed to open binary file %s\n", binaryFileName.c_str());

        return EXIT_FAILURE;
    }

    // Определяем размер файла и количество точек double
    std::streamsize fileSize = binFile.tellg();
    binFile.seekg(0, std::ios::beg);
    std::size_t totalSamples = fileSize / sizeof(double);

    std::printf("File opened successfully.\n");
    std::printf("Binary file size: %lld bytes.\n", fileSize);
    std::printf("Number of samples: %zu\n", totalSamples);

    // 2. Открываем CSV файл для записи
    std::ofstream csvFile(csvFileName);
    if (!csvFile.is_open()) {
        std::printf("Error: Failed to create CSV file %s\n", csvFileName.c_str());
        binFile.close();

        return EXIT_FAILURE;
    }

    // Записываем шапку таблицы CSV
    csvFile << "Time(s),Voltage(V)\n";

    // Настраиваем высокую точность вывода чисел с плавающей точкой (6 знаков после запятой)
    csvFile << std::fixed << std::setprecision(6);

    std::printf("Conversion started, please wait...\n");

    // 3. Чтение и конвертация блоками (чтобы не перегружать ОЗУ, если файл огромный)
    const std::size_t chunkSize = 25000; // Читаем пачками по 25 000 точек
    std::vector<double> buffer(chunkSize);
    std::size_t samplesProcessed = 0;

    while (samplesProcessed < totalSamples)
    {
        // Вычисляем, сколько точек осталось прочитать в текущем блоке
        std::size_t toRead = std::min(chunkSize, totalSamples - samplesProcessed);
        
        // Читаем блок из бинарного файла
        binFile.read(reinterpret_cast<char*>(buffer.data()), toRead * sizeof(double));

        // Записываем считанный блок в CSV
        for (std::size_t i = 0; i < toRead; ++i)
        {
            // Вычисляем время от начала эксперимента для каждой точки
            double currentTime = (samplesProcessed + i) * timeStep;

            // Записываем строку: Время, Значение
            csvFile << currentTime << "," << buffer[i] << "\n";
        }

        samplesProcessed += toRead;

        // Выводим прогресс в консоль
        int progress = static_cast<int>((static_cast<double>(samplesProcessed) / totalSamples) * 100);
        std::printf("\rProgress: %d%% (%zu/%zu)", progress, samplesProcessed, totalSamples);
    }

    // 4. Закрываем файлы
    binFile.close();
    csvFile.close();

    std::printf("\nConversion completed successfully! File saved as: %s\n", csvFileName.c_str());

    return EXIT_SUCCESS;
}