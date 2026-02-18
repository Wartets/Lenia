/**
 * @file Localization.hpp
 * @brief Internationalization (i18n) system for UI text.
 * 
 * Provides translatable strings for all UI elements. Currently supports:
 * - English (default)
 * - French
 * 
 * Usage: Localization::instance().get(TextId::AppTitle)
 */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace lenia {

/**
 * @brief Supported languages for the application
 */
enum class Language {
    English = 0,
    French = 1
};

/**
 * @brief Translation string identifiers
 * 
 * All translatable text in the application should have a corresponding entry here.
 * This ensures compile-time safety and easy identification of missing translations.
 */
enum class TextId {
    // Application title and main UI
    AppTitle,
    MainWindowTitle,
    
    // Section headers
    SectionInfo,
    SectionPerformance,
    SectionGrid,
    SectionDrawingTools,
    SectionPresetsInit,
    SectionSimulation,
    SectionGrowthFunction,
    SectionKernel,
    SectionMultiChannel,
    SectionDisplay,
    SectionAnalysis,
    SectionAccessibility,
    
    // Info section
    InfoCursor,
    InfoValue,
    InfoGrid,
    InfoChannels,
    InfoRules,
    InfoStep,
    InfoShowConsoleStartup,
    InfoShowConsoleTooltip,
    
    // Keybinds
    KeybindsHeader,
    KeybindsText,
    
    // Theory section
    TheoryHeader,
    TheoryFundamentals,
    TheoryFundamentalsText,
    TheoryEquation,
    TheoryKernel,
    TheoryKernelText,
    TheoryGrowthFunction,
    TheoryGrowthFunctionText,
    TheoryTimeIntegration,
    TheoryTimeIntegrationText,
    TheoryMultiChannel,
    TheoryMultiChannelText,
    TheoryEdgeConditions,
    TheoryEdgeConditionsText,
    TheoryWalls,
    TheoryWallsText,
    TheoryPatternCharacteristics,
    TheoryPatternCharacteristicsText,
    TheoryParameterRelationships,
    TheoryParameterRelationshipsText,
    TheoryColormapVisualization,
    TheoryColormapVisualizationText,
    
    // Performance section
    PerfFPS,
    PerfFPSTooltip,
    PerfFrame,
    PerfFrameTime,
    PerfFrameTimeLabel,
    PerfFrameTimeStats,
    PerfGridSize,
    PerfGridSizeCellsM,
    PerfGridSizeCellsK,
    PerfSimulation,
    PerfSimTimeStep,
    PerfThroughput,
    PerfThroughputG,
    PerfThroughputM,
    PerfThroughputK,
    PerfThroughputTooltip,
    PerfKernelOps,
    PerfKernelOpsG,
    PerfKernelOpsM,
    PerfKernelOpsTooltip,
    PerfKernelSize,
    PerfKernelSizeSamples,
    PerfStepsFrame,
    PerfTotalSteps,
    PerfExcellent,
    PerfGood,
    PerfAcceptable,
    PerfSlow,
    PerfPerformance,
    PerfPerformanceTooltip,
    PerfShowResourceMonitor,
    PerfResourceUsage,
    PerfGPUMemory,
    PerfGPUMemoryNA,
    PerfCPUMemory,
    PerfTextureMemory,
    PerfTextureMemoryTooltip,
    PerfFrameTimeGraphTitle,
    PerfFrameTimeGraphXLabel,
    PerfFrameTimeGraphYLabel,
    
    // Grid section
    GridSize,
    GridWidth,
    GridWidthTooltip,
    GridHeight,
    GridHeightTooltip,
    GridTransformations,
    GridFlipHorizontal,
    GridFlipHorizontalTooltip,
    GridFlipVertical,
    GridFlipVerticalTooltip,
    GridRotateCW,
    GridRotateCWTooltip,
    GridRotateCCW,
    GridRotateCCWTooltip,
    GridEdgeConditions,
    GridEdgeModeX,
    GridEdgeModeXTooltip,
    GridEdgeModeY,
    GridEdgeModeYTooltip,
    GridEdgePeriodic,
    GridEdgeClamp,
    GridEdgeMirror,
    GridEdgeFade,
    GridEdgeFadeX,
    GridEdgeFadeXTooltip,
    GridEdgeFadeY,
    GridEdgeFadeYTooltip,
    GridOutsideDisplay,
    GridOutsideDisplayTooltip,
    GridShowTiled,
    GridBackgroundColor,
    GridCheckerPattern,
    
    // Infinite world
    InfiniteWorldMode,
    InfiniteWorldEnable,
    InfiniteWorldEnableTooltip,
    InfiniteWorldSettings,
    InfiniteChunkSize,
    InfiniteChunkSizeTooltip,
    InfiniteLoadRadius,
    InfiniteLoadRadiusTooltip,
    InfiniteMaxChunks,
    InfiniteMaxChunksTooltip,
    InfiniteNavigation,
    InfiniteNavigationTooltip,
    InfiniteChunkPosition,
    InfiniteWorldOffset,
    InfiniteHome,
    InfiniteNavNorth,
    InfiniteNavWest,
    InfiniteNavEast,
    InfiniteNavSouth,
    InfiniteExploreSpeed,
    InfiniteExploreSpeedTooltip,
    InfiniteAutoLoad,
    InfiniteAutoLoadTooltip,
    InfiniteDisplayOptions,
    InfiniteShowChunkGrid,
    InfiniteShowChunkGridTooltip,
    InfiniteEdgeFade,
    InfiniteEdgeFadeTooltip,
    InfinitePersistence,
    InfinitePersistenceTooltip,
    InfinitePersistenceNone,
    InfinitePersistencePreserve,
    InfinitePersistenceSeed,
    InfinitePanTip,
    InfiniteScrollTip,
    
    // Drawing tools
    DrawToolMode,
    DrawToolBrush,
    DrawToolObstacle,
    DrawToolModeTooltip,
    DrawEnableDrawing,
    DrawEnableDrawingTooltip,
    DrawObstacleModeActive,
    DrawBrushModeActive,
    DrawShapeSize,
    DrawShape,
    DrawShapeTooltip,
    DrawShapeCircle,
    DrawShapeSquare,
    DrawShapeDiamond,
    DrawShapeRing,
    DrawShapeStar5,
    DrawShapeStar6,
    DrawShapeHexagon,
    DrawShapeCross,
    DrawShapePlus,
    DrawShapeGaussian,
    DrawShapeNoiseDisc,
    DrawShapeGradientDisc,
    DrawSize,
    DrawSizeTooltip,
    DrawFalloff,
    DrawFalloffTooltip,
    DrawMethod,
    DrawModeFreehand,
    DrawModeLine,
    DrawModeCircle,
    DrawModeRectangle,
    DrawModeTooltip,
    DrawDrawing,
    DrawClickToStart,
    DrawObstacleSettings,
    DrawCellValue,
    DrawCellValueTooltip,
    DrawDisplayColor,
    DrawDisplayColorTooltip,
    DrawAffectedChannels,
    DrawAffectedChannelsTooltip,
    DrawBlendMode,
    DrawBlendModeTooltip,
    DrawBlendReplace,
    DrawBlendMax,
    DrawBlendReplaceStronger,
    DrawBlendBlend,
    DrawBlendErase,
    DrawClearAllObstacles,
    DrawClearAllObstaclesTooltip,
    DrawBrushSettings,
    DrawPaintMode,
    DrawPaintModeTooltip,
    DrawPaintModeSet,
    DrawPaintModeAdd,
    DrawPaintModeSubtract,
    DrawPaintModeMax,
    DrawPaintModeMin,
    DrawPaintModeErase,
    DrawBrushValue,
    DrawBrushValueTooltip,
    DrawStrength,
    DrawStrengthTooltip,
    DrawTargetChannel,
    DrawTargetChannelTooltip,
    DrawSymmetry,
    DrawMirrorX,
    DrawMirrorY,
    DrawMirrorTooltip,
    DrawRadialSymmetry,
    DrawRadialSymmetryTooltip,
    DrawStrokeSpacing,
    DrawBrushSpacing,
    DrawBrushSpacingTooltip,
    DrawSmoothInterpolation,
    DrawSmoothInterpolationTooltip,
    
    // Presets section
    PresetsCategory,
    PresetsSearch,
    PresetsSearchHint,
    PresetsSelected,
    PresetsSpecies,
    PresetsKernel,
    PresetsProcedural,
    PresetsShown,
    PresetsCountShown,
    PresetsRandomize,
    PresetsClear,
    PresetsResetPreset,
    PresetsPlacement,
    PresetsPlacementCenter,
    PresetsPlacementTopLeft,
    PresetsPlacementTopRight,
    PresetsPlacementBottomLeft,
    PresetsPlacementBottomRight,
    PresetsPlacementTop,
    PresetsPlacementBottom,
    PresetsPlacementLeft,
    PresetsPlacementRight,
    PresetsPlacementRandom,
    PresetsPlacementGrid,
    PresetsPlacementTwoPlace,
    PresetsPlacementScatter,
    PresetsCount,
    PresetsScale,
    PresetsRotation,
    PresetsRotation0,
    PresetsRotation90,
    PresetsRotation180,
    PresetsRotation270,
    PresetsMargin,
    PresetsRandomFlip,
    PresetsFlipHorizontal,
    PresetsFlipVertical,
    PresetsPlaceSpacing,
    PresetsMinSeparation,
    PresetsClearGridFirst,
    PresetsApplyPlacement,
    PresetsRadiusRings,
    PresetsMu,
    PresetsSigma,
    
    // Simulation section
    SimPaused,
    SimPausedLabel,
    SimHoldToStep,
    SimStepsPerFrame,
    SimStepsPerFrameTooltip,
    SimStep,
    SimStepFormat,
    SimTime,
    SimTimeMs,
    
    // Growth function section
    GrowthType,
    GrowthTypeTooltip,
    GrowthTypeLenia,
    GrowthTypeStep,
    GrowthTypeGameOfLife,
    GrowthTypeSmoothLife,
    GrowthTypePolynomial,
    GrowthTypeExponential,
    GrowthTypeDoublePeak,
    GrowthTypeAsymptotic,
    GrowthTypeSoftClip,
    GrowthTypeLargerThanLife,
    GrowthTypeQuad4,
    GrowthMu,
    GrowthMuTooltip,
    GrowthSigma,
    GrowthSigmaTooltip,
    GrowthDt,
    GrowthDtTooltip,
    GrowthPlotGoLHint,
    GrowthPlotAsymptoticHint,
    GrowthPlotSoftClipHint,
    GrowthPlotLTLHint,
    GrowthPlotDefaultHint,
    
    // Kernel section
    KernelType,
    KernelTypeTooltip,
    KernelPreset,
    KernelRadius,
    KernelRadiusTooltip,
    KernelRings,
    KernelRingsTooltip,
    KernelRingWeight,
    KernelRingWeightTooltip,
    KernelAdvanced,
    KernelAnisotropy,
    KernelAnisotropyTooltip,
    KernelDirection,
    KernelDirectionTooltip,
    KernelTimeVarying,
    KernelTimeVaryingTooltip,
    KernelPulseFrequency,
    KernelPulseFrequencyTooltip,
    KernelModifier,
    KernelModifierTooltip,
    KernelModifierNone,
    KernelModifierNegativeRing,
    KernelShowPreview,
    KernelPerRuleNote,
    KernelCrossSection,
    KernelCrossSectionWithSize,
    
    // Kernel types
    KernelGaussianShell,
    KernelBump4,
    KernelMultiringGauss,
    KernelMultiringBump4,
    KernelGameOfLife,
    KernelStepUnimodal,
    KernelCosineShell,
    KernelMexicanHat,
    KernelQuad4,
    KernelMultiringQuad4,
    KernelCone,
    KernelTorusDualRing,
    KernelRingSharp,
    KernelGaussianMixture,
    KernelSinc,
    KernelWaveletRicker,
    KernelNegativeRing,
    
    // Multi-channel section
    MultiChannels,
    MultiChannelsTooltip,
    MultiChannelsSingle,
    MultiChannelsRGB,
    MultiRules,
    MultiRulesCount,
    MultiAddRule,
    MultiAddRuleTooltip,
    MultiRemoveRule,
    MultiRemoveRuleTooltip,
    MultiChannelRouting,
    MultiRule,
    MultiRuleSummary,
    MultiSourceChannel,
    MultiDestChannel,
    MultiStrength,
    MultiStrengthH,
    MultiRadiusFrac,
    MultiKernelLabel,
    MultiGrowthLabel,
    
    // Display section
    DisplayMode,
    DisplayModeTooltip,
    DisplayWorld,
    DisplayNeighborSums,
    DisplayGrowthValues,
    DisplayKernel,
    DisplayDelta,
    DisplayVectorField,
    DisplayContourLines,
    DisplayHeatMap,
    DisplayActivityMap,
    DisplayDifference,
    DisplayVectorScale,
    DisplayVectorScaleTooltip,
    DisplayVectorDensity,
    DisplayVectorDensityTooltip,
    DisplayContourLevels,
    DisplayContourLevelsTooltip,
    DisplayLineThickness,
    DisplayLineThicknessTooltip,
    DisplayActivityDecay,
    DisplayActivityDecayTooltip,
    DisplayColormap,
    DisplayColormapTooltip,
    DisplayColormapLenia,
    DisplayColormapViridis,
    DisplayColormapMagma,
    DisplayColormapInferno,
    DisplayColormapPlasma,
    DisplayColormapGrayscale,
    DisplayColormapGrayscaleInv,
    DisplayColormapJet,
    DisplayUseColormapMulti,
    DisplayUseColormapMultiTooltip,
    DisplayBlendMode,
    DisplayBlendModeTooltip,
    DisplayBlendLuminance,
    DisplayBlendAverage,
    DisplayBlendMaxChannel,
    DisplayBlendMinChannel,
    DisplayBlendRedOnly,
    DisplayBlendGreenOnly,
    DisplayBlendBlueOnly,
    DisplayChannelWeights,
    DisplayChannelWeightsTooltip,
    DisplayChannelWeightR,
    DisplayChannelWeightG,
    DisplayChannelWeightB,
    DisplayResetWeights,
    DisplayZoom,
    DisplayZoomTooltip,
    DisplayPanX,
    DisplayPanXTooltip,
    DisplayPanY,
    DisplayPanYTooltip,
    DisplayResetView,
    DisplayResetViewTooltip,
    DisplayCenterView,
    DisplayCenterViewTooltip,
    DisplayBrightness,
    DisplayBrightnessTooltip,
    DisplayContrast,
    DisplayContrastTooltip,
    DisplayGamma,
    DisplayGammaTooltip,
    DisplayFilterMode,
    DisplayFilterModeTooltip,
    DisplayFilterBilinear,
    DisplayFilterNearest,
    DisplayFilterSharpen,
    DisplayEdgeDetect,
    DisplayEdgeDetectTooltip,
    DisplayGlowSettings,
    DisplayGlowStrength,
    DisplayGlowStrengthTooltip,
    DisplayGlowTint,
    DisplayGlowTintTooltip,
    DisplayGlowIntensity,
    DisplayGlowIntensityTooltip,
    DisplayCustomGradient,
    DisplayCustomGradientTooltip,
    DisplayGradientStops,
    DisplayGradientStopsTooltip,
    DisplayGradientStopLabel,
    DisplayGridOverlay,
    DisplayGridOverlayTooltip,
    DisplayGridOpacity,
    DisplayGridOpacityTooltip,
    DisplayGridColor,
    DisplayGridColorTooltip,
    DisplayGridLineThickness,
    DisplayGridLineThicknessTooltip,
    DisplayGridSpacing,
    DisplayGridSpacingTooltip,
    DisplayGridEveryCell,
    DisplayGridCustomInterval,
    DisplayGridInterval,
    DisplayGridIntervalTooltip,
    DisplayGridMajorLines,
    DisplayGridMajorLinesTooltip,
    DisplayGridMajorEvery,
    DisplayGridMajorEveryTooltip,
    DisplayGridMajorOpacity,
    DisplayGridMajorOpacityTooltip,
    DisplayInvertColors,
    DisplayInvertColorsTooltip,
    DisplayShowBoundary,
    DisplayShowBoundaryTooltip,
    DisplayBoundaryColor,
    DisplayBoundaryOpacity,
    DisplayBoundaryStyle,
    DisplayBoundaryStyleTooltip,
    DisplayBoundaryStyleSolid,
    DisplayBoundaryStyleDashed,
    DisplayBoundaryStyleDotted,
    DisplayBoundaryStyleDouble,
    DisplayBoundaryStyleGlow,
    DisplayBoundaryWidth,
    DisplayBoundaryWidthTooltip,
    DisplayDashLength,
    DisplayDashLengthTooltip,
    DisplayAnimateBoundary,
    DisplayAnimateBoundaryTooltip,
    DisplayBGColor,
    DisplayBGColorTooltip,
    DisplayClipNullCells,
    DisplayClipNullCellsTooltip,
    DisplayClipThreshold,
    DisplayClipThresholdTooltip,
    DisplayColormapDeformation,
    DisplayCmapOffset,
    DisplayCmapOffsetTooltip,
    DisplayRangeMin,
    DisplayRangeMinTooltip,
    DisplayRangeMax,
    DisplayRangeMaxTooltip,
    DisplayPowerCurve,
    DisplayPowerCurveTooltip,
    DisplayHueShift,
    DisplayHueShiftTooltip,
    DisplaySaturation,
    DisplaySaturationTooltip,
    DisplayReverseColormap,
    DisplayReverseColormapTooltip,
    DisplayResetColormapDeformation,
    DisplayRGBChannelIntensity,
    
    // Analysis section
    AnalysisEnable,
    AnalysisEnableTooltip,
    AnalysisAutoPause,
    AnalysisAutoPauseTooltip,
    AnalysisAliveThreshold,
    AnalysisAliveThresholdTooltip,
    AnalysisTotalMass,
    AnalysisAliveCells,
    AnalysisAverage,
    AnalysisMinMax,
    AnalysisVariance,
    AnalysisCentroid,
    AnalysisBounds,
    AnalysisStateEmpty,
    AnalysisStateStabilized,
    AnalysisStatePeriodic,
    AnalysisStateActive,
    AnalysisSpecies,
    AnalysisSpeed,
    AnalysisDirection,
    AnalysisOrientation,
    AnalysisGraphs,
    AnalysisMass,
    AnalysisAlive,
    AnalysisCentroidGraph,
    AnalysisSpeedGraph,
    AnalysisDirectionGraph,
    AnalysisAliveCellsGraph,
    AnalysisCentroidXGraph,
    AnalysisCentroidYGraph,
    AnalysisSpeedGraphTitle,
    AnalysisDirectionGraphTitle,
    AnalysisGraphXAxisStep,
    AnalysisGraphYAxisMass,
    AnalysisGraphYAxisCells,
    AnalysisGraphYAxisX,
    AnalysisGraphYAxisY,
    AnalysisGraphYAxisPxPerSec,
    AnalysisGraphYAxisDeg,
    AnalysisDisplayWindow,
    AnalysisDisplayWindowTooltip,
    AnalysisGraphHeight,
    AnalysisAutoYScale,
    AnalysisAutoYScaleTooltip,
    
    // Accessibility section
    AccessibilityLanguage,
    AccessibilityLanguageTooltip,
    AccessibilityEnglish,
    AccessibilityFrench,
    AccessibilityUIScale,
    AccessibilityUIScaleTooltip,
    AccessibilityFontSize,
    AccessibilityFontSizeTooltip,
    AccessibilityHighContrast,
    AccessibilityHighContrastTooltip,
    AccessibilityReduceMotion,
    AccessibilityReduceMotionTooltip,
    AccessibilityKeyboardNav,
    AccessibilityKeyboardNavTooltip,
    AccessibilityFocusIndicators,
    AccessibilityFocusIndicatorsTooltip,
    AccessibilityResetDefaults,
    AccessibilityResetDefaultsTooltip,
    AccessibilitySystemDpiScale,
    AccessibilityEffectiveScale,
    
    // Common
    CommonYes,
    CommonNo,
    CommonOK,
    CommonCancel,
    CommonApply,
    CommonReset,
    CommonDefault,
    CommonEnabled,
    CommonDisabled,
    CommonOn,
    CommonOff,
    CommonAll,
    CommonNone,
    CommonChannel,
    CommonRed,
    CommonGreen,
    CommonBlue,
    CommonAlpha,
    
    // Count marker (must be last)
    _Count
};

/**
 * @brief Localization manager for the application
 * 
 * Singleton class that handles all text translations for the application.
 * Supports multiple languages and runtime language switching.
 */
class Localization {
public:
    /**
     * @brief Get the singleton instance
     */
    static Localization& instance();
    
    /**
     * @brief Initialize the localization system
     * @param defaultLanguage The initial language to use
     */
    void init(Language defaultLanguage = Language::English);
    
    /**
     * @brief Set the current language
     * @param lang The language to switch to
     */
    void setLanguage(Language lang);
    
    /**
     * @brief Get the current language
     * @return The currently active language
     */
    Language getLanguage() const { return m_currentLanguage; }
    
    /**
     * @brief Get the display name for a language
     * @param lang The language
     * @return The display name in the current language
     */
    const char* getLanguageName(Language lang) const;
    
    /**
     * @brief Get available languages
     * @return Vector of available languages
     */
    std::vector<Language> getAvailableLanguages() const;
    
    /**
     * @brief Get translated text by ID
     * @param id The text identifier
     * @return The translated string
     */
    const char* get(TextId id) const;
    
    /**
     * @brief Convenience operator for getting translated text
     * @param id The text identifier
     * @return The translated string
     */
    const char* operator[](TextId id) const { return get(id); }
    
    /**
     * @brief Register a callback for language change events
     * @param callback Function to call when language changes
     */
    void onLanguageChanged(std::function<void(Language)> callback);
    
    /**
     * @brief Save language preference to config file
     */
    void savePreference() const;
    
    /**
     * @brief Load language preference from config file
     * @return The saved language preference, or English if not found
     */
    Language loadPreference() const;

private:
    Localization() = default;
    ~Localization() = default;
    Localization(const Localization&) = delete;
    Localization& operator=(const Localization&) = delete;
    
    void initEnglish();
    void initFrench();
    
    Language m_currentLanguage{Language::English};
    std::unordered_map<Language, std::vector<std::string>> m_translations;
    std::vector<std::function<void(Language)>> m_languageChangeCallbacks;
    bool m_initialized{false};
};

// Convenience macro for getting translated text
#define TR(id) lenia::Localization::instance().get(lenia::TextId::id)

// Convenience macro for getting translated text with variable
#define TRV(id) lenia::Localization::instance()[lenia::TextId::id]

} // namespace lenia
