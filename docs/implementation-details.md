# Implementation Details

## Ray Marching Strategy

Instead of rasterizing geometry, we use **Ray Marching** via a Compute Shader.

1.  **ray generation**: For each pixel on the screen, generate a camera ray in world space.
2.  **Initialization**: Calculate the initial position $(r, \theta, \phi)$ and 4-velocity.
3.  **Step Loop**:
    - Check for intersection with the **Event Horizon** ($r < r_s$). If hit -> Black.
    - Check for intersection with the **Accretion Disk** (plane crossing test).
    - Perform an **RK4 Step** to advance the ray along the geodesic.
    - Check for intersection with **Scene Objects** (stars/planets).
4.  **Coloring**: Accumulate color based on hits (Disk color, Object color, or Background).

## Coordinate System
We use standard spherical coordinates for the physics calculations but map them back to Cartesian for rendering and interaction.

```mermaid
graph TD
    A[Pixel Coordinate] -->|Inverse Projection| B[Camera Ray Direction]
    B -->|Convert to Spherical| C[Initial State (r, θ, φ, dr, dθ, dφ)]
    C -->|RK4 Integration Loop| D[Next State]
    D -->|Check Intersections| E{Hit?}
    E -->|Yes: Event Horizon| F[Return Black]
    E -->|Yes: Disk| G[Return Disk Color]
    E -->|No| C
```

## Acceleration Structures
Accessing scene objects for every step of the ray march is expensive.
- **Uniform Buffer Objects (UBOs)**: We store scene data (Spheres, Disk params, Camera state) in UBOs for fast access by the shader.
- **Bounding Volumes**: Simple sphere-ray intersection checks are done before integrating complex object interactions (though currently all objects are spheres).

## The Compute Shader (`geodesic.comp`)
The core logic resides here.
- `local_size_x = 16, local_size_y = 16`: Thread group size.
- `imageStore`: Writes the final color to a texture.
- `binding = 0`: output image.
- `binding = 1`: Camera UBO.
