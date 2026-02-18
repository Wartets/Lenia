/**
 * @file GLUtils.hpp
 * @brief OpenGL utility functions and debug callback.
 */

#pragma once

#include <glad/glad.h>
#include "Logger.hpp"
#include <cstdio>
#include <cstdlib>

namespace lenia {

/**
 * @brief OpenGL debug message callback for error reporting.
 * 
 * Registered with glDebugMessageCallback when debug context is available.
 * Logs warnings and errors from the GL driver.
 */
inline void APIENTRY glDebugCallback(
    GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei,
    const GLchar* message, const void*)
{
    // Skip notifications (too verbose)
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    const char* srcStr = "?";
    switch (source) {
        case GL_DEBUG_SOURCE_API:             srcStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   srcStr = "Window"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: srcStr = "Shader"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     srcStr = "3rdParty"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     srcStr = "App"; break;
        default: break;
    }
    const char* typeStr = "?";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "UB"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Perf"; break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
        default: break;
    }
    if (type == GL_DEBUG_TYPE_ERROR)
        LOG_ERROR("[GL %s][%s] (id=%u): %s", srcStr, typeStr, id, message);
    else
        LOG_WARN("[GL %s][%s] (id=%u): %s", srcStr, typeStr, id, message);
}

/**
 * @brief Create a 2D texture with immutable storage.
 * @param w Width in pixels
 * @param h Height in pixels  
 * @param internalFormat GL format (GL_R32F, GL_RGBA32F, etc.)
 * @return OpenGL texture handle
 */
inline GLuint createTexture2D(GLsizei w, GLsizei h, GLenum internalFormat) {
    GLuint tex = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);
    glTextureStorage2D(tex, 1, internalFormat, w, h);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return tex;
}

/**
 * @brief Create a 1D texture (for colormaps).
 */
inline GLuint createTexture1D(GLsizei w, GLenum internalFormat) {
    GLuint tex = 0;
    glCreateTextures(GL_TEXTURE_1D, 1, &tex);
    glTextureStorage1D(tex, 1, internalFormat, w);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    return tex;
}

/**
 * @brief Dispatch a 2D compute shader with automatic workgroup calculation.
 * @param w, h Dimensions to cover
 * @param localX, localY Workgroup size (default 16x16)
 */
inline void dispatchCompute2D(GLsizei w, GLsizei h, GLsizei localX = 16, GLsizei localY = 16) {
    // Round up to cover all pixels
    GLsizei gx = (w + localX - 1) / localX;
    GLsizei gy = (h + localY - 1) / localY;
    glDispatchCompute(gx, gy, 1);
}

}
