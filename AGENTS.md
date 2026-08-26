# AI Agent Instructions

This file helps AI coding agents understand the PCI-1716 data acquisition project and be immediately productive.

## Project Overview

C++ application suite for high-speed data acquisition (250 kHz) using Advantech PCI-1716 analog input card.

Full details: [README.md](README.md)

## Build Commands

**Preferred:** `build.bat` — interactive, prompts for configuration, architecture, and toolset.

**Manual:**
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

**Outputs:**
- `build/application/data-logger/Release/data-logger.exe`
- `build/application/data-converter/Release/data-converter.exe`

## Running

**Interactive:** `run.bat` — menu-driven launcher with prompts and validation.

**Direct (data-logger):**
```bash
data-logger.exe --device PCI-1716,BID#0 --start-channel 0 --end-channel 7 --rate 100000 --input-mode unipolar --input-range 10V
```

See README for all command-line arguments.

## Key Architecture

- **Namespace:** `app`
- **Data Logger:** Three threads — main (control), callback (hardware interrupt → queue), writer (queue → binary file)
- **Callback:** `BDAQCALL` convention, high-priority Advantech thread; must not block
- **Queue:** `std::queue` + `std::mutex` + `std::condition_variable`
- **Data Format:** Binary file stores raw `double` samples (no headers)
- **Converter:** Reads binary, computes timestamps from sampling rate, outputs CSV with Russian locale (comma decimal separator)

### Key Interfaces

| Interface | Purpose |
|-----------|---------|
| `IDataAcquisitionDevice` | `initialize()`, `configure()`, `start()`, `stop()`, `dispose()`, `setDataReadyCallback()` |
| `ILogger` | `error()`, `warning()`, `info()`, `debug()` |
| `DataReadyCallback` | `std::function<void(const std::vector<double>&)>` |
| `IDataReader` / `IDataWriter` | Binary/CSV converter interfaces |
| `ITimeFormatter` | Timestamp generation |

## Build Configuration

- **C++17** standard
- **MSVC static linking:** `/MT` (Release) or `/MTd` (Debug)
- **Target:** `library::DAQNavi` wraps Advantech SDK

## Windows-Only

Targets Windows due to Advantech DAQNavi SDK. No cross-platform builds.