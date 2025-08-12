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

Thank you everyone for checking out the video, if you haven't it explains code in detail: https://www.youtube.com/watch?v=8-B6ryuBkCM

## **Building Requirements:**

### Windows

* It is recommended that Windows users install [Microsoft Visual Studio](https://visualstudio.microsoft.com/downloads/) and choose to install the  "Desktop development with C++" as it will give you access to all necessary build requirements and allow you to follow the build instructions directly.

### macOS / Linux

1. C++ Compiler supporting C++17 or newer
   - **Linux**: `g++` or `clang++` is usually available via your system package manager (`apt`, `dnf`, `pacman`, etc.)
   - **macOS**: Install via Xcode Command Line Tools with: `xcode-select --install`

2. [CMake](https://cmake.org/)
   - **Linux**: Available through your system package manager
   - **macOS**: Install via [Homebrew](https://brew.sh): `brew install cmake`

3. [Vcpkg](https://vcpkg.io/en/)
   - **Linux**: Not typically available via package managers — instead, **clone and bootstrap manually**:
     ```bash
     git clone https://github.com/microsoft/vcpkg.git
     cd vcpkg
     ./bootstrap-vcpkg.sh
     ```
   - **macOS**: You can install the `vcpkg` executable via [Homebrew](https://formulae.brew.sh/formula/vcpkg): `brew install vcpkg`
     > ⚠️ However, this does not clone the full source tree. You **must still clone the repo and run** `./bootstrap-vcpkg.sh` to get full vcpkg functionality.

4. [Git](https://git-scm.com/)
   - **Linux**: Available via your package manager
   - **macOS**: Pre-installed, or install via Homebrew: `brew install git`


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

## **How the code works:**
for 2D: simple, just run 2D_lensing.cpp with the nessesary dependencies installed.

for 3D: black_hole.cpp and geodesic.comp work together to run the simuation faster using GPU, essentially it sends over a UBO and geodesic.comp runs heavy calculations using that data.

should work with nessesary dependencies installed, however I have only run it on windows with my GPU so am not sure!

LMK if you would like an in-depth explanation of how the code works aswell :)
