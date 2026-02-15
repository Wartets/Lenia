#pragma once

#include <glad/glad.h>
#include "Utils/Shader.hpp"
#include "UIOverlay.hpp"
#include <string>
#include <vector>

namespace lenia {

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

    Shader& displayShader() { return m_displayShader; }

private:
    Shader m_displayShader;
    GLuint m_vao{0};
    GLuint m_colormapTex{0};
    GLuint m_displaySampler{0};
    std::vector<GLuint> m_customColormapTextures;
    std::vector<std::string> m_customColormapNames;

    void createEmptyVAO();
    void generateColormap();
    GLuint loadColormapFromFile(const std::string& path);
};

}
