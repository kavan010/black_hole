cmake_minimum_required(VERSION 3.21)
project(BlackHoleSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(WIN32)
    # Windows: Enable console for debugging, can be changed to WIN32 for release
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
    add_definitions(-D_USE_MATH_DEFINES)
elseif(APPLE)
    # macOS: Set deployment target and framework paths
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
    set(CMAKE_MACOSX_RPATH ON)
elseif(UNIX)
    # Linux: Enable pthreads
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Dependencies
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# ImGui source files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ImPlot source files
set(IMPLOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/implot)
set(IMPLOT_SOURCES
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
)

# Common dependencies
set(DEPS glfw GLEW::GLEW glm::glm OpenGL::GL)

# Add platform-specific dependencies
if(UNIX AND NOT APPLE)
    list(APPEND DEPS Threads::Threads)
    list(APPEND DEPS ${CMAKE_DL_LIBS})
endif()

# 2D Lensing executable (archived but still buildable)
add_executable(BlackHole2D archive/2D_lensing.cpp)
target_link_libraries(BlackHole2D PRIVATE ${DEPS})
target_include_directories(BlackHole2D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 3D Black Hole executable
add_executable(BlackHole3D
    black_hole.cpp
    src/ui/gui_manager.cpp
    ${IMGUI_SOURCES}
    ${IMPLOT_SOURCES}
)
target_link_libraries(BlackHole3D PRIVATE ${DEPS})
target_include_directories(BlackHole3D PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMPLOT_DIR}
)

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

