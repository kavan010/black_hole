#pragma once
#include <GL/glew.h>
#include <string>
#include "logger.hpp"

/**
 * OpenGL Debug Utilities
 * Provides macros for error checking in debug builds
 */

/**
 * Convert OpenGL error code to human-readable string
 */
inline const char* glErrorString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:          return "GL_NO_ERROR";
        case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
        case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
        case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
        default:                   return "UNKNOWN_GL_ERROR";
    }
}

/**
 * Check for OpenGL errors and log them
 * Returns true if an error was found
 */
inline bool checkGLError(const char* file, int line, const char* call) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        Logger::error("[OpenGL Error] ", glErrorString(error),
                     " (0x", std::hex, error, std::dec, ")");
        Logger::error("  at ", file, ":", line);
        Logger::error("  call: ", call);
        return true;
    }
    return false;
}

/**
 * GL_CHECK macro - wraps OpenGL calls with error checking in debug mode
 *
 * Usage:
 *   GL_CHECK(glGenBuffers(1, &buffer));
 *   GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer));
 *
 * In debug builds (DEBUG defined), this checks for errors after each call.
 * In release builds, it's a no-op (just executes the call).
 */
#ifdef DEBUG
    #define GL_CHECK(call) \
        do { \
            call; \
            if (checkGLError(__FILE__, __LINE__, #call)) { \
                Logger::error("OpenGL error detected, continuing..."); \
            } \
        } while (0)

    #define GL_CHECK_CRITICAL(call) \
        do { \
            call; \
            if (checkGLError(__FILE__, __LINE__, #call)) { \
                Logger::error("Critical OpenGL error, throwing exception"); \
                throw OpenGLException("Critical OpenGL error in " #call, glGetError()); \
            } \
        } while (0)
#else
    // In release mode, just execute the call without checking
    #define GL_CHECK(call) call
    #define GL_CHECK_CRITICAL(call) call
#endif

/**
 * Clear any pending OpenGL errors
 * Useful before a section where you want to ensure clean error state
 */
inline void clearGLErrors() {
    while (glGetError() != GL_NO_ERROR) {
        // Drain error queue
    }
}

/**
 * Check framebuffer completeness
 * Returns true if framebuffer is complete, false otherwise
 */
inline bool checkFramebufferStatus(const char* name = "Framebuffer") {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        const char* statusStr;
        switch (status) {
            case GL_FRAMEBUFFER_UNDEFINED:
                statusStr = "GL_FRAMEBUFFER_UNDEFINED";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                statusStr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                statusStr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                statusStr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                statusStr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                statusStr = "GL_FRAMEBUFFER_UNSUPPORTED";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                statusStr = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                statusStr = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
                break;
            default:
                statusStr = "UNKNOWN_FRAMEBUFFER_STATUS";
                break;
        }
        Logger::error(name, " is not complete: ", statusStr, " (0x", std::hex, status, std::dec, ")");
        return false;
    }
    return true;
}
