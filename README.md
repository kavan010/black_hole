# **black**_**hole**

Black hole simulation project

Here is the black hole raw code, everything will be inside a src bin incase you want to copy the files

I'm writing this as I'm beginning this project (hopefully I complete it ;D) here is what I plan to do:

1. Ray-tracing : add ray tracing to the gravity simulation to simulate gravitational lensing

2. Accretion disk : simulate accreciate disk using the ray tracing + the halos

3. Spacetime curvature : demonstrate visually the "trapdoor in spacetime" that is black holes using spacetime grid

4. [optional] try to make it run realtime ;D

I hope it works :/

Edit: After completion of project -

## **Building Requirements:**

1. C++ Compiler supporting C++ 17 or newer

2. [Cmake](https://cmake.org/)

3. [Vcpkg](https://vcpkg.io/en/)

4. [Git](https://git-scm.com/)

## **Build Instructions:**

1. Clone the repository:
	-  `git clone https://github.com/kavan010/black_hole.git`
2. CD into the C++ folder (all the C++ code, shaders and `CMakeLists.txt` live there)
	- `cd ./black_hole/C++` 
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

### Vulkan version (`BlackHoleVK`)

A Vulkan 1.3 port of the 3D simulation (`black_hole_vk.cpp` + the `*_vk.*` shaders). It is built only when CMake finds the Vulkan headers and `glslc`:

```bash
sudo apt install libvulkan-dev glslc vulkan-validationlayers mesa-vulkan-drivers
cmake -B build -S . && cmake --build build --target BlackHoleVK
cd build && ./BlackHoleVK
```

Or with Docker (no host dependencies besides Docker and a GPU), from the repository root:

```bash
docker build -t black-hole-vk C++
docker run --rm -it --network host -e DISPLAY=$DISPLAY \
  -e XAUTHORITY=/tmp/.xauth -v $XAUTHORITY:/tmp/.xauth:ro -v /tmp/.X11-unix:/tmp/.X11-unix \
  --device /dev/dri black-hole-vk                     # Intel / AMD
  # NVIDIA instead of --device: --gpus all -e NVIDIA_DRIVER_CAPABILITIES=all
```

Details of the port: [`C++/VULKAN_PORT.md`](C++/VULKAN_PORT.md).

## **How the code works:**
for 2D: simple, just run 2D_lensing.cpp with the nessesary dependencies installed.

for 3D: black_hole.cpp and geodesic.comp work together to run the simuation faster using GPU, essentially it sends over a UBO and geodesic.comp runs heavy calculations using that data.

should work with nessesary dependencies installed, however I have only run it on windows with my GPU so am not sure!

LMK if you would like an in-depth explanation of how the code works aswell :)
