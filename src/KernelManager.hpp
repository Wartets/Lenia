#pragma once

#include <glad/glad.h>
#include "Utils/Shader.hpp"
#include <vector>
#include <cstdint>

namespace lenia {

struct KernelConfig {
    int   radius{13};
    int   numRings{1};
    int   kernelType{0};
    int   kernelModifier{0};
    float ringWeights[16]{1.0f};
    float anisotropyStrength{0.0f};
    float anisotropyAngle{0.0f};
    float timePhase{0.0f};
    float pulseFrequency{0.0f};

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
