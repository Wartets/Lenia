/**
 * @file LeniaEngine.hpp
 * @brief Core simulation engine for Lenia cellular automaton.
 * 
 * Implements GPU-accelerated simulation using OpenGL compute shaders.
 * The engine manages the simulation state, kernel convolution, and
 * growth function application.
 */

#pragma once

#include "SimulationState.hpp"
#include "KernelManager.hpp"
#include "Renderer.hpp"
#include "AnalysisManager.hpp"
#include "UIOverlay.hpp"
#include "Utils/Shader.hpp"
#include <string>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace lenia {

/**
 * @brief Grid initialization modes for resetting the simulation.
 */
enum class InitMode : int {
    Random         = 0,   // Uniform random noise
    GaussianSpot   = 1,   // Gaussian blob at center
    GaussianRing   = 2,   // Ring-shaped Gaussian
    CenterSquare   = 3,   // Solid square at center
    RandomSquares  = 4,   // Multiple random squares
    Gradient       = 5,   // Linear gradient
    KernelBlob     = 6,   // Blob using kernel shape
    RandomBinary   = 7,   // Binary random pattern
    Species        = 8    // Load from species file
};

/**
 * @brief Growth function types that determine cell state evolution.
 * 
 * The growth function g(u) maps the neighborhood sum u to a growth rate.
 * Different functions produce different emergent behaviors.
 */
enum class GrowthType : int {
    Lenia           = 0,  // Standard Lenia: g(u) = 2*exp(-((u-μ)/σ)²/2) - 1
    Step            = 1,  // Step function threshold
    GameOfLife      = 2,  // Conway's GoL rules (discrete)
    SmoothLife      = 3,  // Smooth continuous Game of Life
    Polynomial      = 4,  // Polynomial growth curve
    Exponential     = 5,  // Exponential falloff
    DoublePeak      = 6,  // Two growth peaks
    Asymptotic      = 7,  // Asymptotic saturation
    SoftClip        = 8,  // Soft clipping function
    LargerThanLife  = 9,  // Extended neighborhood rules
    Quad4           = 10  // Quad4 automaton variant
};

/**
 * @brief Kernel types that define neighborhood weighting.
 * 
 * The kernel K(r) determines how neighboring cells are weighted
 * in the convolution. Different kernels produce different patterns.
 */
enum class KernelType : int {
    GaussianShell    = 0,  // Smooth Gaussian ring
    Bump4            = 1,  // Bump function to 4th power
    MultiringGauss   = 2,  // Multiple Gaussian rings
    MultiringBump4   = 3,  // Multiple bump function rings
    GameOfLife       = 4,  // 3x3 Moore neighborhood (GoL)
    StepUnimodal     = 5,  // Step function unimodal
    CosineShell      = 6,  // Cosine-weighted shell
    MexicanHat       = 7,  // Difference of Gaussians (DoG)
    Quad4Kernel      = 8,  // Kernel for Quad4 variant
    MultiringQuad4   = 9   // Multi-ring Quad4
};

/**
 * @brief Species/pattern placement modes for initialization.
 */
enum class PlacementMode : int {
    Center      = 0,   // Single placement at grid center
    TopLeft     = 1,
    TopRight    = 2,
    BottomLeft  = 3,
    BottomRight = 4,
    Top         = 5,
    Bottom      = 6,
    Left        = 7,
    Right       = 8,
    Random      = 9,   // Random positions
    Grid        = 10,  // Regular grid arrangement
    TwoPlace    = 11,  // Two corners (interaction study)
    Scatter     = 12   // Non-overlapping random scatter
};

/**
 * @brief Simulation preset containing all parameters for a specific pattern.
 * 
 * Presets store the configuration needed to reproduce known Lenia creatures
 * and interesting patterns discovered through exploration.
 */
struct Preset {
    const char*   name;           // Display name
    const char*   category;       // Category for UI grouping
    float         mu;             // Growth function center
    float         sigma;          // Growth function width
    float         dt;             // Time step size
    int           radius;         // Kernel radius in cells
    int           numRings;       // Number of kernel rings
    float         ringWeights[16];// Weight for each ring
    KernelType    kernelType;     // Kernel shape type
    GrowthType    growthType;     // Growth function type
    InitMode      initMode;       // Initialization mode
    float         initParam1;     // Init parameter 1
    float         initParam2;     // Init parameter 2
    int           gridW;          // Recommended grid width
    int           gridH;          // Recommended grid height
    const char*   speciesFile;    // NPY file for species data
    PlacementMode placement;      // How to place the species
    bool          flipInit;       // Flip vertically on init
    int           cellRows;       // Embedded cell data rows
    int           cellCols;       // Embedded cell data columns
    const float*  cellData;       // Embedded cell data pointer
};

struct KernelPreset {
    const char* name;
    int         kernelType;
    int         numRings;
    float       ringWeights[16];
    int         radius;
};

/**
 * @brief Core simulation engine implementing Lenia cellular automaton.
 * 
 * The engine uses a double-buffered GPU texture approach:
 * 1. Compute shader reads from current state texture
 * 2. Convolves with kernel to get neighborhood sums
 * 3. Applies growth function to compute state change
 * 4. Writes new state to next texture
 * 5. Swap buffers and repeat
 * 
 * The fundamental Lenia update equation is:
 *   A^(t+dt) = clip[A^t + dt * G(K * A^t), 0, 1]
 * 
 * Where:
 * - A is the cell state (0 to 1)
 * - K is the convolution kernel
 * - G is the growth function
 * - dt is the time step
 */
class LeniaEngine {
public:
    LeniaEngine() = default;
    ~LeniaEngine();

    LeniaEngine(const LeniaEngine&) = delete;
    LeniaEngine& operator=(const LeniaEngine&) = delete;

    bool init(const std::string& assetDir);
    void update(const LeniaParams& params, int steps = 1);
    void updateMultiChannel(const LeniaParams& params, int steps = 1);
    void render(int viewportW, int viewportH, const LeniaParams& params);
    void reset(const LeniaParams& params);
    void clear();
    void regenerateKernel(const LeniaParams& params);
    void regenerateRuleKernel(int ruleIndex, const LeniaParams& params);
    void resizeGrid(const LeniaParams& params);
    void applyPreset(int index, LeniaParams& params);
    void applyKernelPreset(int index, LeniaParams& params);
    void randomizeGrid(const LeniaParams& params);
    void loadCellData(const float* data, int rows, int cols, const LeniaParams& params);
    void loadMultiChannelCellData(const struct MultiChannelPreset& mcp, const LeniaParams& params);
    void runAnalysis(float threshold = 0.01f);
    void switchChannelMode(LeniaParams& params, int numChannels);
    void flipGridHorizontal();
    void flipGridVertical();
    void rotateGrid(int direction, LeniaParams& params);
    float getCellValue(int x, int y) const;
    void applyBrush(int x, int y, const LeniaParams& params);
    void applyBrushLine(int x0, int y0, int x1, int y1, const LeniaParams& params);
    void applyBrushCurve(const std::vector<std::pair<int,int>>& points, const LeniaParams& params);
    void applyWall(int x, int y, const LeniaParams& params);
    void applyWallLine(int x0, int y0, int x1, int y1, const LeniaParams& params);
    void applyWallCurve(const std::vector<std::pair<int,int>>& points, const LeniaParams& params);
    GLuint wallTexture() const { return m_wallTex; }
    void clearWalls();

    SimulationState& state() { return m_state; }
    const AnalysisData& analysisData() const { return m_analysisMgr.data(); }
    const AnalysisManager& analysisMgr() const { return m_analysisMgr; }
    GLuint kernelTexture() const { return m_kernelMgr.texture(); }
    int kernelDiameter() const { return m_kernelMgr.diameter(); }
    GLuint ruleKernelTexture(int idx) const { return (idx >= 0 && idx < 16) ? m_ruleKernels[idx].texture() : 0; }
    int ruleKernelDiameter(int idx) const { return (idx >= 0 && idx < 16) ? m_ruleKernels[idx].diameter() : 0; }
    GLuint neighborSumsTexture() const { return m_neighborSumsTex; }
    GLuint growthTexture() const { return m_growthTex; }
    int stepCount() const { return m_stepCount; }
    void resetStepCount() { m_stepCount = 0; }
    void loadCustomColormaps(const std::string& dir) { m_renderer.loadCustomColormaps(dir); }
    int customColormapCount() const { return m_renderer.customColormapCount(); }
    const std::vector<std::string>& customColormapNames() const { return m_renderer.customColormapNames(); }
    const std::vector<ColormapData>& customColormapData() const { return m_renderer.customColormapData(); }

private:
    SimulationState  m_state;
    KernelManager    m_kernelMgr;
    KernelManager    m_ruleKernels[16];
    Renderer         m_renderer;
    AnalysisManager  m_analysisMgr;
    Shader           m_simShader;
    Shader           m_multiChannelShader;
    Shader           m_noiseShader;
    GLuint           m_simUBO{0};
    GLuint           m_multiUBO{0};
    GLuint           m_noiseUBO{0};
    GLuint           m_stateSampler{0};
    GLuint           m_kernelSampler{0};
    GLuint           m_neighborSumsTex{0};
    GLuint           m_growthTex{0};
    GLuint           m_wallTex{0};
    GLuint           m_debugSampler{0};
    int              m_debugTexW{0};
    int              m_debugTexH{0};
    std::string      m_initDir;
    int              m_stepCount{0};

    struct alignas(16) GPUSimParams {
        int32_t gridW;
        int32_t gridH;
        int32_t radius;
        float   dt;
        float   mu;
        float   sigma;
        int32_t growthType;
        float   param1;
        float   param2;
        float   wallValue;
        int32_t wallEnabled;
        int32_t _pad2;
    };

    struct alignas(16) GPUMultiChannelParams {
        int32_t gridW;
        int32_t gridH;
        int32_t radius;
        float   dt;
        float   mu;
        float   sigma;
        int32_t growthType;
        int32_t sourceChannel;
        int32_t destChannel;
        float   growthStrength;
        int32_t rulePass;
        int32_t numRules;
    };

    struct alignas(16) GPUNoiseParams {
        int32_t  gridW;
        int32_t  gridH;
        int32_t  mode;
        uint32_t seed;
        float    param1;
        float    param2;
        float    param3;
        float    param4;
    };

    void createUBOs();
    void loadSpeciesAndPlace(const LeniaParams& params);
    void ensureDebugTextures(int w, int h);
    void enforceObstacles(const LeniaParams& params);
};

}
