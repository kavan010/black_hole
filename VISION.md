# The Vision: Reimagining Black Hole Simulation

> "Simplicity is the ultimate sophistication." - Leonardo da Vinci

## What We're Really Building

This isn't just a physics simulator. It's a bridge between Einstein's mathematics and human intuition. When someone looks at this simulation, they should **feel** the curvature of spacetime, see the inevitability of the event horizon, understand why nothing escapes.

## The Core Philosophy

### 1. Physics First, Always
- **Accuracy is non-negotiable**: Euler integration was a compromise. We don't compromise.
- **Real RK4**: Not "close enough" - mathematically correct 4th-order Runge-Kutta
- **Adaptive stepping**: The code should know when it's near the event horizon and act accordingly
- **Conservation laws**: Energy and angular momentum should be preserved to machine precision

### 2. Beauty Through Simplicity
Current state: 3 different geodesic implementations, god classes, console spam.

Future state:
```cpp
// One source of truth
class Geodesic {
    static Ray integrate(Ray initial, float distance);
};

// Used everywhere - CPU, GPU, 2D, 3D
// No duplication. No divergence. Perfect.
```

### 3. Performance as Art
- Grid regeneration every frame? No. Cache and invalidate only when necessary.
- Console spam? No. Elegant logging system with levels.
- Camera position recalculation? No. Cache and dirty flag.

The code should be **as efficient as it is elegant**.

### 4. Architecture That Sings

Current:
```cpp
struct Engine {
    // 35+ members doing everything
};
```

Future:
```cpp
class BlackHoleSimulation {
    PhysicsEngine physics;      // One responsibility
    RenderPipeline renderer;    // One responsibility
    CameraController camera;    // One responsibility

    void update() {
        physics.step(deltaTime);
        renderer.draw(physics.state, camera);
    }
};
```

Each class does **one thing perfectly**.

## The Three Pillars

### Pillar 1: Numerical Excellence
**Problem**: GPU uses Euler (1st order), CPU uses RK4 (4th order)
- Error accumulation in strong fields
- Photon orbits at r = 1.5rs missed
- Contradicts the physics we're trying to show

**Solution**:
- Implement **true RK4** in GPU shader
- Add **adaptive step size** (RKF45 or similar)
- Validate against analytical solutions (circular photon orbits)
- Document the mathematics inline

### Pillar 2: Architectural Clarity
**Problem**: God class, tight coupling, code duplication

**Solution**:
```
src/
├── physics/
│   ├── geodesic.hpp        // Shared geodesic equations
│   ├── schwarzschild.hpp   // Metric functions
│   └── integrator.hpp      // RK4 integration
├── rendering/
│   ├── compute_renderer.hpp
│   ├── tonemap.hpp
│   └── grid_visualizer.hpp
├── shaders/
│   ├── physics/
│   │   └── geodesic.glsl   // Shared with CPU via generator
│   └── rendering/
│       ├── compute.comp
│       └── tonemap.frag
└── main.cpp                // Orchestration only
```

### Pillar 3: User Experience
**Problem**: No feedback, broken features, hard-coded values

**Solution**:
- **Real-time diagnostics**: FPS, ray count, integration steps
- **Interactive parameters**: Black hole mass, spin, accretion disk properties
- **Visual debugging**: Show photon paths, conserved quantities
- **Presets**: "Event Horizon Tour", "Photon Ring", "Accretion Disk Close-Up"

## What Success Looks Like

### Code Beauty
```cpp
// Before (black_hole.cpp line 773):
cout << "velocity: " << obj.velocity.x << ", "
     << obj.velocity.y << ", " << obj.velocity.z << endl;

// After:
LOG_DEBUG(physics, "Object velocity: {}", obj.velocity);
```

### Physics Accuracy
```glsl
// Before (geodesic.comp line 96):
void rk4Step(inout Ray ray, float dL) {
    vec3 k1a, k1b;
    geodesicRHS(ray, k1a, k1b);
    ray.r += dL * k1a.x;  // Euler!
}

// After:
void rk4Step(inout Ray ray, float dL) {
    // True 4th-order Runge-Kutta integration
    // Conserves energy to within numerical precision
    vec3 k1, k2, k3, k4;
    k1 = geodesicRHS(ray);
    k2 = geodesicRHS(ray + 0.5*dL*k1);
    k3 = geodesicRHS(ray + 0.5*dL*k2);
    k4 = geodesicRHS(ray + dL*k3);
    ray.state += (dL/6.0) * (k1 + 2*k2 + 2*k3 + k4);
}
```

### Architecture Elegance
```cpp
// Before: Everything in main loop
while (!glfwWindowShouldClose(window)) {
    // 100+ lines of mixed concerns
}

// After: Clear separation
while (simulation.isRunning()) {
    float dt = timer.getDelta();

    simulation.update(dt);
    simulation.render();

    window.swap();
}
```

## The Path Forward

### Phase 1: Foundation (Critical Fixes)
1. **Fix RK4 implementation** - Restore numerical accuracy
2. **Remove console spam** - Implement proper logging
3. **Cache grid generation** - 10-100x performance improvement
4. **Fix adaptive resolution** - Actually implement it

**Impact**: Immediately usable, accurate simulation

### Phase 2: Refactoring (Architecture)
1. **Extract classes** - Break up god class
2. **Consolidate geodesics** - One implementation, shared
3. **Add error handling** - Graceful degradation
4. **Document physics** - Inline explanations

**Impact**: Maintainable, extensible codebase

### Phase 3: Polish (User Experience)
1. **Interactive UI** - Real-time parameter adjustment
2. **Visual debugging** - Show integration paths
3. **Presets system** - Guided tours
4. **Performance metrics** - FPS, ray count display

**Impact**: Delightful to use, easy to understand

### Phase 4: Innovation (New Features)
1. **Kerr metric** - Rotating black holes
2. **Bloom effects** - Realistic accretion disk glow
3. **Ray saving** - Export photon paths for analysis
4. **VR support** - Immersive experience

**Impact**: State-of-the-art visualization

## Success Metrics

### Technical
- [ ] Energy conserved to within 1e-10 over 1000 integration steps
- [ ] Grid only regenerates when objects move (<0.1% of frames)
- [ ] No console output during normal operation
- [ ] Build time < 10 seconds
- [ ] 60+ FPS at 1080p on mid-range GPU

### Code Quality
- [ ] Each class has single responsibility
- [ ] No code duplication (DRY principle)
- [ ] 80%+ test coverage for physics
- [ ] All public APIs documented
- [ ] Zero compiler warnings

### User Experience
- [ ] Visual feedback for all controls
- [ ] Smooth 60 FPS interaction
- [ ] Intuitive camera controls
- [ ] Clear visual distinction: photon sphere, ISCO, event horizon
- [ ] Loading time < 2 seconds

## The Reality Distortion Field

When this is done, opening the application should feel like opening a telescope to the universe. The code should be so clean that a physics student could read it and understand general relativity. The performance should be so good that it feels like the simulation is running in hardware.

We're not just fixing bugs. We're crafting an experience that makes the invisible visible, the complex simple, and the mathematical beautiful.

## Now: Let's Build It

The first step is always the hardest. But every journey begins with a single commit.

What do we tackle first?
