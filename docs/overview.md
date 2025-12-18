# Project Overview

## Introduction

The Black Hole Simulation project aims to visualize the visual distortion caused by extreme gravity, a phenomenon known as gravitational lensing. By solving the geodesic equations for light rays passing near a black hole, we can generate physically accurate images of what a black hole might look like to an observer.

## Goals

1.  **Ray-tracing**: Implement ray tracing to simulate gravitational lensing mechanics.
2.  **Accretion Disk**: Simulate a glowing disk of matter orbiting the black hole.
3.  **Spacetime Curvature**: Provide a visual representation of how mass curves spacetime using a distorted grid.
4.  **Real-time Performance**: Optimize calculations to run at interactive frame rates (60FPS+).

## Technical Approach

The simulation is built using C++ and OpenGL. It employs a hybrid approach:
- **CPU**: Handles the main simulation loop, window management, and basic physics for non-relativistic interactions.
- **GPU (Compute Shaders)**: performs the heavy lifting of ray marching through curved spacetime. We use Runge-Kutta 4 (RK4) integration to solve the geodesic equations for each pixel in parallel.

## Simulating Sagittarius A*

The default configuration simulates **Sagittarius A***, the supermassive black hole at the center of the Milky Way galaxy.
- **Mass**: $\approx 8.54 \times 10^{36}$ kg (approx 4 million solar masses).
- **Schwarzschild Radius ($r_s$)**: The event horizon radius, calculated as $r_s = \frac{2GM}{c^2}$.

## Visualizations

### 2D Lensing (CPU)
A simpler 2D implementation (`2D_lensing.cpp`) demonstrates the path of individual light rays as they are deflected by the black hole's gravity.

### 3D Simulation (GPU)
The full 3D simulation (`black_hole.cpp` + `geodesic.comp`) renders a complete scene with:
- A background starfield (or full-screen quad).
- An accretion disk with relativistic Doppler beaming effects (approximated).
- Gravitational redshift/blueshift (visualized via color intensity).
