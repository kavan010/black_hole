# Refactoring Plan: From Good to Insanely Great

> "We're here to put a dent in the universe." - Steve Jobs

Based on the comprehensive codebase analysis and our vision, here's the roadmap to transform this black hole simulation into something truly exceptional.

---

## 🔴 Phase 1: Critical Fixes (Week 1)
**Goal**: Make the simulation numerically accurate and performant

### 1.1 Fix RK4 Integration in GPU Shader ⚠️ CRITICAL
**File**: `geodesic.comp`
**Current State**: Euler method (1st order) with misleading function name
**Impact**: 4 orders of magnitude accuracy loss

**Implementation**:
```glsl
// Current (WRONG):
void rk4Step(inout Ray ray, float dL) {
    vec3 k1a, k1b;
    geodesicRHS(ray, k1a, k1b);
    ray.r += dL * k1a.x;  // Only k1 - this is Euler!
}

// Fixed (CORRECT):
void rk4Step(inout Ray ray, float dL) {
    // Save original state
    Ray r0 = ray;

    // k1: derivative at start
    vec3 k1_pos, k1_vel;
    geodesicRHS(r0, k1_pos, k1_vel);

    // k2: derivative at midpoint using k1
    Ray r2 = r0;
    r2.r += 0.5 * dL * k1_pos.x;
    r2.theta += 0.5 * dL * k1_pos.y;
    r2.phi += 0.5 * dL * k1_pos.z;
    r2.dr += 0.5 * dL * k1_vel.x;
    r2.dtheta += 0.5 * dL * k1_vel.y;
    r2.dphi += 0.5 * dL * k1_vel.z;
    vec3 k2_pos, k2_vel;
    geodesicRHS(r2, k2_pos, k2_vel);

    // k3: derivative at midpoint using k2
    Ray r3 = r0;
    r3.r += 0.5 * dL * k2_pos.x;
    r3.theta += 0.5 * dL * k2_pos.y;
    r3.phi += 0.5 * dL * k2_pos.z;
    r3.dr += 0.5 * dL * k2_vel.x;
    r3.dtheta += 0.5 * dL * k2_vel.y;
    r3.dphi += 0.5 * dL * k2_vel.z;
    vec3 k3_pos, k3_vel;
    geodesicRHS(r3, k3_pos, k3_vel);

    // k4: derivative at endpoint using k3
    Ray r4 = r0;
    r4.r += dL * k3_pos.x;
    r4.theta += dL * k3_pos.y;
    r4.phi += dL * k3_pos.z;
    r4.dr += dL * k3_vel.x;
    r4.dtheta += dL * k3_vel.y;
    r4.dphi += dL * k3_vel.z;
    vec3 k4_pos, k4_vel;
    geodesicRHS(r4, k4_pos, k4_vel);

    // Weighted average (RK4 formula)
    ray.r += (dL/6.0) * (k1_pos.x + 2.0*k2_pos.x + 2.0*k3_pos.x + k4_pos.x);
    ray.theta += (dL/6.0) * (k1_pos.y + 2.0*k2_pos.y + 2.0*k3_pos.y + k4_pos.y);
    ray.phi += (dL/6.0) * (k1_pos.z + 2.0*k2_pos.z + 2.0*k3_pos.z + k4_pos.z);
    ray.dr += (dL/6.0) * (k1_vel.x + 2.0*k2_vel.x + 2.0*k3_vel.x + k4_vel.x);
    ray.dtheta += (dL/6.0) * (k1_vel.y + 2.0*k2_vel.y + 2.0*k3_vel.y + k4_vel.y);
    ray.dphi += (dL/6.0) * (k1_vel.z + 2.0*k2_vel.z + 2.0*k3_vel.z + k4_vel.z);

    // Update Cartesian coordinates
    ray.x = ray.r * sin(ray.theta) * cos(ray.phi);
    ray.y = ray.r * sin(ray.theta) * sin(ray.phi);
    ray.z = ray.r * cos(ray.theta);
}
```

**Validation**:
- Test against analytical circular photon orbit (r = 1.5 rs)
- Energy conservation error < 1e-10
- Compare with CPU-geodesic.cpp results

**Time**: 2-3 hours
**Priority**: 🔴 CRITICAL

---

### 1.2 Remove Console Spam ⚠️ HIGH
**Files**: `black_hole.cpp` lines 116, 719, 722, 773
**Current State**: Prints every frame, kills performance
**Impact**: 360+ console outputs per second at 60 FPS

**Implementation**:

Create `src/utils/logger.hpp`:
```cpp
#pragma once
#include <iostream>
#include <sstream>

enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
private:
    static LogLevel minLevel;

public:
    static void setLevel(LogLevel level) { minLevel = level; }

    template<typename... Args>
    static void debug(Args&&... args) {
        if (minLevel <= LogLevel::DEBUG) {
            log("DEBUG", std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void info(Args&&... args) {
        if (minLevel <= LogLevel::INFO) {
            log("INFO", std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void warn(Args&&... args) {
        if (minLevel <= LogLevel::WARN) {
            log("WARN", std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void error(Args&&... args) {
        log("ERROR", std::forward<Args>(args)...);
    }

private:
    template<typename... Args>
    static void log(const char* level, Args&&... args) {
        std::ostringstream oss;
        oss << "[" << level << "] ";
        ((oss << std::forward<Args>(args)), ...);
        std::cerr << oss.str() << std::endl;
    }
};

inline LogLevel Logger::minLevel = LogLevel::INFO;
```

Replace all console output:
```cpp
// Before:
cout << "velocity: " << obj.velocity.x << ", " << obj.velocity.y << endl;

// After:
Logger::debug("Object velocity: (", obj.velocity.x, ", ", obj.velocity.y, ", ", obj.velocity.z, ")");
```

**Time**: 1 hour
**Priority**: 🔴 HIGH

---

### 1.3 Fix Grid Regeneration ⚠️ HIGH
**File**: `black_hole.cpp` line 783
**Current State**: Regenerates every frame (676 vertices, 2500 indices)
**Impact**: Unnecessary CPU work, GPU upload bandwidth waste

**Implementation**:
```cpp
class GridCache {
    std::vector<ObjectData> lastObjects;
    bool isDirty = true;

public:
    bool needsRegeneration(const std::vector<ObjectData>& currentObjects) {
        if (isDirty || lastObjects.size() != currentObjects.size()) {
            return true;
        }

        for (size_t i = 0; i < currentObjects.size(); ++i) {
            // Check if position changed significantly
            glm::vec3 lastPos = glm::vec3(lastObjects[i].posRadius);
            glm::vec3 currPos = glm::vec3(currentObjects[i].posRadius);
            if (glm::distance(lastPos, currPos) > 1e8f) {  // 100 million meters threshold
                return true;
            }
        }
        return false;
    }

    void markGenerated(const std::vector<ObjectData>& objects) {
        lastObjects = objects;
        isDirty = false;
    }
};

// In main loop:
static GridCache gridCache;
if (gridCache.needsRegeneration(objects)) {
    engine.generateGrid(objects);
    gridCache.markGenerated(objects);
}
```

**Expected Speedup**: 100-1000x reduction in grid generation calls

**Time**: 1 hour
**Priority**: 🔴 HIGH

---

### 1.4 Fix Broken Adaptive Resolution
**File**: `geodesic.comp` lines 118-119, `black_hole.cpp` line 543-544
**Current State**: Both branches return same values

**Implementation**:
```cpp
// In black_hole.cpp:
int COMPUTE_WIDTH_MOVING = 160;   // 80% of full
int COMPUTE_HEIGHT_MOVING = 120;
int COMPUTE_WIDTH_FULL = 200;
int COMPUTE_HEIGHT_FULL = 150;

void dispatchCompute(const Camera& cam) {
    int cw = cam.moving ? COMPUTE_WIDTH_MOVING : COMPUTE_WIDTH_FULL;
    int ch = cam.moving ? COMPUTE_HEIGHT_MOVING : COMPUTE_HEIGHT_FULL;
    // ... rest
}
```

```glsl
// In geodesic.comp:
// Use push constants or uniforms instead of hard-coded values
layout(push_constant) uniform ComputeConfig {
    int width;
    int height;
};

void main() {
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    if (pix.x >= width || pix.y >= height) return;
    // ... rest
}
```

**Time**: 30 minutes
**Priority**: 🟡 MEDIUM

---

### 1.5 Add Physics Documentation
**Files**: `geodesic.comp`, `black_hole.cpp`
**Current State**: No inline comments explaining equations

**Implementation**:
Add comprehensive header comment to `geodesic.comp`:
```glsl
/*
 * SCHWARZSCHILD NULL GEODESIC INTEGRATION
 * =========================================
 *
 * This shader integrates light rays (null geodesics) in the curved spacetime
 * around a non-rotating (Schwarzschild) black hole.
 *
 * METRIC:
 *   ds² = -(1 - rs/r) dt² + (1 - rs/r)⁻¹ dr² + r²(dθ² + sin²θ dφ²)
 *
 * where:
 *   rs = 2GM/c² = Schwarzschild radius (event horizon)
 *   (t, r, θ, φ) = Schwarzschild coordinates
 *
 * CONSERVED QUANTITIES:
 *   E = (1 - rs/r) · dt/dλ           [Energy per unit mass]
 *   L = r² sin²θ · dφ/dλ             [Angular momentum]
 *
 * EQUATIONS OF MOTION:
 *   d²r/dλ² = -(rs/2r²)(1-rs/r)(dt/dλ)² + (rs/2r²)/(1-rs/r)(dr/dλ)²
 *           + r(dθ/dλ)² + r sin²θ(dφ/dλ)²
 *
 *   d²θ/dλ² = -(2/r)(dr/dλ)(dθ/dλ) + sinθ cosθ(dφ/dλ)²
 *
 *   d²φ/dλ² = -(2/r)(dr/dλ)(dφ/dλ) - (2cosθ/sinθ)(dθ/dλ)(dφ/dλ)
 *
 * INTEGRATION:
 *   4th-order Runge-Kutta with fixed step size dλ = 1e7 meters
 *
 * REFERENCES:
 *   - Misner, Thorne, Wheeler: "Gravitation" (1973)
 *   - Chandrasekhar: "Mathematical Theory of Black Holes" (1983)
 */
```

**Time**: 2 hours
**Priority**: 🟡 MEDIUM

---

## 🟡 Phase 2: Architectural Refactoring (Week 2-3)
**Goal**: Clean, maintainable, extensible codebase

### 2.1 Extract Classes from God Class
**Current**: `Engine` struct with 35+ members
**Target**: Single Responsibility Principle

**New Structure**:
```
src/
├── core/
│   ├── application.hpp/cpp        // Main orchestration
│   └── window.hpp/cpp             // GLFW wrapper
├── rendering/
│   ├── compute_renderer.hpp/cpp   // Geodesic compute shader
│   ├── tonemap_pass.hpp/cpp       // HDR → LDR
│   ├── grid_renderer.hpp/cpp      // Spacetime grid
│   └── shader_manager.hpp/cpp     // Shader compilation
├── physics/
│   ├── black_hole.hpp/cpp         // Black hole parameters
│   ├── object.hpp/cpp             // Celestial objects
│   └── gravity_sim.hpp/cpp        // N-body gravity
├── camera/
│   └── orbital_camera.hpp/cpp     // Camera controller
└── utils/
    ├── logger.hpp                 // Logging system
    └── gl_utils.hpp               // OpenGL helpers
```

**Example - ComputeRenderer**:
```cpp
class ComputeRenderer {
public:
    ComputeRenderer(int width, int height);
    ~ComputeRenderer();

    void setResolution(int width, int height);
    void dispatch(const Camera& camera, const std::vector<Object>& objects);
    GLuint getResultTexture() const { return hdrTexture; }

private:
    GLuint computeProgram;
    GLuint hdrTexture;
    GLuint cameraUBO, diskUBO, objectsUBO;
    int width, height;

    void uploadCameraUBO(const Camera& camera);
    void uploadObjectsUBO(const std::vector<Object>& objects);
    void reallocateTexture();
};
```

**Time**: 8-10 hours
**Priority**: 🟡 MEDIUM-HIGH

---

### 2.2 Consolidate Geodesic Implementations
**Problem**: 3 separate implementations (GPU, CPU, 2D)
**Solution**: Single source of truth

**Implementation**:
1. Create `src/physics/geodesic_equations.hpp`:
```cpp
namespace Geodesic {
    struct State {
        double r, theta, phi;
        double dr, dtheta, dphi;
        double E, L;  // Conserved quantities
    };

    // Compute derivatives (d²x/dλ²)
    inline void computeDerivatives(
        const State& state,
        double rs,
        double& d2r, double& d2theta, double& d2phi
    ) {
        double f = 1.0 - rs / state.r;
        double sinTheta = sin(state.theta);
        double cosTheta = cos(state.theta);

        // From Schwarzschild metric
        d2r = -(rs / (2.0 * state.r * state.r)) * f * (state.E / f) * (state.E / f)
            + (rs / (2.0 * state.r * state.r * f)) * state.dr * state.dr
            + state.r * (state.dtheta * state.dtheta + sinTheta * sinTheta * state.dphi * state.dphi);

        d2theta = -2.0 * state.dr * state.dtheta / state.r
                + sinTheta * cosTheta * state.dphi * state.dphi;

        d2phi = -2.0 * state.dr * state.dphi / state.r
              - 2.0 * cosTheta / sinTheta * state.dtheta * state.dphi;
    }

    // RK4 integration step (used by both CPU and GPU)
    inline State rk4Step(const State& state, double rs, double dLambda) {
        // ... RK4 implementation using computeDerivatives
    }
}
```

2. Generate GLSL version automatically from C++ header
3. Remove duplicate implementations

**Time**: 6-8 hours
**Priority**: 🟡 MEDIUM

---

### 2.3 Add Error Handling and Validation
**Current**: Minimal GL error checking
**Target**: Graceful degradation

**Implementation**:
```cpp
// utils/gl_utils.hpp
#define GL_CHECK(call) \
    do { \
        call; \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            Logger::error("OpenGL error ", err, " at ", __FILE__, ":", __LINE__); \
            throw std::runtime_error("OpenGL error"); \
        } \
    } while(0)

// Usage:
GL_CHECK(glDispatchCompute(groupsX, groupsY, 1));
```

**Time**: 2 hours
**Priority**: 🟡 MEDIUM

---

### 2.4 Remove Unused Files
**Files to Archive** (move to `archive/` directory):
- `ray_tracing.cpp` - Unused
- `CPU-geodesic.cpp` - Keep for validation only
- `2D_lensing.cpp` - Separate project

**Time**: 30 minutes
**Priority**: 🟢 LOW

---

## 🟢 Phase 3: User Experience (Week 4)
**Goal**: Delightful to use

### 3.1 Real-Time Performance Metrics
**Implementation**:
```cpp
class PerformanceMonitor {
    std::deque<double> frameTimes;
    double lastFPSUpdate = 0;
    double currentFPS = 0;

public:
    void recordFrame(double deltaTime) {
        frameTimes.push_back(deltaTime);
        if (frameTimes.size() > 60) frameTimes.pop_front();

        double now = glfwGetTime();
        if (now - lastFPSUpdate > 0.5) {  // Update every 500ms
            double avg = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0) / frameTimes.size();
            currentFPS = 1.0 / avg;
            lastFPSUpdate = now;
        }
    }

    double getFPS() const { return currentFPS; }
};
```

Display on screen using ImGui or simple overlay.

**Time**: 3 hours
**Priority**: 🟢 MEDIUM

---

### 3.2 Interactive Parameter Control
**Features**:
- Black hole mass slider
- Accretion disk inner/outer radius
- Camera speed controls
- Integration step size

**Implementation**: Use ImGui for immediate-mode UI

**Time**: 4-6 hours
**Priority**: 🟢 MEDIUM

---

### 3.3 Visual Debugging Modes
**Modes**:
1. **Normal**: Current rendering
2. **Integration Steps**: Show color-coded step count
3. **Photon Paths**: Draw actual ray trajectories
4. **Energy Conservation**: Show E/E₀ deviation

**Time**: 6-8 hours
**Priority**: 🟢 LOW

---

## 🔵 Phase 4: Innovation (Future)
**Goal**: State-of-the-art features

### 4.1 Kerr Metric (Rotating Black Holes)
- Implements rotating black hole (has angular momentum)
- Shows frame dragging
- Ergosphere visualization

**Time**: 20+ hours
**Priority**: 🔵 FUTURE

---

### 4.2 Bloom Post-Processing
- Realistic glow around accretion disk
- Separable Gaussian blur
- HDR bloom threshold

**Time**: 4-6 hours
**Priority**: 🔵 FUTURE

---

### 4.3 Ray Path Export
- Save photon trajectories to CSV
- Analysis in Python/MATLAB
- Scientific visualization

**Time**: 3-4 hours
**Priority**: 🔵 FUTURE

---

## Implementation Strategy

### Week 1: Critical Fixes
- Day 1-2: Fix RK4 integration
- Day 3: Remove console spam
- Day 4: Fix grid regeneration
- Day 5: Testing and validation

### Week 2: Start Refactoring
- Day 1-2: Extract ShaderManager
- Day 3-4: Extract ComputeRenderer
- Day 5: Extract GridRenderer

### Week 3: Complete Refactoring
- Day 1-2: Extract TonemapPass
- Day 3-4: Consolidate geodesic code
- Day 5: Testing and cleanup

### Week 4: Polish
- Day 1-2: Performance monitor
- Day 3-4: Interactive UI
- Day 5: Documentation

---

## Success Criteria

### Code Quality
- [ ] No code duplication (Single geodesic implementation)
- [ ] Each class < 200 lines
- [ ] All public methods documented
- [ ] No compiler warnings
- [ ] Energy conservation error < 1e-10

### Performance
- [ ] 60 FPS at 1080p
- [ ] Grid regenerates < 0.1% of frames
- [ ] Zero console output during normal operation

### User Experience
- [ ] Smooth camera controls
- [ ] Real-time FPS display
- [ ] Clear visual distinction of photon sphere

---

## The First Step

We start with **Phase 1.1: Fix RK4 Integration**.

This is the foundation. Get the physics right, and everything else follows.

Ready to begin?
