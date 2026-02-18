/**
 * @file Shader.hpp
 * @brief OpenGL shader program wrapper for loading and managing shaders.
 */

#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>

namespace lenia {

/**
 * @brief Wrapper for OpenGL shader programs.
 * 
 * Supports both compute shaders (for simulation) and graphics
 * shaders (vertex + fragment for rendering). Provides uniform
 * caching for efficient parameter updates.
 */
class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool loadCompute(const std::string& path);
    bool loadGraphics(const std::string& vertPath, const std::string& fragPath);

    void use() const;
    GLuint id() const { return m_program; }

    void setInt(const std::string& name, int v) const;
    void setFloat(const std::string& name, float v) const;

private:
    GLuint m_program{0};
    mutable std::unordered_map<std::string, GLint> m_locationCache;

    GLint getLocation(const std::string& name) const;
    static std::string readFile(const std::string& path);
    static GLuint compileStage(GLenum type, const std::string& source, const std::string& label);
    static bool linkProgram(GLuint program, const std::string& label);
    void destroy();
};

}
