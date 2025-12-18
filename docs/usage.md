# Usage Guide

## Running the Simulation

Navigate to your build directory where the executables are located.

### 3D Simulation
Run the main executable:
```bash
./black_hole
```
*(On Windows, this might be `.\Debug\black_hole.exe`)*

### 2D Lensing Demo
Run the 2D ray tracer:
```bash
./2D_lensing
```

## Controls (3D Simulation)

### Camera Movement

| Action | Control | Description |
| :--- | :--- | :--- |
| **Orbit** | `Left Mouse Button` + `Drag` | Rotate camera around the black hole. |
| **Zoom In** | `Scroll Up` | Move closer to the black hole. |
| **Zoom Out** | `Scroll Down` | Move further away. |
| **Pan** | `Shift` + `Left Mouse` (or `Middle Mouse`) | Pan the camera view (note: camera attempts to stay centered on the black hole). |

### Simulation Toggles

| Key | Function | Description |
| :--- | :--- | :--- |
| **G** | Toggle Gravity | Turns N-body gravity on/off for orbiting objects (not the light rays, those are always affected). |
| **Right Click** | Hold for Gravity | Temporarily enable gravity while held. |

## Understanding the View

- **Black Realm**: The region inside the event horizon ($r < r_s$) from which no light can escape.
- **Accretion Disk**: The glowing ring of matter. You will notice one side is brighter (Doppler beaming) and the back side of the disk is visible *above* and *below* the black hole due to gravitational lensing.
- **Einstein Ring**: If you align the camera perfectly with a background object, it generates a ring of light.
- **Grid**: The coordinate grid visualizes the warping of space. Near the event horizon, the grid lines plunge downwards, illustrating the "infinite well" potential.
