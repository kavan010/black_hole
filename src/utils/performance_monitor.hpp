#pragma once
#include <deque>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

/**
 * Performance monitoring for real-time metrics
 * Tracks FPS, frame times, and provides statistics
 */
class PerformanceMonitor {
private:
    std::deque<double> frameTimes;
    std::chrono::high_resolution_clock::time_point lastUpdate;
    std::chrono::high_resolution_clock::time_point lastFrame;
    double currentFPS = 0.0;
    double minFrameTime = 0.0;
    double maxFrameTime = 0.0;
    double avgFrameTime = 0.0;
    size_t maxSamples = 60;  // Keep 60 frames of history
    bool firstFrame = true;

public:
    PerformanceMonitor() {
        lastUpdate = std::chrono::high_resolution_clock::now();
        lastFrame = lastUpdate;
    }

    /**
     * Call this once per frame
     * Returns the delta time in seconds
     */
    double recordFrame() {
        auto now = std::chrono::high_resolution_clock::now();

        if (firstFrame) {
            lastFrame = now;
            firstFrame = false;
            return 0.0;
        }

        // Calculate frame time
        std::chrono::duration<double> delta = now - lastFrame;
        double frameTime = delta.count();
        lastFrame = now;

        // Add to history
        frameTimes.push_back(frameTime);
        if (frameTimes.size() > maxSamples) {
            frameTimes.pop_front();
        }

        // Update statistics every 500ms
        std::chrono::duration<double> timeSinceUpdate = now - lastUpdate;
        if (timeSinceUpdate.count() > 0.5 && !frameTimes.empty()) {
            updateStatistics();
            lastUpdate = now;
        }

        return frameTime;
    }

    double getFPS() const { return currentFPS; }
    double getMinFrameTime() const { return minFrameTime * 1000.0; } // in ms
    double getMaxFrameTime() const { return maxFrameTime * 1000.0; } // in ms
    double getAvgFrameTime() const { return avgFrameTime * 1000.0; } // in ms

    /**
     * Get formatted performance string
     * Example: "FPS: 60.2 | Frame: 16.6ms (min: 15.2ms, max: 18.3ms)"
     */
    std::string getFormattedStats() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1);
        oss << "FPS: " << currentFPS
            << " | Frame: " << avgFrameTime * 1000.0 << "ms"
            << " (min: " << minFrameTime * 1000.0 << "ms"
            << ", max: " << maxFrameTime * 1000.0 << "ms)";
        return oss.str();
    }

    /**
     * Get compact performance string
     * Example: "60.2 FPS"
     */
    std::string getCompactStats() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1);
        oss << currentFPS << " FPS";
        return oss.str();
    }

    void setMaxSamples(size_t samples) {
        maxSamples = samples;
        while (frameTimes.size() > maxSamples) {
            frameTimes.pop_front();
        }
    }

private:
    void updateStatistics() {
        if (frameTimes.empty()) return;

        // Calculate average
        double sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0);
        avgFrameTime = sum / frameTimes.size();
        currentFPS = (avgFrameTime > 0.0) ? (1.0 / avgFrameTime) : 0.0;

        // Find min/max
        minFrameTime = *std::min_element(frameTimes.begin(), frameTimes.end());
        maxFrameTime = *std::max_element(frameTimes.begin(), frameTimes.end());
    }
};
