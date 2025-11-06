#pragma once
#include <stdexcept>
#include <string>

/**
 * Custom exception classes for Black Hole Simulation
 * Replaces exit() calls to enable proper error handling and RAII cleanup
 */

/**
 * Base exception for initialization failures
 */
class InitializationException : public std::runtime_error {
public:
    explicit InitializationException(const std::string& message)
        : std::runtime_error("Initialization failed: " + message) {}
};

/**
 * GLFW initialization or window creation failure
 */
class GLFWException : public InitializationException {
public:
    explicit GLFWException(const std::string& message)
        : InitializationException("GLFW error: " + message) {}
};

/**
 * GLEW initialization failure
 */
class GLEWException : public InitializationException {
public:
    explicit GLEWException(const std::string& message)
        : InitializationException("GLEW error: " + message) {}
};

/**
 * Shader compilation or linking failure
 */
class ShaderException : public std::runtime_error {
public:
    explicit ShaderException(const std::string& message)
        : std::runtime_error("Shader error: " + message) {}
};

/**
 * File I/O failure (shader files, presets, etc.)
 */
class FileException : public std::runtime_error {
public:
    explicit FileException(const std::string& filename, const std::string& reason)
        : std::runtime_error("File error: " + filename + " - " + reason) {}
};

/**
 * OpenGL error detected
 */
class OpenGLException : public std::runtime_error {
public:
    explicit OpenGLException(const std::string& message, GLenum error)
        : std::runtime_error("OpenGL error: " + message + " (code: " + std::to_string(error) + ")") {}
};
