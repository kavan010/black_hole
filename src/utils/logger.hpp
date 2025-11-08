#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

/**
 * Simple logging system with log levels
 *
 * Usage:
 *   Logger::setLevel(LogLevel::INFO);
 *   Logger::info("Camera position: ", x, ", ", y, ", ", z);
 *   Logger::debug("This won't be shown if level is INFO");
 *   Logger::error("Critical error occurred!");
 */

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    NONE = 4  // Suppress all logging
};

class Logger {
private:
    static LogLevel minLevel;
    static bool showTimestamp;

    static std::string getTimestamp() {
        if (!showTimestamp) return "";

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ) % 1000;

        std::ostringstream oss;
        oss << "[" << std::put_time(std::localtime(&time), "%H:%M:%S")
            << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        return oss.str();
    }

public:
    static void setLevel(LogLevel level) {
        minLevel = level;
    }

    static void setTimestamp(bool enabled) {
        showTimestamp = enabled;
    }

    template<typename... Args>
    static void debug(Args&&... args) {
        if (minLevel <= LogLevel::DEBUG) {
            log("DEBUG", std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void info(Args&&... args) {
        if (minLevel <= LogLevel::INFO) {
            log("INFO ", std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void warn(Args&&... args) {
        if (minLevel <= LogLevel::WARN) {
            log("WARN ", std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void error(Args&&... args) {
        log("ERROR", std::forward<Args>(args)...);
    }

private:
    template<typename... Args>
    static void log(const char* level, Args&&... args) {
        std::ostringstream oss;
        oss << getTimestamp() << "[" << level << "] ";
        ((oss << std::forward<Args>(args)), ...);
        std::cerr << oss.str() << std::endl;
    }
};

// Static member initialization
inline LogLevel Logger::minLevel = LogLevel::INFO;
inline bool Logger::showTimestamp = false;
