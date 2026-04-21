# Python Implementation

This directory contains a Python port of the 3D black hole visualizer.

It keeps the same high-level rendering approach as the C++ version:

- a `glfw` window and OpenGL context
- a compute shader for geodesic-based black hole rendering
- a grid overlay to visualize spacetime curvature
- orbit camera controls around the black hole

## Python Version

Python 3.10 or newer is recommended.

## Dependencies

The Python version uses:

- `glfw`
- `numpy`
- `PyOpenGL`

Install them with:

```bash
python -m venv .venv
.venv/bin/pip install -r python/requirements.txt
```

## Running

Run the Python version from the repository root:

```bash
.venv/bin/python python/black_hole.py
```

On Wayland systems that need an X11/GLX path for this OpenGL setup:

```bash
GLFW_PLATFORM=x11 .venv/bin/python python/black_hole.py
```

## Controls

- Left mouse drag: orbit camera
- Mouse wheel: zoom
- `G`: toggle gravity updates
- Right mouse button: hold to enable gravity temporarily
