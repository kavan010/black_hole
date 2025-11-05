# Controls and Features

## Camera Controls

### Mouse Controls
- **Left Mouse Drag**: Orbit camera around black hole
  - Horizontal drag: Change azimuth angle
  - Vertical drag: Change elevation angle
- **Mouse Scroll**: Zoom in/out
  - Scroll up: Move closer to black hole
  - Scroll down: Move farther from black hole
  - Range: 10 billion to 1 trillion meters

### Keyboard Controls

#### Physics
- **G**: Toggle gravity simulation on/off
  - When enabled, celestial objects orbit according to Newtonian gravity
  - When disabled, objects remain stationary
- **K**: Toggle Kerr metric (rotating black hole)
  - OFF: Schwarzschild metric (non-rotating)
  - ON: Kerr metric with frame dragging effects
  - See current spin parameter in console
- **[**: Decrease Kerr spin parameter (-0.1)
  - Range: 0.0 (Schwarzschild) to 1.0 (maximal rotation)
  - Only affects rendering when Kerr mode is enabled
- **]**: Increase Kerr spin parameter (+0.1)
  - Higher spin = stronger frame dragging
  - Visible asymmetry in accretion disk

#### Rendering
- **E**: Increase HDR exposure (+0.1)
  - Makes the scene brighter
  - Useful for viewing darker regions
- **Q**: Decrease HDR exposure (-0.1)
  - Makes the scene darker
  - Useful when accretion disk is too bright
- **R**: Reset exposure to default (1.0)
  - Returns to balanced brightness
- **B**: Toggle bloom post-processing
  - Creates realistic glow around bright objects
  - Simulates camera sensor/eye response
- **+** / **=**: Increase bloom strength (+0.01)
  - Stronger glow effect
- **-**: Decrease bloom strength (-0.01)
  - Subtler glow effect

#### Scientific Tools
- **P**: Export single ray path to CSV
  - Exports geodesic from camera center to destination
  - Output: `ray_path.csv` (lambda, r, theta, phi, x, y, z, velocities, E, L)
  - Useful for scientific analysis and validation
- **C**: Export cone pattern to CSV
  - Exports 11 rays in a cone around camera direction
  - Output: `ray_cone.csv` (ray_id, coordinates)
  - Useful for 3D visualization of geodesics
- **Shift + Left Click**: Export ray at cursor position (NEW!)
  - Click anywhere on screen to export that specific geodesic
  - Output: `ray_click_<timestamp>.csv`
  - Perfect for analyzing specific features (Einstein rings, photon sphere)

#### Visualization Modes (NEW in Phase 5!)
- **V**: Cycle through visualization modes
  - **Mode 0 - Normal**: Photorealistic rendering with Shakura-Sunyaev disk model
  - **Mode 1 - Gravitational Redshift**: Color-coded by redshift factor
    * Blue = weak redshift (far from black hole)
    * Yellow-Orange = moderate redshift
    * Red = extreme redshift (near photon sphere)
  - **Mode 2 - Integration Steps**: Shows computational complexity
    * Blue = few steps (quick rays)
    * Red = many steps (complex orbits)
  - **Mode 3 - Energy Conservation**: Validates numerical integrator
    * Green = perfect conservation (RK4 working correctly)
    * Yellow = acceptable
    * Red = poor (integration issues)
  - **Mode 4 - Carter Constant**: Monitors Kerr metric conservation
    * Green = well-conserved (accurate geodesics)
    * Orange-Red = drifting (simplified equations)

## Visual Features

### Black Hole
- **Event Horizon**: Pure black sphere at Schwarzschild radius (1.269×10¹⁰ m)
- **Gravitational Lensing**: Light bends around the black hole
- **Photon Sphere**: Region at 1.5× Schwarzschild radius where photons orbit

### Accretion Disk
- **Location**: Equatorial plane between 2.2× and 5.2× Schwarzschild radius
- **Temperature Coloring**:
  - Inner regions: Blue-white (hotter, ~8× normal brightness)
  - Outer regions: Orange-red (cooler, ~3× normal brightness)
- **HDR Rendering**: Extremely bright, requires tone mapping

### Celestial Objects
- **Yellow sphere**: Star at (4×10¹¹, 0, 0) meters
- **Red sphere**: Star at (0, 0, 4×10¹¹) meters
- Both objects have mass of 1.98892×10³⁰ kg (1 solar mass)
- **Lighting**: Illuminated by accretion disk with Blinn-Phong shading

### Spacetime Grid
- **Visualization**: 25×25 grid showing spacetime curvature
- **Warping**: Grid bends according to Schwarzschild geometry
- **Update**: Only regenerates when objects move significantly (>100 million meters)

## Physics Simulation

### Geodesic Integration
- **Method**: 4th-order Runge-Kutta (RK4)
- **Step size**: 1×10⁷ meters per integration step
- **Max steps**: 60,000 per ray
- **Coordinates**: Spherical (r, θ, φ) with Schwarzschild metric

### Conserved Quantities
- **Energy**: E = (1 - rs/r) · dt/dλ
- **Angular Momentum**: L = r² sin²θ · dφ/dλ
- Both computed at ray initialization and preserved

### Gravity Simulation (when enabled)
- **Force**: Newtonian gravity (F = G·m₁·m₂/r²)
- **Update**: Velocity Verlet integration
- **Note**: Objects will gradually spiral into black hole

## Performance

### Adaptive Resolution
- **While moving camera**: 160×120 compute resolution for smooth interaction
- **When stationary**: 200×150 compute resolution for best quality
- **Automatic switching**: Based on mouse drag/scroll input

### Performance Metrics
- **FPS**: Logged every 5 seconds to console
- **Frame time**: Average, min, max over 60-frame window
- Example output: `FPS: 60.2 | Frame: 16.6ms (min: 15.2ms, max: 18.3ms)`

### Grid Caching
- Grid only regenerates when objects move >100 million meters
- Massive performance improvement (100-1000× fewer regenerations)

## Rendering Pipeline

1. **Compute Pass**: GPU shader integrates geodesics
   - Each pixel traces a light ray backward from camera
   - Integrates through curved spacetime
   - Writes HDR color to floating-point texture

2. **Tone Mapping Pass**: Converts HDR to displayable LDR
   - ACES filmic tone mapping for cinematic look
   - Exposure adjustment (E/Q/R keys)
   - Gamma correction (sRGB)

3. **Grid Overlay**: Draws spacetime curvature grid
   - Blended over raytraced scene
   - Shows geometric warping of space

## Tips

### Best Viewing Angles
- **Edge-on disk**: Elevation ~90° (horizontal view)
  - Shows Einstein ring and photon sphere
  - Maximum gravitational lensing effect
- **Top-down view**: Elevation ~0° or ~180°
  - Clear view of accretion disk structure
  - See orbital mechanics (when gravity enabled)

### Exposure Settings
- **Default (1.0)**: Balanced for most viewing
- **Bright disk (0.5-0.8)**: When focusing on accretion disk details
- **Dark background (1.5-2.0)**: When looking for faint lensed objects

### Performance Optimization
- Keep camera still for full-resolution rendering
- Disable gravity (G key) when not needed for best performance
- Resolution automatically adapts while moving

## Technical Details

### Schwarzschild Metric
```
ds² = -(1 - rs/r) dt² + (1 - rs/r)⁻¹ dr² + r²(dθ² + sin²θ dφ²)
```
where rs = 2GM/c² = 1.269×10¹⁰ m for Sagittarius A*

### Black Hole Parameters
- **Mass**: 8.54×10³⁶ kg (4.3 million solar masses)
- **Schwarzschild radius**: 1.269×10¹⁰ meters
- **Real black hole**: Sagittarius A* at center of Milky Way

### System Requirements
- **GPU**: OpenGL 4.3+ with compute shader support
- **RAM**: 1 GB minimum
- **CPU**: Multi-core recommended for physics simulation
- **OS**: Windows, Linux, macOS (with OpenGL support)

## Troubleshooting

### Performance Issues
- If FPS < 30: Reduce window size or check GPU drivers
- If grid lags: Disable gravity simulation (G key)
- Console spam: Should be eliminated in this version

### Visual Issues
- **Too bright**: Press Q to decrease exposure
- **Too dark**: Press E to increase exposure
- **Blocky rendering**: Camera is moving, will improve when stationary

### Controls Not Working
- Make sure window has focus (click on it)
- Check console for error messages
- Verify GLFW input is functioning

## Logging

### Log Levels
- **INFO** (default): Important events (gravity toggle, exposure changes, performance stats)
- **DEBUG**: Verbose output (grid regeneration, detailed diagnostics)
- **WARN**: Potential issues
- **ERROR**: Critical problems

### Changing Log Level
Edit `main()` in `black_hole.cpp`:
```cpp
Logger::setLevel(LogLevel::DEBUG);  // For verbose output
Logger::setLevel(LogLevel::WARN);   // For quiet mode
```

## Phase 4 Features (Implemented)

All Phase 4 features are now complete! See `PHASE4_FEATURES.md` for detailed documentation:

- ✅ **Kerr Metric**: Rotating black holes with frame dragging
- ✅ **Bloom Post-Processing**: Realistic glow around bright objects
- ✅ **Ray Path Export**: CSV export for scientific analysis

### Kerr Metric Quick Start
1. Press `K` to enable Kerr mode
2. Press `]` several times to increase spin (try 0.7-0.9)
3. Observe asymmetric accretion disk and frame dragging
4. Press `C` to export geodesics and analyze in Python

### Bloom Quick Start
1. Press `B` to toggle bloom
2. Adjust strength with `+` / `-`
3. Combine with exposure controls (`E`/`Q`) for best results

### Ray Export Quick Start
1. Point camera at interesting feature
2. Press `P` for single ray or `C` for cone pattern
3. Analyze CSV files in Python/MATLAB/Excel
4. See `PHASE4_FEATURES.md` for visualization examples

## Future Enhancements (Phase 5)
Potential future additions:
- Adaptive timestepping (RK45)
- Full Carter constant conservation
- Realistic accretion disk physics (Shakura-Sunyaev)
- Gravitational redshift visualization
- Interactive ray selection (click to export)
- VR support
