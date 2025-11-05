#pragma once
#include <GL/glew.h>
#include "logger.hpp"

/**
 * OpenGL error checking utilities
 */

// Check for OpenGL errors and log them
inline void checkGLError(const char* file, int line) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char* errorStr;
        switch (err) {
            case GL_INVALID_ENUM:      errorStr = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:     errorStr = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: errorStr = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:     errorStr = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            default:                   errorStr = "UNKNOWN_ERROR"; break;
        }
        Logger::error("OpenGL error ", errorStr, " (0x", std::hex, err, std::dec, ") at ",
                     file, ":", line);
    }
}

// Macro for checking OpenGL errors
#ifdef NDEBUG
    #define GL_CHECK(call) call
#else
    #define GL_CHECK(call) \
        do { \
            call; \
            checkGLError(__FILE__, __LINE__); \
        } while(0)
#endif

// Macro for checking OpenGL calls with return value
#define GL_CHECK_RETURN(call) \
    [&]() { \
        auto result = call; \
        checkGLError(__FILE__, __LINE__); \
        return result; \
    }()
