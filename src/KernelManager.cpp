#include "KernelManager.hpp"
#include "Utils/GLUtils.hpp"
#include <cstring>

namespace lenia {

KernelManager::~KernelManager() {
    destroyTexture();
    if (m_ubo) glDeleteBuffers(1, &m_ubo);
}

bool KernelManager::init(const std::string& shaderPath) {
    if (!m_shader.loadCompute(shaderPath)) return false;
    ensureUBO();
    return true;
}

void KernelManager::ensureUBO() {
    if (!m_ubo) {
        glCreateBuffers(1, &m_ubo);
        glNamedBufferStorage(m_ubo, sizeof(GPUKernelParams), nullptr, GL_DYNAMIC_STORAGE_BIT);
    }
}

void KernelManager::generate(const KernelConfig& cfg) {
    m_config = cfg;
    m_diameter = cfg.kernelType == 4 ? 3 : cfg.radius * 2;

    destroyTexture();
    m_texture = createTexture2D(m_diameter, m_diameter, GL_R32F);

    GPUKernelParams gpu{};
    gpu.radius              = cfg.radius;
    gpu.numRings            = cfg.numRings;
    gpu.kernelType          = cfg.kernelType;
    gpu.kernelModifier      = cfg.kernelModifier;
    gpu.anisotropyStrength  = cfg.anisotropyStrength;
    gpu.anisotropyAngle     = cfg.anisotropyAngle;
    gpu.timePhase           = cfg.timePhase;
    gpu.pulseFrequency      = cfg.pulseFrequency;
    for (int i = 0; i < 16; ++i) {
        gpu.ringWeights[i][0] = cfg.ringWeights[i];
        gpu.ringWeights[i][1] = 0.0f;
        gpu.ringWeights[i][2] = 0.0f;
        gpu.ringWeights[i][3] = 0.0f;
    }

    glNamedBufferSubData(m_ubo, 0, sizeof(GPUKernelParams), &gpu);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);

    m_shader.use();
    glBindImageTexture(0, m_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    dispatchCompute2D(m_diameter, m_diameter);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    if (cfg.kernelType != 4) {
        normalizeKernel();
    }
}

void KernelManager::updateTimePhase(float phase) {
    if (!m_ubo) return;
    m_config.timePhase = phase;
    
    GPUKernelParams gpu{};
    gpu.radius              = m_config.radius;
    gpu.numRings            = m_config.numRings;
    gpu.kernelType          = m_config.kernelType;
    gpu.kernelModifier      = m_config.kernelModifier;
    gpu.anisotropyStrength  = m_config.anisotropyStrength;
    gpu.anisotropyAngle     = m_config.anisotropyAngle;
    gpu.timePhase           = phase;
    gpu.pulseFrequency      = m_config.pulseFrequency;
    for (int i = 0; i < 16; ++i) {
        gpu.ringWeights[i][0] = m_config.ringWeights[i];
        gpu.ringWeights[i][1] = 0.0f;
        gpu.ringWeights[i][2] = 0.0f;
        gpu.ringWeights[i][3] = 0.0f;
    }

    glNamedBufferSubData(m_ubo, 0, sizeof(GPUKernelParams), &gpu);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);

    m_shader.use();
    glBindImageTexture(0, m_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    dispatchCompute2D(m_diameter, m_diameter);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    if (m_config.kernelType != 4) {
        normalizeKernel();
    }
}

void KernelManager::normalizeKernel() {
    int count = m_diameter * m_diameter;
    std::vector<float> data(count);
    glGetTextureImage(m_texture, 0, GL_RED, GL_FLOAT,
                      count * sizeof(float), data.data());

    double sum = 0.0;
    for (float v : data) sum += v;

    if (sum > 1e-9) {
        float invSum = static_cast<float>(1.0 / sum);
        for (float& v : data) v *= invSum;
        glTextureSubImage2D(m_texture, 0, 0, 0, m_diameter, m_diameter,
                            GL_RED, GL_FLOAT, data.data());
    }
}

void KernelManager::destroyTexture() {
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
}

}
