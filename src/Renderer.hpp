/**
 * @file Renderer.hpp
 * @brief GPU-based visualization rendering for Lenia simulation.
 */

#pragma once

#include <glad/glad.h>
#include "Utils/Shader.hpp"
#include "UIOverlay.hpp"
#include <string>
#include <vector>
#include <array>

namespace lenia {

/**
 * @brief Colormap data storage for custom colormaps.
 */
struct ColormapData {
    std::vector<std::array<float, 4>> colors;  // RGBA color stops
};

/**
 * @brief Renders simulation state to screen with colormapping and effects.
 * 
 * Uses a fullscreen triangle with fragment shader for:
 * - Colormap lookup (multiple built-in + custom file-based)
 * - Brightness/contrast/gamma adjustment
 * - Grid overlay rendering
 * - Edge detection and glow effects
 * - Zoom and pan transformations
 */
class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init(const std::string& vertPath, const std::string& fragPath);
    void draw(GLuint stateTexture, int viewportW, int viewportH, const LeniaParams& params);
    void loadCustomColormaps(const std::string& colormapDir);
    int customColormapCount() const { return static_cast<int>(m_customColormapNames.size()); }
    const std::vector<std::string>& customColormapNames() const { return m_customColormapNames; }
    const std::vector<ColormapData>& customColormapData() const { return m_customColormapData; }

    Shader& displayShader() { return m_displayShader; }

private:
    Shader m_displayShader;
    GLuint m_vao{0};
    GLuint m_colormapTex{0};
    GLuint m_displaySampler{0};
    std::vector<GLuint> m_customColormapTextures;
    std::vector<std::string> m_customColormapNames;
    std::vector<ColormapData> m_customColormapData;

    void createEmptyVAO();
    void generateColormap();
    GLuint loadColormapFromFile(const std::string& path, ColormapData& outData);
};

}
