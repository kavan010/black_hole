#!/bin/bash
# Black Hole Simulation - macOS Packaging Script
# Creates a distributable macOS .app bundle and DMG

echo "================================================"
echo "Black Hole Simulation - macOS Packaging"
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

# App bundle structure
APP_NAME="Black Hole Simulation"
APP_BUNDLE="${APP_NAME}.app"
rm -rf "$APP_BUNDLE"

echo -e "${GREEN}[1/8]${NC} Creating .app bundle structure..."
mkdir -p "$APP_BUNDLE/Contents/MacOS"
mkdir -p "$APP_BUNDLE/Contents/Resources"
mkdir -p "$APP_BUNDLE/Contents/Frameworks"

# Copy executable
echo -e "${GREEN}[2/8]${NC} Copying executable..."
cp build/BlackHole3D "$APP_BUNDLE/Contents/MacOS/"
chmod +x "$APP_BUNDLE/Contents/MacOS/BlackHole3D"

# Copy shaders
echo -e "${GREEN}[3/8]${NC} Copying shaders..."
cp *.vert *.frag *.comp "$APP_BUNDLE/Contents/Resources/" 2>/dev/null

# Copy documentation
echo -e "${GREEN}[4/8]${NC} Copying documentation..."
cp README.md README_CN.md CONTROLS.md CONTROLS_CN.md QUICKSTART_CN.md "$APP_BUNDLE/Contents/Resources/" 2>/dev/null

# Create Info.plist
echo -e "${GREEN}[5/8]${NC} Creating Info.plist..."
cat > "$APP_BUNDLE/Contents/Info.plist" << 'INFOPLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>BlackHole3D</string>
    <key>CFBundleIdentifier</key>
    <string>com.simulation.blackhole</string>
    <key>CFBundleName</key>
    <string>Black Hole Simulation</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.15</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>CFBundleDisplayName</key>
    <string>Black Hole Simulation</string>
</dict>
</plist>
INFOPLIST

# Create launcher script
echo -e "${GREEN}[6/8]${NC} Creating launcher script..."
cat > "$APP_BUNDLE/Contents/MacOS/launcher.sh" << 'LAUNCHER'
#!/bin/bash
# Launcher script for Black Hole Simulation

# Get the directory of the app bundle
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
RESOURCES="$DIR/../Resources"

# Change to resources directory so shaders can be found
cd "$RESOURCES"

# Run the simulation
"$DIR/BlackHole3D"
LAUNCHER

chmod +x "$APP_BUNDLE/Contents/MacOS/launcher.sh"

# Update executable to be the launcher
mv "$APP_BUNDLE/Contents/MacOS/BlackHole3D" "$APP_BUNDLE/Contents/MacOS/BlackHole3D.bin"
mv "$APP_BUNDLE/Contents/MacOS/launcher.sh" "$APP_BUNDLE/Contents/MacOS/BlackHole3D"

# Create README for distribution
echo -e "${GREEN}[7/8]${NC} Creating distribution README..."
cat > "macOS_README.txt" << 'MACREADME'
========================================
Black Hole Simulation - macOS
========================================

Installation:
1. Drag "Black Hole Simulation.app" to your Applications folder
2. Double-click to run

First Launch:
macOS may show a security warning for apps from unidentified developers.
To allow the app:
1. Go to System Preferences > Security & Privacy
2. Click "Open Anyway" for Black Hole Simulation

System Requirements:
- macOS 10.15 (Catalina) or later
- OpenGL 4.3+ capable GPU
- 4GB RAM minimum
- Intel or Apple Silicon (Universal Binary support TBD)

Documentation:
- Inside the .app bundle: Contents/Resources/README.md
- Controls: CONTROLS.md
- Quick Start (Chinese): QUICKSTART_CN.md

Quick Start:
- Left Mouse: Rotate camera
- Right Mouse: Pan view
- Mouse Wheel: Zoom in/out
- Check Control Panel for black hole parameters
- Try Presets panel for famous black holes

========================================
MACREADME

# Create DMG (if hdiutil is available)
echo -e "${GREEN}[8/8]${NC} Creating DMG package..."
if command -v hdiutil &> /dev/null; then
    DMG_NAME="BlackHole_Simulation_macOS.dmg"
    rm -f "$DMG_NAME"

    # Create temporary DMG
    hdiutil create -volname "Black Hole Simulation" \
                   -srcfolder "$APP_BUNDLE" \
                   -ov -format UDZO \
                   "$DMG_NAME"

    echo ""
    echo "================================================"
    echo -e "${GREEN}Package created successfully!${NC}"
    echo "================================================"
    echo ""
    echo "App Bundle: $APP_BUNDLE"
    echo "DMG Package: $DMG_NAME"
    echo "Size: $(du -h $DMG_NAME | cut -f1)"
    echo ""
    echo "Ready for distribution!"
else
    echo ""
    echo "================================================"
    echo -e "${YELLOW}Warning: hdiutil not found${NC}"
    echo "================================================"
    echo ""
    echo "App Bundle created: $APP_BUNDLE"
    echo "To create DMG manually:"
    echo "  hdiutil create -volname 'Black Hole Simulation' -srcfolder '$APP_BUNDLE' -ov -format UDZO BlackHole_Simulation_macOS.dmg"
    echo ""
    echo "You can distribute the .app bundle directly as a ZIP file:"
    echo "  zip -r BlackHole_Simulation_macOS.zip '$APP_BUNDLE'"
fi
