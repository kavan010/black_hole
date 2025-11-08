#pragma once
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "../utils/logger.hpp"
#include "../utils/exceptions.hpp"

/**
 * Manages shader compilation and program linking
 * Separates shader management concerns from Engine class
 */
class ShaderManager {
public:
    /**
     * Create a shader program from inline vertex and fragment shader source
     */
    static GLuint createProgram(const char* vertexSource, const char* fragmentSource) {
        GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertexSource, "inline vertex");
        GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, "inline fragment");

        GLuint program = linkProgram(vertShader, fragShader);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        return program;
    }

    /**
     * Create a shader program from vertex and fragment shader files
     */
    static GLuint createProgramFromFiles(const char* vertPath, const char* fragPath) {
        std::string vertSource = loadShaderFile(vertPath);
        std::string fragSource = loadShaderFile(fragPath);

        GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertSource.c_str(), vertPath);
        GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragSource.c_str(), fragPath);

        GLuint program = linkProgram(vertShader, fragShader);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        return program;
    }

    /**
     * Create a compute shader program from file
     */
    static GLuint createComputeProgram(const char* path) {
        std::string source = loadShaderFile(path);

        GLuint shader = compileShader(GL_COMPUTE_SHADER, source.c_str(), path);

        GLuint program = glCreateProgram();
        glAttachShader(program, shader);
        glLinkProgram(program);

        // Check linking
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint logLen;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetProgramInfoLog(program, logLen, nullptr, log.data());
            std::string errorMsg = "Compute shader link error (" + std::string(path) + "):\n" + log.data();
            Logger::error(errorMsg);
            glDeleteShader(shader);
            throw ShaderException(errorMsg);
        }

        glDeleteShader(shader);
        Logger::info("Compute shader compiled: ", path);
        return program;
    }

private:
    static std::string loadShaderFile(const char* path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::error("Failed to open shader file: ", path);
            throw FileException(path, "Could not open file");
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static GLuint compileShader(GLenum type, const char* source, const char* name) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        // Check compilation
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint logLen;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetShaderInfoLog(shader, logLen, nullptr, log.data());

            const char* typeStr =
                (type == GL_VERTEX_SHADER) ? "Vertex" :
                (type == GL_FRAGMENT_SHADER) ? "Fragment" :
                (type == GL_COMPUTE_SHADER) ? "Compute" : "Unknown";

            std::string errorMsg = std::string(typeStr) + " shader compile error (" + name + "):\n" + log.data();
            Logger::error(errorMsg);
            throw ShaderException(errorMsg);
        }

        return shader;
    }

    static GLuint linkProgram(GLuint vertShader, GLuint fragShader) {
        GLuint program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        // Check linking
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint logLen;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<char> log(logLen);
            glGetProgramInfoLog(program, logLen, nullptr, log.data());
            std::string errorMsg = "Shader program link error:\n" + std::string(log.data());
            Logger::error(errorMsg);
            throw ShaderException(errorMsg);
        }

        return program;
    }
};
