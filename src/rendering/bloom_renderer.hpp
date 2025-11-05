#pragma once
#include <GL/glew.h>
#include "shader_manager.hpp"
#include "../utils/logger.hpp"

/**
 * Bloom Post-Processing Renderer
 * Implements separable Gaussian blur bloom effect
 */
class BloomRenderer {
private:
    GLuint extractProgram;
    GLuint blurProgram;
    GLuint bloomFBO[2];      // Ping-pong framebuffers for blur
    GLuint bloomTextures[2]; // Ping-pong textures
    GLuint quadVAO;
    int width, height;
    bool initialized = false;

public:
    float threshold = 1.0f;     // Brightness threshold for bloom
    float bloomStrength = 0.04f; // Bloom intensity
    bool enabled = true;        // Bloom on/off

    BloomRenderer() = default;

    void initialize(int w, int h, GLuint sharedQuadVAO) {
        width = w;
        height = h;
        quadVAO = sharedQuadVAO;

        // Create extract shader
        const char* quadVert = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;
            out vec2 TexCoord;
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            })";

        std::ifstream extractFile("bloom_extract.frag");
        if (!extractFile.is_open()) {
            Logger::error("Failed to open bloom_extract.frag");
            return;
        }
        std::stringstream extractSS;
        extractSS << extractFile.rdbuf();
        std::string extractSrc = extractSS.str();

        std::ifstream blurFile("gaussian_blur.frag");
        if (!blurFile.is_open()) {
            Logger::error("Failed to open gaussian_blur.frag");
            return;
        }
        std::stringstream blurSS;
        blurSS << blurFile.rdbuf();
        std::string blurSrc = blurSS.str();

        extractProgram = ShaderManager::createProgram(quadVert, extractSrc.c_str());
        blurProgram = ShaderManager::createProgram(quadVert, blurSrc.c_str());

        // Create framebuffers and textures
        glGenFramebuffers(2, bloomFBO);
        glGenTextures(2, bloomTextures);

        for (int i = 0; i < 2; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[i]);
            glBindTexture(GL_TEXTURE_2D, bloomTextures[i]);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width / 4, height / 4, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomTextures[i], 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                Logger::error("Bloom framebuffer ", i, " not complete");
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        initialized = true;
        Logger::info("Bloom renderer initialized (", width / 4, "x", height / 4, ")");
    }

    /**
     * Render bloom effect
     * @param hdrTexture Input HDR texture
     * @return Output bloom texture (blurred bright areas)
     */
    GLuint render(GLuint hdrTexture) {
        if (!initialized || !enabled) {
            // Return black texture if bloom disabled
            return bloomTextures[0];
        }

        // 1. Extract bright areas
        glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[0]);
        glViewport(0, 0, width / 4, height / 4);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(extractProgram);
        glUniform1f(glGetUniformLocation(extractProgram, "threshold"), threshold);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glUniform1i(glGetUniformLocation(extractProgram, "hdrTexture"), 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

        // 2. Blur ping-pong (5 iterations for smooth bloom)
        glUseProgram(blurProgram);
        bool horizontal = true;
        int iterations = 10;

        for (int i = 0; i < iterations; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, bloomFBO[horizontal ? 1 : 0]);
            glUniform1i(glGetUniformLocation(blurProgram, "horizontal"), horizontal);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, bloomTextures[horizontal ? 0 : 1]);
            glUniform1i(glGetUniformLocation(blurProgram, "image"), 0);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

            horizontal = !horizontal;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);

        // Return final blurred texture
        return bloomTextures[0];
    }

    void cleanup() {
        if (initialized) {
            glDeleteFramebuffers(2, bloomFBO);
            glDeleteTextures(2, bloomTextures);
            glDeleteProgram(extractProgram);
            glDeleteProgram(blurProgram);
        }
    }

    ~BloomRenderer() {
        cleanup();
    }
};
