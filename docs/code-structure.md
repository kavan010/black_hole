# Code Structure

## Directory Layout

```mermaid
graph TD
    Root[Project Root]
    Root --> Src[Src Files]
    Root --> Docs[docs/]
    Src --> Main[black_hole.cpp]
    Src --> Demo[2D_lensing.cpp]
    Src --> Shader[geodesic.comp]
    Src --> Vertex[grid.vert]
    Src --> Frag[grid.frag]
    Docs --> Index[index.md]
    Docs --> ...
```

## Key Files

| File | Description |
| :--- | :--- |
| `black_hole.cpp` | **Main Entry Point**. Sets up OpenGL window, Camera, and dispatches compute shaders. |
| `geodesic.comp` | **Compute Shader**. Performs the relativistic ray tracing on the GPU. |
| `2D_lensing.cpp` | **2D Demo**. Standalone CPU-based visualizer for light paths. |
| `grid.vert/frag` | **Visualization**. Shaders for rendering the warped spacetime grid. |

## Class/Struct Overview (`black_hole.cpp`)

```mermaid
classDiagram
    class Engine {
        +Init()
        +Run()
        +dispatchCompute()
        +renderScene()
    }
    class Camera {
        +vec3 position
        +vec3 target
        +processInput()
        +update()
    }
    class BlackHole {
        +double mass
        +double radius
        +double r_s
    }
    class ObjectData {
        +vec4 posRadius
        +vec4 color
        +float mass
    }
    
    Engine --> Camera : uses
    Engine --> BlackHole : contains
    Engine --> ObjectData : manages list of
```

## Data Flow
1. **CPU**: `Engine` updates `Camera` and `Object` positions.
2. **Transfer**: Data is uploaded to GPU `UBOs`.
3. **GPU**: `geodesic.comp` reads UBOs, traces rays, writes to `Texture`.
4. **Display**: `Engine` renders a full-screen quad using the generated `Texture`.
