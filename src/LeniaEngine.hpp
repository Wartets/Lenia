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

enum class InitMode : int {
    Random         = 0,
    GaussianSpot   = 1,
    GaussianRing   = 2,
    CenterSquare   = 3,
    RandomSquares  = 4,
    Gradient       = 5,
    KernelBlob     = 6,
    RandomBinary   = 7,
    Species        = 8
};

enum class GrowthType : int {
    Lenia           = 0,
    Step            = 1,
    GameOfLife      = 2,
    SmoothLife      = 3,
    Polynomial      = 4,
    Exponential     = 5,
    DoublePeak      = 6,
    Asymptotic      = 7,
    SoftClip        = 8,
    LargerThanLife  = 9,
    Quad4           = 10
};

enum class KernelType : int {
    GaussianShell    = 0,
    Bump4            = 1,
    MultiringGauss   = 2,
    MultiringBump4   = 3,
    GameOfLife       = 4,
    StepUnimodal     = 5,
    CosineShell      = 6,
    MexicanHat       = 7,
    Quad4Kernel      = 8,
    MultiringQuad4   = 9
};

enum class PlacementMode : int {
    Center      = 0,
    TopLeft     = 1,
    TopRight    = 2,
    BottomLeft  = 3,
    BottomRight = 4,
    Top         = 5,
    Bottom      = 6,
    Left        = 7,
    Right       = 8,
    Random      = 9,
    Grid        = 10,
    TwoPlace    = 11,
    Scatter     = 12
};

struct Preset {
    const char*   name;
    const char*   category;
    float         mu;
    float         sigma;
    float         dt;
    int           radius;
    int           numRings;
    float         ringWeights[16];
    KernelType    kernelType;
    GrowthType    growthType;
    InitMode      initMode;
    float         initParam1;
    float         initParam2;
    int           gridW;
    int           gridH;
    const char*   speciesFile;
    PlacementMode placement;
    bool          flipInit;
    int           cellRows;
    int           cellCols;
    const float*  cellData;
};

struct KernelPreset {
    const char* name;
    int         kernelType;
    int         numRings;
    float       ringWeights[16];
    int         radius;
};

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
