/**
 * @file KernelManager.cpp
 * @brief Implementation of GPU kernel generation.
 */

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

/**
 * @brief Generate kernel texture using compute shader.
 * 
 * Creates a 2D texture representing the convolution kernel.
 * The kernel is then normalized so all values sum to 1.0.
 */
void KernelManager::generate(const KernelConfig& cfg) {
    m_config = cfg;
    // GoL kernel is always 3x3, others use 2*radius
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

/**
 * @brief Normalize kernel so all values sum to 1.0.
 * 
 * This ensures the convolution produces values in the expected
 * range regardless of kernel size or ring weights.
 */
void KernelManager::normalizeKernel() {
    int count = m_diameter * m_diameter;
    std::vector<float> data(count);
    glGetTextureImage(m_texture, 0, GL_RED, GL_FLOAT,
                      count * sizeof(float), data.data());

    // Sum all kernel values
    double sum = 0.0;
    for (float v : data) sum += v;

    // Divide by sum to normalize
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
