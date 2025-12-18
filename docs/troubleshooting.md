# Troubleshooting

## Common Issues

### 1. "Failed to create GLFW window" or OpenGL Errors
- **Cause**: Your GPU drivers might be outdated or do not support OpenGL 4.3.
- **Fix**: Update your graphics drivers. Ensure your GPU supports Compute Shaders.

### 2. CMake "Could not find GLEW/GLFW"
- **Cause**: Dependencies are not installed or CMake cannot find them.
- **Fix**: Use `vcpkg` and ensure you pass the toolchain file correctly:
  `-DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake`

### 3. Black Screen / No Visuals
- **Cause**: Compute shader might not be running or writing to the texture.
- **Fix**: Check the console output for shader compilation errors.

### 4. Application Crashes Immediately
- **Cause**: Often due to missing assets (shaders) in the working directory.
- **Fix**: Run the executable from the project root or ensure `geodesic.comp`, `grid.vert`, etc., are in the same directory as the executable.

## Still Stuck?
Open an issue on GitHub with your error logs.
