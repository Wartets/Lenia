#include "Shader.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

namespace lenia {

Shader::~Shader() { destroy(); }

Shader::Shader(Shader&& other) noexcept : m_program(other.m_program) {
    other.m_program = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        destroy();
        m_program = other.m_program;
        other.m_program = 0;
        m_locationCache.clear();
    }
    return *this;
}

void Shader::destroy() {
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

bool Shader::loadCompute(const std::string& path) {
    destroy();
    std::string src = readFile(path);
    if (src.empty()) return false;

    GLuint cs = compileStage(GL_COMPUTE_SHADER, src, path);
    if (!cs) return false;

    m_program = glCreateProgram();
    glAttachShader(m_program, cs);
    bool ok = linkProgram(m_program, path);
    glDeleteShader(cs);

    if (!ok) { destroy(); return false; }
    return true;
}

bool Shader::loadGraphics(const std::string& vertPath, const std::string& fragPath) {
    destroy();
    std::string vSrc = readFile(vertPath);
    std::string fSrc = readFile(fragPath);
    if (vSrc.empty() || fSrc.empty()) return false;

    GLuint vs = compileStage(GL_VERTEX_SHADER, vSrc, vertPath);
    GLuint fs = compileStage(GL_FRAGMENT_SHADER, fSrc, fragPath);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    bool ok = linkProgram(m_program, vertPath + "+" + fragPath);
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!ok) { destroy(); return false; }
    return true;
}

void Shader::use() const { glUseProgram(m_program); }

void Shader::setInt(const std::string& name, int v) const {
    glProgramUniform1i(m_program, getLocation(name), v);
}

void Shader::setFloat(const std::string& name, float v) const {
    glProgramUniform1f(m_program, getLocation(name), v);
}

GLint Shader::getLocation(const std::string& name) const {
    auto it = m_locationCache.find(name);
    if (it != m_locationCache.end()) return it->second;
    GLint loc = glGetUniformLocation(m_program, name.c_str());
    m_locationCache[name] = loc;
    return loc;
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Shader file not found: %s", path.c_str());
        return {};
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint Shader::compileStage(GLenum type, const std::string& source, const std::string& label) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        LOG_ERROR("Shader compile error in %s:\n%s", label.c_str(), log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::linkProgram(GLuint program, const std::string& label) {
    glLinkProgram(program);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len);
        glGetProgramInfoLog(program, len, nullptr, log.data());
        LOG_ERROR("Shader link error in %s:\n%s", label.c_str(), log.data());
        return false;
    }
    return true;
}

}
