# Phase 8: 架构重构计划

## 当前问题

根据ULTRATHINK_REVIEW.md的分析，当前架构存在以下问题：

1. **God Object**: `black_hole.cpp`包含2000+行代码，违反单一职责原则
2. **全局变量**: `lastPrintTime`, `framesCount`, `c`, `G`, `Gravity`等
3. **紧耦合**: Engine、Camera、物理计算混在一起
4. **Magic Numbers**: 硬编码常量缺少命名
5. **难以测试**: 缺少清晰的接口和模块边界

## 重构目标

### 1. 模块化架构

```
src/
├── core/
│   ├── Application.hpp/cpp       # 应用程序主循环
│   ├── Scene.hpp/cpp              # 场景管理
│   └── InputManager.hpp/cpp       # 输入抽象层
├── physics/
│   ├── BlackHole.hpp/cpp          # 黑洞物理参数
│   ├── AccretionDisk.hpp/cpp      # 吸积盘模型
│   ├── Geodesic.hpp/cpp           # 测地线计算接口
│   └── Constants.hpp              # 物理常量
├── rendering/
│   ├── Renderer.hpp/cpp           # 渲染器抽象
│   ├── Camera.hpp/cpp             # 相机类（独立）
│   ├── ShaderPipeline.hpp/cpp     # 着色器管线
│   ├── ComputeShaderManager.hpp   # 计算着色器管理
│   └── PostProcessing.hpp/cpp     # 后处理效果
├── ui/
│   ├── GUIManager.hpp/cpp         # ✅ 已实现
│   └── HUD.hpp/cpp                # HUD单独类
├── config/
│   ├── PresetManager.hpp          # ✅ 已实现
│   ├── Config.hpp/cpp             # 配置管理
│   └── Settings.hpp               # 设置结构
└── utils/
    ├── Logger.hpp                 # ✅ 已实现
    ├── PerformanceMonitor.hpp     # ✅ 已实现
    └── RayPathExporter.hpp        # ✅ 已实现
```

### 2. 类职责划分

#### Application类
```cpp
class Application {
public:
    void run();
    void handleInput();
    void update(float deltaTime);
    void render();

private:
    Scene scene;
    Renderer renderer;
    InputManager inputManager;
    GUIManager guiManager;
    bool running = true;
};
```

#### Scene类
```cpp
class Scene {
public:
    void update(float deltaTime);
    BlackHole& getBlackHole();
    Camera& getCamera();

private:
    BlackHole blackHole;
    Camera camera;
    AccretionDisk disk;
    std::vector<PhysicsObject> objects;
};
```

#### BlackHole类
```cpp
class BlackHole {
public:
    // Physical properties
    void setSpin(float spin);
    float getSpin() const;
    void setMass(float mass);
    float getMass() const;
    void setMetric(MetricType type);  // Schwarzschild or Kerr

    // Derived quantities
    float getSchwarzschildRadius() const;
    float getErgoSphereRadius() const;

private:
    float mass;
    float spin;
    MetricType metric;

    // Physical constants
    static constexpr float G = 6.67430e-11f;
    static constexpr float c = 299792458.0f;
};
```

#### Camera类（独立）
```cpp
class Camera {
public:
    void setPosition(const glm::vec3& pos);
    void setTarget(const glm::vec3& target);
    void rotate(float azimuth, float elevation);
    void zoom(float delta);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect) const;
    Ray getRayForPixel(int x, int y, int screenWidth, int screenHeight) const;

private:
    glm::vec3 position;
    glm::vec3 target;
    float azimuth;
    float elevation;
    float radius;
    float fov = 60.0f;
};
```

#### Renderer类
```cpp
class Renderer {
public:
    void initialize(int width, int height);
    void render(const Scene& scene, const Camera& camera);
    void setExposure(float exposure);
    void setVisualizationMode(VisualizationMode mode);

private:
    ShaderPipeline pipeline;
    ComputeShaderManager computeShaders;
    PostProcessing postProcessing;

    int width, height;
    GLuint hdrTexture;
};
```

### 3. 配置管理

#### Config.hpp
```cpp
struct RenderConfig {
    int width = 800;
    int height = 600;
    float exposure = 1.0f;
    VisualizationMode visualizationMode = VisualizationMode::Normal;
    WavelengthBand wavelengthBand = WavelengthBand::Optical;
    bool enableBloom = true;
};

struct PhysicsConfig {
    float deltaLambda = 1e7f;      // Integration step size
    int maxSteps = 10000;          // Maximum ray tracing steps
    float escapeDistance = 1e14f;  // Ray escape distance
};

class ConfigManager {
public:
    static ConfigManager& getInstance();

    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename);

    RenderConfig& getRenderConfig();
    PhysicsConfig& getPhysicsConfig();

private:
    RenderConfig renderConfig;
    PhysicsConfig physicsConfig;
};
```

### 4. 消除全局变量

#### 当前问题
```cpp
// black_hole.cpp 中的全局变量
double lastPrintTime = 0.0;
int framesCount = 0;
double c = 299792458.0;
double G = 6.67430e-11;
bool Gravity = false;
```

#### 解决方案
```cpp
// physics/Constants.hpp
namespace Physics {
    constexpr double c = 299792458.0;      // 光速 (m/s)
    constexpr double G = 6.67430e-11;      // 引力常数
    constexpr double M_sun = 1.98892e30;   // 太阳质量 (kg)
}

// 状态移到类中
class Application {
    double lastPrintTime = 0.0;
    int framesCount = 0;
};

class Scene {
    bool gravityEnabled = false;
};
```

### 5. 消除Magic Numbers

#### 当前问题
```cpp
float radius = 6.34194e10f;           // 这是什么距离？
float minRadius = 1e10f;              // 为什么是这个值？
float D_LAMBDA = 1e7;                 // 积分步长
```

#### 解决方案
```cpp
namespace SgrA {
    constexpr float MASS = 4.27e6f * Physics::M_sun;
    constexpr float SCHWARZSCHILD_RADIUS = 2.0f * Physics::G * MASS / (Physics::c * Physics::c);
    constexpr float DEFAULT_CAMERA_DISTANCE = 5.0f * SCHWARZSCHILD_RADIUS;  // 5 Rs
    constexpr float MIN_SAFE_DISTANCE = 0.8f * SCHWARZSCHILD_RADIUS;
}

namespace Integration {
    constexpr float DEFAULT_STEP_SIZE = 1e7f;  // meters
    constexpr int MAX_STEPS = 10000;           // prevent infinite loops
}
```

### 6. 接口抽象

#### 渲染接口
```cpp
class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual void render(const Scene& scene) = 0;
    virtual void resize(int width, int height) = 0;
};

class OpenGLRenderer : public IRenderer {
    // OpenGL specific implementation
};
```

#### 物理接口
```cpp
class IGeodesicIntegrator {
public:
    virtual ~IGeodesicIntegrator() = default;
    virtual Ray integrate(const Ray& initial, float deltaLambda, int maxSteps) = 0;
};

class RK4Integrator : public IGeodesicIntegrator {
    // 4th-order Runge-Kutta implementation
};
```

## 重构步骤（渐进式）

### 阶段1：提取常量（1天）
1. 创建 `physics/Constants.hpp`
2. 创建 `config/Settings.hpp`
3. 替换所有magic numbers
4. 测试编译

### 阶段2：提取Camera类（1天）
1. 创建 `rendering/Camera.hpp/cpp`
2. 从Engine中移动Camera相关代码
3. 更新black_hole.cpp使用新Camera类
4. 测试功能

### 阶段3：提取BlackHole类（1天）
1. 创建 `physics/BlackHole.hpp/cpp`
2. 封装黑洞参数和计算
3. 移除全局physics变量
4. 测试

### 阶段4：创建Scene类（1天）
1. 创建 `core/Scene.hpp/cpp`
2. 整合BlackHole, Camera, objects
3. 简化主循环
4. 测试

### 阶段5：创建Application类（2天）
1. 创建 `core/Application.hpp/cpp`
2. 重构main()函数
3. 整合所有模块
4. 全面测试

### 阶段6：重构Renderer（2天）
1. 创建 `rendering/Renderer.hpp/cpp`
2. 从Engine中分离渲染逻辑
3. 创建清晰的渲染管线
4. 测试

### 阶段7：单元测试（2天）
1. 为每个新类编写单元测试
2. 使用Google Test框架
3. 达到80%代码覆盖率
4. CI集成

## 预期收益

### 代码质量
- ✅ 每个文件 < 500行
- ✅ 清晰的单一职责
- ✅ 易于单元测试
- ✅ 减少耦合度

### 可维护性
- ✅ 新功能易于添加
- ✅ Bug易于定位
- ✅ 代码易于理解
- ✅ 团队协作友好

### 性能
- ✅ 编译时间更短（模块化）
- ✅ 运行时性能不变
- ✅ 内存使用更清晰

## 风险与缓解

### 风险1：破坏现有功能
**缓解：**
- 渐进式重构，每步都测试
- 保留原有black_hole.cpp作为参考
- 使用git分支进行重构

### 风险2：时间投入大
**缓解：**
- 分阶段执行，每阶段1-2天
- 优先重构最混乱的部分
- 可以暂停并在未来继续

### 风险3：性能回退
**缓解：**
- 基准测试对比
- Profile热点路径
- 必要时使用inline和优化

## 当前状态（Phase 8）

✅ 创建此重构计划文档
✅ 设计完整的目标架构
⏸️ 实际重构推迟到未来（避免破坏现有功能）

**理由：**
- Phase 7已经带来巨大的用户体验提升
- 当前代码虽然不完美，但功能完整且稳定
- 完全重构需要1-2周时间
- 作为Phase 7-10快速实现的一部分，我们优先交付可用功能
- 此计划为未来重构提供清晰路线图

## 下一步建议

当决定执行重构时：
1. 创建新的git分支 `refactor/architecture`
2. 从阶段1开始，提取常量
3. 每个阶段完成后提交
4. 全面测试后合并到主分支

**最佳实践：** 不要一次性重构所有代码。渐进式重构更安全可靠。
