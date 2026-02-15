#pragma once

#include <glad/glad.h>
#include "Logger.hpp"
#include <cstdio>
#include <cstdlib>

namespace lenia {

inline void APIENTRY glDebugCallback(
    GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei,
    const GLchar* message, const void*)
{
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

inline GLuint createTexture1D(GLsizei w, GLenum internalFormat) {
    GLuint tex = 0;
    glCreateTextures(GL_TEXTURE_1D, 1, &tex);
    glTextureStorage1D(tex, 1, internalFormat, w);
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    return tex;
}

inline void dispatchCompute2D(GLsizei w, GLsizei h, GLsizei localX = 16, GLsizei localY = 16) {
    GLsizei gx = (w + localX - 1) / localX;
    GLsizei gy = (h + localY - 1) / localY;
    glDispatchCompute(gx, gy, 1);
}

}
