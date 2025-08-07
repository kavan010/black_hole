# Set the minimum required version of CMake and define the project
cmake_minimum_required(VERSION 3.10)
project(BlackHoleSim)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the OpenGL policy to prefer modern GLVND
cmake_policy(SET CMP0072 NEW)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

# --- Define the main 3D simulation executable ---
add_executable(black_hole_3d src/black_hole.cpp)

# Link libraries to the 3D simulation
target_link_libraries(black_hole_3d PRIVATE
    GLEW::GLEW
    glfw
    OpenGL::GL
    glm::glm
)

# --- Define the 2D lensing executable ---
add_executable(black_hole_2d src/2D_lensing.cpp)
target_link_libraries(black_hole_2d PRIVATE
    GLEW::GLEW
    glfw
    OpenGL::GL
    glm::glm
)

# --- Define the CPU geodesic executable ---
add_executable(cpu_geodesic src/CPU-geodesic.cpp)
target_link_libraries(cpu_geodesic PRIVATE
    GLEW::GLEW
    glfw
    OpenGL::GL
    glm::glm
)

# --- Copy shader files to the build directory ---
# This ensures the executable can find them at runtime.
add_custom_command(
    TARGET black_hole_3d POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/geodesic.comp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/grid.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/grid.frag
        # The destination must be the LAST argument
        $<TARGET_FILE_DIR:black_hole_3d>
)

# --- Installation ---
# This section is optional but good practice.
install(TARGETS black_hole_3d black_hole_2d cpu_geodesic
        RUNTIME DESTINATION bin)

set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/geodesic.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/grid.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/grid.frag
)
install(FILES ${SHADER_FILES}
        DESTINATION bin)
