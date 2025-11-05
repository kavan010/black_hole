#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP

#include <GLFW/glfw3.h>
#include <string>
#include "../config/preset_manager.hpp"

// GUI数据结构 - 简化的接口，避免循环依赖
struct GUIState {
    // Black hole parameters
    float* kerrSpin = nullptr;
    bool* useKerr = nullptr;

    // Rendering parameters
    float* exposure = nullptr;
    int* visualizationMode = nullptr;
    int* wavelengthBand = nullptr;

    // Camera parameters
    float* cameraRadius = nullptr;
    float* cameraAzimuth = nullptr;
    float* cameraElevation = nullptr;

    // Window size
    int* windowWidth = nullptr;
    int* windowHeight = nullptr;
};

/**
 * @brief GUI管理器 - 使用ImGui创建用户界面
 *
 * 这个类负责：
 * - ImGui的初始化和清理
 * - 渲染所有GUI面板
 * - 处理用户交互
 */
class GUIManager {
public:
    GUIManager();
    ~GUIManager();

    /**
     * @brief 初始化ImGui
     * @param window GLFW窗口指针
     * @param glsl_version OpenGL着色器语言版本字符串（如 "#version 430"）
     */
    void initialize(GLFWwindow* window, const char* glsl_version);

    /**
     * @brief 清理ImGui资源
     */
    void shutdown();

    /**
     * @brief 渲染所有GUI元素
     * @param guiState GUI状态数据（包含指向各种参数的指针）
     * @param deltaTime 帧时间（秒）
     * @param fps 当前FPS
     */
    void render(GUIState& guiState, double deltaTime, double fps);

    /**
     * @brief 开始新的ImGui帧
     */
    void newFrame();

    /**
     * @brief 渲染ImGui绘制数据
     */
    void renderDrawData();

    // UI状态
    bool showMainPanel = true;          // 显示主参数面板
    bool showPerformancePanel = true;   // 显示性能监控面板
    bool showHUD = true;                // 显示抬头显示（HUD）
    bool showDemoWindow = false;        // 显示ImGui演示窗口
    bool showAboutWindow = false;       // 显示关于窗口
    bool showPresetsPanel = false;      // 显示预设面板
    bool showWelcomeWindow = false;     // 显示欢迎窗口（首次使用）

private:
    /**
     * @brief 渲染主参数控制面板
     */
    void renderMainPanel(GUIState& guiState);

    /**
     * @brief 渲染性能监控面板
     */
    void renderPerformancePanel(double deltaTime, double fps);

    /**
     * @brief 渲染HUD（抬头显示）
     */
    void renderHUD(GUIState& guiState);

    /**
     * @brief 渲染预设管理面板
     */
    void renderPresetsPanel(GUIState& guiState);

    /**
     * @brief 渲染关于窗口
     */
    void renderAboutWindow();

    /**
     * @brief 渲染欢迎窗口（FTUE - First Time User Experience）
     */
    void renderWelcomeWindow();

    /**
     * @brief 应用深色主题
     */
    void applyDarkTheme();

    /**
     * @brief 应用亮色主题
     */
    void applyLightTheme();

    // 内部状态
    bool initialized = false;
    int currentTheme = 0;  // 0 = Dark, 1 = Light

    // 性能数据历史（用于绘制图表）
    static const int PERF_HISTORY_SIZE = 100;
    float fpsHistory[PERF_HISTORY_SIZE] = {};
    float frameTimeHistory[PERF_HISTORY_SIZE] = {};
    int perfHistoryOffset = 0;

    // 预设管理器
    PresetManager presetManager;
    int selectedPresetIndex = 0;
};

#endif // GUI_MANAGER_HPP
