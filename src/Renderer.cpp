#include "Renderer.hpp"
#include "Utils/GLUtils.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstring>

namespace lenia {

Renderer::~Renderer() {
    if (m_vao)            glDeleteVertexArrays(1, &m_vao);
    if (m_colormapTex)    glDeleteTextures(1, &m_colormapTex);
    if (m_displaySampler) glDeleteSamplers(1, &m_displaySampler);
    for (auto t : m_customColormapTextures)
        if (t) glDeleteTextures(1, &t);
}

bool Renderer::init(const std::string& vertPath, const std::string& fragPath) {
    if (!m_displayShader.loadGraphics(vertPath, fragPath)) return false;
    createEmptyVAO();
    generateColormap();

    glCreateSamplers(1, &m_displaySampler);
    glSamplerParameteri(m_displaySampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(m_displaySampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return true;
}

void Renderer::draw(GLuint stateTexture, int viewportW, int viewportH, const LeniaParams& params) {
    glViewport(0, 0, viewportW, viewportH);
    glClearColor(params.bgR, params.bgG, params.bgB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum filter = (params.filterMode == 1) ? GL_NEAREST : GL_LINEAR;
    glSamplerParameteri(m_displaySampler, GL_TEXTURE_MIN_FILTER, filter);
    glSamplerParameteri(m_displaySampler, GL_TEXTURE_MAG_FILTER, filter);

    m_displayShader.use();

    m_displayShader.setFloat("uZoom", params.zoom);
    GLint panLoc = glGetUniformLocation(m_displayShader.id(), "uPan");
    if (panLoc >= 0)
        glProgramUniform2f(m_displayShader.id(), panLoc, params.panX, params.panY);
    m_displayShader.setInt("uColormapMode", params.colormapMode);
    m_displayShader.setFloat("uBrightness", params.brightness);
    m_displayShader.setFloat("uContrast", params.contrast);
    float gridAspect = (params.gridH > 0) ? static_cast<float>(params.gridW) / static_cast<float>(params.gridH) : 1.0f;
    float viewAspect = (viewportH > 0) ? static_cast<float>(viewportW) / static_cast<float>(viewportH) : 1.0f;
    m_displayShader.setFloat("uGridAspect", gridAspect);
    m_displayShader.setFloat("uViewAspect", viewAspect);
    m_displayShader.setInt("uFilterMode", params.filterMode);
    m_displayShader.setFloat("uEdgeStrength", params.edgeStrength);
    m_displayShader.setFloat("uGlowStrength", params.glowStrength);
    m_displayShader.setFloat("uGamma", params.gamma);
    m_displayShader.setInt("uInvertColors", params.invertColors ? 1 : 0);
    m_displayShader.setInt("uShowGrid", params.showGrid ? 1 : 0);
    m_displayShader.setFloat("uGridOpacity", params.gridOpacity);
    m_displayShader.setInt("uGridW", params.gridW);
    m_displayShader.setInt("uGridH", params.gridH);
    m_displayShader.setInt("uMultiChannel", params.numChannels > 1 ? 1 : 0);
    m_displayShader.setInt("uDisplayMode", params.displayMode);

    m_displayShader.setInt("uClipNullCells", params.clipToZero ? 1 : 0);
    m_displayShader.setFloat("uClipThreshold", params.clipThreshold);
    GLint bgLoc = glGetUniformLocation(m_displayShader.id(), "uBgColor");
    if (bgLoc >= 0)
        glProgramUniform3f(m_displayShader.id(), bgLoc, params.bgR, params.bgG, params.bgB);

    GLint glcLoc = glGetUniformLocation(m_displayShader.id(), "uGridLineColor");
    if (glcLoc >= 0)
        glProgramUniform3f(m_displayShader.id(), glcLoc, params.gridLineR, params.gridLineG, params.gridLineB);
    m_displayShader.setFloat("uGridLineThickness", params.gridLineThickness);
    m_displayShader.setInt("uGridSpacingMode", params.gridSpacingMode);
    m_displayShader.setInt("uGridCustomSpacing", std::max(1, params.gridCustomSpacing));
    m_displayShader.setInt("uGridMajorLines", params.gridMajorLines ? 1 : 0);
    m_displayShader.setInt("uGridMajorEvery", params.gridMajorEvery);
    m_displayShader.setFloat("uGridMajorOpacity", params.gridMajorOpacity);

    m_displayShader.setFloat("uCmapOffset", params.cmapOffset);
    m_displayShader.setFloat("uCmapRange0", params.cmapRange0);
    m_displayShader.setFloat("uCmapRange1", params.cmapRange1);
    m_displayShader.setFloat("uCmapPower", params.cmapPower);
    m_displayShader.setFloat("uCmapHueShift", params.cmapHueShift);
    m_displayShader.setFloat("uCmapSaturation", params.cmapSaturation);
    m_displayShader.setInt("uCmapReverse", params.cmapReverse ? 1 : 0);

    m_displayShader.setInt("uShowBoundary", params.showBoundary ? 1 : 0);
    GLint bcLoc = glGetUniformLocation(m_displayShader.id(), "uBoundaryColor");
    if (bcLoc >= 0)
        glProgramUniform3f(m_displayShader.id(), bcLoc, params.boundaryR, params.boundaryG, params.boundaryB);
    m_displayShader.setFloat("uBoundaryOpacity", params.boundaryOpacity);
    m_displayShader.setInt("uBoundaryStyle", params.boundaryStyle);
    m_displayShader.setFloat("uBoundaryThickness", params.boundaryThickness);
    m_displayShader.setInt("uBoundaryAnimate", params.boundaryAnimate ? 1 : 0);
    m_displayShader.setFloat("uBoundaryDashLength", params.boundaryDashLength);
    m_displayShader.setFloat("uTime", static_cast<float>(glfwGetTime()));

    m_displayShader.setInt("uMultiChannelBlend", params.multiChannelBlend);
    GLint cwLoc = glGetUniformLocation(m_displayShader.id(), "uChannelWeights");
    if (cwLoc >= 0)
        glProgramUniform3f(m_displayShader.id(), cwLoc, params.channelWeightR, params.channelWeightG, params.channelWeightB);
    m_displayShader.setInt("uUseColormapForMultichannel", params.useColormapForMultichannel ? 1 : 0);

    m_displayShader.setInt("uEdgeModeX", params.edgeModeX);
    m_displayShader.setInt("uEdgeModeY", params.edgeModeY);
    m_displayShader.setFloat("uEdgeValueX", params.edgeValueX);
    m_displayShader.setFloat("uEdgeValueY", params.edgeValueY);
    m_displayShader.setFloat("uEdgeFadeX", params.edgeFadeX);
    m_displayShader.setFloat("uEdgeFadeY", params.edgeFadeY);
    m_displayShader.setInt("uDisplayEdgeMode", params.displayEdgeMode);
    
    m_displayShader.setInt("uContourLevels", params.contourLevels);
    m_displayShader.setFloat("uContourThickness", params.contourThickness);
    m_displayShader.setFloat("uVectorFieldScale", params.vectorFieldScale);
    m_displayShader.setInt("uVectorFieldDensity", params.vectorFieldDensity);
    GLint gcLoc = glGetUniformLocation(m_displayShader.id(), "uGlowColor");
    if (gcLoc >= 0)
        glProgramUniform3f(m_displayShader.id(), gcLoc, params.glowR, params.glowG, params.glowB);
    m_displayShader.setFloat("uGlowIntensity", params.glowIntensity);

    glBindTextureUnit(0, stateTexture);
    glBindSampler(0, m_displaySampler);

    GLuint cmapTex = m_colormapTex;
    int cmIdx = params.colormapMode;
    if (cmIdx >= 8 && (cmIdx - 8) < static_cast<int>(m_customColormapTextures.size())) {
        cmapTex = m_customColormapTextures[cmIdx - 8];
    }
    glBindTextureUnit(1, cmapTex);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glBindSampler(0, 0);
}

void Renderer::createEmptyVAO() {
    glCreateVertexArrays(1, &m_vao);
}

void Renderer::generateColormap() {
    constexpr int SIZE = 1024;
    std::vector<float> pixels(SIZE * 4);

    struct ColorStop { float t, r, g, b; };
    std::vector<ColorStop> stops = {
        {0.00f, 0.000f, 0.000f, 0.050f},
        {0.15f, 0.050f, 0.020f, 0.200f},
        {0.30f, 0.100f, 0.050f, 0.400f},
        {0.40f, 0.300f, 0.050f, 0.350f},
        {0.50f, 0.600f, 0.150f, 0.100f},
        {0.60f, 0.900f, 0.400f, 0.050f},
        {0.70f, 1.000f, 0.700f, 0.100f},
        {0.80f, 1.000f, 0.900f, 0.300f},
        {0.90f, 1.000f, 1.000f, 0.600f},
        {1.00f, 1.000f, 1.000f, 1.000f},
    };

    for (int i = 0; i < SIZE; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(SIZE - 1);

        int idx = 0;
        for (int s = 0; s < static_cast<int>(stops.size()) - 1; ++s) {
            if (t >= stops[s].t && t <= stops[s + 1].t) { idx = s; break; }
        }

        float range = stops[idx + 1].t - stops[idx].t;
        float local = (range > 0.0f) ? (t - stops[idx].t) / range : 0.0f;
        local = local * local * (3.0f - 2.0f * local);

        pixels[i * 4 + 0] = stops[idx].r + local * (stops[idx + 1].r - stops[idx].r);
        pixels[i * 4 + 1] = stops[idx].g + local * (stops[idx + 1].g - stops[idx].g);
        pixels[i * 4 + 2] = stops[idx].b + local * (stops[idx + 1].b - stops[idx].b);
        pixels[i * 4 + 3] = 1.0f;
    }

    m_colormapTex = createTexture1D(SIZE, GL_RGBA32F);
    glTextureSubImage1D(m_colormapTex, 0, 0, SIZE, GL_RGBA, GL_FLOAT, pixels.data());
}

GLuint Renderer::loadColormapFromFile(const std::string& path, ColormapData& outData) {
    std::ifstream f(path);
    if (!f.is_open()) return 0;

    std::vector<float> data;
    outData.colors.clear();
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        float r, g, b, a;
        if (iss >> r >> g >> b >> a) {
            data.push_back(r);
            data.push_back(g);
            data.push_back(b);
            data.push_back(a);
            outData.colors.push_back({r, g, b, a});
        }
    }

    int count = static_cast<int>(data.size() / 4);
    if (count < 2) return 0;

    GLuint tex = createTexture1D(count, GL_RGBA32F);
    glTextureSubImage1D(tex, 0, 0, count, GL_RGBA, GL_FLOAT, data.data());
    return tex;
}

void Renderer::loadCustomColormaps(const std::string& colormapDir) {
    namespace fs = std::filesystem;
    if (!fs::exists(colormapDir) || !fs::is_directory(colormapDir)) return;

    std::vector<std::pair<std::string, std::string>> entries;
    for (auto& entry : fs::directory_iterator(colormapDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            std::string stem = entry.path().stem().string();
            entries.emplace_back(stem, entry.path().string());
        }
    }

    std::sort(entries.begin(), entries.end());

    for (auto& [name, path] : entries) {
        ColormapData cmapData;
        GLuint tex = loadColormapFromFile(path, cmapData);
        if (tex != 0) {
            m_customColormapTextures.push_back(tex);
            m_customColormapData.push_back(std::move(cmapData));
            std::string displayName = name;
            auto pos = displayName.find("-colormap");
            if (pos != std::string::npos)
                displayName.erase(pos);
            displayName[0] = static_cast<char>(std::toupper(displayName[0]));
            m_customColormapNames.push_back(displayName);
        }
    }
}

}
