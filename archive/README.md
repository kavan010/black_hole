# Archived Files

This directory contains legacy implementations that have been superseded by the current codebase.

## Files

### `CPU-geodesic.cpp`
- **Purpose**: CPU-based geodesic integration with OpenMP parallelization
- **Status**: Superseded by GPU compute shader (`geodesic.comp`)
- **Kept for**: Reference implementation, validation testing
- **Performance**: ~10-100x slower than GPU version

### `ray_tracing.cpp`
- **Purpose**: Early raytracing attempt
- **Status**: Unused, superseded by current implementation
- **Kept for**: Historical reference

### `2D_lensing.cpp`
- **Purpose**: 2D gravitational lensing visualization
- **Status**: Separate educational demo
- **Kept for**: Teaching tool, simplified example

## Why Archived?

These files were moved to reduce confusion and maintain a clean main directory. The active codebase uses:
- `black_hole.cpp` - Main 3D simulation with HDR rendering
- `geodesic.comp` - GPU compute shader for geodesic integration

## Restoration

If you need to restore any of these files:
```bash
git mv archive/<filename> .
```
