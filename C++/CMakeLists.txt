cmake_minimum_required(VERSION 3.21)
project(BlackHoleSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Common dependencies
set(DEPS glfw GLEW::GLEW glm::glm OpenGL::GL)

# 2D Lensing executable
add_executable(BlackHole2D 2D_lensing.cpp)
target_link_libraries(BlackHole2D PRIVATE ${DEPS})
target_include_directories(BlackHole2D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 3D Black Hole executable
add_executable(BlackHole3D black_hole.cpp)
target_link_libraries(BlackHole3D PRIVATE ${DEPS})
target_include_directories(BlackHole3D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Shader files (copy to output dir)
file(GLOB SHADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.comp"
)

foreach(shader ${SHADERS})
    add_custom_command(TARGET BlackHole3D POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${shader}
        $<TARGET_FILE_DIR:BlackHole3D>
    )
endforeach()


# Vulkan port (optional: only when the Vulkan headers/loader and glslc are found)
find_package(Vulkan COMPONENTS glslc)
if(Vulkan_FOUND AND Vulkan_GLSLC_EXECUTABLE)
    add_executable(BlackHoleVK black_hole_vk.cpp)
    target_link_libraries(BlackHoleVK PRIVATE glfw glm::glm Vulkan::Vulkan)
    target_include_directories(BlackHoleVK PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    # GLSL -> SPIR-V at build time; the .spv files are copied next to the executable like the GL shaders
    set(VK_SHADERS geodesic_vk.comp grid_vk.vert grid_vk.frag quad_vk.vert quad_vk.frag)
    set(VK_SPV "")
    foreach(src ${VK_SHADERS})
        set(spv ${CMAKE_BINARY_DIR}/shaders/${src}.spv)
        add_custom_command(OUTPUT ${spv}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} --target-env=vulkan1.3 ${CMAKE_CURRENT_SOURCE_DIR}/${src} -o ${spv}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src})
        list(APPEND VK_SPV ${spv})
    endforeach()
    add_custom_target(BlackHoleVKShaders DEPENDS ${VK_SPV})
    add_dependencies(BlackHoleVK BlackHoleVKShaders)
    foreach(spv ${VK_SPV})
        add_custom_command(TARGET BlackHoleVK POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${spv} $<TARGET_FILE_DIR:BlackHoleVK>)
    endforeach()
else()
    message(STATUS "Vulkan or glslc not found: BlackHoleVK target skipped")
endif()
