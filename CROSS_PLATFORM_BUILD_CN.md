# 黑洞模拟 - 跨平台构建完整指南

本文档详细说明如何在Windows、Linux和macOS三个平台上编译和运行黑洞模拟程序。

---

## 📋 目录

- [系统要求](#系统要求)
- [Windows构建指南](#windows构建指南)
- [Linux构建指南](#linux构建指南)
- [macOS构建指南](#macos构建指南)
- [常见问题](#常见问题)
- [性能优化](#性能优化)

---

## 系统要求

### 最低要求

| 组件 | 要求 |
|------|------|
| **操作系统** | Windows 10+, Linux (Ubuntu 20.04+), macOS 10.15+ |
| **CPU** | 双核处理器 |
| **RAM** | 4 GB |
| **GPU** | 支持OpenGL 4.3+ |
| **存储** | 500 MB |

### 推荐配置

| 组件 | 推荐 |
|------|------|
| **CPU** | 四核及以上 |
| **RAM** | 8 GB+ |
| **GPU** | 独立显卡 (NVIDIA/AMD/Intel Iris) |
| **存储** | SSD |

---

## Windows构建指南

### 方法1：使用Visual Studio（推荐）

#### 1. 安装必要软件

**Visual Studio 2022：**
- 下载：https://visualstudio.microsoft.com/
- 安装时选择"使用C++的桌面开发"工作负载
- 包括CMake工具

**CMake：**
- 下载：https://cmake.org/download/
- 或通过Visual Studio安装程序安装

**vcpkg（依赖管理）：**
```powershell
# 克隆vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# 引导vcpkg
.\bootstrap-vcpkg.bat

# 集成到系统
.\vcpkg integrate install
```

#### 2. 安装依赖

```powershell
# 使用vcpkg安装依赖
cd C:\path\to\black_hole
vcpkg install glew:x64-windows glfw3:x64-windows glm:x64-windows
```

#### 3. 构建项目

**选项A：使用提供的脚本**
```batch
# 双击运行
build_windows.bat

# 或在命令提示符中
.\build_windows.bat
```

**选项B：手动构建**
```powershell
# 配置
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg路径]/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 17 2022" -A x64

# 编译
cmake --build build --config Release

# 运行
.\build\Release\BlackHole3D.exe
```

#### 4. 在Visual Studio中打开

```powershell
# 生成解决方案
cmake -B build -S . -G "Visual Studio 17 2022"

# 打开.sln文件
start build\BlackHoleSim.sln
```

在Visual Studio中：
1. 设置BlackHole3D为启动项目
2. 选择Release配置
3. 按F5运行

### 方法2：使用MinGW

#### 1. 安装MSYS2

下载并安装：https://www.msys2.org/

#### 2. 安装依赖

```bash
# 在MSYS2终端中
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-glm
```

#### 3. 构建

```bash
cmake -B build -S . -G "MinGW Makefiles"
cmake --build build
./build/BlackHole3D.exe
```

### Windows常见问题

**问题1：找不到OpenGL**
```
解决：更新显卡驱动
NVIDIA：https://www.nvidia.com/drivers
AMD：https://www.amd.com/support
Intel：https://www.intel.com/content/www/us/en/download-center
```

**问题2：DLL缺失**
```
解决：确保vcpkg依赖正确安装
或复制DLL到可执行文件目录
```

**问题3：CMake找不到编译器**
```
解决：在安装Visual Studio时选择C++工具
或安装MinGW并添加到PATH
```

---

## Linux构建指南

### 支持的发行版

- Ubuntu 20.04+
- Debian 11+
- Fedora 34+
- Arch Linux
- 其他使用OpenGL 4.3+驱动的发行版

### Ubuntu/Debian

#### 1. 安装依赖

```bash
# 更新包列表
sudo apt update

# 安装编译工具
sudo apt install build-essential cmake

# 安装OpenGL和图形库
sudo apt install libglew-dev libglfw3-dev libglm-dev libgl1-mesa-dev

# 可选：安装显卡驱动
# NVIDIA
sudo ubuntu-drivers autoinstall

# AMD
sudo apt install mesa-vulkan-drivers
```

#### 2. 构建项目

**选项A：使用提供的脚本**
```bash
# 添加执行权限
chmod +x build_linux.sh

# 运行脚本
./build_linux.sh

# 运行程序
./build/BlackHole3D
```

**选项B：手动构建**
```bash
# 克隆项目（如果还没有）
git clone https://github.com/kavan010/black_hole.git
cd black_hole

# 配置
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# 编译（使用所有CPU核心）
cmake --build build -- -j$(nproc)

# 运行
./build/BlackHole3D
```

### Fedora

```bash
# 安装依赖
sudo dnf install gcc-c++ cmake
sudo dnf install glew-devel glfw-devel glm-devel mesa-libGL-devel

# 构建（同Ubuntu）
chmod +x build_linux.sh
./build_linux.sh
```

### Arch Linux

```bash
# 安装依赖
sudo pacman -S base-devel cmake
sudo pacman -S glew glfw-x11 glm mesa

# 构建（同Ubuntu）
chmod +x build_linux.sh
./build_linux.sh
```

### Linux常见问题

**问题1：OpenGL版本不支持**
```bash
# 检查OpenGL版本
glxinfo | grep "OpenGL version"

# 如果版本 < 4.3，需要更新驱动
# NVIDIA
sudo add-apt-repository ppa:graphics-drivers/ppa
sudo apt update
sudo apt install nvidia-driver-525

# AMD开源驱动应该已足够
sudo apt install mesa-utils
```

**问题2：libGL找不到**
```bash
# 安装mesa
sudo apt install libgl1-mesa-glx libgl1-mesa-dri

# 或重新安装OpenGL
sudo apt install --reinstall libgl1-mesa-dev
```

**问题3：权限问题**
```bash
# 确保用户在video组
sudo usermod -aG video $USER

# 重新登录或
newgrp video
```

**问题4：Wayland问题**
```bash
# 如果使用Wayland，切换到X11
# 在登录界面选择"Ubuntu on Xorg"
# 或设置环境变量
export GDK_BACKEND=x11
./build/BlackHole3D
```

---

## macOS构建指南

### 支持的版本

- macOS 10.15 (Catalina) 或更新
- macOS 11 (Big Sur)
- macOS 12 (Monterey)
- macOS 13 (Ventura)
- macOS 14 (Sonoma)

### 1. 安装Xcode Command Line Tools

```bash
xcode-select --install
```

### 2. 安装Homebrew

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. 安装依赖

```bash
# 安装CMake和依赖
brew install cmake glew glfw glm

# 或使用vcpkg（跨平台方案）
brew install vcpkg
vcpkg install glew glfw3 glm
```

### 4. 构建项目

**选项A：使用提供的脚本**
```bash
# 添加执行权限
chmod +x build_macos.sh

# 运行脚本
./build_macos.sh

# 运行程序
./build/BlackHole3D
```

**选项B：手动构建**
```bash
# 配置
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# 编译（使用所有CPU核心）
cmake --build build -- -j$(sysctl -n hw.ncpu)

# 运行
./build/BlackHole3D
```

### macOS特殊说明

#### M1/M2 Mac（Apple Silicon）

```bash
# 确保使用arm64架构
arch -arm64 brew install glew glfw glm

# 构建时指定架构
cmake -B build -S . -DCMAKE_OSX_ARCHITECTURES=arm64

# 或构建通用二进制（x86_64和arm64）
cmake -B build -S . -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
```

#### Intel Mac

```bash
# 正常构建即可
./build_macos.sh
```

### macOS常见问题

**问题1：安全警告**
```
现象：macOS阻止运行"未经验证的开发者"
解决：
1. 打开"系统偏好设置" > "安全性与隐私"
2. 在"通用"标签下，点击"仍要打开"
3. 或使用命令：
   xattr -d com.apple.quarantine ./build/BlackHole3D
```

**问题2：OpenGL已弃用警告**
```
现象：macOS 10.14+显示OpenGL弃用警告
说明：这是正常的，程序仍能运行
未来：可能需要迁移到Metal API
```

**问题3：找不到OpenGL**
```bash
# 确保系统OpenGL框架存在
ls /System/Library/Frameworks/OpenGL.framework

# 如果缺失，重新安装Xcode Command Line Tools
sudo rm -rf /Library/Developer/CommandLineTools
xcode-select --install
```

**问题4：Homebrew路径问题**
```bash
# M1/M2 Mac的Homebrew在/opt/homebrew
# Intel Mac的Homebrew在/usr/local

# 添加到PATH（M1/M2）
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zshrc
source ~/.zshrc

# 或手动指定库路径
export CMAKE_PREFIX_PATH=/opt/homebrew
```

---

## 常见问题（所有平台）

### 编译问题

**Q: CMake版本太旧**
```
错误：CMake 3.21 or higher is required
解决：
- Windows: 下载最新CMake安装程序
- Linux: sudo apt install cmake （或从源码编译）
- macOS: brew upgrade cmake
```

**Q: 找不到OpenGL 4.3支持**
```
解决：
1. 更新显卡驱动到最新版本
2. 检查GPU是否支持OpenGL 4.3+
3. 在集成显卡上可能需要降低质量
```

**Q: 链接错误**
```
解决：
1. 确保所有依赖都正确安装
2. 清理构建：rm -rf build && mkdir build
3. 重新配置和构建
```

### 运行问题

**Q: 黑屏或崩溃**
```
原因：
- OpenGL驱动问题
- GPU不支持必需的功能

解决：
1. 更新显卡驱动
2. 检查控制台错误信息
3. 尝试降低窗口分辨率
```

**Q: 性能很慢（<10 FPS）**
```
解决：
1. 确保使用独立显卡（不是集成显卡）
2. 关闭其他占用GPU的程序
3. 降低窗口大小
4. 关闭泛光效果（按B键）
```

**Q: 着色器编译错误**
```
解决：
1. 确保着色器文件在可执行文件旁边
2. 检查OpenGL版本是否 >= 4.3
3. 查看控制台的详细错误信息
```

---

## 性能优化

### Windows优化

```
1. 在NVIDIA控制面板中：
   - 设置为"高性能"模式
   - 禁用垂直同步（如果想要>60 FPS）

2. 在Windows设置中：
   - 设置为"高性能"电源计划
   - 关闭游戏栏和录制功能

3. 编译优化：
   - 使用Release配置（不是Debug）
   - 启用编译器优化
```

### Linux优化

```bash
# 使用高性能GPU配置
# NVIDIA
nvidia-settings -a "[gpu:0]/GPUPowerMizerMode=1"

# AMD
echo "performance" | sudo tee /sys/class/drm/card0/device/power_dpm_force_performance_level

# Intel
sudo cpupower frequency-set -g performance

# 禁用合成器（临时）
killall picom  # 或你使用的合成器
```

### macOS优化

```
1. 在"节能"设置中：
   - 禁用自动切换显卡
   - 选择"高性能"

2. 关闭后台程序

3. 使用活动监视器：
   - 确认使用独立GPU
   - 检查CPU/GPU使用率
```

---

## 验证安装

在所有平台上，构建成功后运行：

```bash
# 显示版本信息（如果实现）
./build/BlackHole3D --version

# 运行程序
./build/BlackHole3D
```

**预期结果：**
- 窗口打开显示黑洞
- 控制台输出性能信息
- FPS > 30（推荐配置）
- 鼠标和键盘响应

---

## 开发者信息

### 调试构建

```bash
# 构建Debug版本
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# 使用调试器
# GDB (Linux/macOS)
gdb ./build/BlackHole3D

# LLDB (macOS)
lldb ./build/BlackHole3D

# Visual Studio (Windows)
# 在VS中打开项目，设置断点，按F5
```

### 性能分析

```bash
# Linux - perf
perf record ./build/BlackHole3D
perf report

# macOS - Instruments
instruments -t "Time Profiler" ./build/BlackHole3D

# Windows - Visual Studio Profiler
# 在VS中：调试 > 性能探查器
```

---

## 获取帮助

如果遇到问题：

1. **检查文档：**
   - README_CN.md - 项目概览
   - QUICKSTART_CN.md - 快速开始
   - CONTROLS_CN.md - 控制说明

2. **查看日志：**
   - 程序会输出详细的错误信息
   - 截图发送到Issues

3. **GitHub Issues：**
   - https://github.com/kavan010/black_hole/issues
   - 提供：系统信息、错误日志、复现步骤

4. **社区：**
   - 在Issues中搜索类似问题
   - 参与讨论

---

## 总结

| 平台 | 难度 | 推荐方法 | 预期时间 |
|------|------|----------|----------|
| **Windows** | 中等 | Visual Studio + vcpkg | 30-60分钟 |
| **Linux** | 简单 | apt + 脚本 | 10-20分钟 |
| **macOS** | 中等 | Homebrew + 脚本 | 20-40分钟 |

**最简单的方法：**
- Windows: 运行 `build_windows.bat`
- Linux: 运行 `./build_linux.sh`
- macOS: 运行 `./build_macos.sh`

**祝你构建成功！** 🚀
