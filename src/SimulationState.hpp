/**
 * @file SimulationState.hpp
 * @brief Double-buffered GPU texture state for cellular automaton simulation.
 */

#pragma once

#include <glad/glad.h>
#include <cstdint>
#include <vector>

namespace lenia {

/**
 * @brief Manages double-buffered GPU textures for ping-pong rendering.
 * 
 * The simulation uses two textures: one for reading the current state
 * and one for writing the next state. After each simulation step,
 * the textures are swapped. This allows efficient GPU-based updates
 * without read-write hazards.
 */
class SimulationState {
public:
    SimulationState() = default;
    ~SimulationState();

    SimulationState(const SimulationState&) = delete;
    SimulationState& operator=(const SimulationState&) = delete;

    void init(int width, int height, GLenum internalFormat = GL_R32F);
    void resize(int width, int height);
    void swap();
    void clear();
    void uploadRegion(int dstX, int dstY, int w, int h, const float* data);
    void uploadRegionRGBA(int dstX, int dstY, int w, int h, const float* data);

    GLuint currentTexture() const { return m_textures[m_current]; }
    GLuint nextTexture()    const { return m_textures[1 - m_current]; }

    int width()  const { return m_width; }
    int height() const { return m_height; }
    GLenum format() const { return m_format; }

private:
    GLuint m_textures[2]{0, 0};  // Double-buffered state textures
    int    m_current{0};          // Index of current read texture (0 or 1)
    int    m_width{0};            // Grid width in cells
    int    m_height{0};           // Grid height in cells
    GLenum m_format{GL_R32F};     // Texture format (R32F or RGBA32F)

    void createTextures();
    void destroyTextures();
};

}
