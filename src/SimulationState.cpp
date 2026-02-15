#include "SimulationState.hpp"
#include "Utils/GLUtils.hpp"
#include <vector>
#include <algorithm>
#include <cstring>

namespace lenia {

SimulationState::~SimulationState() {
    destroyTextures();
}

void SimulationState::init(int width, int height, GLenum internalFormat) {
    m_width  = width;
    m_height = height;
    m_format = internalFormat;
    m_current = 0;
    createTextures();
}

void SimulationState::resize(int width, int height) {
    if (width == m_width && height == m_height) return;

    int oldW = m_width;
    int oldH = m_height;
    int components = (m_format == GL_RGBA32F) ? 4 : 1;
    GLenum pixelFormat = (m_format == GL_RGBA32F) ? GL_RGBA : GL_RED;

    std::vector<float> oldData(oldW * oldH * components, 0.0f);
    if (m_textures[m_current]) {
        glGetTextureImage(m_textures[m_current], 0, pixelFormat, GL_FLOAT,
                          oldW * oldH * components * static_cast<int>(sizeof(float)), oldData.data());
    }

    destroyTextures();
    m_width  = width;
    m_height = height;
    m_current = 0;
    createTextures();

    int copyW = std::min(oldW, width);
    int copyH = std::min(oldH, height);
    int offX = (width  - copyW) / 2;
    int offY = (height - copyH) / 2;
    int srcOffX = (oldW - copyW) / 2;
    int srcOffY = (oldH - copyH) / 2;

    std::vector<float> region(copyW * copyH * components, 0.0f);
    for (int y = 0; y < copyH; ++y) {
        std::memcpy(&region[y * copyW * components],
                    &oldData[(srcOffY + y) * oldW * components + srcOffX * components],
                    copyW * components * sizeof(float));
    }

    glTextureSubImage2D(m_textures[0], 0, offX, offY, copyW, copyH,
                        pixelFormat, GL_FLOAT, region.data());
    glTextureSubImage2D(m_textures[1], 0, offX, offY, copyW, copyH,
                        pixelFormat, GL_FLOAT, region.data());
}

void SimulationState::swap() {
    m_current = 1 - m_current;
}

void SimulationState::clear() {
    if (m_format == GL_RGBA32F) {
        float zero[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        glClearTexImage(m_textures[0], 0, GL_RGBA, GL_FLOAT, zero);
        glClearTexImage(m_textures[1], 0, GL_RGBA, GL_FLOAT, zero);
    } else {
        float zero = 0.0f;
        glClearTexImage(m_textures[0], 0, GL_RED, GL_FLOAT, &zero);
        glClearTexImage(m_textures[1], 0, GL_RED, GL_FLOAT, &zero);
    }
    m_current = 0;
}

void SimulationState::uploadRegion(int dstX, int dstY, int w, int h, const float* data) {
    glTextureSubImage2D(m_textures[m_current], 0, dstX, dstY, w, h,
                        GL_RED, GL_FLOAT, data);
    glTextureSubImage2D(m_textures[1 - m_current], 0, dstX, dstY, w, h,
                        GL_RED, GL_FLOAT, data);
}

void SimulationState::uploadRegionRGBA(int dstX, int dstY, int w, int h, const float* data) {
    glTextureSubImage2D(m_textures[m_current], 0, dstX, dstY, w, h,
                        GL_RGBA, GL_FLOAT, data);
    glTextureSubImage2D(m_textures[1 - m_current], 0, dstX, dstY, w, h,
                        GL_RGBA, GL_FLOAT, data);
}

void SimulationState::createTextures() {
    for (int i = 0; i < 2; ++i) {
        m_textures[i] = createTexture2D(m_width, m_height, m_format);
    }
    clear();
}

void SimulationState::destroyTextures() {
    for (int i = 0; i < 2; ++i) {
        if (m_textures[i]) {
            glDeleteTextures(1, &m_textures[i]);
            m_textures[i] = 0;
        }
    }
}

}
