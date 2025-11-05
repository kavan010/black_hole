# Black Hole Simulation

Real-time gravitational lensing and black hole visualization using GPU-accelerated geodesic raytracing.

![Black Hole Simulation](https://img.shields.io/badge/OpenGL-4.3+-blue) ![C++17](https://img.shields.io/badge/C++-17-orange) ![Physics](https://img.shields.io/badge/Physics-General%20Relativity-red)

## ✨ Features

- **Accurate Physics**: 4th-order Runge-Kutta integration of Schwarzschild geodesics
- **HDR Rendering**: High Dynamic Range pipeline with ACES filmic tone mapping
- **Real-time Performance**: 60+ FPS at 1080p with adaptive resolution
- **Interactive Controls**: Mouse-driven camera orbiting, keyboard exposure adjustment
- **Spacetime Visualization**: Curved grid showing gravitational warping
- **Newtonian Gravity**: Optional N-body simulation of orbiting objects

## 🚀 Quick Start

```bash
./build/BlackHole3D
```

**Basic Controls:**
- **Mouse Drag**: Orbit camera around black hole
- **Mouse Scroll**: Zoom in/out
- **E/Q**: Increase/decrease HDR exposure
- **R**: Reset exposure to default
- **G**: Toggle gravity simulation

For complete controls and features, see **[CONTROLS.md](CONTROLS.md)**

## 📚 Documentation

- **[CONTROLS.md](CONTROLS.md)** - Complete usage guide, controls, and features
- **[HDR_RENDERING.md](HDR_RENDERING.md)** - HDR rendering pipeline technical details
- **[CLAUDE.md](CLAUDE.md)** - Project philosophy and coding principles
- **[VISION.md](VISION.md)** - Future roadmap and architectural vision
- **[REFACTORING_PLAN.md](REFACTORING_PLAN.md)** - Detailed refactoring plan (3 phases)
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

See [PROJECT_REVIEW.md](PROJECT_REVIEW.md) for detailed analysis.

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
