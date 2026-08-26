# PCI-1716 Data Acquisition System

C++ application suite for high-speed data acquisition using the Advantech PCI-1716 analog input card.

## Features

- **Real-time data acquisition** at up to 250 kHz sampling rate
- **Multi-channel support** (up to 16 channels)
- **Thread-safe architecture** with callback-based data handling
- **Binary data storage** for high-performance logging
- **CSV conversion** with timestamp generation
- **Windows-native** implementation using Advantech DAQNavi SDK

## System Requirements

- Windows operating system
- Advantech PCI-1716 analog input card
- Advantech DAQNavi SDK installed
- Visual Studio 2022 (or compatible MSVC compiler)
- CMake 3.20 or higher

## Quick Start

### Build the Project

Using the interactive build script:
```bash
build.bat
```

Or using CMake directly:
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Run Data Logger

Using the interactive launcher:
```bash
run.bat
```
Select option 1 and follow the prompts to configure acquisition parameters.

Or run directly:
```bash
data-logger.exe --device PCI-1716,BID#0 --start-channel 0 --end-channel 7 --rate 100000 --output daq_data.bin --input-mode unipolar --input-range 10V
```

### Convert Binary to CSV

Using the interactive launcher:
```bash
run.bat
```
Select option 2 and follow the prompts to convert binary files to CSV.

Or run directly:
```bash
data-converter.exe --input daq_data.bin --output daq_data.csv
```

### Interactive Launcher (`run.bat`)

The project now includes an interactive batch script that simplifies running both executables:

- **Menu-driven interface** with options for data-logger and data-converter
- **Interactive argument input** with default values and validation
- **Automatic console clearing** for improved user experience
- **Error handling** with clear feedback

Simply run `run.bat` from the project root and follow the on-screen prompts.

## Project Structure

```
pci-1716/
├── application/
│   ├── data-logger/      # Real-time acquisition application
│   │   ├── core/         # Core engine and interfaces
│   │   ├── devices/      # Device implementations (Advantech)
│   │   ├── logging/      # Logging infrastructure
│   │   ├── storage/      # Binary file storage
│   │   └── main/         # Application entry point
│   └── data-converter/   # Binary-to-CSV converter
│       ├── core/         # Conversion logic
│       ├── factory/      # Component factory
│       └── main/         # Application entry point
├── library/
│   └── DAQNavi/         # Advantech SDK wrapper
├── build/               # Build output directory
├── AGENTS.md            # AI agent instructions
├── CMakeLists.txt       # Root CMake configuration
└── build.bat            # Interactive build script
```

## Architecture Overview

### Data Logger

The data logger uses a multithreaded architecture:

1. **Main thread**: Initializes the device, handles configuration, and manages the acquisition lifecycle
2. **Callback thread**: Executes in the Advantech driver context, receives hardware interrupts, and pushes data to the queue
3. **Writer thread**: Reads from the queue and writes samples to a binary file

#### Class Diagram

```mermaid
classDiagram
    class IDataAcquisitionDevice {
        <<interface>>
        +initialize(deviceDescription) bool
        +configure(startChannel, channelCount, samplesPerChannel, samplingRate, inputMode, inputRange) bool
        +start() bool
        +stop() void
        +dispose() void
        +setDataReadyCallback(callback) void
        +isRunning() const bool
    }

    class AdvantechDevice {
        -m_logger: ILogger
        -m_device: InstantAiCtrl*
        -m_aiCtrl: AiCtrl*
        -m_isRunning: atomic~bool~
        -m_callback: DataReadyCallback
        +initialize(deviceDescription) bool
        +configure(...) bool
        +start() bool
        +stop() void
        +dispose() void
        +setDataReadyCallback(callback) void
        +isRunning() const bool
        -onDataReady(device, data) void
    }

    class ILogger {
        <<interface>>
        +error(message) void
        +warning(message) void
        +info(message) void
        +debug(message) void
    }

    class Logger {
        -m_logLevel: LogLevel
        +error(message) void
        +warning(message) void
        +info(message) void
        +debug(message) void
    }

    class DataProcessingEngine {
        -m_queue: queue~vector~double~~
        -m_mutex: mutex
        -m_cv: condition_variable
        -m_writer: IDataWriter
        -m_logger: ILogger
        -m_isRunning: atomic~bool~
        +pushData(data) void
        +start() void
        +stop() void
        -writerThread() void
    }

    class IDataWriter {
        <<interface>>
        +open(filePath) bool
        +write(data) bool
        +close() void
        +isOpen() const bool
    }

    class BinaryFileWriter {
        -m_fileStream: ofstream
        -m_logger: ILogger
        -m_isOpen: bool
        +open(filePath) bool
        +write(data) bool
        +close() void
        +isOpen() const bool
    }

    class AcquisitionManager {
        -m_device: IDataAcquisitionDevice
        -m_engine: DataProcessingEngine
        -m_writer: IDataWriter
        -m_logger: ILogger
        -m_options: CommandLineOptions
        +initialize() bool
        +startAcquisition() bool
        +waitForStop() void
        +stopAcquisition() void
        +shutdown() void
    }

    IDataAcquisitionDevice <|.. AdvantechDevice : implements
    ILogger <|.. Logger : implements
    IDataWriter <|.. BinaryFileWriter : implements
    AcquisitionManager --> IDataAcquisitionDevice : uses
    AcquisitionManager --> DataProcessingEngine : uses
    AcquisitionManager --> IDataWriter : uses
    AcquisitionManager --> ILogger : uses
    AdvantechDevice --> ILogger : uses
    DataProcessingEngine --> IDataWriter : uses
    DataProcessingEngine --> ILogger : uses
    BinaryFileWriter --> ILogger : uses
```

#### Sequence Diagram (Data Acquisition)

```mermaid
sequenceDiagram
    participant Main as Main Thread
    participant Manager as AcquisitionManager
    participant Device as AdvantechDevice
    participant Engine as DataProcessingEngine
    participant Writer as BinaryFileWriter
    participant Driver as Advantech Driver

    Main->>Manager: initialize()
    Manager->>Device: initialize(deviceDescription)
    Device-->>Manager: true
    Manager->>Device: configure(config)
    Device-->>Manager: true
    Manager->>Engine: start()
    Engine->>Engine: start writer thread
    Manager->>Manager: waitForStop()
    Manager->>Device: setDataReadyCallback(onDataReady)
    Manager->>Device: start()

    loop Until stop
        Driver->>Device: onDataReady(data)
        Device->>Engine: pushData(data)
        Engine->>Engine: enqueue data
        Engine->>Engine: notify writer thread
        Engine->>Writer: write(data)
        Writer->>Writer: write to binary file
    end

    Main->>Manager: stopAcquisition()
    Manager->>Device: stop()
    Manager->>Engine: stop()
    Engine->>Engine: stop writer thread
    Manager->>Device: dispose()
```

### Data Converter

The converter reads binary files and generates CSV output with timestamps:

- Binary files store raw `double` samples with no headers
- Timestamps are computed from the sampling rate
- Output uses Russian locale (comma decimal separator)

#### Class Diagram

```mermaid
classDiagram
    class IDataReader {
        <<interface>>
        +open(filePath) bool
        +readHeader(header) bool
        +readFrames(frames, maxFrames) bool
        +getTotalFrames() size_t
        +isOpen() const bool
        +close() void
    }

    class BinaryReader {
        -m_fileStream: ifstream
        -m_filePath: string
        -m_isOpen: bool
        -m_totalFrames: size_t
        -m_header: DataHeader
        +open(filePath) bool
        +readHeader(header) bool
        +readFrames(frames, maxFrames) bool
        +getTotalFrames() size_t
        +isOpen() const bool
        +close() void
    }

    class IDataWriter {
        <<interface>>
        +open(filePath) bool
        +write(data) bool
        +close() void
        +isOpen() const bool
    }

    class CsvWriter {
        -m_fileStream: ofstream
        -m_isOpen: bool
        -m_separator: char
        -m_decimalSeparator: char
        +open(filePath) bool
        +write(data) bool
        +close() void
        +isOpen() const bool
    }

    class ITimeFormatter {
        <<interface>>
        +format(timeSeconds) string
    }

    class TimeFormatter {
        -m_locale: locale
        -m_format: string
        +format(timeSeconds) string
    }

    class DataConverter {
        -m_reader: IDataReader
        -m_writer: IDataWriter
        -m_formatter: ITimeFormatter
        +convert(inputPath, outputPath) bool
        -processFrames() bool
    }

    IDataReader <|.. BinaryReader : implements
    IDataWriter <|.. CsvWriter : implements
    ITimeFormatter <|.. TimeFormatter : implements
    DataConverter --> IDataReader : uses
    DataConverter --> IDataWriter : uses
    DataConverter --> ITimeFormatter : uses
```

#### Sequence Diagram (Conversion Process)

```mermaid
sequenceDiagram
    participant Main as Main
    participant Converter as DataConverter
    participant Reader as BinaryReader
    participant Writer as CsvWriter
    participant Formatter as TimeFormatter

    Main->>Converter: convert(inputPath, outputPath)
    Converter->>Reader: open(inputPath)
    Reader-->>Converter: true
    Converter->>Reader: readHeader(header)
    Reader-->>Converter: header
    Converter->>Writer: open(outputPath)
    Writer-->>Converter: true

    loop Read all frames
        Converter->>Reader: readFrames(frames, batchSize)
        Reader-->>Converter: frames vector
        loop Each frame
            Converter->>Formatter: format(frame.time)
            Formatter-->>Converter: timestamp string
            Converter->>Writer: write(frame)
            Writer->>Writer: write CSV row
        end
    end

    Converter->>Reader: close()
    Converter->>Writer: close()
    Converter-->>Main: true
```

### Component Interfaces

All code lives in the `app` namespace. Key interfaces:

- `IDataAcquisitionDevice`: Device abstraction
- `ILogger`: Logging infrastructure
- `DataReadyCallback`: Data reception callback
- `IDataReader` / `IDataWriter`: Converter interfaces
- `ITimeFormatter`: Timestamp generation

## Configuration

### Data Logger Command-Line Arguments

| Argument | Description | Default |
|----------|-------------|---------|
| `--device` | Device description (e.g., `PCI-1716,BID#0`) | `DemoDevice,BID#0` |
| `--start-channel` | First channel to acquire (0-15) | `0` |
| `--end-channel` | Last channel to acquire (0-15) | `15` |
| `--rate` | Sampling rate in Hz (max 250000) | `250000` |
| `--samples-per-channel` | Buffer size in samples per channel | `25000` |
| `--output` | Output binary file name | `daq_data.bin` |
| `--input-mode` | Input mode: `bipolar` or `unipolar` | `bipolar` |
| `--input-range` | Input range: `10V`, `5V`, `2.5V`, `1.25V` | `10V` |

### Data Converter Command-Line Arguments

| Argument | Description | Default |
|----------|-------------|---------|
| `--input-file` | Input binary file path | Required |
| `--output-file` | Output CSV file path | Required |
| `--help` | Show help message | — |
| `--version` | Show version information | — |

Example:
```bash
data-converter.exe --input daq_data.bin --output measurements.csv
```

> **Note:** The converter uses Russian locale by default. CSV uses semicolon (`;`) as field separator and comma (`,`) as decimal separator for proper Excel compatibility. Time format is `DD.MM.YYYY HH:MM:SS,mmm` with milliseconds separated by comma.

## Common Issues

1. **Device not found**: Verify Board ID in Advantech Navigator utility
2. **Data loss**: Increase buffer size: `aiCtrl->getBuffer()->setLength(1000000)`
3. **CSV decimal separators**: Uses comma by default (Russian locale). Remove locale for dot separator

## Development

### Build Configuration

- C++17 standard required
- MSVC static linking (`/MT` or `/MTd`)
- The `library::DAQNavi` target wraps the Advantech SDK

### Thread Safety

- Callback thread runs in high-priority Advantech context
- Writer thread owns the queue and handles disk I/O
- `std::atomic<bool>` prevents data races on `m_isRunning`
- Callbacks must not block; only push data to the queue

## License

This project is distributed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

For discussions, ideas, and suggestions, please create an [Issue](https://github.com/andredubov/advantech-data-logger/issues) or submit a [Pull Request](https://github.com/andredubov/advantech-data-logger/pulls).

---

For more detailed information, see [AGENTS.md](AGENTS.md) for AI agent instructions and build details.
