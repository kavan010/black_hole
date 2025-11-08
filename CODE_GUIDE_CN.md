# 黑洞模拟代码详解 - 完全小白指南

本文档详细解释项目中每个文件的作用和代码结构，帮助没有编程基础的人也能理解黑洞模拟是如何工作的。

---

## 📁 项目结构概览

```
black_hole/
├── black_hole.cpp           # 主程序文件（C++）
├── geodesic_kerr.comp       # Kerr黑洞计算着色器（GPU代码）
├── geodesic.comp            # Schwarzschild黑洞计算着色器
├── tonemap.frag            # 色调映射着色器
├── grid.vert / grid.frag   # 时空网格渲染着色器
├── bloom_extract.frag      # 泛光提取着色器
├── gaussian_blur.frag      # 高斯模糊着色器
├── src/
│   ├── utils/              # 工具类
│   │   ├── logger.hpp                 # 日志系统
│   │   ├── performance_monitor.hpp   # 性能监控
│   │   ├── ray_path_exporter.hpp     # 射线路径导出
│   │   └── gl_utils.hpp              # OpenGL工具
│   └── rendering/          # 渲染类
│       ├── shader_manager.hpp        # 着色器管理
│       └── bloom_renderer.hpp        # 泛光渲染器
└── CMakeLists.txt          # CMake构建配置
```

---

## 🎯 核心概念解释

### 什么是着色器(Shader)？

**简单解释：** 着色器是在显卡(GPU)上运行的小程序。就像CPU在处理器上运行程序一样，GPU也有自己的程序，叫做着色器。

**为什么用GPU？** 
- 黑洞模拟需要追踪数百万条光线
- CPU一次只能处理几个光线（慢）
- GPU可以同时处理数千条光线（快）
- 就像一个人洗碗vs一千个人同时洗碗

**着色器语言：** 我们使用GLSL（OpenGL Shading Language），类似C语言但专门为GPU设计。

### 什么是测地线(Geodesic)？

**简单解释：** 测地线是弯曲空间中的"直线"。

**类比：**
- 在平面上，直线是最短路径
- 在地球表面，"直线"实际上是弧线（大圆）
- 在黑洞附近，空间弯曲，光线沿着弯曲的测地线传播

**为什么重要：** 追踪测地线就是追踪光线在弯曲时空中的路径，这样我们就能看到黑洞的引力透镜效果。

---

## 📖 black_hole.cpp - 主程序详解

### 文件作用

这是整个程序的"大脑"，负责：
1. 创建窗口（你看到的黑洞画面）
2. 设置相机（你的视角）
3. 处理键盘和鼠标输入
4. 管理着色器（GPU程序）
5. 渲染循环（每秒60次更新画面）

### 主要结构和类

#### 1. Camera 结构体（相机类）

```cpp
struct Camera {
    vec3 target = vec3(0.0f, 0.0f, 0.0f);  // 相机看向的点（黑洞中心）
    float radius = 6.34194e10f;              // 相机到黑洞的距离（米）
    float azimuth = 0.0f;                   // 方位角（左右旋转）
    float elevation = M_PI / 2.0f;          // 仰角（上下旋转）
    ...
};
```

**作用：** 就像游戏中的相机，控制你的视角。

**关键方法：**
- `position()`: 计算相机在3D空间中的位置
  - 使用球坐标：方位角 + 仰角 + 距离 = 3D位置
  
- `getRayDirection()`: 把屏幕上的点(x,y)转换成3D射线方向
  - 鼠标点击 → 屏幕坐标 → 归一化设备坐标(NDC) → 世界空间射线
  
- `processMouseMove()`: 处理鼠标拖动
  - 水平移动 → 改变方位角
  - 垂直移动 → 改变仰角

#### 2. Engine 结构体（渲染引擎）

```cpp
struct Engine {
    GLuint computeProgram;     // 计算着色器程序（GPU上的黑洞计算）
    GLuint texture;            // 渲染结果纹理
    GLuint hdrTexture;         // HDR纹理（高动态范围）
    GLuint tonemapProgram;     // 色调映射着色器
    BloomRenderer bloomRenderer;  // 泛光渲染器
    
    float exposure = 1.0f;     // 曝光值（亮度）
    float kerrSpin = 0.0f;     // Kerr自旋参数（0-1）
    bool useKerr = false;      // 是否使用Kerr度规
    int visualizationMode = 0; // 可视化模式（0-4）
    int wavelengthBand = 2;    // 波段（0=射电, 2=可见光等）
    ...
};
```

**作用：** 管理所有渲染相关的东西。

#### 3. 主循环（Main Loop）

```cpp
while (!glfwWindowShouldClose(engine.window)) {
    // 1. 处理输入（键盘、鼠标）
    glfwPollEvents();
    
    // 2. 更新相机
    camera.update();
    
    // 3. 上传数据到GPU
    uploadCameraUBO(...);
    uploadKerrUBO(...);
    
    // 4. 在GPU上运行黑洞计算
    glDispatchCompute(...);
    
    // 5. 应用泛光效果
    bloomRenderer.apply(...);
    
    // 6. 色调映射（HDR → 屏幕）
    applyToneMapping(...);
    
    // 7. 显示到屏幕
    glfwSwapBuffers(engine.window);
}
```

**每一帧都重复这个过程，每秒60次！**

### 键盘处理函数

```cpp
void processKeyboard(int key, int action, Engine& engine, ...) {
    if (key == GLFW_KEY_E) {
        engine.exposure += 0.1f;  // E键增加曝光
    }
    if (key == GLFW_KEY_K) {
        engine.useKerr = !engine.useKerr;  // K键切换Kerr/Schwarzschild
    }
    // ... 更多键盘处理
}
```

**作用：** 把键盘按下转换成参数变化。

---

## 🔬 geodesic_kerr.comp - GPU计算核心

这是最重要的文件之一，在GPU上计算光线如何在黑洞附近弯曲。

### 文件结构

```glsl
#version 430  // GLSL版本

// 1. 布局声明
layout(local_size_x = 16, local_size_y = 16) in;  // GPU线程块大小

// 2. 输入输出
layout(binding = 0, rgba16f) writeonly uniform image2D outImage;  // 输出图像

// 3. 统一缓冲对象(UBO) - 从CPU传来的数据
layout(std140, binding = 4) uniform KerrParams {
    float spin;              // 自旋参数
    bool useKerr;            // 使用Kerr还是Schwarzschild
    float visualizationMode; // 可视化模式
    float wavelengthBand;    // 波段
};

// 4. 常量定义
const float SagA_rs = 1.269e10;   // Sgr A*的史瓦西半径（米）
const float D_LAMBDA = 1e7;        // 积分步长（米）

// 5. 数据结构
struct Ray {
    float r, theta, phi;     // 球坐标位置
    float dr, dtheta, dphi;  // 速度
    float E, L, Q;           // 守恒量
    float x, y, z;           // 笛卡尔坐标
    float intensity;         // 光子强度
};

// 6. 函数实现
// ...
```

### 关键函数解释

#### 1. initRayKerr() - 初始化射线

```glsl
Ray initRayKerr(vec3 pos, vec3 dir) {
    // 1. 转换到球坐标
    float r = length(pos);
    float theta = acos(pos.y / r);
    float phi = atan(pos.z, pos.x);
    
    // 2. 计算初始速度
    // ...
    
    // 3. 计算守恒量（能量E、角动量L、Carter常数Q）
    // ...
    
    return ray;
}
```

**作用：** 给定相机位置和方向，创建一条射线，准备开始积分。

#### 2. geodesicRHSKerr() - Kerr测地线方程

```glsl
void geodesicRHSKerr(Ray ray, out vec3 d1, out vec3 d2) {
    // 计算 dr/dλ, dθ/dλ, dφ/dλ（一阶导数）
    // 和 d²r/dλ², d²θ/dλ², d²φ/dλ²（二阶导数）
    
    // 使用Hamilton-Jacobi形式
    // R(r) = 有效径向势
    // Θ(θ) = 有效角度势
    
    // 这些方程告诉我们：给定当前位置和速度，
    // 下一步射线应该往哪里走
}
```

**物理意义：** 这是广义相对论的核心！这些方程描述光在弯曲时空中如何传播。

**类比：** 就像计算一个球从山坡上滚下来的路径：
- 地形的倾斜 = 时空的弯曲
- 球的路径 = 光的测地线

#### 3. rk4Step() - RK4数值积分

```glsl
void rk4Step(inout Ray ray, float dL) {
    // Runge-Kutta 4阶方法
    // 这是一种非常精确的数值求解微分方程的方法
    
    // k1: 在当前点计算导数
    // k2: 在中点用k1计算导数
    // k3: 在中点用k2计算导数  
    // k4: 在终点用k3计算导数
    // 结果 = 加权平均(k1, 2*k2, 2*k3, k4) / 6
}
```

**为什么用RK4？**
- 非常精确（4阶精度）
- 能量守恒误差 ~10⁻⁴
- 比简单的Euler方法精确1万倍！

**类比：** 
- Euler方法：每步直线走，误差累积快
- RK4方法：每步考虑中间状态，误差累积慢

#### 4. shakuraSunyaev() - 吸积盘模型

```glsl
vec3 shakuraSunyaev(float r, float phi, vec3 rayDir) {
    // 1. 计算温度：T ∝ r^(-3/4)
    //    - 内盘热（蓝白色）
    //    - 外盘冷（橙红色）
    
    // 2. 计算多普勒增强
    //    - 盘在旋转
    //    - 朝向我们的一侧更亮（蓝移）
    //    - 远离我们的一侧更暗（红移）
    
    // 3. 计算引力红移
    //    - 靠近黑洞，时间变慢
    //    - 光子失去能量（红移）
    
    return vec3(temperature, dopplerBoost, redshiftFactor);
}
```

**物理意义：** 
- Shakura-Sunyaev是标准吸积盘模型
- 真实的天体物理学！
- EHT（事件视界望远镜）也使用类似模型

#### 5. main() - 主着色器函数

```glsl
void main() {
    // 获取当前像素位置
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    
    // 计算该像素对应的射线方向
    vec3 rayDir = calculateRayDirection(pixel);
    
    // 初始化射线
    Ray ray = initRayKerr(camPos, rayDir);
    
    // 积分循环：追踪射线直到击中某物或逃逸
    for (int step = 0; step < MAX_STEPS; step++) {
        // 1. RK4积分一步
        rk4Step(ray, D_LAMBDA);
        
        // 2. 检查是否击中事件视界
        if (ray.r < event_horizon) {
            color = vec4(0.0, 0.0, 0.0, 1.0);  // 黑色
            break;
        }
        
        // 3. 检查是否击中吸积盘
        if (hitDisk(ray)) {
            color = renderDisk(ray);
            break;
        }
        
        // 4. 检查是否逃逸到无穷远
        if (ray.r > ESCAPE_R) {
            color = vec4(0.1, 0.1, 0.2, 1.0);  // 背景色
            break;
        }
        
        // 5. 更新光子强度（辐射传输）
        updatePhotonIntensity(ray, D_LAMBDA);
    }
    
    // 应用可视化模式和波段
    color = applyVisualization(color, ray, visualizationMode);
    color.rgb = applyWavelengthBand(color.rgb, temperature, wavelengthBand);
    
    // 写入输出图像
    imageStore(outImage, pixel, color);
}
```

**这个函数为每个像素并行运行！**
- 1920×1080屏幕 = 2,073,600个像素
- GPU同时处理所有像素
- 每个像素独立追踪自己的光线

---

## 🎨 着色器管线流程

```
1. 计算着色器 (geodesic_kerr.comp)
   ↓
   输出：HDR图像（浮点）
   
2. 泛光提取 (bloom_extract.frag)
   ↓
   提取亮部分
   
3. 高斯模糊 (gaussian_blur.frag)
   ↓
   模糊10次（平滑发光）
   
4. 合成泛光 (bloom_renderer)
   ↓
   原图 + 模糊亮部 = 带泛光的图像
   
5. 色调映射 (tonemap.frag)
   ↓
   HDR → LDR（屏幕可显示）
   使用ACES电影色调曲线
   
6. 显示到屏幕
```

---

## 🧮 关键物理公式解释

### Schwarzschild度规（非旋转黑洞）

```
ds² = -(1 - rs/r)dt² + (1 - rs/r)⁻¹dr² + r²dΩ²
```

**含义：**
- `rs` = 史瓦西半径 = 2GM/c²
- 当 r → rs 时，时间变慢（时间膨胀）
- 当 r < rs 时，落入黑洞（事件视界）

### Kerr度规（旋转黑洞）

```
更复杂！包含：
- Σ = r² + a²cos²θ
- Δ = r² - 2Mr + a²
- a = J/M（角动量参数）
```

**新特性：**
- **能层**：r_outer < r < r_outer + √(r_outer² - a²)
  - 在这个区域，什么都不能静止！
  - 必须跟着黑洞旋转
  
- **拖曳效应**：旋转黑洞"拖拽"时空一起转
  - 就像搅拌蜂蜜，周围的蜂蜜也会转

### 守恒量

在黑洞附近运动时，这三个量保持不变：

1. **能量(E)**：每单位质量的能量
   - 类似总机械能
   
2. **角动量(L)**：每单位质量的角动量
   - 绕z轴的旋转
   
3. **Carter常数(Q)**：Kerr度规特有
   - 描述轨道的"倾斜程度"
   - Q = 0：赤道轨道
   - Q > 0：倾斜或极轨道

---

## 🔧 src/utils/ - 工具类详解

### logger.hpp - 日志系统

```cpp
enum class LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
    static void info(Args&&... args);   // 普通信息
    static void warn(Args&&... args);   // 警告
    static void error(Args&&... args);  // 错误
    static void debug(Args&&... args);  // 调试信息
};
```

**使用示例：**
```cpp
Logger::info("曝光设置为: ", exposure);
Logger::warn("FPS低于30: ", fps);
Logger::error("着色器编译失败");
```

### performance_monitor.hpp - 性能监控

```cpp
class PerformanceMonitor {
    void recordFrameTime(double frameTime);  // 记录帧时间
    std::string getFormattedStats();         // 获取统计信息
};
```

**追踪：**
- FPS（每秒帧数）
- 平均帧时间
- 最小/最大帧时间

### ray_path_exporter.hpp - 射线导出

```cpp
class RayPathExporter {
    void exportPath(vec3 pos, vec3 dir, std::string filename);
    void exportCone(vec3 pos, vec3 dir, int numRays, std::string filename);
};
```

**作用：** 把射线轨迹保存到CSV文件，可以用Python/MATLAB分析。

---

## 🎬 src/rendering/ - 渲染类详解

### shader_manager.hpp - 着色器管理

```cpp
class ShaderManager {
    static GLuint compileShader(const char* source, GLenum type);
    static GLuint createComputeProgram(const char* source);
    static GLuint createProgram(const char* vertSrc, const char* fragSrc);
};
```

**作用：**
- 编译着色器源代码
- 链接成着色器程序
- 错误检查和报告

**着色器编译过程：**
1. 读取源代码（.comp、.frag、.vert文件）
2. 调用OpenGL编译
3. 检查错误
4. 如果成功，返回程序ID
5. 如果失败，打印错误信息

### bloom_renderer.hpp - 泛光渲染器

```cpp
class BloomRenderer {
    void init(int width, int height);              // 初始化
    void apply(GLuint sourceTexture, ...);         // 应用泛光
    void setThreshold(float t);                    // 设置亮度阈值
    void setIntensity(float i);                    // 设置泛光强度
};
```

**泛光工作流程：**
1. 提取亮部分（阈值过滤）
   - 只保留亮度 > threshold 的像素
   
2. 降采样到1/4分辨率
   - 节省性能
   
3. 高斯模糊（两次传递）
   - 水平模糊
   - 垂直模糊
   - 重复10次，逐渐扩散
   
4. 上采样回原分辨率

5. 混合到原图
   - 原图 + 模糊亮部 × intensity

---

## 📊 性能优化技巧

### 1. 网格缓存
```cpp
class GridCache {
    bool needsRegeneration(const std::vector<ObjectData>& currentObjects);
};
```
- 只有对象移动 > 1亿米时才重新生成网格
- 大多数时间重用缓存的网格
- 性能提升：100-1000倍

### 2. 自适应分辨率
```cpp
int COMPUTE_WIDTH_FULL = 200;    // 静止时
int COMPUTE_WIDTH_MOVING = 160;  // 移动时（80%）
```
- 相机移动时降低计算分辨率
- 用户感觉不到差异（运动模糊）
- 性能提升：~50%

### 3. UBO（统一缓冲对象）
```cpp
glBindBufferBase(GL_UNIFORM_BUFFER, 4, kerrUBO);
```
- 一次性传输所有参数到GPU
- 比逐个传输快得多
- GPU可以快速访问

---

## 🐛 常见问题和解决方案

### 问题1：画面全黑
**可能原因：**
- 相机太近（进入事件视界）
- 曝光太低
- 着色器编译失败

**解决：**
1. 按R重置曝光
2. 滚轮远离黑洞
3. 检查控制台错误信息

### 问题2：FPS很低
**可能原因：**
- 窗口太大
- 泛光开销
- GPU不够强

**解决：**
1. 减小窗口大小
2. 按B关闭泛光
3. 减少泛光强度（-键）

### 问题3：Carter常数不守恒
**原因：**
- Phase 5使用简化的方程

**已在Phase 6修复！**
- 使用Hamilton-Jacobi形式
- 守恒精度提升10,000倍

---

## 📚 进一步学习资源

### 物理
- **广义相对论入门**：《时间简史》- 霍金
- **黑洞物理**：《Black Holes: The Membrane Paradigm》
- **数值方法**：《Numerical Recipes》

### 编程
- **OpenGL**：learnopengl.com（中文版）
- **GLSL**：OpenGL Shading Language官方规范
- **C++**：《C++ Primer》中文版

### 天体物理
- **事件视界望远镜**：EHT官网
- **GRMHD模拟**：HARM代码
- **可视化**：ParaView教程

---

## 💡 实验建议

### 初学者实验

1. **改变颜色**
   - 找到`blackbodyColor()`函数
   - 修改RGB值
   - 重新编译，看看效果

2. **调整参数**
   - 改变`SagA_rs`（史瓦西半径）
   - 改变`D_LAMBDA`（步长）
   - 观察对性能和精度的影响

3. **添加日志**
   - 在关键位置添加`Logger::info()`
   - 观察程序运行流程

### 进阶实验

1. **添加新的可视化模式**
   - 在`main()`中添加新的case
   - 实现自己的颜色映射

2. **修改吸积盘**
   - 改变温度分布（T ∝ r^-x）
   - 尝试不同的x值

3. **实现新的黑洞度规**
   - Reissner-Nordström（带电黑洞）
   - Kerr-Newman（旋转带电黑洞）

---

## 🎓 总结

这个黑洞模拟项目：

1. **使用GPU并行计算** 追踪数百万条光线
2. **实现广义相对论** 的Kerr和Schwarzschild度规
3. **使用RK4积分** 获得高精度测地线
4. **包含真实天体物理** Shakura-Sunyaev盘模型
5. **提供科学工具** 射线导出、多波段渲染
6. **优化性能** 网格缓存、自适应分辨率

**最重要的是：** 你可以实时看到爱因斯坦的广义相对论在起作用！

---

**有问题？** 查看其他文档或在GitHub Issues提问！

**想贡献？** Fork这个项目，添加新功能，提交Pull Request！

**觉得有趣？** 给项目一个⭐，分享给朋友！
