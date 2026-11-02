# Vulkan build of the black hole simulation (BlackHoleVK).
#
# Build (from the repository root):  docker build -t black-hole-vk C++
#   or, from inside the C++ folder:   docker build -t black-hole-vk .
#
# Run (needs a GPU and an X11/XWayland display; Wayland-only sessions work through XWayland):
#   X="--network host -e DISPLAY=$DISPLAY -e XAUTHORITY=/tmp/.xauth -v $XAUTHORITY:/tmp/.xauth:ro -v /tmp/.X11-unix:/tmp/.X11-unix"
#   # Intel / AMD (Mesa):
#   docker run --rm -it $X --device /dev/dri black-hole-vk
#   # NVIDIA (needs nvidia-container-toolkit; this is the variant tested on an RTX 3050):
#   docker run --rm -it $X --gpus all -e NVIDIA_DRIVER_CAPABILITIES=all black-hole-vk
# Using XAUTHORITY avoids loosening the X server with `xhost +local:docker`.
#
# Only the Vulkan target is built; the OpenGL targets need a GL context, which is not worth the trouble in a container.

# ---------- build stage ----------
FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
# GLEW/GL headers are only here because CMakeLists.txt does find_package(GLEW/OpenGL REQUIRED) for the GL targets.
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake \
        libglfw3-dev libglm-dev libglew-dev libgl-dev \
        libvulkan-dev glslc \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt sim.h black_hole_vk.cpp black_hole.cpp 2D_lensing.cpp ./
COPY geodesic_vk.comp grid_vk.vert grid_vk.frag quad_vk.vert quad_vk.frag ./
# black_hole.cpp / 2D_lensing.cpp are copied only so CMake can configure; they are not compiled (--target BlackHoleVK).
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target BlackHoleVK -j"$(nproc)"

# ---------- runtime stage ----------
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
# mesa-vulkan-drivers: Intel/AMD + lavapipe (CPU fallback). NVIDIA's driver is injected by nvidia-container-toolkit.
RUN apt-get update && apt-get install -y --no-install-recommends \
        libvulkan1 mesa-vulkan-drivers libglfw3 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
# The executable loads the .spv shaders from its working directory.
COPY --from=build /src/build/BlackHoleVK /src/build/*.spv ./
CMD ["./BlackHoleVK"]
