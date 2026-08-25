@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=build\application
set LOGGER_EXE=%BUILD_DIR%\data-logger\Release\data-logger.exe
set CONVERTER_EXE=%BUILD_DIR%\data-converter\Release\data-converter.exe

:menu
cls
echo ========================================
echo   PCI-1716 Data Acquisition Tools
echo ========================================
echo.
echo  1. Run data-logger (acquisition)
echo  2. Run data-converter (binary to CSV)
echo  3. Exit
echo.

set /p choice="Select an option (1-3): "
if "%choice%"=="1" goto logger
if "%choice%"=="2" goto converter
if "%choice%"=="3" goto end

echo Invalid choice. Please try again.
goto menu

:logger
if not exist "%LOGGER_EXE%" (
    echo.
    echo Error: data-logger.exe not found at %LOGGER_EXE%
    echo Please build the project first using build.bat
    echo.
    pause
    goto menu
)
echo.
echo ========================================
echo   data-logger arguments
echo ========================================
echo.
echo [--device]        Device description (e.g., PCI-1716,BID#0 or DemoDevice,BID#0)
echo [--start-channel] First channel (0-15)
echo [--end-channel]   Last channel (0-15)
echo [--rate]          Sampling rate in Hz (max 250000)
echo [--samples]       Samples per channel
echo [--output]        Output binary file name
echo.
echo Press Enter to use default values, or type your value.
echo.

set DEVICE=
set /p DEVICE="Device [DemoDevice,BID#0]: "
if "!DEVICE!"=="" set DEVICE=DemoDevice,BID#0

set START_CH=
set /p START_CH="Start channel [0]: "
if "!START_CH!"=="" set START_CH=0

set END_CH=
set /p END_CH="End channel [15]: "
if "!END_CH!"=="" set END_CH=15

set RATE=
set /p RATE="Sampling rate [250000]: "
if "!RATE!"=="" set RATE=250000

set SAMPLES=
set /p SAMPLES="Samples per channel [25000]: "
if "!SAMPLES!"=="" set SAMPLES=25000

set OUTPUT=
set /p OUTPUT="Output file [daq_data.bin]: "
if "!OUTPUT!"=="" set OUTPUT=daq_data.bin

echo.
echo Running data-logger with:
echo   --device       !DEVICE!
echo   --start-channel !START_CH!
echo   --end-channel   !END_CH!
echo   --rate          !RATE!
echo   --samples       !SAMPLES!
echo   --output        !OUTPUT!
echo.
"%LOGGER_EXE%" --device "!DEVICE!" --start-channel !START_CH! --end-channel !END_CH! --rate !RATE! --samples-per-channel !SAMPLES! --output "!OUTPUT!"
if errorlevel 1 (
    echo.
    echo data-logger exited with error code %errorlevel%
)
pause
goto menu

:converter
if not exist "%CONVERTER_EXE%" (
    echo.
    echo Error: data-converter.exe not found at %CONVERTER_EXE%
    echo Please build the project first using build.bat
    echo.
    pause
    goto menu
)
echo.
echo ========================================
echo   data-converter arguments
echo ========================================
echo.
echo [--input]   Input binary file name
echo [--output]  Output CSV file name
echo.
echo Press Enter to use default values, or type your value.
echo.

set INPUT=
set /p INPUT="Input file [daq_data.bin]: "
if "!INPUT!"=="" set INPUT=daq_data.bin

set OUTPUT_CSV=
set /p OUTPUT_CSV="Output CSV file [daq_data.csv]: "
if "!OUTPUT_CSV!"=="" set OUTPUT_CSV=daq_data.csv

echo.
echo Running data-converter with:
echo   --input  !INPUT!
echo   --output !OUTPUT_CSV!
echo.
"%CONVERTER_EXE%" --input "!INPUT!" --output "!OUTPUT_CSV!"
if errorlevel 1 (
    echo.
    echo data-converter exited with error code %errorlevel%
)
pause
goto menu

:end
echo.
echo Exiting...
exit /b 0
