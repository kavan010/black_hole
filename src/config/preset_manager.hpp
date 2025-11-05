#ifndef PRESET_MANAGER_HPP
#define PRESET_MANAGER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>

/**
 * @brief 场景预设 - 包含所有可配置参数
 *
 * 这个结构保存黑洞模拟的完整状态，可以保存和加载
 */
struct Preset {
    std::string name;
    std::string description;

    // Black hole parameters
    float kerrSpin = 0.0f;
    bool useKerr = false;

    // Rendering parameters
    float exposure = 1.0f;
    int visualizationMode = 0;
    int wavelengthBand = 2;  // Default: Optical

    // Camera parameters
    float cameraRadius = 6.34194e10f;
    float cameraAzimuth = 0.0f;
    float cameraElevation = 1.5708f;  // π/2

    /**
     * @brief 序列化为简单的文本格式
     */
    std::string serialize() const {
        std::ostringstream oss;
        oss << "name=" << name << "\n";
        oss << "description=" << description << "\n";
        oss << "kerrSpin=" << kerrSpin << "\n";
        oss << "useKerr=" << (useKerr ? 1 : 0) << "\n";
        oss << "exposure=" << exposure << "\n";
        oss << "visualizationMode=" << visualizationMode << "\n";
        oss << "wavelengthBand=" << wavelengthBand << "\n";
        oss << "cameraRadius=" << cameraRadius << "\n";
        oss << "cameraAzimuth=" << cameraAzimuth << "\n";
        oss << "cameraElevation=" << cameraElevation << "\n";
        return oss.str();
    }

    /**
     * @brief 从文本格式反序列化
     */
    static Preset deserialize(const std::string& data) {
        Preset preset;
        std::istringstream iss(data);
        std::string line;

        while (std::getline(iss, line)) {
            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "name") preset.name = value;
            else if (key == "description") preset.description = value;
            else if (key == "kerrSpin") preset.kerrSpin = std::stof(value);
            else if (key == "useKerr") preset.useKerr = (std::stoi(value) != 0);
            else if (key == "exposure") preset.exposure = std::stof(value);
            else if (key == "visualizationMode") preset.visualizationMode = std::stoi(value);
            else if (key == "wavelengthBand") preset.wavelengthBand = std::stoi(value);
            else if (key == "cameraRadius") preset.cameraRadius = std::stof(value);
            else if (key == "cameraAzimuth") preset.cameraAzimuth = std::stof(value);
            else if (key == "cameraElevation") preset.cameraElevation = std::stof(value);
        }

        return preset;
    }
};

/**
 * @brief 预设管理器 - 管理场景预设的加载、保存和应用
 */
class PresetManager {
public:
    PresetManager() {
        initializeBuiltinPresets();
    }

    /**
     * @brief 获取所有预设列表
     */
    const std::vector<Preset>& getPresets() const {
        return presets;
    }

    /**
     * @brief 根据索引获取预设
     */
    const Preset* getPreset(size_t index) const {
        if (index < presets.size()) {
            return &presets[index];
        }
        return nullptr;
    }

    /**
     * @brief 根据名称获取预设
     */
    const Preset* getPresetByName(const std::string& name) const {
        for (const auto& preset : presets) {
            if (preset.name == name) {
                return &preset;
            }
        }
        return nullptr;
    }

    /**
     * @brief 添加新预设
     */
    void addPreset(const Preset& preset) {
        presets.push_back(preset);
    }

    /**
     * @brief 保存预设到文件
     */
    bool savePreset(const Preset& preset, const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        file << preset.serialize();
        file.close();
        return true;
    }

    /**
     * @brief 从文件加载预设
     */
    bool loadPreset(const std::string& filename, Preset& preset) const {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        preset = Preset::deserialize(buffer.str());
        return true;
    }

    /**
     * @brief 保存所有预设到目录
     */
    bool saveAllPresets(const std::string& directory) const {
        for (const auto& preset : presets) {
            std::string filename = directory + "/" + preset.name + ".preset";
            if (!savePreset(preset, filename)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief 应用预设到GUI状态
     * Note: Implementation in gui_manager.cpp to avoid circular dependency
     */
    void applyPresetToGUIState(const Preset& preset, struct GUIState& guiState);

    /**
     * @brief 从GUI状态创建预设
     * Note: Implementation in gui_manager.cpp to avoid circular dependency
     */
    Preset createPresetFromGUIState(const struct GUIState& guiState, const std::string& name, const std::string& description);

private:
    std::vector<Preset> presets;

    /**
     * @brief 初始化内置预设
     */
    void initializeBuiltinPresets() {
        // Preset 1: Schwarzschild (Non-rotating)
        {
            Preset p;
            p.name = "Schwarzschild";
            p.description = "Non-rotating black hole (classical General Relativity)";
            p.kerrSpin = 0.0f;
            p.useKerr = false;
            p.exposure = 1.0f;
            p.visualizationMode = 0;
            p.wavelengthBand = 2;
            p.cameraRadius = 6.34194e10f;
            p.cameraAzimuth = 0.0f;
            p.cameraElevation = 1.5708f;
            presets.push_back(p);
        }

        // Preset 2: M87* (EHT Observation)
        {
            Preset p;
            p.name = "M87* (EHT)";
            p.description = "M87* as observed by Event Horizon Telescope (spin~0.94)";
            p.kerrSpin = 0.94f;
            p.useKerr = true;
            p.exposure = 1.5f;
            p.visualizationMode = 0;
            p.wavelengthBand = 0;  // Radio
            p.cameraRadius = 5.0e10f;
            p.cameraAzimuth = 0.785f;  // 45 degrees
            p.cameraElevation = 1.22f;  // ~70 degrees
            presets.push_back(p);
        }

        // Preset 3: Sagittarius A*
        {
            Preset p;
            p.name = "Sgr A*";
            p.description = "Sagittarius A* - Milky Way's supermassive black hole";
            p.kerrSpin = 0.7f;
            p.useKerr = true;
            p.exposure = 1.2f;
            p.visualizationMode = 0;
            p.wavelengthBand = 0;  // Radio
            p.cameraRadius = 4.5e10f;
            p.cameraAzimuth = 0.0f;
            p.cameraElevation = 1.5708f;
            presets.push_back(p);
        }

        // Preset 4: Interstellar "Gargantua"
        {
            Preset p;
            p.name = "Gargantua";
            p.description = "Interstellar movie - near-maximal rotation";
            p.kerrSpin = 0.998f;
            p.useKerr = true;
            p.exposure = 2.0f;
            p.visualizationMode = 0;
            p.wavelengthBand = 2;  // Optical
            p.cameraRadius = 8.0e10f;
            p.cameraAzimuth = 0.5f;
            p.cameraElevation = 1.4f;
            presets.push_back(p);
        }

        // Preset 5: Redshift Analysis
        {
            Preset p;
            p.name = "Redshift Analysis";
            p.description = "Visualize gravitational redshift effects";
            p.kerrSpin = 0.5f;
            p.useKerr = true;
            p.exposure = 1.0f;
            p.visualizationMode = 1;  // Redshift mode
            p.wavelengthBand = 2;
            p.cameraRadius = 6.0e10f;
            p.cameraAzimuth = 0.0f;
            p.cameraElevation = 1.5708f;
            presets.push_back(p);
        }

        // Preset 6: X-Ray Binary
        {
            Preset p;
            p.name = "X-Ray Binary";
            p.description = "X-ray emission from accretion disk";
            p.kerrSpin = 0.8f;
            p.useKerr = true;
            p.exposure = 1.8f;
            p.visualizationMode = 0;
            p.wavelengthBand = 3;  // X-ray
            p.cameraRadius = 5.5e10f;
            p.cameraAzimuth = 1.57f;
            p.cameraElevation = 1.3f;
            presets.push_back(p);
        }
    }
};

#endif // PRESET_MANAGER_HPP
