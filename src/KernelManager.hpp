/**
 * @file KernelManager.hpp
 * @brief GPU-accelerated kernel generation for Lenia convolution.
 * 
 * The kernel defines the neighborhood weighting for the convolution
 * operation in the Lenia update equation: K * A^t
 */

#pragma once

#include <glad/glad.h>
#include "Utils/Shader.hpp"
#include <vector>
#include <cstdint>

namespace lenia {

/**
 * @brief Configuration parameters for kernel generation.
 */
struct KernelConfig {
    int   radius{13};              // Kernel radius in cells
    int   numRings{1};             // Number of concentric rings
    int   kernelType{0};           // Shape type (GaussianShell, Bump4, etc.)
    int   kernelModifier{0};       // Additional shape modifier
    float ringWeights[16]{1.0f};   // Weight for each ring (up to 16)
    float anisotropyStrength{0.0f};// Directional asymmetry strength
    float anisotropyAngle{0.0f};   // Angle of anisotropy
    float timePhase{0.0f};         // Phase for time-varying kernels
    float pulseFrequency{0.0f};    // Frequency for pulsing kernels

    bool operator==(const KernelConfig& o) const {
        if (radius != o.radius || numRings != o.numRings || kernelType != o.kernelType) return false;
        if (kernelModifier != o.kernelModifier) return false;
        if (anisotropyStrength != o.anisotropyStrength || anisotropyAngle != o.anisotropyAngle) return false;
        if (pulseFrequency != o.pulseFrequency) return false;
        for (int i = 0; i < numRings; ++i)
            if (ringWeights[i] != o.ringWeights[i]) return false;
        return true;
    }
    bool operator!=(const KernelConfig& o) const { return !(*this == o); }
};

/**
 * @brief Generates and manages convolution kernels on the GPU.
 * 
 * The kernel is stored as a 2D texture representing the weights
 * for each cell in the neighborhood. Kernels are automatically
 * normalized so their sum equals 1.0.
 * 
 * For Lenia, common kernel shapes include:
 * - Gaussian shell: exp(-(r/R - 0.5)^2 / (2σ^2))
 * - Multi-ring: Multiple concentric Gaussian shells
 * - Game of Life: 3x3 Moore neighborhood
 */
class KernelManager {
public:
    KernelManager() = default;
    ~KernelManager();

    KernelManager(const KernelManager&) = delete;
    KernelManager& operator=(const KernelManager&) = delete;

    bool init(const std::string& shaderPath);
    void generate(const KernelConfig& cfg);
    void updateTimePhase(float phase);

    GLuint texture() const { return m_texture; }
    int diameter()   const { return m_diameter; }

    const KernelConfig& config() const { return m_config; }
    bool needsTimeUpdate() const { return m_config.pulseFrequency > 0.001f; }

private:
    Shader       m_shader;
    GLuint       m_texture{0};
    GLuint       m_ubo{0};
    int          m_diameter{0};
    KernelConfig m_config{};

    void destroyTexture();
    void ensureUBO();
    void normalizeKernel();

    struct alignas(16) GPUKernelParams {
        int32_t radius;
        int32_t numRings;
        int32_t kernelType;
        int32_t kernelModifier;
        float   ringWeights[16][4];
        float   anisotropyStrength;
        float   anisotropyAngle;
        float   timePhase;
        float   pulseFrequency;
    };
};

}
