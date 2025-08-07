# Black Hole Simulation

This project is a C++ and OpenGL-based simulation of a black hole. It visually demonstrates gravitational lensing, spacetime curvature, and accretion disks. The simulation is accelerated using GPU compute shaders for real-time performance.

## Features

*   **3D Black Hole Simulation**: A real-time 3D simulation of a black hole with a compute shader for calculating geodesics.
*   **2D Gravitational Lensing**: A 2D demonstration of how a black hole bends light.
*   **CPU-based Geodesic Calculation**: A CPU implementation for calculating null geodesics.
*   **Spacetime Grid Visualization**: Visualizes the curvature of spacetime around the black hole.

## Getting Started

### Prerequisites

Here's the requirements btw:
*   A C++17 compatible compiler (like GCC or Clang)
*   CMake (version 3.10 or higher)
*   OpenGL
*   The following libraries:
    *   GLEW (The OpenGL Extension Wrangler Library)
    *   GLFW (A multi-platform library for OpenGL, OpenGL ES and Vulkan)
    *   GLM (OpenGL Mathematics)

On many Linux distributions, you can install these with your package manager. For example, on Ubuntu (debian based distross):
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libglew-dev libglfw3-dev libglm-dev
```

### Building the Project

1.  Clone the repository:
    ```bash
    git clone <repository-url>
    cd black_hole
    ```
2.  Create a build directory and navigate into it:
    ```bash
    mkdir build
    cd build
    ```
3.  Run CMake to configure the project:
    ```bash
    cmake ..
    ```
4.  Compile the project using Make:
    ```bash
    make
    ```
This will create three executables in the `build` directory: `black_hole_3d`, `black_hole_2d`, and `cpu_geodesic`.

## Usage

After building the project, you can run the simulations from the `build` directory.

*   **3D Simulation**: `./black_hole_3d`
*   **2D Lensing Demo**: `./black_hole_2d`
*   **CPU Geodesic Demo**: `./cpu_geodesic`
(the Geodesic and 2D lensing does not work somehow)

### 3D Simulation Controls

*   **Orbit**: Click and drag the left mouse button to orbit the camera around the black hole.
*   **Pan**: Click and drag the right mouse button to pan the camera.
*   **Zoom**: Use the mouse scroll wheel to zoom in and out.
*   **Toggle Gravity**: Press the `G` key to toggle the gravitational effects on the orbiting bodies.

## How it Works

The simulation uses different techniques to visualize the black hole.

*   **2D Lensing (`black_hole_2d`)**: This is a simplified 2D simulation that shows how light rays are bent by the black hole's gravity. It uses a 4th-order Runge-Kutta (RK4) integrator to solve the geodesic equations.

*   **3D Simulation (`black_hole_3d`)**: This is the main simulation. It uses a compute shader (`geodesic.comp`) to trace a large number of light rays in parallel on the GPU. This allows for real-time rendering of gravitational lensing effects in 3D. The camera and object data are passed to the GPU using Uniform Buffer Objects (UBOs).

*   **CPU Geodesic (`cpu_geodesic`)**: This provides a CPU-based implementation for calculating the paths of light rays (null geodesics) in 3D space around the black hole. It serves as a reference and educational tool for understanding the underlying physics calculations.

---
*Original README*

---

# black_hole
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

Quickly to run the file you should install opengl (GLFW) and OpenGL wrangler GLEW: https://glew.sourceforge.net/
I'm on windows and used msys2 for installation though.

How the code works:

for 2D: simple, just run 2D_lensing.cpp with the nessesary dependencies installed.

for 3D: black_hole.cpp and geodesic.comp work together to run the simuation faster using GPU, essentially it sends over a UBO and geodesic.comp runs heavy calculations using that data.
should work with nessesary dependencies installed, however I have only run it on windows with my GPU so am not sure!

LMK if you would like an in-depth explanation of how the code works aswell :)
