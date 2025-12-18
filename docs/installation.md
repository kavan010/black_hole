# Installation Guide

## System Requirements

- **OS**: Windows (tested), Linux (Debian/Ubuntu supported).
- **GPU**: Graphics card supporting **OpenGL 4.3** or higher (required for Compute Shaders).
- **Storage**: ~50 MB for source and build files.

## Prerequisites

You need the following tools installed on your system:

1.  **C++ Compiler**: Supporting C++17 or newer.
2.  **CMake**: [Download CMake](https://cmake.org/).
3.  **Vcpkg**: [Download Vcpkg](https://vcpkg.io/en/) (C++ Package Manager).
4.  **Git**: [Download Git](https://git-scm.com/).

## Dependencies

The project relies on these libraries (handled via Vcpkg):
- `glew`
- `glfw3`
- `glm`

## Build Instructions

### Using Vcpkg (Recommended)

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/kavan010/black_hole.git
    cd black_hole
    ```

2.  **Install dependencies**:
    ```bash
    vcpkg install
    ```

3.  **Integrate Vcpkg**:
    ```bash
    vcpkg integrate install
    ```
    *Copy the cmake toolchain path output by this command (e.g., `-DCMAKE_TOOLCHAIN_FILE=...`).*

4.  **Configure and Build**:
    ```bash
    mkdir build
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=[YOUR_VCPKG_PATH]/scripts/buildsystems/vcpkg.cmake
    cmake --build .
    ```

### Linux (Debian/Ubuntu) Alternative

If you prefer not to use Vcpkg on Linux, you can install the dev packages directly:

1.  **Install Packages**:
    ```bash
    sudo apt update
    sudo apt install build-essential cmake libglew-dev libglfw3-dev libglm-dev libgl1-mesa-dev
    ```

2.  **Build**:
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

## Executables

After a successful build, you will find the executables in the `build/Debug` (Windows) or `build/` (Linux) directory:
- `black_hole`: The main 3D GPU simulation.
- `2D_lensing`: The 2D CPU-based ray visualization.
