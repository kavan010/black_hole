# HDR Rendering Pipeline

## Overview

The black hole simulation now includes a High Dynamic Range (HDR) rendering pipeline that provides more realistic and visually stunning rendering of the accretion disk and black hole environment.

## Features Implemented

### 1. HDR Framebuffer
- Uses `RGBA16F` floating-point textures to store high dynamic range color values
- Allows brightness values to exceed 1.0, which is essential for rendering extremely bright objects like accretion disks
- Supports a much wider range of light intensities compared to traditional 8-bit rendering

### 2. Physically-Based Accretion Disk Lighting
- **Temperature-based coloring**: Inner regions are hotter and appear blue-white, while outer regions are cooler and appear reddish-orange
- **Intensity gradient**: Brightness increases towards the inner edge of the disk (3.0x to 8.0x normal brightness)
- **HDR values**: Disk colors can exceed 1.0, creating realistic bloom and glare effects

### 3. Enhanced Object Lighting
- Objects are illuminated by the accretion disk as a light source
- Includes ambient, diffuse, and specular lighting components
- Distance-based light attenuation for realistic falloff
- Specular highlights create glossy reflections

### 4. Tone Mapping
The tone mapping shader (`tonemap.frag`) converts HDR values to displayable LDR (Low Dynamic Range) values using:

- **ACES Filmic Tone Mapping**: Provides a cinematic look with natural color grading
- **Exposure Control**: User-adjustable exposure to brighten or darken the scene
- **Gamma Correction**: Applies sRGB gamma for correct display on monitors

Alternative tone mapping operators are also included in the shader (commented out):
- Reinhard tone mapping
- Uncharted 2 tone mapping

### 5. Exposure Controls

#### Keyboard Shortcuts:
- **E**: Increase exposure (+0.1)
- **Q**: Decrease exposure (-0.1)
- **R**: Reset exposure to 1.0

## Technical Details

### Shader Changes

**geodesic.comp**:
- Changed output format from `rgba8` to `rgba16f`
- Updated accretion disk color calculation with temperature-based gradient
- Enhanced object lighting with proper HDR values
- Specular highlights for glossy materials

**tonemap.frag** (new file):
- ACES filmic tone mapping operator
- Exposure control uniform
- Gamma correction (sRGB)

### Engine Changes

**black_hole.cpp**:
- Added `hdrTexture` for floating-point color storage
- Added `tonemapProgram` shader for tone mapping pass
- Added `exposure` parameter with keyboard controls
- Modified `dispatchCompute()` to use RGBA16F texture format
- Modified `drawFullScreenQuad()` to apply tone mapping

## Usage Tips

1. **Bright Scenes**: If the accretion disk appears too bright, press **Q** to decrease exposure
2. **Dark Scenes**: If the scene is too dark, press **E** to increase exposure
3. **Default View**: Press **R** to reset to default exposure (1.0)

## Visual Improvements

The HDR pipeline provides several visual enhancements:

1. **Realistic Brightness**: The accretion disk now appears extremely bright, as it should in reality
2. **Better Contrast**: Dark areas (like the black hole) remain dark while bright areas can be much brighter
3. **Natural Color Gradients**: Temperature-based coloring creates realistic color transitions
4. **Improved Detail**: More detail is preserved in both bright and dark areas
5. **Cinematic Look**: ACES tone mapping provides film-like color grading

## Future Enhancements

Possible future additions to the HDR pipeline:

1. **Bloom Effect**: Add glow around bright objects
2. **Chromatic Aberration**: Simulate lens effects for more realism
3. **Adaptive Exposure**: Automatic exposure adjustment based on scene brightness
4. **Multiple Tone Mapping Options**: Allow users to switch between different tone mapping operators
5. **HDR Screenshots**: Save screenshots in HDR format (EXR)

## Performance

The HDR pipeline has minimal performance impact:
- RGBA16F textures use 2x memory compared to RGBA8, but modern GPUs handle this efficiently
- Tone mapping is a simple full-screen pass that runs very fast
- Overall performance should be nearly identical to the previous LDR pipeline
