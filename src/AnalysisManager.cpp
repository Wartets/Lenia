/**
 * @file AnalysisManager.cpp
 * @brief Implementation of simulation analysis and pattern detection.
 */

#include "AnalysisManager.hpp"
#include "Utils/GLUtils.hpp"
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>

namespace lenia {

AnalysisManager::~AnalysisManager() {
    if (m_ssbo) glDeleteBuffers(1, &m_ssbo);
    if (m_ubo) glDeleteBuffers(1, &m_ubo);
    if (m_sampler) glDeleteSamplers(1, &m_sampler);
}

bool AnalysisManager::init(const std::string& shaderPath) {
    if (!m_shader.loadCompute(shaderPath)) return false;

    glCreateBuffers(1, &m_ssbo);
    glNamedBufferStorage(m_ssbo, sizeof(AnalysisData), nullptr,
                         GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);

    glCreateBuffers(1, &m_ubo);
    glNamedBufferStorage(m_ubo, sizeof(GPUAnalysisParams), nullptr, GL_DYNAMIC_STORAGE_BIT);

    glCreateSamplers(1, &m_sampler);
    glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return true;
}

void AnalysisManager::analyze(GLuint stateTexture, int gridW, int gridH, float threshold) {
    AnalysisData zero{};
    glNamedBufferSubData(m_ssbo, 0, sizeof(AnalysisData), &zero);

    GPUAnalysisParams params{};
    params.gridW = gridW;
    params.gridH = gridH;
    params.threshold = threshold;
    params.pass = 0;
    glNamedBufferSubData(m_ubo, 0, sizeof(GPUAnalysisParams), &params);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_ubo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);

    m_shader.use();
    glBindTextureUnit(0, stateTexture);
    glBindSampler(0, m_sampler);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindSampler(0, 0);

    void* ptr = glMapNamedBufferRange(m_ssbo, 0, sizeof(AnalysisData), GL_MAP_READ_BIT);
    if (ptr) {
        std::memcpy(&m_data, ptr, sizeof(AnalysisData));
        glUnmapNamedBuffer(m_ssbo);
    }

    m_massHistory[m_historyHead] = m_data.totalMass;
    m_aliveHistory[m_historyHead] = static_cast<float>(m_data.aliveCount);
    m_centroidXHistory[m_historyHead] = m_data.centroidX;
    m_centroidYHistory[m_historyHead] = m_data.centroidY;

    computeMovementAndOrientation();

    m_speedHistory[m_historyHead] = m_movementSpeed;
    m_directionHistory[m_historyHead] = m_movementDirection;
    m_orientationHistory[m_historyHead] = m_orientation;

    m_historyHead = (m_historyHead + 1) % HISTORY_SIZE;
    if (m_historyCount < HISTORY_SIZE) m_historyCount++;

    m_empty = (m_data.aliveCount == 0);
    m_stabilized = false;

    if (m_historyCount >= STABLE_WINDOW) {
        float maxMass = -1e30f;
        float minMass = 1e30f;
        for (int i = 0; i < STABLE_WINDOW; ++i) {
            int idx = (m_historyHead - 1 - i + HISTORY_SIZE) % HISTORY_SIZE;
            float m = m_massHistory[idx];
            if (m > maxMass) maxMass = m;
            if (m < minMass) minMass = m;
        }
        float range = maxMass - minMass;
        float avg = (maxMass + minMass) * 0.5f;
        float relRange = (avg > 1e-6f) ? range / avg : range;
        m_stabilized = (relRange < 0.001f) && !m_empty;
    }

    m_analyzeCounter++;
    if (m_analyzeCounter % PERIOD_CHECK_INTERVAL == 0) {
        detectPeriodicity();
    }
}

/**
 * @brief Detect periodic behavior using autocorrelation.
 * 
 * Computes autocorrelation of mass history at various lags
 * to find repeating patterns. A high correlation at a specific
 * lag indicates the pattern has that period.
 */
void AnalysisManager::detectPeriodicity() {
    m_periodic = false;
    m_period = 0;
    m_periodConfidence = 0.0f;

    int n = m_historyCount;
    if (n < MIN_PERIOD * 3) return;  // Need enough data

    int maxLag = std::min(MAX_PERIOD, n / 2);
    if (maxLag < MIN_PERIOD) return;

    // Compute mean of mass history
    float mean = 0.0f;
    for (int i = 0; i < n; ++i) {
        int idx = (m_historyHead - n + i + HISTORY_SIZE) % HISTORY_SIZE;
        mean += m_massHistory[idx];
    }
    mean /= static_cast<float>(n);

    // Compute variance
    float var = 0.0f;
    for (int i = 0; i < n; ++i) {
        int idx = (m_historyHead - n + i + HISTORY_SIZE) % HISTORY_SIZE;
        float d = m_massHistory[idx] - mean;
        var += d * d;
    }
    if (var < 1e-10f) return;  // Constant signal, no periodicity

    // Find lag with highest autocorrelation
    int bestLag = 0;
    float bestCorr = -1.0f;

    for (int lag = MIN_PERIOD; lag <= maxLag; ++lag) {
        float corr = 0.0f;
        int count = n - lag;
        for (int i = 0; i < count; ++i) {
            int idx1 = (m_historyHead - n + i + HISTORY_SIZE) % HISTORY_SIZE;
            int idx2 = (m_historyHead - n + i + lag + HISTORY_SIZE) % HISTORY_SIZE;
            corr += (m_massHistory[idx1] - mean) * (m_massHistory[idx2] - mean);
        }
        corr /= var;
        if (corr > bestCorr) {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    if (bestCorr >= PERIOD_THRESHOLD && bestLag >= MIN_PERIOD) {
        m_periodic = true;
        m_period = bestLag;
        m_periodConfidence = bestCorr;
    }
}

void AnalysisManager::computeMovementAndOrientation() {
    if (m_data.aliveCount == 0) {
        m_movementSpeed = 0.0f;
        m_movementDirection = 0.0f;
        m_orientation = 0.0f;
        m_hasPrevCentroid = false;
        return;
    }

    if (m_hasPrevCentroid) {
        float dx = m_data.centroidX - m_prevCentroidX;
        float dy = m_data.centroidY - m_prevCentroidY;
        m_movementSpeed = std::sqrt(dx * dx + dy * dy);
        m_movementDirection = std::atan2(dy, dx) * (180.0f / 3.14159265f);
    } else {
        m_movementSpeed = 0.0f;
        m_movementDirection = 0.0f;
    }

    m_prevCentroidX = m_data.centroidX;
    m_prevCentroidY = m_data.centroidY;
    m_hasPrevCentroid = true;

    float bw = m_data.boundMaxX - m_data.boundMinX;
    float bh = m_data.boundMaxY - m_data.boundMinY;
    if (bw > 0.001f || bh > 0.001f) {
        m_orientation = std::atan2(bh, bw) * (180.0f / 3.14159265f);
    } else {
        m_orientation = 0.0f;
    }

    if (m_data.aliveCount > 0 && m_data.totalPixels > 0) {
        float density = static_cast<float>(m_data.aliveCount) / static_cast<float>(m_data.totalPixels);
        float area = std::max(1.0f, bw * bh);
        float speciesArea = 3.14159265f * 10.0f * 10.0f;
        int est = std::max(1, static_cast<int>(area / std::max(speciesArea, 1.0f)));
        if (density > 0.3f) est = 1;
        m_speciesCount = std::min(est, m_data.aliveCount);
    } else {
        m_speciesCount = 0;
    }
}

}
