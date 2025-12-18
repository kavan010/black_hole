# **black**_**hole**

> **ALL RIGHTS RESERVED ® & COPYRIGHT © 2025** [Kavan010](https://github.com/kavan010)  
> **Documentation updated and repository maintained by:** [Kavicastelo](https://github.com/kavicastelo)  
> **This is a fork of [Kavan010's original repository](https://github.com/kavan010/black_hole).**

## Project Overview

This repository contains a simulation of black holes, originally developed by [Kavan010](https://github.com/kavan010). I (Kavicastelo) have updated the documentation, added additional features like GitHub Pages, and created a social preview banner. The goal is to simulate and visualize black hole phenomena using ray tracing, accretion disks, and spacetime curvature.

### Features:
1. **Ray-tracing** simulation
2. **Accretion disk** visualization
3. **Spacetime curvature** demonstration
4. **[Optional] Real-time** simulation feature

For more details, check out the **[original project](https://github.com/kavan010/black_hole)**.

---

## **Building Requirements:**

1. C++ Compiler supporting C++ 17 or newer
2. [CMake](https://cmake.org/)
3. [Vcpkg](https://vcpkg.io/en/)
4. [Git](https://git-scm.com/)

---

## **Build Instructions:**

1. Clone the repository:
   - `git clone https://github.com/kavicastelo/black_hole.git`  *(or the original repo if using that)*
2. CD into the newly cloned directory:
   - `cd ./black_hole`
3. Install dependencies with Vcpkg:
   - `vcpkg install`
4. Get the vcpkg cmake toolchain file path:
   - `vcpkg integrate install`
   - This will output something like: `CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"`
5. Create a build directory:
   - `mkdir build`
6. Configure the project with CMake:
   - `cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`
7. Build the project:
   - `cmake --build build`
8. Run the program:
   - The executables will be located in the `build` folder

---

### Alternative: Debian/Ubuntu apt workaround

If you don't want to use vcpkg, or you just need a quick way to install the native development packages on Debian/Ubuntu, install these packages and then run the normal CMake steps above:

```bash
sudo apt update
sudo apt install build-essential cmake \
	libglew-dev libglfw3-dev libglm-dev libgl1-mesa-dev
```

This provides the GLEW, GLFW, GLM and OpenGL development files so `find_package(...)` calls in CMakeLists.txt can locate the libraries. After installing, run the `cmake -B build -S .` and `cmake --build build` commands as shown in the Build Instructions.

## **How the code works:**
**for 2D**: simple, just run `2D_lensing.cpp` with the nessesary dependencies installed.

**for 3D**: `black_hole.cpp` and `geodesic.comp` work together to run the simuation faster using GPU, essentially it sends over a UBO and `geodesic.comp` runs heavy calculations using that data.

It should work with the necessary dependencies installed; however, I have only run it on Windows with my GPU, so compatibility with other setups might vary.

Let me know if you'd like an in-depth explanation of how the code works!

---

## Credits:
- **Original Code:** [Kavan010](https://github.com/kavan010)
- **Fork and Documentation Updates:** [Kavicastelo](https://github.com/kavicastelo)

## License:
This repository does not currently have an official license. Please refer to the [original project](https://github.com/kavan010/black_hole) for usage and licensing details.
