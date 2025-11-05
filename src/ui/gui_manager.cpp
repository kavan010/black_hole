#include <GL/glew.h>  // MUST be first, before any other GL headers
#include "gui_manager.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <cmath>

GUIManager::GUIManager() {
}

GUIManager::~GUIManager() {
    if (initialized) {
        shutdown();
    }
}

void GUIManager::initialize(GLFWwindow* window, const char* glsl_version) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Apply dark theme by default
    applyDarkTheme();

    initialized = true;
    std::cout << "[GUI] ImGui initialized successfully\n";
}

void GUIManager::shutdown() {
    if (initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        initialized = false;
    }
}

void GUIManager::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUIManager::renderDrawData() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::render(GUIState& guiState, double deltaTime, double fps) {
    newFrame();

    // Render different panels based on visibility flags
    if (showMainPanel) {
        renderMainPanel(guiState);
    }

    if (showPerformancePanel) {
        renderPerformancePanel(deltaTime, fps);
    }

    if (showHUD) {
        renderHUD(guiState);
    }

    if (showPresetsPanel) {
        renderPresetsPanel(guiState);
    }

    if (showAboutWindow) {
        renderAboutWindow();
    }

    if (showWelcomeWindow) {
        renderWelcomeWindow();
    }

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    renderDrawData();
}

void GUIManager::renderMainPanel(GUIState& guiState) {
    ImGui::Begin("Control Panel", &showMainPanel);

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Black Hole Simulation");
    ImGui::Separator();

    // ===== BLACK HOLE PARAMETERS =====
    if (ImGui::CollapsingHeader("Black Hole Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();

        // Kerr/Schwarzschild toggle
        if (guiState.useKerr) {
            bool useKerrBool = *guiState.useKerr;
            if (ImGui::Checkbox("Use Kerr Metric (Rotating)", &useKerrBool)) {
                *guiState.useKerr = useKerrBool;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Toggle between Schwarzschild (non-rotating) and Kerr (rotating) black holes");
            }
        }

        ImGui::Spacing();

        // Kerr spin parameter
        if (guiState.kerrSpin && guiState.useKerr && *guiState.useKerr) {
            ImGui::Text("Kerr Spin Parameter:");
            if (ImGui::SliderFloat("##spin", guiState.kerrSpin, 0.0f, 0.998f, "a = %.3f")) {
                // Spin changed
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("0 = Schwarzschild, 0.998 = Near-maximal rotation\nM87*: ~0.94, Sgr A*: ~0.5-0.9");
            }

            // Quick preset buttons
            ImGui::Text("Quick Presets:");
            if (ImGui::Button("Non-rotating")) *guiState.kerrSpin = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button("Moderate")) *guiState.kerrSpin = 0.5f;
            ImGui::SameLine();
            if (ImGui::Button("M87*")) *guiState.kerrSpin = 0.94f;
            ImGui::SameLine();
            if (ImGui::Button("Maximal")) *guiState.kerrSpin = 0.998f;
        }

        ImGui::Spacing();
    }

    // ===== RENDERING SETTINGS =====
    if (ImGui::CollapsingHeader("Rendering Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Spacing();

        // Exposure control
        if (guiState.exposure) {
            ImGui::Text("HDR Exposure:");
            if (ImGui::SliderFloat("##exposure", guiState.exposure, 0.1f, 10.0f, "%.2f")) {
                // Exposure changed
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Adjust brightness of the rendered image");
            }
        }

        ImGui::Spacing();

        // Visualization mode
        if (guiState.visualizationMode) {
            ImGui::Text("Visualization Mode:");
            const char* modes[] = {
                "Normal (Realistic)",
                "Gravitational Redshift",
                "Ray Path Steps",
                "Energy Conservation",
                "Carter Constant"
            };
            int currentMode = *guiState.visualizationMode;
            if (ImGui::Combo("##vizmode", &currentMode, modes, 5)) {
                *guiState.visualizationMode = currentMode;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Switch between different visualization modes for scientific analysis");
            }
        }

        ImGui::Spacing();

        // Wavelength band
        if (guiState.wavelengthBand) {
            ImGui::Text("Wavelength Band:");
            const char* bands[] = {
                "Radio (1.3mm)",
                "Infrared",
                "Optical (Visible)",
                "X-Ray",
                "Multi-Wavelength"
            };
            int currentBand = *guiState.wavelengthBand;
            if (ImGui::Combo("##waveband", &currentBand, bands, 5)) {
                *guiState.wavelengthBand = currentBand;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Simulate different electromagnetic wavelengths\nRadio: Event Horizon Telescope observations");
            }
        }

        ImGui::Spacing();
    }

    // ===== CAMERA CONTROLS =====
    if (ImGui::CollapsingHeader("Camera Controls")) {
        ImGui::Spacing();

        if (guiState.cameraRadius) {
            ImGui::Text("Distance from Black Hole:");
            float logRadius = std::log10(*guiState.cameraRadius);
            if (ImGui::SliderFloat("##radius", &logRadius, 10.0f, 12.0f, "10^%.1f m")) {
                *guiState.cameraRadius = std::pow(10.0f, logRadius);
            }
            ImGui::Text("Distance: %.2e meters", *guiState.cameraRadius);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Use mouse wheel to zoom, or drag with right button");
            }
        }

        if (guiState.cameraAzimuth && guiState.cameraElevation) {
            ImGui::Text("Viewing Angle:");
            float azimuthDeg = *guiState.cameraAzimuth * 180.0f / 3.14159f;
            float elevationDeg = *guiState.cameraElevation * 180.0f / 3.14159f;
            ImGui::Text("Azimuth: %.1f°", azimuthDeg);
            ImGui::Text("Elevation: %.1f°", elevationDeg);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Use left mouse button to rotate camera");
            }
        }

        ImGui::Spacing();
    }

    // ===== WINDOW CONTROLS =====
    if (ImGui::CollapsingHeader("Window & UI")) {
        ImGui::Spacing();

        ImGui::Checkbox("Show Performance Panel", &showPerformancePanel);
        ImGui::Checkbox("Show HUD Overlay", &showHUD);
        ImGui::Checkbox("Show Presets Panel", &showPresetsPanel);

        ImGui::Spacing();

        if (ImGui::Button("About", ImVec2(100, 0))) {
            showAboutWindow = !showAboutWindow;
        }
        ImGui::SameLine();
        if (ImGui::Button("ImGui Demo", ImVec2(100, 0))) {
            showDemoWindow = !showDemoWindow;
        }

        ImGui::Spacing();

        // Theme selection
        ImGui::Text("Theme:");
        if (ImGui::RadioButton("Dark", currentTheme == 0)) {
            applyDarkTheme();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Light", currentTheme == 1)) {
            applyLightTheme();
        }

        ImGui::Spacing();
    }

    // ===== HELP TEXT =====
    if (ImGui::CollapsingHeader("Keyboard Shortcuts")) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Camera:");
        ImGui::BulletText("Left Mouse: Rotate camera");
        ImGui::BulletText("Right Mouse: Pan view");
        ImGui::BulletText("Mouse Wheel: Zoom in/out");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Black Hole:");
        ImGui::BulletText("K: Toggle Kerr metric");
        ImGui::BulletText("[/]: Decrease/Increase spin");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Rendering:");
        ImGui::BulletText("E/R: Adjust exposure");
        ImGui::BulletText("V: Cycle visualization modes");
        ImGui::BulletText("B: Cycle wavelength bands");
        ImGui::Spacing();
    }

    ImGui::End();
}

void GUIManager::renderPerformancePanel(double deltaTime, double fps) {
    // Update performance history
    fpsHistory[perfHistoryOffset] = static_cast<float>(fps);
    frameTimeHistory[perfHistoryOffset] = static_cast<float>(deltaTime * 1000.0); // Convert to ms
    perfHistoryOffset = (perfHistoryOffset + 1) % PERF_HISTORY_SIZE;

    ImGui::Begin("Performance", &showPerformancePanel);

    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame Time: %.2f ms", deltaTime * 1000.0);
    ImGui::PlotLines("FPS", fpsHistory, PERF_HISTORY_SIZE, perfHistoryOffset, nullptr, 0.0f, 120.0f, ImVec2(0, 80));
    ImGui::PlotLines("Frame Time (ms)", frameTimeHistory, PERF_HISTORY_SIZE, perfHistoryOffset, nullptr, 0.0f, 50.0f, ImVec2(0, 80));

    ImGui::End();
}

void GUIManager::renderHUD(GUIState& guiState) {
    // Simple HUD overlay at top-right corner
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                     ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoFocusOnAppearing |
                                     ImGuiWindowFlags_NoNav;

    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;
    ImVec2 window_pos, window_pos_pivot;
    window_pos.x = work_pos.x + work_size.x - PAD;
    window_pos.y = work_pos.y + PAD;
    window_pos_pivot.x = 1.0f;
    window_pos_pivot.y = 0.0f;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.35f);

    if (ImGui::Begin("HUD", &showHUD, window_flags)) {
        ImGui::Text("Black Hole Simulation");
        ImGui::Separator();
        if (guiState.cameraRadius) {
            ImGui::Text("Camera Distance: %.2e m", *guiState.cameraRadius);
        }
        // Add more HUD info as needed
    }
    ImGui::End();
}

void GUIManager::renderPresetsPanel(GUIState& guiState) {
    // Will be implemented in Phase 7.3
    ImGui::Begin("Presets", &showPresetsPanel);
    ImGui::Text("Preset system coming in Phase 7.3");
    ImGui::End();
}

void GUIManager::renderAboutWindow() {
    ImGui::Begin("About", &showAboutWindow);

    ImGui::Text("Black Hole Simulation");
    ImGui::Text("Version: Phase 7 (GUI Integration)");
    ImGui::Separator();
    ImGui::Text("Real-time general relativity visualization");
    ImGui::Text("Supports both Schwarzschild and Kerr metrics");
    ImGui::Separator();
    ImGui::Text("Built with:");
    ImGui::BulletText("OpenGL 4.3+ (Compute Shaders)");
    ImGui::BulletText("GLFW (Window Management)");
    ImGui::BulletText("Dear ImGui (User Interface)");
    ImGui::BulletText("GLM (Mathematics)");

    ImGui::End();
}

void GUIManager::renderWelcomeWindow() {
    // Will be implemented in Phase 7.5 (FTUE)
    ImGui::Begin("Welcome", &showWelcomeWindow);
    ImGui::Text("Welcome to Black Hole Simulation!");
    ImGui::Text("FTUE will be implemented in Phase 7.5");
    if (ImGui::Button("Close")) {
        showWelcomeWindow = false;
    }
    ImGui::End();
}

void GUIManager::applyDarkTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Customize colors for a more modern look
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 8);

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.90f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

    currentTheme = 0;
}

void GUIManager::applyLightTheme() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 8);

    currentTheme = 1;
}
