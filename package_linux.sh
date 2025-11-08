#!/bin/bash
# Black Hole Simulation - Linux Packaging Script
# Creates a distributable Linux package (tar.gz)

echo "================================================"
echo "Black Hole Simulation - Linux Packaging"
echo "================================================"
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if build exists
if [ ! -f "build/BlackHole3D" ]; then
    echo -e "${RED}[ERROR]${NC} BlackHole3D executable not found!"
    echo "Please build the project first: cd build && make"
    exit 1
fi

# Create package directory
PKG_DIR="BlackHole_Simulation_Linux"
rm -rf "$PKG_DIR"
mkdir -p "$PKG_DIR"

echo -e "${GREEN}[1/7]${NC} Creating package directory: $PKG_DIR"

# Copy executable
echo -e "${GREEN}[2/7]${NC} Copying executable..."
cp build/BlackHole3D "$PKG_DIR/"
chmod +x "$PKG_DIR/BlackHole3D"

# Copy shaders
echo -e "${GREEN}[3/7]${NC} Copying shaders..."
cp *.vert *.frag *.comp "$PKG_DIR/" 2>/dev/null

# Copy documentation
echo -e "${GREEN}[4/7]${NC} Copying documentation..."
cp README.md README_CN.md CONTROLS.md CONTROLS_CN.md QUICKSTART_CN.md "$PKG_DIR/" 2>/dev/null

# Create run script
echo -e "${GREEN}[5/7]${NC} Creating run script..."
cat > "$PKG_DIR/run.sh" << 'RUNSCRIPT'
#!/bin/bash
# Black Hole Simulation Launcher

# Check OpenGL version
echo "Checking OpenGL support..."
glxinfo | grep "OpenGL version" || echo "Warning: glxinfo not found. Install mesa-utils to check OpenGL version."

# Set library path if needed
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:./lib"

# Run the simulation
./BlackHole3D

# Check exit code
if [ $? -ne 0 ]; then
    echo ""
    echo "Application exited with an error."
    echo "Please check that your GPU supports OpenGL 4.3+"
    read -p "Press Enter to close..."
fi
RUNSCRIPT

chmod +x "$PKG_DIR/run.sh"

# Create README
echo -e "${GREEN}[6/7]${NC} Creating quick start guide..."
cat > "$PKG_DIR/START_HERE.txt" << 'STARTHERE'
========================================
Black Hole Simulation - Linux
========================================

Quick Start:
1. Run: ./run.sh
2. Use mouse to rotate/zoom camera
3. Check Control Panel for parameters
4. Try presets for famous black holes

Documentation:
- README.md: Full documentation
- CONTROLS.md: Keyboard shortcuts
- QUICKSTART_CN.md: Chinese quick start

System Requirements:
- Linux (Ubuntu 20.04+, Fedora 35+, etc.)
- OpenGL 4.3+ capable GPU
- 4GB RAM minimum
- GLFW, GLEW, GLM libraries

Installation:
Ubuntu/Debian:
  sudo apt install libglfw3 libglew2.2 libglm-dev

Fedora:
  sudo dnf install glfw glew glm

Arch:
  sudo pacman -S glfw glew glm

========================================
STARTHERE

# Create tar.gz package
echo -e "${GREEN}[7/7]${NC} Creating tar.gz package..."
tar -czf "${PKG_DIR}.tar.gz" "$PKG_DIR"

echo ""
echo "================================================"
echo -e "${GREEN}Package created successfully!${NC}"
echo "================================================"
echo ""
echo "Location: ${PKG_DIR}.tar.gz"
echo "Size: $(du -h ${PKG_DIR}.tar.gz | cut -f1)"
echo ""
echo "To install:"
echo "  tar -xzf ${PKG_DIR}.tar.gz"
echo "  cd $PKG_DIR"
echo "  ./run.sh"
echo ""
echo "Ready for distribution!"
