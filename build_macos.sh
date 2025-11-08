#!/bin/bash
# =========================================
# Black Hole Simulation - macOS Build Script
# =========================================

set -e

echo ""
echo "=========================================="
echo "Black Hole Simulation - macOS Build"
echo "=========================================="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}[ERROR]${NC} CMake not found!"
    echo "Install with: brew install cmake"
    echo "Or download from: https://cmake.org/download/"
    exit 1
fi

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo -e "${YELLOW}[WARN]${NC} Homebrew not found!"
    echo "Homebrew makes installing dependencies easier."
    echo "Install from: https://brew.sh"
    echo ""
fi

# Check for Xcode Command Line Tools
if ! xcode-select -p &> /dev/null; then
    echo -e "${RED}[ERROR]${NC} Xcode Command Line Tools not found!"
    echo "Install with: xcode-select --install"
    exit 1
fi

# Check for dependencies
echo "Checking for dependencies..."
echo ""

MISSING_DEPS=0

# Check for GLEW
if ! brew list glew &> /dev/null && [ ! -f "/usr/local/include/GL/glew.h" ]; then
    echo -e "${YELLOW}[MISSING]${NC} GLEW"
    MISSING_DEPS=1
fi

# Check for GLFW
if ! brew list glfw &> /dev/null && [ ! -f "/usr/local/include/GLFW/glfw3.h" ]; then
    echo -e "${YELLOW}[MISSING]${NC} GLFW"
    MISSING_DEPS=1
fi

# Check for GLM
if ! brew list glm &> /dev/null && [ ! -d "/usr/local/include/glm" ]; then
    echo -e "${YELLOW}[MISSING]${NC} GLM"
    MISSING_DEPS=1
fi

if [ $MISSING_DEPS -eq 1 ]; then
    echo ""
    echo -e "${YELLOW}Some dependencies are missing!${NC}"
    echo ""
    echo "Install all dependencies with:"
    echo ""
    echo "  brew install cmake glew glfw glm"
    echo ""
    echo "Or use vcpkg:"
    echo "  vcpkg install glew glfw3 glm"
    echo ""
    read -p "Do you want to continue anyway? (y/n): " -n 1 -r
    echo ""
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
else
    echo -e "${GREEN}[OK]${NC} All dependencies found"
fi

# Create build directory
mkdir -p build

# Configure with CMake
echo ""
echo "[1/3] Configuring with CMake..."
echo ""
if cmake -B build -S . -DCMAKE_BUILD_TYPE=Release; then
    echo -e "${GREEN}[OK]${NC} Configuration successful"
else
    echo -e "${RED}[ERROR]${NC} CMake configuration failed!"
    exit 1
fi

# Build the project
echo ""
echo "[2/3] Building project..."
echo ""
if cmake --build build -- -j$(sysctl -n hw.ncpu); then
    echo -e "${GREEN}[OK]${NC} Build successful"
else
    echo -e "${RED}[ERROR]${NC} Build failed!"
    exit 1
fi

# Check if executables were created
echo ""
echo "[3/3] Verifying executables..."
echo ""

if [ -f "build/BlackHole3D" ]; then
    echo -e "${GREEN}[OK]${NC} BlackHole3D created"
else
    echo -e "${RED}[ERROR]${NC} BlackHole3D not found!"
    exit 1
fi

if [ -f "build/BlackHole2D" ]; then
    echo -e "${GREEN}[OK]${NC} BlackHole2D created"
else
    echo -e "${YELLOW}[WARN]${NC} BlackHole2D not found (optional)"
fi

# Success!
echo ""
echo "=========================================="
echo -e "${GREEN}Build Successful!${NC}"
echo "=========================================="
echo ""
echo "Executables:"
echo "  - build/BlackHole3D"
echo "  - build/BlackHole2D"
echo ""
echo "To run:"
echo "  ./build/BlackHole3D"
echo ""
echo "Note for macOS:"
echo "  - May need to allow the app in Security & Privacy settings"
echo "  - First run might be slow (shader compilation)"
echo "  - Use dedicated GPU if available (check Activity Monitor)"
echo ""
