#pragma once

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <functional>
#include <utility>
#include <vector>
#include <string>
#include <array>

namespace lenia {

struct Preset;
struct AnalysisData;
class AnalysisManager;

struct ChannelKernelRule {
    float ringWeights[16]{1.0f};
    float mu{0.15f};
    float sigma{0.015f};
    float growthStrength{1.0f};
    float radiusFraction{1.0f};
    int   numRings{1};
    int   sourceChannel{0};
    int   destChannel{0};
    int   kernelType{0};
    int   growthType{0};
};

struct LeniaParams {
    float mu{0.15f};
    float sigma{0.015f};
    float dt{0.1f};
    int   radius{13};
    int   numRings{1};
    float ringWeights[16]{1.0f};
    int   kernelType{0};
    int   growthType{0};
    int   gridW{478};
    int   gridH{478};
    int   noiseMode{0};
    float noiseParam1{0.0f};
    float noiseParam2{0.0f};
    float noiseParam3{0.0f};
    float noiseParam4{0.0f};
    int   colormapMode{0};
    float zoom{1.0f};
    float panX{0.0f};
    float panY{0.0f};
    float brightness{0.5f};
    float contrast{1.0f};
    int   filterMode{0};
    float glowStrength{0.0f};
    float edgeStrength{0.0f};
    float trailDecay{0.0f};
    bool  showGrid{false};
    float gridOpacity{0.2f};
    float bgR{0.0f};
    float bgG{0.0f};
    float bgB{0.05f};
    bool  showAnalysis{false};
    bool  showKernelPreview{true};
    float analysisThreshold{0.01f};
    float gamma{1.0f};
    bool  invertColors{false};
    int   numChannels{1};
    int   numKernelRules{0};
    ChannelKernelRule kernelRules[16];

    bool  autoPause{true};
    bool  showMassGraph{true};
    bool  showAliveGraph{true};
    bool  showCentroidGraph{false};
    bool  showSpeedGraph{false};
    bool  showDirectionGraph{false};
    int   graphTimeWindow{0};
    float graphHeight{100.0f};
    bool  graphAutoScale{true};
    float graphMassMax{0.0f};
    float graphAliveMax{0.0f};

    int   placementMode{0};
    int   placementCount{1};
    bool  placementFlipH{false};
    bool  placementFlipV{false};
    bool  placementRandomFlip{true};
    float placementSpacing{0.1f};
    int   speciesPresetIdx{-1};
    float placementMargin{0.05f};
    float placementScale{1.0f};
    int   placementRotation{0};
    bool  placementClearFirst{true};
    int   placementMinSeparation{0};

    float gridLineR{0.5f};
    float gridLineG{0.5f};
    float gridLineB{0.6f};
    float gridLineThickness{1.0f};
    int   gridSpacingMode{0};
    int   gridCustomSpacing{1};
    bool  gridMajorLines{false};
    int   gridMajorEvery{10};
    float gridMajorOpacity{0.5f};

    bool  clipToZero{false};
    float clipThreshold{0.001f};
    int   displayMode{0};
    bool  showBoundary{false};
    float boundaryR{1.0f};
    float boundaryG{1.0f};
    float boundaryB{1.0f};
    float boundaryOpacity{0.5f};
    int   boundaryStyle{0};
    float boundaryThickness{2.0f};
    bool  boundaryGlow{false};
    bool  boundaryAnimate{false};
    float boundaryDashLength{10.0f};
    
    int   multiChannelBlend{0};
    float channelWeightR{1.0f};
    float channelWeightG{1.0f};
    float channelWeightB{1.0f};
    bool  useColormapForMultichannel{false};
    
    int   contourLevels{10};
    float contourThickness{1.0f};
    float vectorFieldScale{1.0f};
    int   vectorFieldDensity{20};
    float activityDecay{0.95f};
    float glowR{1.0f};
    float glowG{1.0f};
    float glowB{1.0f};
    float glowIntensity{1.0f};
    int   gradientStops{5};
    float gradientColors[15]{0.0f, 0.0f, 0.2f, 0.2f, 0.0f, 0.5f, 0.5f, 0.2f, 0.1f, 0.8f, 0.6f, 0.1f, 1.0f, 1.0f, 0.8f};

    float cmapOffset{0.0f};
    float cmapRange0{0.0f};
    float cmapRange1{1.0f};
    float cmapPower{1.0f};
    float cmapHueShift{0.0f};
    float cmapSaturation{1.0f};
    bool  cmapReverse{false};

    float blurStrength{0.0f};
    float sharpenStrength{0.0f};
    float embossStrength{0.0f};
    int   postFilterMode{0};

    int   edgeModeX{0};
    int   edgeModeY{0};
    float edgeValueX{0.0f};
    float edgeValueY{0.0f};
    float edgeFadeX{0.1f};
    float edgeFadeY{0.1f};
    int   displayEdgeMode{0};

    // Kernel advanced features
    float kernelAnisotropy{0.0f};
    float kernelAnisotropyAngle{0.0f};
    bool  kernelTimeVarying{false};
    float kernelPulseFrequency{0.0f};
    int   kernelModifier{0};
    
    // Memory/GPU monitoring
    bool  showResourceMonitor{false};
    int   gpuMemoryUsedMB{0};
    int   gpuMemoryTotalMB{0};
    float gpuUtilization{0.0f};
    float cpuMemoryUsedMB{0.0f};

    bool  infiniteWorldMode{false};
    int   chunkSize{128};
    int   loadedChunksRadius{2};
    int   viewChunkX{0};
    int   viewChunkY{0};
    float worldExploreSpeed{1.0f};
    bool  autoLoadChunks{true};
    int   maxLoadedChunks{25};
    bool  chunkBoundaryVisible{false};
    int   chunkPersistence{0};
    float chunkFadeDistance{2.0f};

    int   brushShape{0};
    int   brushSize{10};
    float brushStrength{1.0f};
    float brushFalloff{0.5f};
    int   brushMode{0};
    float brushValue{1.0f};
    int   brushChannel{0};
    bool  brushSymmetryX{false};
    bool  brushSymmetryY{false};
    bool  brushSymmetryRadial{false};
    int   brushRadialCount{4};
    float brushNoiseAmount{0.0f};
    float brushRotation{0.0f};
    bool  brushRandomRotation{false};
    int   brushBlendMode{0};
    float brushSpacing{1.0f};
    bool  brushEnabled{false};
    int   brushPattern{0};
    float brushDensity{1.0f};
    bool  brushSmooth{false};
    float brushJitter{0.0f};

    int   brushDrawMode{0};
    int   brushLineStartX{-1};
    int   brushLineStartY{-1};
    int   brushLineEndX{-1};
    int   brushLineEndY{-1};
    bool  brushLineDrawing{false};
    int   brushCtrl1X{-1};
    int   brushCtrl1Y{-1};
    int   brushCtrl2X{-1};
    int   brushCtrl2Y{-1};
    bool  brushCtrl1Set{false};
    bool  brushCtrl2Set{false};
    int   brushCurvePoints{100};
    int   brushCurveType{0};
    float brushCurveAmplitude{0.1f};
    float brushCurveFrequency{5.0f};
    float brushCurvePhase{0.0f};
    bool  brushPreview{true};
    float brushAngleSnap{0.0f};
    bool  brushConstrainAxis{false};
    int   brushPolygonSides{6};
    bool  brushPolygonConnect{true};
    bool  brushPolygonFilled{false};
    std::vector<float> brushPolygonVertices;
    float brushStarInnerRatio{0.5f};
    float brushSpiralTurns{3.0f};
    float brushSpiralGrowth{1.0f};
    bool  brushFilled{true};
    float brushOutlineWidth{1.0f};
    int   brushGradientMode{0};
    float brushGradientAngle{0.0f};
    bool  brushPressureSensitivity{false};
    float brushPressureMin{0.1f};
    float brushPressureMax{1.0f};
    float brushAspectRatio{1.0f};
    bool  brushAntiAlias{false};
    float brushSoftness{0.0f};
    int   brushTextureMode{0};
    float brushTextureScale{1.0f};
    float brushTextureRotation{0.0f};
    float brushGradientStart[3]{0.0f, 0.0f, 0.0f};
    float brushGradientEnd[3]{1.0f, 1.0f, 1.0f};

    bool  wallEnabled{false};
    int   wallDrawMode{0};
    float wallR{0.5f};
    float wallG{0.5f};
    float wallB{0.5f};
    float wallA{1.0f};
    float wallThickness{3.0f};
    float wallValue{1.0f};
    int   wallType{0};
    bool  wallAffectsAllChannels{true};
    bool  wallAffectsCh0{true};
    bool  wallAffectsCh1{true};
    bool  wallAffectsCh2{true};
    int   wallChannel{0};
    float wallDamping{1.0f};
    float wallReflection{0.0f};
    float wallAbsorption{1.0f};
    bool  wallSolid{true};
    float wallPermeability{0.0f};
    int   wallShape{0};
    float wallFalloff{0.0f};
    bool  wallInvert{false};
    float wallNoiseAmount{0.0f};
    int   wallBlendMode{0};
    bool  wallPreview{true};
    int   wallStartX{-1};
    int   wallStartY{-1};
    int   wallEndX{-1};
    int   wallEndY{-1};
    bool  wallLineDrawing{false};
    int   wallCtrl1X{-1};
    int   wallCtrl1Y{-1};
    int   wallCtrl2X{-1};
    int   wallCtrl2Y{-1};
    int   wallCurveType{0};
    int   wallPolygonSides{4};
    float wallRadius{50.0f};
    bool  wallFilled{false};
    float wallOutlineOnly{0.0f};
    std::vector<float> wallPolygonVertices;

    bool  showConsoleOnStartup{true};
};

struct UICallbacks {
    std::function<void()> onReset;
    std::function<void()> onClear;
    std::function<void()> onRandomize;
    std::function<void()> onKernelChanged;
    std::function<void()> onGridResized;
    std::function<void(int)> onPresetSelected;
    std::function<void(int)> onKernelPresetSelected;
    std::function<void(int)> onChannelModeChanged;
    std::function<void(int)> onRuleKernelChanged;
    std::function<std::pair<GLuint,int>(int)> getRuleKernelInfo;
    std::function<void(bool)> onFlipHorizontal;
    std::function<void(bool)> onFlipVertical;
    std::function<void(int)> onRotateGrid;
    std::function<float(int,int)> getCellValue;
    std::function<std::vector<float>(int, int&, int&, int&)> getSpeciesPreviewData;
    std::function<void(int, int, const LeniaParams&)> onBrushApply;
    std::function<void(int, int, int, int, const LeniaParams&)> onBrushLine;
    std::function<void(const std::vector<std::pair<int,int>>&, const LeniaParams&)> onBrushCurve;
    std::function<void(int, int, const LeniaParams&)> onWallApply;
    std::function<void(int, int, int, int, const LeniaParams&)> onWallLine;
    std::function<void(const std::vector<std::pair<int,int>>&, const LeniaParams&)> onWallCurve;
    std::function<void()> onClearWalls;
};

class UIOverlay {
public:
    UIOverlay() = default;
    ~UIOverlay();

    UIOverlay(const UIOverlay&) = delete;
    UIOverlay& operator=(const UIOverlay&) = delete;

    bool init(GLFWwindow* window);
    void beginFrame();
    void render(LeniaParams& params, bool& paused, int& stepsPerFrame, bool& showUI,
                const AnalysisData* analysis, const AnalysisManager* analysisMgr,
                GLuint kernelTex, int kernelDiam, int stepCount, float simTimeMs,
                int mouseGridX, int mouseGridY, float mouseValue, bool mouseInGrid);
    void endFrame();
    void shutdown();

    void setCallbacks(UICallbacks cb) { m_callbacks = std::move(cb); }
    void setPresetNames(const std::vector<std::string>& names) { m_presetNames = names; }
    void setKernelPresetNames(const std::vector<std::string>& names) { m_kernelPresetNames = names; }
    void setCustomColormapNames(const std::vector<std::string>& names) { m_customColormapNames = names; }
    void setCustomColormapData(const std::vector<std::vector<std::array<float, 4>>>& data) { m_customColormapData = data; }
    int selectedPreset() const { return m_selectedPreset; }
    void setSelectedPreset(int idx) { m_selectedPreset = idx; }
    void setSelectedCategory(int idx) { m_selectedCategory = idx; }
    void triggerPauseOverlay(bool isPaused);
    void updatePauseOverlay(float deltaTime);
    void renderPauseOverlay(int windowW, int windowH);

private:
    bool                     m_initialized{false};
    UICallbacks              m_callbacks;
    std::vector<std::string> m_presetNames;
    std::vector<std::string> m_kernelPresetNames;
    std::vector<std::string> m_customColormapNames;
    std::vector<std::vector<std::array<float, 4>>> m_customColormapData;
    int                      m_selectedPreset{0};
    int                      m_selectedKernelPreset{0};
    int                      m_selectedCategory{0};
    char                     m_presetSearchBuf[128]{};

    float m_frameTimeHistory[120]{};
    int   m_frameTimeHead{0};
    int   m_frameTimeCount{0};
    float m_gpuTimeHistory[120]{};
    int   m_gpuTimeHead{0};

    bool  m_sectionDetached[12]{};
    float m_pauseOverlayAlpha{0.0f};
    bool  m_pauseOverlayPlaying{false};
    bool  m_lastPausedState{true};

    void drawGrowthPlot(LeniaParams& params);
    void drawKernelCrossSection(GLuint kernelTex, int kernelDiam);
    void drawColorbar(const LeniaParams& params);
    void drawPresetPreview(const Preset& preset, float size, const LeniaParams& params);
    void drawKernelPreview(GLuint kernelTex, int kernelDiam, float size);
    void drawGraphWithAxes(const char* label, const float* data, int count, float yMin, float yMax,
                           const char* xLabel, const char* yLabel, float height, unsigned int lineColor);
    bool sectionHeader(const char* label, int sectionIdx, bool defaultOpen = false);
};

}
