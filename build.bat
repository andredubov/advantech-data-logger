@echo off
setlocal

echo Select configuratin:
echo 1 - Release
echo 2 - Debug
set /p choice="Enter 1 or 2: "

if "%choice%"=="1" (
    set CONFIG=Release
) else if "%choice%"=="2" (
    set CONFIG=Debug
) else (
    echo Invalid choice!
    pause
    exit /b 1
)

echo.
echo Select platform architecture:
echo 1 - Win32
echo 2 - x64
set /p arch="Enter 1-2: "

if "%arch%"=="1" (
    set ARCH=Win32
) else if "%arch%"=="2" (
    set ARCH=x64
) else (
    echo Invalid architecture choice!
    pause
    exit /b 1
)

echo.
echo Select toolset:
echo 1 - v140_xp (VS 2015)
echo 2 - v141_xp (VS 2017)
echo 3 - v141 (VS 2017)
echo 4 - v142 (VS 2019)
echo 5 - v143 (VS 2022)
echo 6 - ClangCL
echo 7 - None (default)
set /p toolset_option="Enter 1-7: "
echo.

set TOOLSET=
if "%toolset_option%"=="1" set TOOLSET=v140_xp
if "%toolset_option%"=="2" set TOOLSET=v141_xp
if "%toolset_option%"=="3" set TOOLSET=v141
if "%toolset_option%"=="4" set TOOLSET=v142
if "%toolset_option%"=="5" set TOOLSET=v143
if "%toolset_option%"=="6" set TOOLSET=ClangCL

echo Building for %ARCH% with toolset: %TOOLSET%
echo Building in configuration: %CONFIG%
echo.
rmdir /s /q .\build
cmake -G "Visual Studio 17 2022" -A %ARCH% -S . -B ./build -DCMAKE_GENERATOR_TOOLSET=%TOOLSET% -D_WIN32_WINNT=0x0601
cmake --build ./build --config %CONFIG% --target ALL_BUILD -j 6 --