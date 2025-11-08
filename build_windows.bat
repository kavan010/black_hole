@echo off
REM =========================================
REM Black Hole Simulation - Windows Build Script
REM =========================================

echo.
echo ==========================================
echo Black Hole Simulation - Windows Build
echo ==========================================
echo.

REM Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake not found! Please install CMake first.
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)

REM Check for vcpkg or system libraries
echo Checking for dependencies...
echo.

REM Option 1: vcpkg (recommended)
if exist "vcpkg.json" (
    echo [INFO] Found vcpkg.json
    echo.
    echo To use vcpkg, run these commands first:
    echo   1. vcpkg install
    echo   2. vcpkg integrate install
    echo   3. Copy the CMAKE_TOOLCHAIN_FILE path from the output
    echo.
    set /p USE_VCPKG="Are dependencies already installed via vcpkg? (y/n): "
    
    if /i "%USE_VCPKG%"=="y" (
        set /p TOOLCHAIN="Enter vcpkg toolchain path (or press Enter to skip): "
        if not "!TOOLCHAIN!"=="" (
            set CMAKE_ARGS=-DCMAKE_TOOLCHAIN_FILE=!TOOLCHAIN!
        )
    ) else (
        echo.
        echo Please install dependencies first:
        echo   vcpkg install
        echo   vcpkg integrate install
        echo.
        pause
        exit /b 1
    )
)

REM Create build directory
if not exist "build" mkdir build

REM Configure with CMake
echo.
echo [1/3] Configuring with CMake...
echo.
cmake -B build -S . %CMAKE_ARGS% -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] CMake configuration failed!
    echo.
    echo Troubleshooting:
    echo   1. Make sure Visual Studio 2022 is installed
    echo   2. Or use a different generator: cmake -G
    echo   3. Check that vcpkg dependencies are installed
    echo.
    pause
    exit /b 1
)

REM Build the project
echo.
echo [2/3] Building project...
echo.
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

REM Success!
echo.
echo ==========================================
echo Build Successful!
echo ==========================================
echo.
echo Executables:
echo   - build\Release\BlackHole3D.exe
echo   - build\Release\BlackHole2D.exe
echo.
echo To run:
echo   cd build\Release
echo   BlackHole3D.exe
echo.
pause
