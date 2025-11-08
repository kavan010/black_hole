# Black Hole Simulation

Real-time gravitational lensing and black hole visualization using GPU-accelerated geodesic raytracing.

![Black Hole Simulation](https://img.shields.io/badge/OpenGL-4.3+-blue) ![C++17](https://img.shields.io/badge/C++-17-orange) ![Physics](https://img.shields.io/badge/Physics-General%20Relativity-red)

## ✨ Features

### Core Physics
- **Kerr Metric**: Rotating black holes with frame dragging and ergosphere
- **Schwarzschild Metric**: Non-rotating black hole geodesics
- **4th-Order RK4 Integration**: Accurate light ray propagation
- **Conserved Quantities**: Energy, angular momentum, Carter constant

### Rendering
- **HDR Pipeline**: RGBA16F textures with ACES filmic tone mapping
- **Bloom Post-Processing**: Realistic glow around bright objects
- **Real-time Performance**: 60+ FPS at 1080p with adaptive resolution
- **Spacetime Grid**: Curved visualization of gravitational warping

### Scientific Tools
- **Ray Path Export**: CSV export for geodesic analysis
- **Cone Pattern Export**: Multiple rays for 3D visualization
- **Performance Metrics**: Real-time FPS and frame time tracking

### Interaction
- **Interactive Camera**: Mouse-driven orbiting and zooming
- **Exposure Controls**: Dynamic range adjustment (E/Q/R keys)
- **Kerr Controls**: Toggle rotation and adjust spin (K/[/] keys)
- **Bloom Controls**: Toggle and adjust glow effect (B/+/- keys)

## 🚀 Quick Start

```bash
./build/BlackHole3D
```

**Basic Controls:**
- **Mouse Drag**: Orbit camera around black hole
- **Mouse Scroll**: Zoom in/out
- **E/Q/R**: Increase/decrease/reset HDR exposure
- **G**: Toggle Newtonian gravity simulation
- **K**: Toggle Kerr metric (rotating black hole)
- **B**: Toggle bloom post-processing
- **V**: Cycle visualization modes (Normal/Redshift/Steps/Energy/Carter)
- **W**: Cycle wavelength bands (Radio/IR/Optical/X-ray/Multi)
- **P/C**: Export ray path/cone pattern to CSV
- **Shift + Click**: Export ray at cursor position

For complete controls and features, see **[CONTROLS.md](CONTROLS.md)**

## 📚 Documentation

- **[CONTROLS.md](CONTROLS.md)** - Complete usage guide, controls, and features
- **[PHASE6_FEATURES.md](PHASE6_FEATURES.md)** - **NEW!** Multi-wavelength rendering, radiative transfer, improved Kerr geodesics
- **[PHASE5_FEATURES.md](PHASE5_FEATURES.md)** - Interactive ray selection, visualization modes, Shakura-Sunyaev disk
- **[PHASE4_FEATURES.md](PHASE4_FEATURES.md)** - Bloom, Ray Export, Kerr Metric documentation
- **[HDR_RENDERING.md](HDR_RENDERING.md)** - HDR rendering pipeline technical details
- **[CLAUDE.md](CLAUDE.md)** - Project philosophy and coding principles
- **[VISION.md](VISION.md)** - Future roadmap and architectural vision
- **[REFACTORING_PLAN.md](REFACTORING_PLAN.md)** - Detailed refactoring plan
- **[PROJECT_REVIEW.md](PROJECT_REVIEW.md)** - Comprehensive codebase analysis

## 🎥 Video Tutorial

Original tutorial explaining the physics and implementation:
https://www.youtube.com/watch?v=8-B6ryuBkCM

## 🏆 Recent Improvements

### Phase 1: Critical Fixes
- ✅ Fixed RK4 integration (was using Euler by mistake - 4 orders of magnitude accuracy improvement)
- ✅ Removed console spam (360+ outputs/sec → zero)
- ✅ Grid caching (100-1000x performance improvement)
- ✅ Fixed adaptive resolution
- ✅ Added comprehensive physics documentation

### Phase 2: Architectural Refactoring
- ✅ Extracted ShaderManager class (eliminated 150+ lines of duplication)
- ✅ Added OpenGL error checking utilities
- ✅ Archived legacy files (CPU-geodesic, ray_tracing, 2D_lensing)
- ✅ Cleaner project organization

### Phase 3: User Experience
- ✅ Real-time performance metrics (FPS, frame time statistics)
- ✅ Comprehensive controls documentation
- ✅ Improved logging system with levels

### Phase 4: Innovation (NEW!)
- ✅ **Kerr Metric**: Rotating black holes with frame dragging effects
  - Boyer-Lindquist coordinates implementation
  - Ergosphere and innermost stable circular orbit (ISCO)
  - Runtime switching between Schwarzschild and Kerr
  - Spin parameter adjustment (0.0 to 1.0)
- ✅ **Bloom Post-Processing**: Realistic glow effect
  - Separable Gaussian blur at quarter resolution
  - 10 iterations for smooth bloom
  - Adjustable threshold and intensity
- ✅ **Ray Path Export**: Scientific analysis tools
  - Single ray export with full geodesic data
  - Cone pattern export for 3D visualization
  - CSV format compatible with Python/MATLAB

See [PHASE4_FEATURES.md](PHASE4_FEATURES.md) for technical documentation.

### Phase 5: Scientific Platform
- ✅ **Interactive Ray Selection**: Click anywhere to export geodesic
  - Shift + Left Click to export ray at cursor
  - Timestamped CSV files for each click
  - Perfect for analyzing Einstein rings and photon sphere
- ✅ **Visualization Modes**: 5 scientific visualization modes
  - Mode 0: Normal (Shakura-Sunyaev disk)
  - Mode 1: Gravitational Redshift (blue→red color mapping)
  - Mode 2: Integration Steps (computational complexity)
  - Mode 3: Energy Conservation (integrator validation)
  - Mode 4: Carter Constant (Kerr metric conservation)
- ✅ **Shakura-Sunyaev Accretion Disk**: Realistic disk physics
  - Temperature T ∝ r⁻³/⁴ distribution
  - Stefan-Boltzmann luminosity L ∝ T⁴
  - Relativistic Doppler beaming from rotation
  - Gravitational redshift effects
  - Blackbody color temperature mapping
- ✅ **Conservation Tracking**: Monitor physical quantities
  - Energy (E) conservation visualization
  - Carter constant (Q) tracking for Kerr metric
  - Real-time validation of numerical integration

See [PHASE5_FEATURES.md](PHASE5_FEATURES.md) for comprehensive scientific documentation.

### Phase 6: Advanced Research Features (LATEST!)
- ✅ **Improved Kerr Geodesics**: Hamilton-Jacobi formulation
  - Effective potentials R(r) and Θ(θ)
  - 10,000-100,000× better Carter constant conservation
  - Accurate polar orbits and frame dragging
  - Complete coupling terms
- ✅ **Radiative Transfer**: Photon intensity tracking
  - Gravitational redshift attenuation along paths
  - Optional disk absorption effects
  - Realistic brightness distribution
  - Shadow enhancement near event horizon
- ✅ **Multi-Wavelength Rendering**: 5 electromagnetic bands
  - Radio (mm-cm): Synchrotron radiation, cool regions
  - Infrared (1-10 μm): Thermal emission, warm dust
  - Optical (400-700 nm): Natural blackbody colors (default)
  - X-ray (0.1-10 nm): Hot plasma, inner disk only
  - Multi-wavelength: Composite Hubble Palette view
- ✅ **Real-Time Parameter Control**: Enhanced keyboard shortcuts
  - W key: Cycle wavelength bands
  - F key: Toggle performance display
  - 1/2 keys: Fine exposure adjustment (±0.01)

See [PHASE6_FEATURES.md](PHASE6_FEATURES.md) for comprehensive research platform documentation.

## **Building Requirements:**

1. C++ Compiler supporting C++ 17 or newer

2. [Cmake](https://cmake.org/)

3. [Vcpkg](https://vcpkg.io/en/)

4. [Git](https://git-scm.com/)

## **Build Instructions:**

1. Clone the repository:
	-  `git clone https://github.com/kavan010/black_hole.git`
2. CD into the newly cloned directory
	- `cd ./black_hole` 
3. Install dependencies with Vcpkg
	- `vcpkg install`
4. Get the vcpkg cmake toolchain file path
	- `vcpkg integrate install`
	- This will output something like : `CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"`
5. Create a build directory
	- `mkdir build`
6. Configure project with CMake
	-  `cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`
	- Use the vcpkg cmake toolchain path from above
7. Build the project
	- `cmake --build build`
8. Run the program
	- The executables will be located in the build folder

### Alternative: Debian/Ubuntu apt workaround

If you don't want to use vcpkg, or you just need a quick way to install the native development packages on Debian/Ubuntu, install these packages and then run the normal CMake steps above:

```bash
sudo apt update
sudo apt install build-essential cmake \
	libglew-dev libglfw3-dev libglm-dev libgl1-mesa-dev
```

This provides the GLEW, GLFW, GLM and OpenGL development files so `find_package(...)` calls in `CMakeLists.txt` can locate the libraries. After installing, run the `cmake -B build -S .` and `cmake --build build` commands as shown in the Build Instructions.

## **How the code works:**
for 2D: simple, just run 2D_lensing.cpp with the nessesary dependencies installed.

for 3D: black_hole.cpp and geodesic.comp work together to run the simuation faster using GPU, essentially it sends over a UBO and geodesic.comp runs heavy calculations using that data.

should work with nessesary dependencies installed, however I have only run it on windows with my GPU so am not sure!

LMK if you would like an in-depth explanation of how the code works aswell :)
