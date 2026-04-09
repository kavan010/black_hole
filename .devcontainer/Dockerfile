FROM mcr.microsoft.com/devcontainers/base:ubuntu

# Install Ubuntu packages
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get install -y \
        build-essential=12.10ubuntu1 \
        git=1:2.43.0-1ubuntu7.3 \
        curl=8.5.0-2ubuntu10.6 \
        pkg-config=1.8.1-2build1 \
        unzip=6.0-28ubuntu4.1 \
        make=4.3-4.1build2 \
        wget=1.21.4-1ubuntu4.1 \
        # --- OpenGL + X11 dependencies for GLEW + GLFW ---
        libx11-dev=2:1.8.7-1build1 \
        libxrandr-dev=2:1.5.2-2build1 \
        libxinerama-dev=2:1.1.4-3build1 \
        libxcursor-dev=1:1.2.1-1build1 \
        libxi-dev=2:1.8.1-1build1 \
        xorg-dev=1:7.7+23ubuntu3 \
        libgl1-mesa-dev=25.0.7-0ubuntu0.24.04.2 \
        libglu1-mesa-dev=9.0.2-1.1build1 \
    && apt-get clean

# Install CMake 4.0.2
RUN mkdir -p /usr/local \
    && cd /usr/local \
    && wget https://github.com/Kitware/CMake/releases/download/v4.0.2/cmake-4.0.2-linux-x86_64.sh \
    && chmod +x cmake-4.0.2-linux-x86_64.sh \
    && ./cmake-4.0.2-linux-x86_64.sh --skip-license

# Switch to vscode temporarily for installing vcpkg
USER vscode

RUN git clone https://github.com/microsoft/vcpkg.git /home/vscode/vcpkg \
    && /home/vscode/vcpkg/bootstrap-vcpkg.sh -disableMetrics

ENV VCPKG_ROOT=/home/vscode/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"
