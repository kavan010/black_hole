@echo off
REM Black Hole Simulation - Windows Packaging Script
REM Creates a distributable Windows package

echo ================================================
echo Black Hole Simulation - Windows Packaging
echo ================================================
echo.

REM Check if build exists
if not exist "build\BlackHole3D.exe" (
    echo [ERROR] BlackHole3D.exe not found!
    echo Please build the project first.
    pause
    exit /b 1
)

REM Create package directory
set PKG_DIR=BlackHole_Simulation_Windows
if exist "%PKG_DIR%" rmdir /s /q "%PKG_DIR%"
mkdir "%PKG_DIR%"

echo [1/6] Creating package directory: %PKG_DIR%

REM Copy executable
echo [2/6] Copying executable...
copy "build\BlackHole3D.exe" "%PKG_DIR%\" >nul

REM Copy shaders
echo [3/6] Copying shaders...
copy "*.vert" "%PKG_DIR%\" >nul
copy "*.frag" "%PKG_DIR%\" >nul
copy "*.comp" "%PKG_DIR%\" >nul

REM Copy documentation
echo [4/6] Copying documentation...
copy "README.md" "%PKG_DIR%\" >nul
copy "README_CN.md" "%PKG_DIR%\" >nul
copy "CONTROLS.md" "%PKG_DIR%\" >nul
copy "CONTROLS_CN.md" "%PKG_DIR%\" >nul
copy "QUICKSTART_CN.md" "%PKG_DIR%\" >nul

REM Copy dependencies (if any DLLs needed)
echo [5/6] Checking dependencies...
REM vcpkg or manual DLL copies would go here

REM Create quick start file
echo [6/6] Creating quick start guide...
(
echo ========================================
echo Black Hole Simulation - Windows
echo ========================================
echo.
echo Quick Start:
echo 1. Double-click BlackHole3D.exe
echo 2. Use mouse to rotate/zoom camera
echo 3. Check Control Panel for parameters
echo 4. Try presets for famous black holes
echo.
echo Documentation:
echo - README.md: Full documentation
echo - CONTROLS.md: Keyboard shortcuts
echo - QUICKSTART_CN.md: Chinese quick start
echo.
echo System Requirements:
echo - Windows 10/11
echo - OpenGL 4.3+ capable GPU
echo - 4GB RAM minimum
echo.
echo ========================================
) > "%PKG_DIR%\START_HERE.txt"

REM Create ZIP package
echo.
echo Creating ZIP package...
powershell Compress-Archive -Path "%PKG_DIR%\*" -DestinationPath "%PKG_DIR%.zip" -Force

echo.
echo ================================================
echo Package created successfully!
echo ================================================
echo.
echo Location: %PKG_DIR%.zip
echo Size:
dir "%PKG_DIR%.zip" | find "%PKG_DIR%.zip"
echo.
echo Ready for distribution!
pause
