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
- **G**: Toggle gravity simulation on/off
  - When enabled, celestial objects orbit according to Newtonian gravity
  - When disabled, objects remain stationary
- **E**: Increase HDR exposure (+0.1)
  - Makes the scene brighter
  - Useful for viewing darker regions
- **Q**: Decrease HDR exposure (-0.1)
  - Makes the scene darker
  - Useful when accretion disk is too bright
- **R**: Reset exposure to default (1.0)
  - Returns to balanced brightness

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

## Future Features
See `REFACTORING_PLAN.md` Phase 4 for planned enhancements:
- Kerr metric (rotating black holes)
- Bloom post-processing
- Ray path visualization
- Interactive parameter sliders
- VR support
