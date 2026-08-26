# AI Agent Instructions

This file helps AI coding agents understand the PCI-1716 data acquisition project and be immediately productive.

## Project Overview

C++ application suite for high-speed data acquisition (250 kHz) using Advantech PCI-1716 analog input card. See [README.md](README.md) for full details.

## Build Commands

### Using build.bat (interactive)
```bash
build.bat
```
Prompts for configuration (Release/Debug), architecture (Win32/x64), and toolset (v140_xp through v143, ClangCL).

### Using CMake directly
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Build outputs
- `build/application/data-logger/Release/data-logger.exe`
- `build/application/data-converter/Release/data-converter.exe`

## Key Files

| File | Purpose |
|------|---------|
| `application/data-logger/src/main.cpp` | Real-time acquisition with multithreaded writer |
| `application/data-converter/src/main.cpp` | Binary-to-CSV converter |
| `library/DAQNavi/inc/bdaqctrl.h` | Advantech DAQNavi SDK interface |
| `application/data-logger/devices/advantech/AdvantechDevice.hpp` | Device implementation for PCI-1716 |
| `application/data-logger/core/interfaces/IDataAcquisitionDevice.hpp` | Device abstraction interface |
| `application/data-converter/core/inc/IDataReader.hpp` | Binary reader interface |
| `application/data-converter/core/inc/IDataWriter.hpp` | CSV writer interface |
| `application/data-converter/core/inc/ITimeFormatter.hpp` | Timestamp generation interface |

## Architecture Notes

- **Data Logger**: Three threads — main (initialization/control), callback (hardware interrupt → queue push), writer (queue → binary file)
- **Queue**: Uses `std::queue` + `std::mutex` + `std::condition_variable` for thread-safe handoff
- **Callbacks**: `BDAQCALL` convention; runs in high-priority Advantech thread
- **Data format**: Binary file stores raw `double` samples (no headers)
- **Converter**: Reads binary file, computes timestamps from sampling rate, outputs CSV with Russian locale (comma decimal separator)

### Component Architecture

All code lives in the `app` namespace. Key interfaces:
- `IDataAcquisitionDevice`: Device abstraction with `initialize()`, `configure()`, `start()`, `stop()`, `dispose()`, `setDataReadyCallback()`
- `ILogger`: Logging interface with `error()`, `warning()`, `info()`, `debug()`
- `DataReadyCallback`: `std::function<void(const std::vector<double>&)>` for receiving sample data
- `IDataReader` / `IDataWriter`: Converter interfaces for reading binary and writing CSV
- `ITimeFormatter`: Timestamp generation interface

### Thread Safety

- The callback thread runs in the Advantech driver context with high priority
- The writer thread owns the queue and writes to disk
- `std::atomic<bool>` is used for `m_isRunning` to avoid data races
- The callback must not block; it only pushes data to the queue

### CMake Configuration

- C++17 standard required
- MSVC static linking (`/MT` or `/MTd`)
- The `library::DAQNavi` target wraps the Advantech SDK
- Output executables: `data-logger.exe` and `data-converter.exe`

## Command-Line Arguments (data-logger)

All configuration is done via command-line arguments. Run `data-logger.exe --help` for details.

| Argument | Description | Default |
|----------|-------------|---------|
| `--device` | Device description (e.g., `PCI-1716,BID#0` or `DemoDevice,BID#0`) | `DemoDevice,BID#0` |
| `--start-channel` | First channel to acquire (0-15) | `0` |
| `--end-channel` | Last channel to acquire (0-15) | `15` |
| `--rate` | Sampling rate in Hz (max 250000) | `250000` |
| `--samples-per-channel` | Buffer size in samples per channel | `25000` |
| `--output` | Output binary file name | `daq_data.bin` |
| `--input-mode` | Input mode: `bipolar` or `unipolar` | `bipolar` |
| `--input-range` | Input range: `10V`, `5V`, `2.5V`, `1.25V` | `10V` |
| `--help` | Show help | — |
| `--version` | Show version | — |

Example:
```bash
data-logger.exe --device PCI-1716,BID#0 --start-channel 0 --end-channel 7 --rate 100000 --input-mode unipolar --input-range 10V
```

## Common Issues

1. **"Device not found"**: Verify Board ID in Advantech Navigator; update `deviceDescription`
2. **Data loss**: Increase `aiCtrl->getBuffer()->setLength(1000000)`
3. **CSV decimal separators**: Uses Russian locale (comma). Remove `csvFile.imbue(std::locale("Russian"))` for dot separator

## Windows-Only

This project targets Windows due to Advantech DAQNavi SDK limitations. Do not attempt cross-platform builds.