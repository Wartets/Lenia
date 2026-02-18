/**
 * @file AnalysisManager.hpp
 * @brief Real-time analysis of simulation state for pattern detection.
 */

#pragma once

#include <glad/glad.h>
#include "Utils/Shader.hpp"
#include <string>

namespace lenia {

/**
 * @brief Statistical data computed from simulation state.
 * 
 * Updated each frame when analysis is enabled. Used for:
 * - Auto-pause on extinction or stabilization
 * - Pattern characterization (size, position, movement)
 * - History graphs in the UI
 */
struct AnalysisData {
    float totalMass{0.0f};     // Sum of all cell values
    float maxVal{0.0f};        // Maximum cell value
    float minVal{0.0f};        // Minimum non-zero cell value
    int   aliveCount{0};       // Number of cells above threshold
    float centroidX{0.0f};     // Center of mass X coordinate
    float centroidY{0.0f};     // Center of mass Y coordinate
    float weightedX{0.0f};     // Weighted centroid X
    float weightedY{0.0f};     // Weighted centroid Y
    int   totalPixels{0};      // Total grid cells
    float avgVal{0.0f};        // Average cell value
    float variance{0.0f};      // Value variance
    float boundMinX{0.0f};     // Bounding box min X
    float boundMinY{0.0f};     // Bounding box min Y
    float boundMaxX{0.0f};     // Bounding box max X
    float boundMaxY{0.0f};     // Bounding box max Y
    int   pad0{0};             // Padding for GPU alignment
};

/**
 * @brief Computes statistics and detects patterns in simulation state.
 * 
 * Uses a GPU compute shader to efficiently analyze the entire grid,
 * then performs CPU-side periodicity detection and movement tracking.
 */
class AnalysisManager {
public:
    AnalysisManager() = default;
    ~AnalysisManager();

    AnalysisManager(const AnalysisManager&) = delete;
    AnalysisManager& operator=(const AnalysisManager&) = delete;

    bool init(const std::string& shaderPath);
    void analyze(GLuint stateTexture, int gridW, int gridH, float threshold = 0.01f);
    const AnalysisData& data() const { return m_data; }

    float massHistory(int i) const { return m_massHistory[i % HISTORY_SIZE]; }
    float aliveHistory(int i) const { return m_aliveHistory[i % HISTORY_SIZE]; }
    float centroidXHistory(int i) const { return m_centroidXHistory[i % HISTORY_SIZE]; }
    float centroidYHistory(int i) const { return m_centroidYHistory[i % HISTORY_SIZE]; }
    int historyCount() const { return m_historyCount; }
    int historyHead() const { return m_historyHead; }
    bool isStabilized() const { return m_stabilized; }
    bool isEmpty() const { return m_empty; }
    bool isPeriodic() const { return m_periodic; }
    int detectedPeriod() const { return m_period; }
    float periodConfidence() const { return m_periodConfidence; }

    float speedHistory(int i) const { return m_speedHistory[i % HISTORY_SIZE]; }
    float directionHistory(int i) const { return m_directionHistory[i % HISTORY_SIZE]; }
    float orientationHistory(int i) const { return m_orientationHistory[i % HISTORY_SIZE]; }
    int speciesCount() const { return m_speciesCount; }
    float movementSpeed() const { return m_movementSpeed; }
    float movementDirection() const { return m_movementDirection; }
    float orientation() const { return m_orientation; }

    // Analysis configuration constants
    static constexpr int HISTORY_SIZE = 512;       // Frames of history to keep
    static constexpr int STABLE_WINDOW = 30;       // Frames to check for stability
    static constexpr int MIN_PERIOD = 4;           // Minimum detectable period
    static constexpr int MAX_PERIOD = 200;         // Maximum detectable period
    static constexpr int PERIOD_CHECK_INTERVAL = 16;  // How often to check periodicity
    static constexpr float PERIOD_THRESHOLD = 0.85f;  // Correlation threshold for period

private:
    Shader m_shader;
    GLuint m_ssbo{0};
    GLuint m_ubo{0};
    AnalysisData m_data;
    float m_massHistory[HISTORY_SIZE]{};
    float m_aliveHistory[HISTORY_SIZE]{};
    float m_centroidXHistory[HISTORY_SIZE]{};
    float m_centroidYHistory[HISTORY_SIZE]{};
    float m_speedHistory[HISTORY_SIZE]{};
    float m_directionHistory[HISTORY_SIZE]{};
    float m_orientationHistory[HISTORY_SIZE]{};
    int m_historyHead{0};
    int m_historyCount{0};
    int m_analyzeCounter{0};
    GLuint m_sampler{0};
    bool m_stabilized{false};
    bool m_empty{false};
    bool m_periodic{false};
    int m_period{0};
    float m_periodConfidence{0.0f};
    int m_speciesCount{0};
    float m_movementSpeed{0.0f};
    float m_movementDirection{0.0f};
    float m_orientation{0.0f};
    float m_prevCentroidX{0.0f};
    float m_prevCentroidY{0.0f};
    bool m_hasPrevCentroid{false};

    void detectPeriodicity();
    void computeMovementAndOrientation();

    struct alignas(16) GPUAnalysisParams {
        int gridW;
        int gridH;
        float threshold;
        int pass;
    };
};

}
