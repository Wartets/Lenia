/**
 * @file Localization.cpp
 * @brief Implementation of internationalization system.
 * 
 * String translations are defined inline for each language.
 * Language preference is persisted to lenia_lang.cfg.
 */

#include "Localization.hpp"
#include <fstream>
#include <sstream>

namespace lenia {

/// Singleton accessor
Localization& Localization::instance() {
    static Localization instance;
    return instance;
}

void Localization::init(Language defaultLanguage) {
    (void)defaultLanguage;  // Intentionally unused, preferences are loaded from file
    if (m_initialized) return;
    
    // Initialize all language translations
    initEnglish();
    initFrench();
    
    // Load saved preference or use default
    Language savedLang = loadPreference();
    m_currentLanguage = savedLang;
    
    m_initialized = true;
}

void Localization::setLanguage(Language lang) {
    if (lang == m_currentLanguage) return;
    
    m_currentLanguage = lang;
    
    // Notify all registered callbacks
    for (const auto& callback : m_languageChangeCallbacks) {
        callback(lang);
    }
    
    // Save preference
    savePreference();
}

const char* Localization::getLanguageName(Language lang) const {
    switch (lang) {
        case Language::English: return "English";
        case Language::French:  return "Français";
        default: return "Unknown";
    }
}

std::vector<Language> Localization::getAvailableLanguages() const {
    return { Language::English, Language::French };
}

const char* Localization::get(TextId id) const {
    auto it = m_translations.find(m_currentLanguage);
    if (it == m_translations.end()) {
        // Fallback to English
        it = m_translations.find(Language::English);
        if (it == m_translations.end()) {
            return "[MISSING TRANSLATION]";
        }
    }
    
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(it->second.size())) {
        return "[INVALID TEXT ID]";
    }
    
    return it->second[idx].c_str();
}

void Localization::onLanguageChanged(std::function<void(Language)> callback) {
    m_languageChangeCallbacks.push_back(std::move(callback));
}

void Localization::savePreference() const {
    std::ofstream file("lenia_lang.cfg");
    if (file.is_open()) {
        file << static_cast<int>(m_currentLanguage) << "\n";
    }
}

Language Localization::loadPreference() const {
    std::ifstream file("lenia_lang.cfg");
    if (file.is_open()) {
        int langVal;
        if (file >> langVal) {
            if (langVal >= 0 && langVal <= static_cast<int>(Language::French)) {
                return static_cast<Language>(langVal);
            }
        }
    }
    return Language::English;
}

void Localization::initEnglish() {
    auto& texts = m_translations[Language::English];
    texts.resize(static_cast<size_t>(TextId::_Count));
    
    // Application title and main UI
    texts[static_cast<int>(TextId::AppTitle)] = "Lenia Explorer";
    texts[static_cast<int>(TextId::MainWindowTitle)] = "Lenia Explorer";
    
    // Section headers
    texts[static_cast<int>(TextId::SectionInfo)] = "Info";
    texts[static_cast<int>(TextId::SectionPerformance)] = "Performance";
    texts[static_cast<int>(TextId::SectionGrid)] = "Grid";
    texts[static_cast<int>(TextId::SectionDrawingTools)] = "Drawing Tools";
    texts[static_cast<int>(TextId::SectionPresetsInit)] = "Presets & Initialization";
    texts[static_cast<int>(TextId::SectionSimulation)] = "Simulation";
    texts[static_cast<int>(TextId::SectionGrowthFunction)] = "Growth Function";
    texts[static_cast<int>(TextId::SectionKernel)] = "Kernel";
    texts[static_cast<int>(TextId::SectionMultiChannel)] = "Multi-Channel";
    texts[static_cast<int>(TextId::SectionDisplay)] = "Display";
    texts[static_cast<int>(TextId::SectionAnalysis)] = "Analysis";
    texts[static_cast<int>(TextId::SectionAccessibility)] = "Accessibility";
    
    // Info section
    texts[static_cast<int>(TextId::InfoCursor)] = "Cursor: (%d, %d)";
    texts[static_cast<int>(TextId::InfoValue)] = "Value: %.5f";
    texts[static_cast<int>(TextId::InfoGrid)] = "Grid: %d x %d  |  Step: %d";
    texts[static_cast<int>(TextId::InfoChannels)] = "Channels: %d  |  Rules: %d";
    texts[static_cast<int>(TextId::InfoRules)] = "Rules";
    texts[static_cast<int>(TextId::InfoStep)] = "Step";
    texts[static_cast<int>(TextId::InfoShowConsoleStartup)] = "Show Console on Startup";
    texts[static_cast<int>(TextId::InfoShowConsoleTooltip)] = "If enabled, the console window will appear when starting the application.\nRequires restart to take effect.";
    
    // Keybinds
    texts[static_cast<int>(TextId::KeybindsHeader)] = "Keybinds";
    texts[static_cast<int>(TextId::KeybindsText)] = 
        "Space: Pause/Resume\n"
        "S: Single step | Hold S: Step @5fps\n"
        "Shift+S: Step @10fps\n"
        "R: Reset | C: Clear\n"
        "+/-: Zoom | Arrows: Pan\n"
        "Home: Reset View | Tab: Toggle UI\n"
        "1-5: Set steps/frame\n"
        "F11: Fullscreen | Esc: Quit";
    
    // Theory section
    texts[static_cast<int>(TextId::TheoryHeader)] = "Theory";
    texts[static_cast<int>(TextId::TheoryFundamentals)] = "Lenia Fundamentals";
    texts[static_cast<int>(TextId::TheoryFundamentalsText)] = 
        "Lenia is a continuous cellular automaton system that generalizes discrete CA like "
        "Conway's Game of Life into a continuous domain. Unlike discrete CA with binary states "
        "and integer neighbor counts, Lenia uses continuous cell states in [0,1], continuous "
        "space via smooth kernels, and continuous time via differential integration.";
    texts[static_cast<int>(TextId::TheoryEquation)] = 
        "The fundamental equation governing Lenia is:\n"
        "  A(t+dt) = clip( A(t) + dt * G(K * A) )\n\n"
        "Where:\n"
        "  A(t) = cell state field at time t (values in [0,1])\n"
        "  K = convolution kernel (weighted neighborhood)\n"
        "  K * A = potential field (neighborhood sums)\n"
        "  G() = growth mapping function\n"
        "  dt = time step (integration rate)\n"
        "  clip() = clamps result to [0,1]";
    texts[static_cast<int>(TextId::TheoryKernel)] = "Convolution Kernel";
    texts[static_cast<int>(TextId::TheoryKernelText)] = 
        "The kernel K defines how neighbors influence each cell. It is typically radially "
        "symmetric and normalized (sums to 1). The kernel radius R determines the range of "
        "interaction - larger R creates larger, more complex patterns but requires more computation.\n\n"
        "Common kernel shapes:\n"
        "- Gaussian Shell: exp(-(r-peaks)^2/w^2), smooth bell-shaped rings\n"
        "- Bump4: (4r(1-r))^4, polynomial with compact support\n"
        "- Quad4: Polynomial kernel variant for specific dynamics\n"
        "- Multi-ring: Multiple concentric rings with independent weights (B values)\n\n"
        "The kernel is sampled on a (2R+1)x(2R+1) grid centered on each cell.";
    texts[static_cast<int>(TextId::TheoryGrowthFunction)] = "Growth Function G(u)";
    texts[static_cast<int>(TextId::TheoryGrowthFunctionText)] = 
        "The growth function G maps potential U to a growth rate in [-1, +1]. This determines "
        "how cells respond to their neighborhood sum:\n\n"
        "- G(u) > 0: Cell value increases (growth/birth)\n"
        "- G(u) < 0: Cell value decreases (decay/death)\n"
        "- G(u) = 0: Cell remains stable\n\n"
        "Standard Lenia Growth (Gaussian):\n"
        "  G(u) = 2 * exp(-((u - mu)^2) / (2 * sigma^2)) - 1\n\n"
        "The parameters mu and sigma control pattern behavior.";
    texts[static_cast<int>(TextId::TheoryTimeIntegration)] = "Time Integration (dt)";
    texts[static_cast<int>(TextId::TheoryTimeIntegrationText)] = 
        "The time step dt controls how much change is applied per simulation step:\n\n"
        "- Small dt (0.01-0.1): Smooth, continuous evolution\n"
        "- Medium dt (0.1-0.5): Standard Lenia range\n"
        "- Large dt (0.5-1.0): Discrete-like behavior";
    texts[static_cast<int>(TextId::TheoryMultiChannel)] = "Multi-Channel Systems";
    texts[static_cast<int>(TextId::TheoryMultiChannelText)] = 
        "Multi-channel Lenia extends the system to multiple interacting fields (channels). "
        "Each channel is an independent state field that can influence other channels through "
        "kernel rules.";
    texts[static_cast<int>(TextId::TheoryEdgeConditions)] = "Edge Conditions";
    texts[static_cast<int>(TextId::TheoryEdgeConditionsText)] = 
        "Edge conditions determine what happens at grid boundaries:\n"
        "- Periodic (Wrap): Edges connect to opposite sides\n"
        "- Clamp to Edge: Values at the boundary are extended beyond\n"
        "- Mirror: Values are reflected at boundaries";
    texts[static_cast<int>(TextId::TheoryWalls)] = "Walls";
    texts[static_cast<int>(TextId::TheoryWallsText)] = 
        "Walls are persistent obstacles that affect simulation dynamics.";
    texts[static_cast<int>(TextId::TheoryPatternCharacteristics)] = "Pattern Characteristics";
    texts[static_cast<int>(TextId::TheoryPatternCharacteristicsText)] = 
        "Lenia can produce various pattern types:\n"
        "- Solitons (Gliders): Self-sustaining, moving structures\n"
        "- Oscillators: Patterns that cycle through states\n"
        "- Still Lifes: Stable, unchanging patterns\n"
        "- Chaotic/Turbulent: Unpredictable dynamics";
    texts[static_cast<int>(TextId::TheoryParameterRelationships)] = "Parameter Relationships";
    texts[static_cast<int>(TextId::TheoryParameterRelationshipsText)] = 
        "Key parameter interactions affect pattern behavior:\n"
        "- mu and Kernel: Higher mu values require denser neighborhoods\n"
        "- sigma and Stability: Narrow sigma creates precise but fragile patterns\n"
        "- dt and Pattern Speed: Smaller dt makes patterns move slower";
    texts[static_cast<int>(TextId::TheoryColormapVisualization)] = "Colormap & Visualization";
    texts[static_cast<int>(TextId::TheoryColormapVisualizationText)] = 
        "Display modes for understanding simulation state:\n"
        "- World View: Shows cell states with chosen colormap\n"
        "- Neighbor Sums: Visualizes potential field\n"
        "- Growth Values: Shows current growth rate field";
    
    // Performance section
    texts[static_cast<int>(TextId::PerfFPS)] = "FPS: %.1f";
    texts[static_cast<int>(TextId::PerfFPSTooltip)] = "Current frames per second.\nGreen: 55+ (excellent)\nYellow: 30-55 (good)\nOrange: 15-30 (acceptable)\nRed: <15 (slow)";
    texts[static_cast<int>(TextId::PerfFrame)] = "Frame";
    texts[static_cast<int>(TextId::PerfFrameTime)] = "Frame: %.2f ms (avg)";
    texts[static_cast<int>(TextId::PerfFrameTimeLabel)] = "Frame Time:";
    texts[static_cast<int>(TextId::PerfFrameTimeStats)] = "min=%.2f  avg=%.2f  max=%.2f ms";
    texts[static_cast<int>(TextId::PerfGridSize)] = "Grid Size:";
    texts[static_cast<int>(TextId::PerfGridSizeCellsM)] = "%d x %d = %.2fM cells";
    texts[static_cast<int>(TextId::PerfGridSizeCellsK)] = "%d x %d = %.1fK cells";
    texts[static_cast<int>(TextId::PerfSimulation)] = "Simulation:";
    texts[static_cast<int>(TextId::PerfSimTimeStep)] = "%.2f ms/step  (%.2f ms total)";
    texts[static_cast<int>(TextId::PerfThroughput)] = "Throughput:";
    texts[static_cast<int>(TextId::PerfThroughputG)] = "%.2f Gcells/s";
    texts[static_cast<int>(TextId::PerfThroughputM)] = "%.1f Mcells/s";
    texts[static_cast<int>(TextId::PerfThroughputK)] = "%.0f Kcells/s";
    texts[static_cast<int>(TextId::PerfThroughputTooltip)] = "Processing throughput in cells updated per second.";
    texts[static_cast<int>(TextId::PerfKernelOps)] = "Kernel Ops:";
    texts[static_cast<int>(TextId::PerfKernelOpsG)] = "%.2f Gops/step";
    texts[static_cast<int>(TextId::PerfKernelOpsM)] = "%.1f Mops/step";
    texts[static_cast<int>(TextId::PerfKernelOpsTooltip)] = "Kernel convolution operations per simulation step (cells x kernel size).";
    texts[static_cast<int>(TextId::PerfKernelSize)] = "Kernel Size:";
    texts[static_cast<int>(TextId::PerfKernelSizeSamples)] = "%dx%d = %d samples";
    texts[static_cast<int>(TextId::PerfStepsFrame)] = "Steps/Frame:";
    texts[static_cast<int>(TextId::PerfTotalSteps)] = "Total Steps:";
    texts[static_cast<int>(TextId::PerfExcellent)] = "Excellent";
    texts[static_cast<int>(TextId::PerfGood)] = "Good";
    texts[static_cast<int>(TextId::PerfAcceptable)] = "Acceptable";
    texts[static_cast<int>(TextId::PerfSlow)] = "Slow";
    texts[static_cast<int>(TextId::PerfPerformance)] = "Performance: %s";
    texts[static_cast<int>(TextId::PerfPerformanceTooltip)] = "Reduce grid size or kernel radius to improve performance.";
    texts[static_cast<int>(TextId::PerfShowResourceMonitor)] = "Show Resource Monitor";
    texts[static_cast<int>(TextId::PerfResourceUsage)] = "Resource Usage:";
    texts[static_cast<int>(TextId::PerfGPUMemory)] = "GPU Memory: %d / %d MB (%.0f%%)";
    texts[static_cast<int>(TextId::PerfGPUMemoryNA)] = "GPU Memory: N/A";
    texts[static_cast<int>(TextId::PerfCPUMemory)] = "CPU Memory: %.1f MB";
    texts[static_cast<int>(TextId::PerfTextureMemory)] = "Texture Memory: ~%.2f MB";
    texts[static_cast<int>(TextId::PerfTextureMemoryTooltip)] = "Estimated GPU memory for simulation textures.\n2x grid textures + kernel texture.";
    texts[static_cast<int>(TextId::PerfFrameTimeGraphTitle)] = "Frame Time";
    texts[static_cast<int>(TextId::PerfFrameTimeGraphXLabel)] = "frames";
    texts[static_cast<int>(TextId::PerfFrameTimeGraphYLabel)] = "ms";
    
    // Grid section
    texts[static_cast<int>(TextId::GridSize)] = "Size: %d x %d (%s cells)";
    texts[static_cast<int>(TextId::GridWidth)] = "Width";
    texts[static_cast<int>(TextId::GridWidthTooltip)] = "Grid width in cells. Larger grids allow more complex patterns but are slower. Must be >= 32.";
    texts[static_cast<int>(TextId::GridHeight)] = "Height";
    texts[static_cast<int>(TextId::GridHeightTooltip)] = "Grid height in cells. The grid wraps toroidally (edges connect).";
    texts[static_cast<int>(TextId::GridTransformations)] = "Transformations:";
    texts[static_cast<int>(TextId::GridFlipHorizontal)] = "<->";
    texts[static_cast<int>(TextId::GridFlipHorizontalTooltip)] = "Flip horizontally (mirror left-right).";
    texts[static_cast<int>(TextId::GridFlipVertical)] = "^v";
    texts[static_cast<int>(TextId::GridFlipVerticalTooltip)] = "Flip vertically (mirror top-bottom).";
    texts[static_cast<int>(TextId::GridRotateCW)] = "->|";
    texts[static_cast<int>(TextId::GridRotateCWTooltip)] = "Rotate 90 degrees clockwise.";
    texts[static_cast<int>(TextId::GridRotateCCW)] = "|<-";
    texts[static_cast<int>(TextId::GridRotateCCWTooltip)] = "Rotate the grid 90 degrees counter-clockwise.";
    texts[static_cast<int>(TextId::GridEdgeConditions)] = "Edge Conditions:";
    texts[static_cast<int>(TextId::GridEdgeModeX)] = "X Edge";
    texts[static_cast<int>(TextId::GridEdgeModeXTooltip)] = "Horizontal edge behavior:\n- Periodic: Wraps around (toroidal)\n- Clamp: Uses edge values\n- Mirror: Reflects at boundaries";
    texts[static_cast<int>(TextId::GridEdgeModeY)] = "Y Edge";
    texts[static_cast<int>(TextId::GridEdgeModeYTooltip)] = "Vertical edge behavior:\n- Periodic: Wraps around (toroidal)\n- Clamp: Uses edge values\n- Mirror: Reflects at boundaries";
    texts[static_cast<int>(TextId::GridEdgePeriodic)] = "Periodic (Wrap)";
    texts[static_cast<int>(TextId::GridEdgeClamp)] = "Clamp to Edge";
    texts[static_cast<int>(TextId::GridEdgeMirror)] = "Mirror";
    texts[static_cast<int>(TextId::GridEdgeFade)] = "Edge Fade:";
    texts[static_cast<int>(TextId::GridEdgeFadeX)] = "X Fade";
    texts[static_cast<int>(TextId::GridEdgeFadeXTooltip)] = "Fade distance at horizontal edges (0 = hard edge, 0.5 = half grid).";
    texts[static_cast<int>(TextId::GridEdgeFadeY)] = "Y Fade";
    texts[static_cast<int>(TextId::GridEdgeFadeYTooltip)] = "Fade distance at vertical edges (0 = hard edge, 0.5 = half grid).";
    texts[static_cast<int>(TextId::GridOutsideDisplay)] = "Outside Display";
    texts[static_cast<int>(TextId::GridOutsideDisplayTooltip)] = "How to display areas outside the grid:\n- Tiled: Repeats based on edge mode\n- Background: Shows background color\n- Checker: Shows a checker pattern";
    texts[static_cast<int>(TextId::GridShowTiled)] = "Show Tiled";
    texts[static_cast<int>(TextId::GridBackgroundColor)] = "Background Color";
    texts[static_cast<int>(TextId::GridCheckerPattern)] = "Checker Pattern";
    
    // Infinite world
    texts[static_cast<int>(TextId::InfiniteWorldMode)] = "Infinite World Mode";
    texts[static_cast<int>(TextId::InfiniteWorldEnable)] = "Enable Infinite World";
    texts[static_cast<int>(TextId::InfiniteWorldEnableTooltip)] = "Enable exploration of an infinite procedural world.\nUse mouse drag (middle-click or Ctrl+right-click) to pan.\nEdge conditions become periodic (wrapping).";
    texts[static_cast<int>(TextId::InfiniteWorldSettings)] = "World Settings:";
    texts[static_cast<int>(TextId::InfiniteChunkSize)] = "Chunk Size";
    texts[static_cast<int>(TextId::InfiniteChunkSizeTooltip)] = "Size of each world chunk in cells.";
    texts[static_cast<int>(TextId::InfiniteLoadRadius)] = "Load Radius";
    texts[static_cast<int>(TextId::InfiniteLoadRadiusTooltip)] = "Number of chunks to keep loaded around the view center.";
    texts[static_cast<int>(TextId::InfiniteMaxChunks)] = "Max Chunks";
    texts[static_cast<int>(TextId::InfiniteMaxChunksTooltip)] = "Maximum number of chunks to keep in memory.";
    texts[static_cast<int>(TextId::InfiniteNavigation)] = "Navigation:";
    texts[static_cast<int>(TextId::InfiniteNavigationTooltip)] = "Navigate between chunks. Use mouse drag to pan within a chunk.";
    texts[static_cast<int>(TextId::InfiniteChunkPosition)] = "Chunk Position: (%d, %d)";
    texts[static_cast<int>(TextId::InfiniteWorldOffset)] = "World Offset: (%.2f, %.2f)";
    texts[static_cast<int>(TextId::InfiniteHome)] = "Home";
    texts[static_cast<int>(TextId::InfiniteNavNorth)] = "N";
    texts[static_cast<int>(TextId::InfiniteNavWest)] = "W";
    texts[static_cast<int>(TextId::InfiniteNavEast)] = "E";
    texts[static_cast<int>(TextId::InfiniteNavSouth)] = "S";
    texts[static_cast<int>(TextId::InfiniteExploreSpeed)] = "Explore Speed";
    texts[static_cast<int>(TextId::InfiniteExploreSpeedTooltip)] = "Speed multiplier for keyboard navigation.";
    texts[static_cast<int>(TextId::InfiniteAutoLoad)] = "Auto-Load Chunks";
    texts[static_cast<int>(TextId::InfiniteAutoLoadTooltip)] = "Automatically load new chunks as you explore.";
    texts[static_cast<int>(TextId::InfiniteDisplayOptions)] = "Display Options:";
    texts[static_cast<int>(TextId::InfiniteShowChunkGrid)] = "Show Chunk Grid";
    texts[static_cast<int>(TextId::InfiniteShowChunkGridTooltip)] = "Display borders between chunks.";
    texts[static_cast<int>(TextId::InfiniteEdgeFade)] = "Edge Fade";
    texts[static_cast<int>(TextId::InfiniteEdgeFadeTooltip)] = "Fade at world edges (0 = no fade).";
    texts[static_cast<int>(TextId::InfinitePersistence)] = "Persistence";
    texts[static_cast<int>(TextId::InfinitePersistenceTooltip)] = "How chunk state is handled:\n- None: Chunks reset when unloaded\n- Preserve: Keeps state in memory\n- Seed-Based: Regenerates from seed";
    texts[static_cast<int>(TextId::InfinitePersistenceNone)] = "None (Clear)";
    texts[static_cast<int>(TextId::InfinitePersistencePreserve)] = "Preserve State";
    texts[static_cast<int>(TextId::InfinitePersistenceSeed)] = "Seed-Based";
    texts[static_cast<int>(TextId::InfinitePanTip)] = "Tip: Middle-click or Ctrl+Right-click to pan";
    texts[static_cast<int>(TextId::InfiniteScrollTip)] = "Scroll wheel to zoom";
    
    // Drawing tools
    texts[static_cast<int>(TextId::DrawToolMode)] = "Tool Mode";
    texts[static_cast<int>(TextId::DrawToolBrush)] = "Brush (Living Cells)";
    texts[static_cast<int>(TextId::DrawToolObstacle)] = "Obstacle (Barrier)";
    texts[static_cast<int>(TextId::DrawToolModeTooltip)] = "Brush paints cells that evolve with the simulation.\nObstacle creates barriers where cells are held at a fixed value.";
    texts[static_cast<int>(TextId::DrawEnableDrawing)] = "Enable Drawing";
    texts[static_cast<int>(TextId::DrawEnableDrawingTooltip)] = "Enable or disable drawing on the simulation grid.";
    texts[static_cast<int>(TextId::DrawObstacleModeActive)] = "OBSTACLE MODE ACTIVE";
    texts[static_cast<int>(TextId::DrawBrushModeActive)] = "BRUSH MODE ACTIVE";
    texts[static_cast<int>(TextId::DrawShapeSize)] = "Shape & Size";
    texts[static_cast<int>(TextId::DrawShape)] = "Shape";
    texts[static_cast<int>(TextId::DrawShapeTooltip)] = "Shape of the brush/obstacle.\n- Ring: Hollow circle\n- Star: 5 or 6 pointed star\n- Hexagon: Hexagonal shape\n- Cross/Plus: Cross patterns\n- Gaussian Blob: Soft falloff\n- Noise Disc: Random texture\n- Gradient Disc: Linear gradient";
    texts[static_cast<int>(TextId::DrawShapeCircle)] = "Circle";
    texts[static_cast<int>(TextId::DrawShapeSquare)] = "Square";
    texts[static_cast<int>(TextId::DrawShapeDiamond)] = "Diamond";
    texts[static_cast<int>(TextId::DrawShapeRing)] = "Ring";
    texts[static_cast<int>(TextId::DrawShapeStar5)] = "Star (5pt)";
    texts[static_cast<int>(TextId::DrawShapeStar6)] = "Star (6pt)";
    texts[static_cast<int>(TextId::DrawShapeHexagon)] = "Hexagon";
    texts[static_cast<int>(TextId::DrawShapeCross)] = "Cross";
    texts[static_cast<int>(TextId::DrawShapePlus)] = "Plus";
    texts[static_cast<int>(TextId::DrawShapeGaussian)] = "Gaussian Blob";
    texts[static_cast<int>(TextId::DrawShapeNoiseDisc)] = "Noise Disc";
    texts[static_cast<int>(TextId::DrawShapeGradientDisc)] = "Gradient Disc";
    texts[static_cast<int>(TextId::DrawSize)] = "Size";
    texts[static_cast<int>(TextId::DrawSizeTooltip)] = "Size of the brush in cells.";
    texts[static_cast<int>(TextId::DrawFalloff)] = "Falloff";
    texts[static_cast<int>(TextId::DrawFalloffTooltip)] = "Edge softness. 0 = hard edge, 1 = smooth fade.";
    texts[static_cast<int>(TextId::DrawMethod)] = "Draw Method";
    texts[static_cast<int>(TextId::DrawModeFreehand)] = "Freehand";
    texts[static_cast<int>(TextId::DrawModeLine)] = "Line";
    texts[static_cast<int>(TextId::DrawModeCircle)] = "Circle";
    texts[static_cast<int>(TextId::DrawModeRectangle)] = "Rectangle";
    texts[static_cast<int>(TextId::DrawModeTooltip)] = "Freehand: Click and drag to draw\nLine: Click start, release at end\nCircle: Click center, drag radius\nRectangle: Click corner, drag to opposite corner";
    texts[static_cast<int>(TextId::DrawDrawing)] = "Drawing... (Right-click to cancel)";
    texts[static_cast<int>(TextId::DrawClickToStart)] = "Click on grid to start drawing";
    texts[static_cast<int>(TextId::DrawObstacleSettings)] = "Obstacle Settings";
    texts[static_cast<int>(TextId::DrawCellValue)] = "Cell Value";
    texts[static_cast<int>(TextId::DrawCellValueTooltip)] = "Fixed value for cells inside obstacles.\n0.0 = dead/empty (blocks life)\n1.0 = fully alive (creates permanent life)";
    texts[static_cast<int>(TextId::DrawDisplayColor)] = "Display Color";
    texts[static_cast<int>(TextId::DrawDisplayColorTooltip)] = "Visual color of obstacles in the display.";
    texts[static_cast<int>(TextId::DrawAffectedChannels)] = "Affected Channels:";
    texts[static_cast<int>(TextId::DrawAffectedChannelsTooltip)] = "Which channels the obstacle affects.";
    texts[static_cast<int>(TextId::DrawBlendMode)] = "Blend Mode";
    texts[static_cast<int>(TextId::DrawBlendModeTooltip)] = "Replace: Overwrite existing\nMax: Keep stronger value\nBlend: Smooth blend\nErase: Remove obstacles";
    texts[static_cast<int>(TextId::DrawBlendReplace)] = "Replace";
    texts[static_cast<int>(TextId::DrawBlendMax)] = "Max";
    texts[static_cast<int>(TextId::DrawBlendReplaceStronger)] = "Replace If Stronger";
    texts[static_cast<int>(TextId::DrawBlendBlend)] = "Blend";
    texts[static_cast<int>(TextId::DrawBlendErase)] = "Erase";
    texts[static_cast<int>(TextId::DrawClearAllObstacles)] = "Clear All Obstacles";
    texts[static_cast<int>(TextId::DrawClearAllObstaclesTooltip)] = "Remove all obstacles from the simulation.";
    texts[static_cast<int>(TextId::DrawBrushSettings)] = "Brush Settings";
    texts[static_cast<int>(TextId::DrawPaintMode)] = "Paint Mode";
    texts[static_cast<int>(TextId::DrawPaintModeTooltip)] = "Set: Replace cell value\nAdd: Add to existing\nSubtract: Subtract from existing\nMax/Min: Keep larger/smaller\nErase: Set to zero";
    texts[static_cast<int>(TextId::DrawPaintModeSet)] = "Set";
    texts[static_cast<int>(TextId::DrawPaintModeAdd)] = "Add";
    texts[static_cast<int>(TextId::DrawPaintModeSubtract)] = "Subtract";
    texts[static_cast<int>(TextId::DrawPaintModeMax)] = "Max";
    texts[static_cast<int>(TextId::DrawPaintModeMin)] = "Min";
    texts[static_cast<int>(TextId::DrawPaintModeErase)] = "Erase";
    texts[static_cast<int>(TextId::DrawBrushValue)] = "Value";
    texts[static_cast<int>(TextId::DrawBrushValueTooltip)] = "Cell value to paint.";
    texts[static_cast<int>(TextId::DrawStrength)] = "Strength";
    texts[static_cast<int>(TextId::DrawStrengthTooltip)] = "Intensity multiplier.";
    texts[static_cast<int>(TextId::DrawTargetChannel)] = "Target Channel";
    texts[static_cast<int>(TextId::DrawTargetChannelTooltip)] = "Which channel(s) to paint.";
    texts[static_cast<int>(TextId::DrawSymmetry)] = "Symmetry";
    texts[static_cast<int>(TextId::DrawMirrorX)] = "Mirror X";
    texts[static_cast<int>(TextId::DrawMirrorY)] = "Mirror Y";
    texts[static_cast<int>(TextId::DrawMirrorTooltip)] = "Mirror strokes across the grid center.";
    texts[static_cast<int>(TextId::DrawRadialSymmetry)] = "Radial Symmetry";
    texts[static_cast<int>(TextId::DrawRadialSymmetryTooltip)] = "Rotational symmetry around grid center.";
    texts[static_cast<int>(TextId::DrawStrokeSpacing)] = "Stroke Spacing";
    texts[static_cast<int>(TextId::DrawBrushSpacing)] = "Brush Spacing";
    texts[static_cast<int>(TextId::DrawBrushSpacingTooltip)] = "Distance between stroke applications when dragging.";
    texts[static_cast<int>(TextId::DrawSmoothInterpolation)] = "Smooth Interpolation";
    texts[static_cast<int>(TextId::DrawSmoothInterpolationTooltip)] = "Interpolate positions when moving quickly.";
    
    // Presets section
    texts[static_cast<int>(TextId::PresetsCategory)] = "Category";
    texts[static_cast<int>(TextId::PresetsSearch)] = "Search";
    texts[static_cast<int>(TextId::PresetsSearchHint)] = "Search presets...";
    texts[static_cast<int>(TextId::PresetsSelected)] = "Selected: %s";
    texts[static_cast<int>(TextId::PresetsSpecies)] = "Species";
    texts[static_cast<int>(TextId::PresetsKernel)] = "Kernel";
    texts[static_cast<int>(TextId::PresetsProcedural)] = "Procedural";
    texts[static_cast<int>(TextId::PresetsShown)] = "Showing %d presets";
    texts[static_cast<int>(TextId::PresetsCountShown)] = "%d presets (%d shown)";
    texts[static_cast<int>(TextId::PresetsRandomize)] = "Randomize";
    texts[static_cast<int>(TextId::PresetsClear)] = "Clear";
    texts[static_cast<int>(TextId::PresetsResetPreset)] = "Reset to Preset";
    texts[static_cast<int>(TextId::PresetsPlacement)] = "Placement";
    texts[static_cast<int>(TextId::PresetsPlacementCenter)] = "Center";
    texts[static_cast<int>(TextId::PresetsPlacementTopLeft)] = "Top Left";
    texts[static_cast<int>(TextId::PresetsPlacementTopRight)] = "Top Right";
    texts[static_cast<int>(TextId::PresetsPlacementBottomLeft)] = "Bottom Left";
    texts[static_cast<int>(TextId::PresetsPlacementBottomRight)] = "Bottom Right";
    texts[static_cast<int>(TextId::PresetsPlacementTop)] = "Top";
    texts[static_cast<int>(TextId::PresetsPlacementBottom)] = "Bottom";
    texts[static_cast<int>(TextId::PresetsPlacementLeft)] = "Left";
    texts[static_cast<int>(TextId::PresetsPlacementRight)] = "Right";
    texts[static_cast<int>(TextId::PresetsPlacementRandom)] = "Random";
    texts[static_cast<int>(TextId::PresetsPlacementGrid)] = "Grid";
    texts[static_cast<int>(TextId::PresetsPlacementTwoPlace)] = "Two-Place";
    texts[static_cast<int>(TextId::PresetsPlacementScatter)] = "Scatter";
    texts[static_cast<int>(TextId::PresetsCount)] = "Count";
    texts[static_cast<int>(TextId::PresetsScale)] = "Scale";
    texts[static_cast<int>(TextId::PresetsRotation)] = "Rotation";
    texts[static_cast<int>(TextId::PresetsRotation0)] = "0°";
    texts[static_cast<int>(TextId::PresetsRotation90)] = "90°";
    texts[static_cast<int>(TextId::PresetsRotation180)] = "180°";
    texts[static_cast<int>(TextId::PresetsRotation270)] = "270°";
    texts[static_cast<int>(TextId::PresetsMargin)] = "Margin";
    texts[static_cast<int>(TextId::PresetsRandomFlip)] = "Random Flip";
    texts[static_cast<int>(TextId::PresetsFlipHorizontal)] = "Flip Horizontal";
    texts[static_cast<int>(TextId::PresetsFlipVertical)] = "Flip Vertical";
    texts[static_cast<int>(TextId::PresetsPlaceSpacing)] = "Spacing";
    texts[static_cast<int>(TextId::PresetsMinSeparation)] = "Min Separation";
    texts[static_cast<int>(TextId::PresetsClearGridFirst)] = "Clear Grid First";
    texts[static_cast<int>(TextId::PresetsApplyPlacement)] = "Apply Placement";
    texts[static_cast<int>(TextId::PresetsRadiusRings)] = "R=%d rings=%d";
    texts[static_cast<int>(TextId::PresetsMu)] = "mu=%.3f";
    texts[static_cast<int>(TextId::PresetsSigma)] = "sigma=%.4f";
    
    // Simulation section
    texts[static_cast<int>(TextId::SimPaused)] = "PAUSED (Space to run, S to step)";
    texts[static_cast<int>(TextId::SimPausedLabel)] = "Paused (Space)";
    texts[static_cast<int>(TextId::SimHoldToStep)] = "Hold S for continuous stepping";
    texts[static_cast<int>(TextId::SimStepsPerFrame)] = "Steps/Frame";
    texts[static_cast<int>(TextId::SimStepsPerFrameTooltip)] = "Number of simulation steps computed per rendered frame.\nHigher = faster simulation, potentially choppier visuals.\nUse keys 1-5 to set directly.";
    texts[static_cast<int>(TextId::SimStep)] = "Step";
    texts[static_cast<int>(TextId::SimStepFormat)] = "Step: %d";
    texts[static_cast<int>(TextId::SimTime)] = "Time";
    texts[static_cast<int>(TextId::SimTimeMs)] = "Sim: %.2f ms";
    
    // Growth function section
    texts[static_cast<int>(TextId::GrowthType)] = "Growth Type";
    texts[static_cast<int>(TextId::GrowthTypeTooltip)] = "Mathematical function that maps neighborhood potential to growth rate.\nGaussian is standard Lenia, others create different dynamics.";
    texts[static_cast<int>(TextId::GrowthTypeLenia)] = "Lenia (Gaussian)";
    texts[static_cast<int>(TextId::GrowthTypeStep)] = "Step Function";
    texts[static_cast<int>(TextId::GrowthTypeGameOfLife)] = "Game of Life";
    texts[static_cast<int>(TextId::GrowthTypeSmoothLife)] = "SmoothLife";
    texts[static_cast<int>(TextId::GrowthTypePolynomial)] = "Polynomial";
    texts[static_cast<int>(TextId::GrowthTypeExponential)] = "Exponential";
    texts[static_cast<int>(TextId::GrowthTypeDoublePeak)] = "Double Peak";
    texts[static_cast<int>(TextId::GrowthTypeAsymptotic)] = "Asymptotic";
    texts[static_cast<int>(TextId::GrowthTypeSoftClip)] = "Soft Clip";
    texts[static_cast<int>(TextId::GrowthTypeLargerThanLife)] = "Larger Than Life";
    texts[static_cast<int>(TextId::GrowthTypeQuad4)] = "Quad4";
    texts[static_cast<int>(TextId::GrowthMu)] = "Mu (μ)";
    texts[static_cast<int>(TextId::GrowthMuTooltip)] = "Growth center point. Cells with this neighborhood sum grow fastest.\nTypically 0.1-0.3 for small patterns, higher for larger ones.";
    texts[static_cast<int>(TextId::GrowthSigma)] = "Sigma (σ)";
    texts[static_cast<int>(TextId::GrowthSigmaTooltip)] = "Growth width. Controls how tolerant growth is to neighborhood variation.\nSmaller = precise but fragile. Larger = robust but less defined.";
    texts[static_cast<int>(TextId::GrowthDt)] = "Time Step (dt)";
    texts[static_cast<int>(TextId::GrowthDtTooltip)] = "Integration step size. Controls speed and smoothness.\nSmall (0.01-0.1): Smooth motion. Large (0.5-1.0): Discrete jumps.";
    texts[static_cast<int>(TextId::GrowthPlotGoLHint)] = "Game of Life B3/S23 (x-axis: neighbor count 0-8)";
    texts[static_cast<int>(TextId::GrowthPlotAsymptoticHint)] = "Asymptotic target(U)-A  mu=%.4f sigma=%.4f";
    texts[static_cast<int>(TextId::GrowthPlotSoftClipHint)] = "SoftClip sigmoid  mu=%.4f sigma=%.4f";
    texts[static_cast<int>(TextId::GrowthPlotLTLHint)] = "Larger-than-Life  mu=%.4f sigma=%.4f";
    texts[static_cast<int>(TextId::GrowthPlotDefaultHint)] = "Growth(U)  mu=%.4f  sigma=%.4f";
    
    // Kernel section
    texts[static_cast<int>(TextId::KernelType)] = "Kernel Type";
    texts[static_cast<int>(TextId::KernelTypeTooltip)] = "Shape of the convolution kernel that defines neighborhood weights.";
    texts[static_cast<int>(TextId::KernelPreset)] = "Kernel Preset";
    texts[static_cast<int>(TextId::KernelRadius)] = "Radius";
    texts[static_cast<int>(TextId::KernelRadiusTooltip)] = "Kernel radius in cells. Larger = bigger patterns but slower.\nTypical range: 6-20. Standard Orbium uses R=13.";
    texts[static_cast<int>(TextId::KernelRings)] = "Rings";
    texts[static_cast<int>(TextId::KernelRingsTooltip)] = "Number of concentric rings in the kernel.\nEach ring can have independent weight (B value).";
    texts[static_cast<int>(TextId::KernelRingWeight)] = "Ring %d Weight";
    texts[static_cast<int>(TextId::KernelRingWeightTooltip)] = "Weight (B value) for ring %d. Controls influence of neighbors at this distance.";
    texts[static_cast<int>(TextId::KernelAdvanced)] = "Advanced";
    texts[static_cast<int>(TextId::KernelAnisotropy)] = "Anisotropy";
    texts[static_cast<int>(TextId::KernelAnisotropyTooltip)] = "Directional bias in the kernel (0 = isotropic, 1 = highly directional).";
    texts[static_cast<int>(TextId::KernelDirection)] = "Direction";
    texts[static_cast<int>(TextId::KernelDirectionTooltip)] = "Angle of anisotropic bias in degrees.";
    texts[static_cast<int>(TextId::KernelTimeVarying)] = "Time-Varying";
    texts[static_cast<int>(TextId::KernelTimeVaryingTooltip)] = "Enable time-varying kernel modulation.";
    texts[static_cast<int>(TextId::KernelPulseFrequency)] = "Pulse Frequency";
    texts[static_cast<int>(TextId::KernelPulseFrequencyTooltip)] = "Frequency of kernel pulsation.";
    texts[static_cast<int>(TextId::KernelModifier)] = "Modifier";
    texts[static_cast<int>(TextId::KernelModifierTooltip)] = "Additional kernel modifications.";
    texts[static_cast<int>(TextId::KernelModifierNone)] = "None";
    texts[static_cast<int>(TextId::KernelModifierNegativeRing)] = "Negative Ring";
    texts[static_cast<int>(TextId::KernelShowPreview)] = "Show Kernel Preview";
    texts[static_cast<int>(TextId::KernelPerRuleNote)] = "Note: Kernel parameters can be set per-rule in Multi-Channel mode.";
    texts[static_cast<int>(TextId::KernelCrossSection)] = "Kernel Cross-Section";
    texts[static_cast<int>(TextId::KernelCrossSectionWithSize)] = "Kernel Cross-Section (%dx%d)";
    
    // Kernel types
    texts[static_cast<int>(TextId::KernelGaussianShell)] = "Gaussian Shell";
    texts[static_cast<int>(TextId::KernelBump4)] = "Bump4";
    texts[static_cast<int>(TextId::KernelMultiringGauss)] = "Multi-ring Gaussian";
    texts[static_cast<int>(TextId::KernelMultiringBump4)] = "Multi-ring Bump4";
    texts[static_cast<int>(TextId::KernelGameOfLife)] = "Game of Life";
    texts[static_cast<int>(TextId::KernelStepUnimodal)] = "Step (Unimodal)";
    texts[static_cast<int>(TextId::KernelCosineShell)] = "Cosine Shell";
    texts[static_cast<int>(TextId::KernelMexicanHat)] = "Mexican Hat";
    texts[static_cast<int>(TextId::KernelQuad4)] = "Quad4";
    texts[static_cast<int>(TextId::KernelMultiringQuad4)] = "Multi-ring Quad4";
    texts[static_cast<int>(TextId::KernelCone)] = "Cone";
    texts[static_cast<int>(TextId::KernelTorusDualRing)] = "Torus (Dual Ring)";
    texts[static_cast<int>(TextId::KernelRingSharp)] = "Ring (Sharp)";
    texts[static_cast<int>(TextId::KernelGaussianMixture)] = "Gaussian Mixture";
    texts[static_cast<int>(TextId::KernelSinc)] = "Sinc";
    texts[static_cast<int>(TextId::KernelWaveletRicker)] = "Wavelet (Ricker)";
    texts[static_cast<int>(TextId::KernelNegativeRing)] = "Negative Ring";
    
    // Multi-channel section
    texts[static_cast<int>(TextId::MultiChannels)] = "Channels";
    texts[static_cast<int>(TextId::MultiChannelsTooltip)] = "Number of independent state channels.\n1 = single channel (grayscale)\n3 = RGB channels (color)";
    texts[static_cast<int>(TextId::MultiChannelsSingle)] = "1 (Single)";
    texts[static_cast<int>(TextId::MultiChannelsRGB)] = "3 (RGB)";
    texts[static_cast<int>(TextId::MultiRules)] = "Rules";
    texts[static_cast<int>(TextId::MultiRulesCount)] = "Rules: %d";
    texts[static_cast<int>(TextId::MultiAddRule)] = "Add Rule";
    texts[static_cast<int>(TextId::MultiAddRuleTooltip)] = "Add a new kernel rule for channel interactions.";
    texts[static_cast<int>(TextId::MultiRemoveRule)] = "Remove";
    texts[static_cast<int>(TextId::MultiRemoveRuleTooltip)] = "Remove this kernel rule.";
    texts[static_cast<int>(TextId::MultiChannelRouting)] = "Channel Routing";
    texts[static_cast<int>(TextId::MultiRule)] = "Rule %d";
    texts[static_cast<int>(TextId::MultiRuleSummary)] = "r%d m=%.3f s=%.4f h=%.2f";
    texts[static_cast<int>(TextId::MultiSourceChannel)] = "Source Channel";
    texts[static_cast<int>(TextId::MultiDestChannel)] = "Destination Channel";
    texts[static_cast<int>(TextId::MultiStrength)] = "Strength";
    texts[static_cast<int>(TextId::MultiStrengthH)] = "Strength (h)";
    texts[static_cast<int>(TextId::MultiRadiusFrac)] = "Radius Fraction";
    texts[static_cast<int>(TextId::MultiKernelLabel)] = "Kernel";
    texts[static_cast<int>(TextId::MultiGrowthLabel)] = "Growth";
    
    // Display section
    texts[static_cast<int>(TextId::DisplayMode)] = "Display Mode";
    texts[static_cast<int>(TextId::DisplayModeTooltip)] = "What to visualize:\n- World: Cell states\n- Sums: Convolution result\n- Growth: Current growth values\n- Kernel: Kernel shape\n- Delta: Change per step";
    texts[static_cast<int>(TextId::DisplayWorld)] = "World";
    texts[static_cast<int>(TextId::DisplayNeighborSums)] = "Neighbor Sums";
    texts[static_cast<int>(TextId::DisplayGrowthValues)] = "Growth Values";
    texts[static_cast<int>(TextId::DisplayKernel)] = "Kernel";
    texts[static_cast<int>(TextId::DisplayDelta)] = "Delta";
    texts[static_cast<int>(TextId::DisplayVectorField)] = "Vector Field";
    texts[static_cast<int>(TextId::DisplayContourLines)] = "Contour Lines";
    texts[static_cast<int>(TextId::DisplayHeatMap)] = "Heat Map";
    texts[static_cast<int>(TextId::DisplayActivityMap)] = "Activity Map";
    texts[static_cast<int>(TextId::DisplayDifference)] = "Difference";
    texts[static_cast<int>(TextId::DisplayVectorScale)] = "Vector Scale";
    texts[static_cast<int>(TextId::DisplayVectorScaleTooltip)] = "Scale of vector arrows.";
    texts[static_cast<int>(TextId::DisplayVectorDensity)] = "Vector Density";
    texts[static_cast<int>(TextId::DisplayVectorDensityTooltip)] = "Number of vectors per axis.";
    texts[static_cast<int>(TextId::DisplayContourLevels)] = "Contour Levels";
    texts[static_cast<int>(TextId::DisplayContourLevelsTooltip)] = "Number of contour lines to draw.";
    texts[static_cast<int>(TextId::DisplayLineThickness)] = "Line Thickness";
    texts[static_cast<int>(TextId::DisplayLineThicknessTooltip)] = "Thickness of contour lines.";
    texts[static_cast<int>(TextId::DisplayActivityDecay)] = "Activity Decay";
    texts[static_cast<int>(TextId::DisplayActivityDecayTooltip)] = "How quickly activity fades.";
    texts[static_cast<int>(TextId::DisplayColormap)] = "Colormap";
    texts[static_cast<int>(TextId::DisplayColormapTooltip)] = "Color scheme for visualizing scalar values.";
    texts[static_cast<int>(TextId::DisplayColormapLenia)] = "Lenia";
    texts[static_cast<int>(TextId::DisplayColormapViridis)] = "Viridis";
    texts[static_cast<int>(TextId::DisplayColormapMagma)] = "Magma";
    texts[static_cast<int>(TextId::DisplayColormapInferno)] = "Inferno";
    texts[static_cast<int>(TextId::DisplayColormapPlasma)] = "Plasma";
    texts[static_cast<int>(TextId::DisplayColormapGrayscale)] = "Grayscale";
    texts[static_cast<int>(TextId::DisplayColormapGrayscaleInv)] = "Grayscale Inv.";
    texts[static_cast<int>(TextId::DisplayColormapJet)] = "Jet";
    texts[static_cast<int>(TextId::DisplayUseColormapMulti)] = "Use Colormap for Multi-Channel";
    texts[static_cast<int>(TextId::DisplayUseColormapMultiTooltip)] = "Apply colormap to multi-channel display instead of RGB mapping.";
    texts[static_cast<int>(TextId::DisplayBlendMode)] = "Blend Mode";
    texts[static_cast<int>(TextId::DisplayBlendModeTooltip)] = "How to combine multi-channel values for display.";
    texts[static_cast<int>(TextId::DisplayBlendLuminance)] = "Luminance";
    texts[static_cast<int>(TextId::DisplayBlendAverage)] = "Average";
    texts[static_cast<int>(TextId::DisplayBlendMaxChannel)] = "Max Channel";
    texts[static_cast<int>(TextId::DisplayBlendMinChannel)] = "Min Channel";
    texts[static_cast<int>(TextId::DisplayBlendRedOnly)] = "Red Only";
    texts[static_cast<int>(TextId::DisplayBlendGreenOnly)] = "Green Only";
    texts[static_cast<int>(TextId::DisplayBlendBlueOnly)] = "Blue Only";
    texts[static_cast<int>(TextId::DisplayChannelWeights)] = "Channel Weights";
    texts[static_cast<int>(TextId::DisplayChannelWeightsTooltip)] = "Custom weights for luminance calculation. Standard is R=0.299, G=0.587, B=0.114.";
    texts[static_cast<int>(TextId::DisplayChannelWeightR)] = "R Weight";
    texts[static_cast<int>(TextId::DisplayChannelWeightG)] = "G Weight";
    texts[static_cast<int>(TextId::DisplayChannelWeightB)] = "B Weight";
    texts[static_cast<int>(TextId::DisplayResetWeights)] = "Reset Weights";
    texts[static_cast<int>(TextId::DisplayZoom)] = "Zoom";
    texts[static_cast<int>(TextId::DisplayZoomTooltip)] = "View magnification level.";
    texts[static_cast<int>(TextId::DisplayPanX)] = "Pan X";
    texts[static_cast<int>(TextId::DisplayPanXTooltip)] = "Horizontal view offset.";
    texts[static_cast<int>(TextId::DisplayPanY)] = "Pan Y";
    texts[static_cast<int>(TextId::DisplayPanYTooltip)] = "Vertical view offset.";
    texts[static_cast<int>(TextId::DisplayResetView)] = "Reset View";
    texts[static_cast<int>(TextId::DisplayResetViewTooltip)] = "Reset zoom and pan to defaults.";
    texts[static_cast<int>(TextId::DisplayCenterView)] = "Center View";
    texts[static_cast<int>(TextId::DisplayCenterViewTooltip)] = "Center the view on the grid.";
    texts[static_cast<int>(TextId::DisplayBrightness)] = "Brightness";
    texts[static_cast<int>(TextId::DisplayBrightnessTooltip)] = "Overall brightness adjustment.";
    texts[static_cast<int>(TextId::DisplayContrast)] = "Contrast";
    texts[static_cast<int>(TextId::DisplayContrastTooltip)] = "Contrast adjustment.";
    texts[static_cast<int>(TextId::DisplayGamma)] = "Gamma";
    texts[static_cast<int>(TextId::DisplayGammaTooltip)] = "Gamma correction for display.";
    texts[static_cast<int>(TextId::DisplayFilterMode)] = "Filter Mode";
    texts[static_cast<int>(TextId::DisplayFilterModeTooltip)] = "Texture filtering when zoomed.";
    texts[static_cast<int>(TextId::DisplayFilterBilinear)] = "Bilinear";
    texts[static_cast<int>(TextId::DisplayFilterNearest)] = "Nearest";
    texts[static_cast<int>(TextId::DisplayFilterSharpen)] = "Sharpen";
    texts[static_cast<int>(TextId::DisplayEdgeDetect)] = "Edge Detection";
    texts[static_cast<int>(TextId::DisplayEdgeDetectTooltip)] = "Highlight edges in the visualization.";
    texts[static_cast<int>(TextId::DisplayGlowSettings)] = "Glow Settings";
    texts[static_cast<int>(TextId::DisplayGlowStrength)] = "Glow Strength";
    texts[static_cast<int>(TextId::DisplayGlowStrengthTooltip)] = "Intensity of glow effect.";
    texts[static_cast<int>(TextId::DisplayGlowTint)] = "Glow Tint";
    texts[static_cast<int>(TextId::DisplayGlowTintTooltip)] = "Color of the glow effect.";
    texts[static_cast<int>(TextId::DisplayGlowIntensity)] = "Glow Intensity";
    texts[static_cast<int>(TextId::DisplayGlowIntensityTooltip)] = "Brightness of glow effect.";
    texts[static_cast<int>(TextId::DisplayCustomGradient)] = "Custom Gradient";
    texts[static_cast<int>(TextId::DisplayCustomGradientTooltip)] = "Colors for custom gradient mapping. Applied when using custom colormap.";
    texts[static_cast<int>(TextId::DisplayGradientStops)] = "Gradient Stops";
    texts[static_cast<int>(TextId::DisplayGradientStopsTooltip)] = "Number of color stops in custom gradient.";
    texts[static_cast<int>(TextId::DisplayGradientStopLabel)] = "Stop %d";
    texts[static_cast<int>(TextId::DisplayGridOverlay)] = "Grid Overlay";
    texts[static_cast<int>(TextId::DisplayGridOverlayTooltip)] = "Show grid lines over the visualization.";
    texts[static_cast<int>(TextId::DisplayGridOpacity)] = "Grid Opacity";
    texts[static_cast<int>(TextId::DisplayGridOpacityTooltip)] = "Transparency of grid lines.";
    texts[static_cast<int>(TextId::DisplayGridColor)] = "Grid Color";
    texts[static_cast<int>(TextId::DisplayGridColorTooltip)] = "Color of grid lines.";
    texts[static_cast<int>(TextId::DisplayGridLineThickness)] = "Grid Line Thickness";
    texts[static_cast<int>(TextId::DisplayGridLineThicknessTooltip)] = "Width of grid lines.";
    texts[static_cast<int>(TextId::DisplayGridSpacing)] = "Grid Spacing";
    texts[static_cast<int>(TextId::DisplayGridSpacingTooltip)] = "Distance between grid lines.";
    texts[static_cast<int>(TextId::DisplayGridEveryCell)] = "Every Cell";
    texts[static_cast<int>(TextId::DisplayGridCustomInterval)] = "Custom Interval";
    texts[static_cast<int>(TextId::DisplayGridInterval)] = "Interval";
    texts[static_cast<int>(TextId::DisplayGridIntervalTooltip)] = "Custom grid line spacing.";
    texts[static_cast<int>(TextId::DisplayGridMajorLines)] = "Major Lines";
    texts[static_cast<int>(TextId::DisplayGridMajorLinesTooltip)] = "Show emphasized lines at regular intervals.";
    texts[static_cast<int>(TextId::DisplayGridMajorEvery)] = "Major Every";
    texts[static_cast<int>(TextId::DisplayGridMajorEveryTooltip)] = "Interval for major grid lines.";
    texts[static_cast<int>(TextId::DisplayGridMajorOpacity)] = "Major Opacity";
    texts[static_cast<int>(TextId::DisplayGridMajorOpacityTooltip)] = "Opacity of major grid lines.";
    texts[static_cast<int>(TextId::DisplayInvertColors)] = "Invert Colors";
    texts[static_cast<int>(TextId::DisplayInvertColorsTooltip)] = "Invert all colors in the display.";
    texts[static_cast<int>(TextId::DisplayShowBoundary)] = "Show Boundary";
    texts[static_cast<int>(TextId::DisplayShowBoundaryTooltip)] = "Display grid boundary indicator.";
    texts[static_cast<int>(TextId::DisplayBoundaryColor)] = "Boundary Color";
    texts[static_cast<int>(TextId::DisplayBoundaryOpacity)] = "Boundary Opacity";
    texts[static_cast<int>(TextId::DisplayBoundaryStyle)] = "Boundary Style";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleTooltip)] = "Visual style of the boundary indicator.";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleSolid)] = "Solid";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleDashed)] = "Dashed";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleDotted)] = "Dotted";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleDouble)] = "Double";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleGlow)] = "Glow";
    texts[static_cast<int>(TextId::DisplayBoundaryWidth)] = "Boundary Width";
    texts[static_cast<int>(TextId::DisplayBoundaryWidthTooltip)] = "Width of the boundary line.";
    texts[static_cast<int>(TextId::DisplayDashLength)] = "Dash Length";
    texts[static_cast<int>(TextId::DisplayDashLengthTooltip)] = "Length of dashes for dashed boundary style.";
    texts[static_cast<int>(TextId::DisplayAnimateBoundary)] = "Animate Boundary";
    texts[static_cast<int>(TextId::DisplayAnimateBoundaryTooltip)] = "Animate the boundary indicator.";
    texts[static_cast<int>(TextId::DisplayBGColor)] = "Background Color";
    texts[static_cast<int>(TextId::DisplayBGColorTooltip)] = "Color of the background behind the grid.";
    texts[static_cast<int>(TextId::DisplayClipNullCells)] = "Clip Null Cells";
    texts[static_cast<int>(TextId::DisplayClipNullCellsTooltip)] = "Treat very small values as zero.";
    texts[static_cast<int>(TextId::DisplayClipThreshold)] = "Clip Threshold";
    texts[static_cast<int>(TextId::DisplayClipThresholdTooltip)] = "Values below this are treated as zero.";
    texts[static_cast<int>(TextId::DisplayColormapDeformation)] = "Colormap Deformation";
    texts[static_cast<int>(TextId::DisplayCmapOffset)] = "Offset";
    texts[static_cast<int>(TextId::DisplayCmapOffsetTooltip)] = "Shift the colormap cyclically.";
    texts[static_cast<int>(TextId::DisplayRangeMin)] = "Range Min";
    texts[static_cast<int>(TextId::DisplayRangeMinTooltip)] = "Minimum value mapped to colormap start.";
    texts[static_cast<int>(TextId::DisplayRangeMax)] = "Range Max";
    texts[static_cast<int>(TextId::DisplayRangeMaxTooltip)] = "Maximum value mapped to colormap end.";
    texts[static_cast<int>(TextId::DisplayPowerCurve)] = "Power Curve";
    texts[static_cast<int>(TextId::DisplayPowerCurveTooltip)] = "Apply power curve to colormap mapping.";
    texts[static_cast<int>(TextId::DisplayHueShift)] = "Hue Shift";
    texts[static_cast<int>(TextId::DisplayHueShiftTooltip)] = "Rotate colors around the hue circle.";
    texts[static_cast<int>(TextId::DisplaySaturation)] = "Saturation";
    texts[static_cast<int>(TextId::DisplaySaturationTooltip)] = "Scale saturation of the output color. 0 = grayscale, 1 = original, >1 = oversaturated.";
    texts[static_cast<int>(TextId::DisplayReverseColormap)] = "Reverse Colormap";
    texts[static_cast<int>(TextId::DisplayReverseColormapTooltip)] = "Reverse the direction of the colormap lookup.";
    texts[static_cast<int>(TextId::DisplayResetColormapDeformation)] = "Reset Colormap Deformation";
    texts[static_cast<int>(TextId::DisplayRGBChannelIntensity)] = "RGB Channel Intensity";
    
    // Analysis section
    texts[static_cast<int>(TextId::AnalysisEnable)] = "Enable Analysis";
    texts[static_cast<int>(TextId::AnalysisEnableTooltip)] = "Compute live statistics about the simulation state using a GPU analysis shader.";
    texts[static_cast<int>(TextId::AnalysisAutoPause)] = "Auto-Pause";
    texts[static_cast<int>(TextId::AnalysisAutoPauseTooltip)] = "Automatically pause when the simulation is detected as empty or stabilized.";
    texts[static_cast<int>(TextId::AnalysisAliveThreshold)] = "Alive Threshold";
    texts[static_cast<int>(TextId::AnalysisAliveThresholdTooltip)] = "Minimum cell value to be counted as 'alive'. Used for alive cell count, stabilization, and empty detection.";
    texts[static_cast<int>(TextId::AnalysisTotalMass)] = "Total Mass: %.2f";
    texts[static_cast<int>(TextId::AnalysisAliveCells)] = "Alive Cells: %d / %d (%.1f%%)";
    texts[static_cast<int>(TextId::AnalysisAverage)] = "Average: %.4f";
    texts[static_cast<int>(TextId::AnalysisMinMax)] = "Min: %.4f  Max: %.4f";
    texts[static_cast<int>(TextId::AnalysisVariance)] = "Variance: %.6f";
    texts[static_cast<int>(TextId::AnalysisCentroid)] = "Centroid: (%.1f, %.1f)";
    texts[static_cast<int>(TextId::AnalysisBounds)] = "Bounds: (%.0f,%.0f)-(%.0f,%.0f)";
    texts[static_cast<int>(TextId::AnalysisStateEmpty)] = "State: EMPTY";
    texts[static_cast<int>(TextId::AnalysisStateStabilized)] = "State: STABILIZED";
    texts[static_cast<int>(TextId::AnalysisStatePeriodic)] = "State: PERIODIC (T=%d, conf=%.0f%%)";
    texts[static_cast<int>(TextId::AnalysisStateActive)] = "State: Active";
    texts[static_cast<int>(TextId::AnalysisSpecies)] = "Species (est): %d";
    texts[static_cast<int>(TextId::AnalysisSpeed)] = "Speed: %.3f cells/step";
    texts[static_cast<int>(TextId::AnalysisDirection)] = "Direction: %.1f deg";
    texts[static_cast<int>(TextId::AnalysisOrientation)] = "Orientation: %.1f deg";
    texts[static_cast<int>(TextId::AnalysisGraphs)] = "Graphs";
    texts[static_cast<int>(TextId::AnalysisMass)] = "Mass";
    texts[static_cast<int>(TextId::AnalysisAlive)] = "Alive";
    texts[static_cast<int>(TextId::AnalysisCentroidGraph)] = "Centroid";
    texts[static_cast<int>(TextId::AnalysisSpeedGraph)] = "Speed";
    texts[static_cast<int>(TextId::AnalysisDirectionGraph)] = "Direction";
    texts[static_cast<int>(TextId::AnalysisAliveCellsGraph)] = "Alive Cells";
    texts[static_cast<int>(TextId::AnalysisCentroidXGraph)] = "Centroid X";
    texts[static_cast<int>(TextId::AnalysisCentroidYGraph)] = "Centroid Y";
    texts[static_cast<int>(TextId::AnalysisSpeedGraphTitle)] = "Speed";
    texts[static_cast<int>(TextId::AnalysisDirectionGraphTitle)] = "Direction";
    texts[static_cast<int>(TextId::AnalysisGraphXAxisStep)] = "step";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisMass)] = "mass";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisCells)] = "cells";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisX)] = "x";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisY)] = "y";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisPxPerSec)] = "px/s";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisDeg)] = "deg";
    texts[static_cast<int>(TextId::AnalysisDisplayWindow)] = "Display Window";
    texts[static_cast<int>(TextId::AnalysisDisplayWindowTooltip)] = "Number of history steps to display. 0 = show all available data.";
    texts[static_cast<int>(TextId::AnalysisGraphHeight)] = "Graph Height";
    texts[static_cast<int>(TextId::AnalysisAutoYScale)] = "Auto Y Scale";
    texts[static_cast<int>(TextId::AnalysisAutoYScaleTooltip)] = "Automatically scale Y axis to fit visible data.";
    
    // Accessibility section
    texts[static_cast<int>(TextId::AccessibilityLanguage)] = "Language";
    texts[static_cast<int>(TextId::AccessibilityLanguageTooltip)] = "Select the user interface language.";
    texts[static_cast<int>(TextId::AccessibilityEnglish)] = "English";
    texts[static_cast<int>(TextId::AccessibilityFrench)] = "Français";
    texts[static_cast<int>(TextId::AccessibilityUIScale)] = "UI Scale";
    texts[static_cast<int>(TextId::AccessibilityUIScaleTooltip)] = "Scale the user interface for high-resolution displays. Affects all UI elements.";
    texts[static_cast<int>(TextId::AccessibilityFontSize)] = "Font Size";
    texts[static_cast<int>(TextId::AccessibilityFontSizeTooltip)] = "Base font size for all text in the interface.";
    texts[static_cast<int>(TextId::AccessibilityHighContrast)] = "High Contrast";
    texts[static_cast<int>(TextId::AccessibilityHighContrastTooltip)] = "Enable high contrast mode for better visibility. Increases color contrast and text readability.";
    texts[static_cast<int>(TextId::AccessibilityReduceMotion)] = "Reduce Motion";
    texts[static_cast<int>(TextId::AccessibilityReduceMotionTooltip)] = "Reduce or disable animations and transitions for motion-sensitive users.";
    texts[static_cast<int>(TextId::AccessibilityKeyboardNav)] = "Keyboard Navigation";
    texts[static_cast<int>(TextId::AccessibilityKeyboardNavTooltip)] = "Enable keyboard navigation for the UI (Tab/Shift+Tab, arrows, and activation).";
    texts[static_cast<int>(TextId::AccessibilityFocusIndicators)] = "Focus Indicators";
    texts[static_cast<int>(TextId::AccessibilityFocusIndicatorsTooltip)] = "Show visible focus outlines for keyboard navigation.";
    texts[static_cast<int>(TextId::AccessibilityResetDefaults)] = "Reset to Defaults";
    texts[static_cast<int>(TextId::AccessibilityResetDefaultsTooltip)] = "Reset all accessibility settings to their default values.";
    texts[static_cast<int>(TextId::AccessibilitySystemDpiScale)] = "System DPI Scale: %.2fx";
    texts[static_cast<int>(TextId::AccessibilityEffectiveScale)] = "Effective Scale: %.2fx";
    
    // Common
    texts[static_cast<int>(TextId::CommonYes)] = "Yes";
    texts[static_cast<int>(TextId::CommonNo)] = "No";
    texts[static_cast<int>(TextId::CommonOK)] = "OK";
    texts[static_cast<int>(TextId::CommonCancel)] = "Cancel";
    texts[static_cast<int>(TextId::CommonApply)] = "Apply";
    texts[static_cast<int>(TextId::CommonReset)] = "Reset";
    texts[static_cast<int>(TextId::CommonDefault)] = "Default";
    texts[static_cast<int>(TextId::CommonEnabled)] = "Enabled";
    texts[static_cast<int>(TextId::CommonDisabled)] = "Disabled";
    texts[static_cast<int>(TextId::CommonOn)] = "On";
    texts[static_cast<int>(TextId::CommonOff)] = "Off";
    texts[static_cast<int>(TextId::CommonAll)] = "All";
    texts[static_cast<int>(TextId::CommonNone)] = "None";
    texts[static_cast<int>(TextId::CommonChannel)] = "Channel";
    texts[static_cast<int>(TextId::CommonRed)] = "Red";
    texts[static_cast<int>(TextId::CommonGreen)] = "Green";
    texts[static_cast<int>(TextId::CommonBlue)] = "Blue";
    texts[static_cast<int>(TextId::CommonAlpha)] = "Alpha";
}

void Localization::initFrench() {
    auto& texts = m_translations[Language::French];
    texts.resize(static_cast<size_t>(TextId::_Count));
    
    // Application title and main UI
    texts[static_cast<int>(TextId::AppTitle)] = "Lenia Explorer";
    texts[static_cast<int>(TextId::MainWindowTitle)] = "Lenia Explorer";
    
    // Section headers
    texts[static_cast<int>(TextId::SectionInfo)] = "Info";
    texts[static_cast<int>(TextId::SectionPerformance)] = "Performance";
    texts[static_cast<int>(TextId::SectionGrid)] = "Grille";
    texts[static_cast<int>(TextId::SectionDrawingTools)] = "Outils de Dessin";
    texts[static_cast<int>(TextId::SectionPresetsInit)] = "Présets & Initialisation";
    texts[static_cast<int>(TextId::SectionSimulation)] = "Simulation";
    texts[static_cast<int>(TextId::SectionGrowthFunction)] = "Fonction de Croissance";
    texts[static_cast<int>(TextId::SectionKernel)] = "Noyau";
    texts[static_cast<int>(TextId::SectionMultiChannel)] = "Multi-Canal";
    texts[static_cast<int>(TextId::SectionDisplay)] = "Affichage";
    texts[static_cast<int>(TextId::SectionAnalysis)] = "Analyse";
    texts[static_cast<int>(TextId::SectionAccessibility)] = "Accessibilité";
    
    // Info section
    texts[static_cast<int>(TextId::InfoCursor)] = "Curseur : (%d, %d)";
    texts[static_cast<int>(TextId::InfoValue)] = "Valeur : %.5f";
    texts[static_cast<int>(TextId::InfoGrid)] = "Grille : %d x %d  |  Étape : %d";
    texts[static_cast<int>(TextId::InfoChannels)] = "Canaux : %d  |  Règles : %d";
    texts[static_cast<int>(TextId::InfoRules)] = "Règles";
    texts[static_cast<int>(TextId::InfoStep)] = "Étape";
    texts[static_cast<int>(TextId::InfoShowConsoleStartup)] = "Afficher la Console au Démarrage";
    texts[static_cast<int>(TextId::InfoShowConsoleTooltip)] = "Si activé, la fenêtre console apparaîtra au démarrage.\nNécessite un redémarrage pour prendre effet.";
    
    // Keybinds
    texts[static_cast<int>(TextId::KeybindsHeader)] = "Raccourcis Clavier";
    texts[static_cast<int>(TextId::KeybindsText)] = 
        "Espace : Pause/Reprise\n"
        "S : Une étape | Maintenir S : Étape @5fps\n"
        "Maj+S : Étape @10fps\n"
        "R : Réinitialiser | C : Effacer\n"
        "+/- : Zoom | Flèches : Panoramique\n"
        "Début : Réinitialiser l'affichage | Tab : Basculer l'IU\n"
        "1-5 : Définir étapes/frame\n"
        "F11 : Plein écran | Échap : Quitter";
    
    // Theory section
    texts[static_cast<int>(TextId::TheoryHeader)] = "Théorie";
    texts[static_cast<int>(TextId::TheoryFundamentals)] = "Fondamentaux de Lenia";
    texts[static_cast<int>(TextId::TheoryFundamentalsText)] = 
        "Lenia est un système d'automate cellulaire continu qui généralise les automates discrets comme "
        "le Jeu de la Vie de Conway dans un domaine continu. Contrairement aux automates discrets avec états binaires "
        "et comptages de voisins entiers, Lenia utilise des états cellulaires continus dans [0,1], des espaces continus "
        "via des noyaux lisses, et le temps continu via l'intégration différentielle.";
    texts[static_cast<int>(TextId::TheoryEquation)] = 
        "L'équation fondamentale régissant Lenia est :\n"
        "  A(t+dt) = clip( A(t) + dt * G(K * A) )\n\n"
        "Où :\n"
        "  A(t) = champ d'état cellulaire au temps t (valeurs dans [0,1])\n"
        "  K = noyau de convolution (voisinage pondéré)\n"
        "  K * A = champ potentiel (sommes de voisinage)\n"
        "  G() = fonction de mappage de croissance\n"
        "  dt = pas de temps (taux d'intégration)\n"
        "  clip() = limite le résultat à [0,1]";
    texts[static_cast<int>(TextId::TheoryKernel)] = "Noyau de Convolution";
    texts[static_cast<int>(TextId::TheoryKernelText)] = 
        "Le noyau K définit comment les voisins influencent chaque cellule. Il est généralement symétrique radialement "
        "et normalisé (somme à 1). Le rayon du noyau R détermine la portée de l'interaction - un R plus grand crée des "
        "motifs plus grands et plus complexes mais nécessite plus de calcul.\n\n"
        "Formes de noyau courantes :\n"
        "- Shell Gaussien : exp(-(r-pics)^2/w^2), anneaux en forme de cloche lisse\n"
        "- Bump4 : (4r(1-r))^4, polynôme à support compact\n"
        "- Quad4 : variante de noyau polynomial pour dynamiques spécifiques\n"
        "- Multi-anneau : anneaux concentriques multiples avec poids indépendants (valeurs B)\n\n"
        "Le noyau est échantillonné sur une grille (2R+1)x(2R+1) centrée sur chaque cellule.";
    texts[static_cast<int>(TextId::TheoryGrowthFunction)] = "Fonction de Croissance G(u)";
    texts[static_cast<int>(TextId::TheoryGrowthFunctionText)] = 
        "La fonction de croissance G mappe le potentiel U à un taux de croissance dans [-1, +1]. Cela détermine "
        "comment les cellules répondent à leur somme de voisinage :\n\n"
        "- G(u) > 0 : La valeur cellulaire augmente (croissance/naissance)\n"
        "- G(u) < 0 : La valeur cellulaire diminue (décroissance/mort)\n"
        "- G(u) = 0 : La cellule reste stable\n\n"
        "Croissance Lenia Standard (Gaussienne) :\n"
        "  G(u) = 2 * exp(-((u - mu)^2) / (2 * sigma^2)) - 1\n\n"
        "Les paramètres mu et sigma contrôlent le comportement du motif.";
    texts[static_cast<int>(TextId::TheoryTimeIntegration)] = "Intégration Temporelle (dt)";
    texts[static_cast<int>(TextId::TheoryTimeIntegrationText)] = 
        "Le pas de temps dt contrôle la quantité de changement appliquée par étape de simulation :\n\n"
        "- Petit dt (0.01-0.1) : Évolution lisse et continue\n"
        "- Moyen dt (0.1-0.5) : Plage Lenia standard\n"
        "- Grand dt (0.5-1.0) : Comportement semblable à un discret";
    texts[static_cast<int>(TextId::TheoryMultiChannel)] = "Systèmes Multi-Canaux";
    texts[static_cast<int>(TextId::TheoryMultiChannelText)] = 
        "Lenia multi-canal étend le système à plusieurs champs interagissants (canaux). "
        "Chaque canal est un champ d'état indépendant qui peut influencer les autres canaux "
        "par le biais de règles de noyau.";
    texts[static_cast<int>(TextId::TheoryEdgeConditions)] = "Conditions aux Bords";
    texts[static_cast<int>(TextId::TheoryEdgeConditionsText)] = 
        "Les conditions aux bords déterminent ce qui se passe aux limites de la grille :\n"
        "- Périodique (Wrap) : Les bords se connectent aux côtés opposés\n"
        "- Clamp au Bord : Les valeurs à la limite sont étendues au-delà\n"
        "- Miroir : Les valeurs sont réfléchies aux limites";
    texts[static_cast<int>(TextId::TheoryWalls)] = "Murs";
    texts[static_cast<int>(TextId::TheoryWallsText)] = 
        "Les murs sont des obstacles persistants qui affectent la dynamique de la simulation.";
    texts[static_cast<int>(TextId::TheoryPatternCharacteristics)] = "Caractéristiques des Motifs";
    texts[static_cast<int>(TextId::TheoryPatternCharacteristicsText)] = 
        "Lenia peut produire divers types de motifs :\n"
        "- Solitons (Glisseurs) : Structures auto-entretenues et mouvantes\n"
        "- Oscillateurs : Motifs qui parcourent les états cycliquement\n"
        "- Vies Stables : Motifs stables et immuables\n"
        "- Chaotique/Turbulent : Dynamiques imprévisibles";
    texts[static_cast<int>(TextId::TheoryParameterRelationships)] = "Relations Entre Paramètres";
    texts[static_cast<int>(TextId::TheoryParameterRelationshipsText)] = 
        "Les interactions clés des paramètres affectent le comportement du motif :\n"
        "- mu et Noyau : Les valeurs mu plus élevées nécessitent des voisinages plus denses\n"
        "- sigma et Stabilité : Un sigma étroit crée des motifs précis mais fragiles\n"
        "- dt et Vitesse du Motif : Un dt plus petit rend les motifs plus lents";
    texts[static_cast<int>(TextId::TheoryColormapVisualization)] = "Colormap & Visualisation";
    texts[static_cast<int>(TextId::TheoryColormapVisualizationText)] = 
        "Modes d'affichage pour comprendre l'état de la simulation :\n"
        "- Affichage Monde : Affiche les états cellulaires avec la colormap choisie\n"
        "- Sommes des Voisins : Visualise le champ potentiel\n"
        "- Valeurs de Croissance : Affiche le champ de taux de croissance actuel";
    
    // Performance section (copying some from English for brevity in large sections)
    texts[static_cast<int>(TextId::PerfFPS)] = "FPS : %.1f";
    texts[static_cast<int>(TextId::PerfFPSTooltip)] = "Cadences par seconde actuelles.\nVert : 55+ (excellent)\nJaune : 30-55 (bon)\nOrange : 15-30 (acceptable)\nRouge : <15 (lent)";
    texts[static_cast<int>(TextId::PerfFrame)] = "Frame";
    texts[static_cast<int>(TextId::PerfFrameTime)] = "Frame : %.2f ms (moy)";
    texts[static_cast<int>(TextId::PerfFrameTimeLabel)] = "Temps de Frame :";
    texts[static_cast<int>(TextId::PerfFrameTimeStats)] = "min=%.2f  moy=%.2f  max=%.2f ms";
    texts[static_cast<int>(TextId::PerfGridSize)] = "Taille de Grille :";
    texts[static_cast<int>(TextId::PerfGridSizeCellsM)] = "%d x %d = %.2fM cellules";
    texts[static_cast<int>(TextId::PerfGridSizeCellsK)] = "%d x %d = %.1fK cellules";
    texts[static_cast<int>(TextId::PerfSimulation)] = "Simulation :";
    texts[static_cast<int>(TextId::PerfSimTimeStep)] = "%.2f ms/étape  (%.2f ms total)";
    texts[static_cast<int>(TextId::PerfThroughput)] = "Débit :";
    texts[static_cast<int>(TextId::PerfThroughputG)] = "%.2f Gcellules/s";
    texts[static_cast<int>(TextId::PerfThroughputM)] = "%.1f Mcellules/s";
    texts[static_cast<int>(TextId::PerfThroughputK)] = "%.0f Kcellules/s";
    texts[static_cast<int>(TextId::PerfThroughputTooltip)] = "Débit de traitement en cellules mises à jour par seconde.";
    texts[static_cast<int>(TextId::PerfKernelOps)] = "Opérations Noyau :";
    texts[static_cast<int>(TextId::PerfKernelOpsG)] = "%.2f Gops/étape";
    texts[static_cast<int>(TextId::PerfKernelOpsM)] = "%.1f Mops/étape";
    texts[static_cast<int>(TextId::PerfKernelOpsTooltip)] = "Opérations de convolution de noyau par étape de simulation (cellules x taille du noyau).";
    texts[static_cast<int>(TextId::PerfKernelSize)] = "Taille du Noyau :";
    texts[static_cast<int>(TextId::PerfKernelSizeSamples)] = "%dx%d = %d échantillons";
    texts[static_cast<int>(TextId::PerfStepsFrame)] = "Étapes/Frame :";
    texts[static_cast<int>(TextId::PerfTotalSteps)] = "Total Étapes :";
    texts[static_cast<int>(TextId::PerfExcellent)] = "Excellent";
    texts[static_cast<int>(TextId::PerfGood)] = "Bon";
    texts[static_cast<int>(TextId::PerfAcceptable)] = "Acceptable";
    texts[static_cast<int>(TextId::PerfSlow)] = "Lent";
    texts[static_cast<int>(TextId::PerfPerformance)] = "Performance : %s";
    texts[static_cast<int>(TextId::PerfPerformanceTooltip)] = "Réduisez la taille de la grille ou le rayon du noyau pour améliorer la performance.";
    texts[static_cast<int>(TextId::PerfShowResourceMonitor)] = "Afficher le Moniteur de Ressources";
    texts[static_cast<int>(TextId::PerfResourceUsage)] = "Utilisation des Ressources :";
    texts[static_cast<int>(TextId::PerfGPUMemory)] = "Mémoire GPU : %d / %d MB (%.0f%%)";
    texts[static_cast<int>(TextId::PerfGPUMemoryNA)] = "Mémoire GPU : N/A";
    texts[static_cast<int>(TextId::PerfCPUMemory)] = "Mémoire CPU : %.1f MB";
    texts[static_cast<int>(TextId::PerfTextureMemory)] = "Mémoire de Texture : ~%.2f MB";
    texts[static_cast<int>(TextId::PerfTextureMemoryTooltip)] = "Mémoire GPU estimée pour les textures de simulation.\n2x textures de grille + texture de noyau.";
    texts[static_cast<int>(TextId::PerfFrameTimeGraphTitle)] = "Temps de Frame";
    texts[static_cast<int>(TextId::PerfFrameTimeGraphXLabel)] = "frames";
    texts[static_cast<int>(TextId::PerfFrameTimeGraphYLabel)] = "ms";
    
    // Grid section - from English for brevity
    texts[static_cast<int>(TextId::GridSize)] = "Taille : %d x %d (%s cellules)";
    texts[static_cast<int>(TextId::GridWidth)] = "Largeur";
    texts[static_cast<int>(TextId::GridWidthTooltip)] = "Largeur de la grille en cellules. Les grilles plus grandes permettent des motifs plus complexes mais sont plus lentes. Doit être >= 32.";
    texts[static_cast<int>(TextId::GridHeight)] = "Hauteur";
    texts[static_cast<int>(TextId::GridHeightTooltip)] = "Hauteur de la grille en cellules. La grille s'enroule toroïdalement (les bords se connectent).";
    texts[static_cast<int>(TextId::GridTransformations)] = "Transformations :";
    texts[static_cast<int>(TextId::GridFlipHorizontal)] = "<->";
    texts[static_cast<int>(TextId::GridFlipHorizontalTooltip)] = "Retourner horizontalement (miroir gauche-droite).";
    texts[static_cast<int>(TextId::GridFlipVertical)] = "^v";
    texts[static_cast<int>(TextId::GridFlipVerticalTooltip)] = "Retourner verticalement (miroir haut-bas).";
    texts[static_cast<int>(TextId::GridRotateCW)] = "->|";
    texts[static_cast<int>(TextId::GridRotateCWTooltip)] = "Tourner 90 degrés dans le sens horaire.";
    texts[static_cast<int>(TextId::GridRotateCCW)] = "|<-";
    texts[static_cast<int>(TextId::GridRotateCCWTooltip)] = "Tourner la grille 90 degrés dans le sens antihoraire.";
    texts[static_cast<int>(TextId::GridEdgeConditions)] = "Conditions aux Bords :";
    texts[static_cast<int>(TextId::GridEdgeModeX)] = "Bord X";
    texts[static_cast<int>(TextId::GridEdgeModeXTooltip)] = "Comportement des bords horizontaux :\n- Périodique : Enroule autour (toroïdal)\n- Clamp : Utilise les valeurs des bords\n- Miroir : Réfléchit aux limites";
    texts[static_cast<int>(TextId::GridEdgeModeY)] = "Bord Y";
    texts[static_cast<int>(TextId::GridEdgeModeYTooltip)] = "Comportement des bords verticaux :\n- Périodique : Enroule autour (toroïdal)\n- Clamp : Utilise les valeurs des bords\n- Miroir : Réfléchit aux limites";
    texts[static_cast<int>(TextId::GridEdgePeriodic)] = "Périodique (Wrap)";
    texts[static_cast<int>(TextId::GridEdgeClamp)] = "Clamp au Bord";
    texts[static_cast<int>(TextId::GridEdgeMirror)] = "Miroir";
    texts[static_cast<int>(TextId::GridEdgeFade)] = "Fondu des Bords :";
    texts[static_cast<int>(TextId::GridEdgeFadeX)] = "Fondu X";
    texts[static_cast<int>(TextId::GridEdgeFadeXTooltip)] = "Distance de fondu aux bords horizontaux (0 = bord dur, 0.5 = demi-grille).";
    texts[static_cast<int>(TextId::GridEdgeFadeY)] = "Fondu Y";
    texts[static_cast<int>(TextId::GridEdgeFadeYTooltip)] = "Distance de fondu aux bords verticaux (0 = bord dur, 0.5 = demi-grille).";
    texts[static_cast<int>(TextId::GridOutsideDisplay)] = "Affichage Extérieur";
    texts[static_cast<int>(TextId::GridOutsideDisplayTooltip)] = "Affichage des zones en dehors de la grille.";
    texts[static_cast<int>(TextId::GridShowTiled)] = "Afficher Mosaïque";
    texts[static_cast<int>(TextId::GridBackgroundColor)] = "Couleur de Fond";
    texts[static_cast<int>(TextId::GridCheckerPattern)] = "Motif en Damier";
    
    // Infinite world
    texts[static_cast<int>(TextId::InfiniteWorldMode)] = "Mode Monde Infini";
    texts[static_cast<int>(TextId::InfiniteWorldEnable)] = "Activer le Monde Infini";
    texts[static_cast<int>(TextId::InfiniteWorldEnableTooltip)] = "Explorez un monde procédural infini.";
    texts[static_cast<int>(TextId::InfiniteWorldSettings)] = "Paramètres du Monde :";
    texts[static_cast<int>(TextId::InfiniteChunkSize)] = "Taille des Blocs";
    texts[static_cast<int>(TextId::InfiniteChunkSizeTooltip)] = "Taille de chaque bloc en cellules.";
    texts[static_cast<int>(TextId::InfiniteLoadRadius)] = "Rayon de Chargement";
    texts[static_cast<int>(TextId::InfiniteLoadRadiusTooltip)] = "Nombre de blocs à charger.";
    texts[static_cast<int>(TextId::InfiniteMaxChunks)] = "Max Blocs";
    texts[static_cast<int>(TextId::InfiniteMaxChunksTooltip)] = "Maximum de blocs en mémoire.";
    texts[static_cast<int>(TextId::InfiniteNavigation)] = "Navigation :";
    texts[static_cast<int>(TextId::InfiniteNavigationTooltip)] = "Naviguer entre les blocs.";
    texts[static_cast<int>(TextId::InfiniteChunkPosition)] = "Position : (%d, %d)";
    texts[static_cast<int>(TextId::InfiniteWorldOffset)] = "Décalage : (%.2f, %.2f)";
    texts[static_cast<int>(TextId::InfiniteHome)] = "Origine";
    texts[static_cast<int>(TextId::InfiniteNavNorth)] = "N";
    texts[static_cast<int>(TextId::InfiniteNavWest)] = "O";
    texts[static_cast<int>(TextId::InfiniteNavEast)] = "E";
    texts[static_cast<int>(TextId::InfiniteNavSouth)] = "S";
    texts[static_cast<int>(TextId::InfiniteExploreSpeed)] = "Vitesse d'Exploration";
    texts[static_cast<int>(TextId::InfiniteExploreSpeedTooltip)] = "Vitesse de navigation.";
    texts[static_cast<int>(TextId::InfiniteAutoLoad)] = "Chargement Auto";
    texts[static_cast<int>(TextId::InfiniteAutoLoadTooltip)] = "Charger automatiquement les nouveaux blocs.";
    texts[static_cast<int>(TextId::InfiniteDisplayOptions)] = "Options d'Affichage :";
    texts[static_cast<int>(TextId::InfiniteShowChunkGrid)] = "Afficher Grille des Blocs";
    texts[static_cast<int>(TextId::InfiniteShowChunkGridTooltip)] = "Afficher les bordures des blocs.";
    texts[static_cast<int>(TextId::InfiniteEdgeFade)] = "Fondu des Bords";
    texts[static_cast<int>(TextId::InfiniteEdgeFadeTooltip)] = "Fondu aux bords du monde.";
    texts[static_cast<int>(TextId::InfinitePersistence)] = "Persistance";
    texts[static_cast<int>(TextId::InfinitePersistenceTooltip)] = "Gestion de l'état des blocs.";
    texts[static_cast<int>(TextId::InfinitePersistenceNone)] = "Aucune (Effacer)";
    texts[static_cast<int>(TextId::InfinitePersistencePreserve)] = "Préserver l'État";
    texts[static_cast<int>(TextId::InfinitePersistenceSeed)] = "Basé sur Graine";
    texts[static_cast<int>(TextId::InfinitePanTip)] = "Astuce : Clic-molette pour déplacer";
    texts[static_cast<int>(TextId::InfiniteScrollTip)] = "Molette pour zoomer";
    
    // Drawing tools
    texts[static_cast<int>(TextId::DrawToolMode)] = "Mode Outil";
    texts[static_cast<int>(TextId::DrawToolBrush)] = "Pinceau (Cellules)";
    texts[static_cast<int>(TextId::DrawToolObstacle)] = "Obstacle (Barrière)";
    texts[static_cast<int>(TextId::DrawToolModeTooltip)] = "Pinceau peint des cellules. Obstacle crée des barrières.";
    texts[static_cast<int>(TextId::DrawEnableDrawing)] = "Activer le Dessin";
    texts[static_cast<int>(TextId::DrawEnableDrawingTooltip)] = "Activer ou désactiver le dessin.";
    texts[static_cast<int>(TextId::DrawObstacleModeActive)] = "MODE OBSTACLE ACTIF";
    texts[static_cast<int>(TextId::DrawBrushModeActive)] = "MODE PINCEAU ACTIF";
    texts[static_cast<int>(TextId::DrawShapeSize)] = "Forme & Taille";
    texts[static_cast<int>(TextId::DrawShape)] = "Forme";
    texts[static_cast<int>(TextId::DrawShapeTooltip)] = "Forme du pinceau/obstacle.";
    texts[static_cast<int>(TextId::DrawShapeCircle)] = "Cercle";
    texts[static_cast<int>(TextId::DrawShapeSquare)] = "Carré";
    texts[static_cast<int>(TextId::DrawShapeDiamond)] = "Losange";
    texts[static_cast<int>(TextId::DrawShapeRing)] = "Anneau";
    texts[static_cast<int>(TextId::DrawShapeStar5)] = "Étoile (5pt)";
    texts[static_cast<int>(TextId::DrawShapeStar6)] = "Étoile (6pt)";
    texts[static_cast<int>(TextId::DrawShapeHexagon)] = "Hexagone";
    texts[static_cast<int>(TextId::DrawShapeCross)] = "Croix";
    texts[static_cast<int>(TextId::DrawShapePlus)] = "Plus";
    texts[static_cast<int>(TextId::DrawShapeGaussian)] = "Gaussien";
    texts[static_cast<int>(TextId::DrawShapeNoiseDisc)] = "Disque de Bruit";
    texts[static_cast<int>(TextId::DrawShapeGradientDisc)] = "Disque en Dégradé";
    texts[static_cast<int>(TextId::DrawSize)] = "Taille";
    texts[static_cast<int>(TextId::DrawSizeTooltip)] = "Taille du pinceau en cellules.";
    texts[static_cast<int>(TextId::DrawFalloff)] = "Atténuation";
    texts[static_cast<int>(TextId::DrawFalloffTooltip)] = "Douceur des bords.";
    texts[static_cast<int>(TextId::DrawMethod)] = "Méthode de Dessin";
    texts[static_cast<int>(TextId::DrawModeFreehand)] = "Main Libre";
    texts[static_cast<int>(TextId::DrawModeLine)] = "Ligne";
    texts[static_cast<int>(TextId::DrawModeCircle)] = "Cercle";
    texts[static_cast<int>(TextId::DrawModeRectangle)] = "Rectangle";
    texts[static_cast<int>(TextId::DrawModeTooltip)] = "Main Libre : Cliquer-glisser\nLigne : Cliquer début, relâcher fin";
    texts[static_cast<int>(TextId::DrawDrawing)] = "Dessin... (Clic-droit pour annuler)";
    texts[static_cast<int>(TextId::DrawClickToStart)] = "Cliquer sur la grille pour commencer";
    texts[static_cast<int>(TextId::DrawObstacleSettings)] = "Paramètres d'Obstacle";
    texts[static_cast<int>(TextId::DrawCellValue)] = "Valeur de Cellule";
    texts[static_cast<int>(TextId::DrawCellValueTooltip)] = "Valeur fixe pour les obstacles.";
    texts[static_cast<int>(TextId::DrawDisplayColor)] = "Couleur d'Affichage";
    texts[static_cast<int>(TextId::DrawDisplayColorTooltip)] = "Couleur visuelle des obstacles.";
    texts[static_cast<int>(TextId::DrawAffectedChannels)] = "Canaux Affectés :";
    texts[static_cast<int>(TextId::DrawAffectedChannelsTooltip)] = "Quels canaux l'obstacle affecte.";
    texts[static_cast<int>(TextId::DrawBlendMode)] = "Mode de Fusion";
    texts[static_cast<int>(TextId::DrawBlendModeTooltip)] = "Comment fusionner avec l'existant.";
    texts[static_cast<int>(TextId::DrawBlendReplace)] = "Remplacer";
    texts[static_cast<int>(TextId::DrawBlendMax)] = "Max";
    texts[static_cast<int>(TextId::DrawBlendReplaceStronger)] = "Remplacer si Plus Fort";
    texts[static_cast<int>(TextId::DrawBlendBlend)] = "Fusionner";
    texts[static_cast<int>(TextId::DrawBlendErase)] = "Effacer";
    texts[static_cast<int>(TextId::DrawClearAllObstacles)] = "Effacer Tous les Obstacles";
    texts[static_cast<int>(TextId::DrawClearAllObstaclesTooltip)] = "Supprimer tous les obstacles.";
    texts[static_cast<int>(TextId::DrawBrushSettings)] = "Paramètres du Pinceau";
    texts[static_cast<int>(TextId::DrawPaintMode)] = "Mode de Peinture";
    texts[static_cast<int>(TextId::DrawPaintModeTooltip)] = "Comment appliquer la valeur.";
    texts[static_cast<int>(TextId::DrawPaintModeSet)] = "Définir";
    texts[static_cast<int>(TextId::DrawPaintModeAdd)] = "Ajouter";
    texts[static_cast<int>(TextId::DrawPaintModeSubtract)] = "Soustraire";
    texts[static_cast<int>(TextId::DrawPaintModeMax)] = "Max";
    texts[static_cast<int>(TextId::DrawPaintModeMin)] = "Min";
    texts[static_cast<int>(TextId::DrawPaintModeErase)] = "Effacer";
    texts[static_cast<int>(TextId::DrawBrushValue)] = "Valeur";
    texts[static_cast<int>(TextId::DrawBrushValueTooltip)] = "Valeur à peindre.";
    texts[static_cast<int>(TextId::DrawStrength)] = "Intensité";
    texts[static_cast<int>(TextId::DrawStrengthTooltip)] = "Multiplicateur d'intensité.";
    texts[static_cast<int>(TextId::DrawTargetChannel)] = "Canal Cible";
    texts[static_cast<int>(TextId::DrawTargetChannelTooltip)] = "Canal(aux) à peindre.";
    texts[static_cast<int>(TextId::DrawSymmetry)] = "Symétrie";
    texts[static_cast<int>(TextId::DrawMirrorX)] = "Miroir X";
    texts[static_cast<int>(TextId::DrawMirrorY)] = "Miroir Y";
    texts[static_cast<int>(TextId::DrawMirrorTooltip)] = "Refléter les traits au centre.";
    texts[static_cast<int>(TextId::DrawRadialSymmetry)] = "Symétrie Radiale";
    texts[static_cast<int>(TextId::DrawRadialSymmetryTooltip)] = "Symétrie de rotation.";
    texts[static_cast<int>(TextId::DrawStrokeSpacing)] = "Espacement des Traits";
    texts[static_cast<int>(TextId::DrawBrushSpacing)] = "Espacement du Pinceau";
    texts[static_cast<int>(TextId::DrawBrushSpacingTooltip)] = "Distance entre applications.";
    texts[static_cast<int>(TextId::DrawSmoothInterpolation)] = "Interpolation Lisse";
    texts[static_cast<int>(TextId::DrawSmoothInterpolationTooltip)] = "Interpoler les positions.";
    
    // Presets section
    texts[static_cast<int>(TextId::PresetsCategory)] = "Catégorie";
    texts[static_cast<int>(TextId::PresetsSearch)] = "Recherche";
    texts[static_cast<int>(TextId::PresetsSearchHint)] = "Rechercher préréglages...";
    texts[static_cast<int>(TextId::PresetsSelected)] = "Sélectionné : %s";
    texts[static_cast<int>(TextId::PresetsSpecies)] = "Espèces";
    texts[static_cast<int>(TextId::PresetsKernel)] = "Noyau";
    texts[static_cast<int>(TextId::PresetsProcedural)] = "Procédural";
    texts[static_cast<int>(TextId::PresetsShown)] = "%d préréglages affichés";
    texts[static_cast<int>(TextId::PresetsCountShown)] = "%d préréglages (%d affichés)";
    texts[static_cast<int>(TextId::PresetsRandomize)] = "Aléatoire";
    texts[static_cast<int>(TextId::PresetsClear)] = "Effacer";
    texts[static_cast<int>(TextId::PresetsResetPreset)] = "Réinitialiser au Préréglage";
    texts[static_cast<int>(TextId::PresetsPlacement)] = "Placement";
    texts[static_cast<int>(TextId::PresetsPlacementCenter)] = "Centre";
    texts[static_cast<int>(TextId::PresetsPlacementTopLeft)] = "Haut Gauche";
    texts[static_cast<int>(TextId::PresetsPlacementTopRight)] = "Haut Droite";
    texts[static_cast<int>(TextId::PresetsPlacementBottomLeft)] = "Bas Gauche";
    texts[static_cast<int>(TextId::PresetsPlacementBottomRight)] = "Bas Droite";
    texts[static_cast<int>(TextId::PresetsPlacementTop)] = "Haut";
    texts[static_cast<int>(TextId::PresetsPlacementBottom)] = "Bas";
    texts[static_cast<int>(TextId::PresetsPlacementLeft)] = "Gauche";
    texts[static_cast<int>(TextId::PresetsPlacementRight)] = "Droite";
    texts[static_cast<int>(TextId::PresetsPlacementRandom)] = "Aléatoire";
    texts[static_cast<int>(TextId::PresetsPlacementGrid)] = "Grille";
    texts[static_cast<int>(TextId::PresetsPlacementTwoPlace)] = "Deux Places";
    texts[static_cast<int>(TextId::PresetsPlacementScatter)] = "Dispersion";
    texts[static_cast<int>(TextId::PresetsCount)] = "Nombre";
    texts[static_cast<int>(TextId::PresetsScale)] = "Échelle";
    texts[static_cast<int>(TextId::PresetsRotation)] = "Rotation";
    texts[static_cast<int>(TextId::PresetsRotation0)] = "0°";
    texts[static_cast<int>(TextId::PresetsRotation90)] = "90°";
    texts[static_cast<int>(TextId::PresetsRotation180)] = "180°";
    texts[static_cast<int>(TextId::PresetsRotation270)] = "270°";
    texts[static_cast<int>(TextId::PresetsMargin)] = "Marge";
    texts[static_cast<int>(TextId::PresetsRandomFlip)] = "Retournement Aléatoire";
    texts[static_cast<int>(TextId::PresetsFlipHorizontal)] = "Retourner Horizontal";
    texts[static_cast<int>(TextId::PresetsFlipVertical)] = "Retourner Vertical";
    texts[static_cast<int>(TextId::PresetsPlaceSpacing)] = "Espacement";
    texts[static_cast<int>(TextId::PresetsMinSeparation)] = "Séparation Min";
    texts[static_cast<int>(TextId::PresetsClearGridFirst)] = "Effacer la Grille d'Abord";
    texts[static_cast<int>(TextId::PresetsApplyPlacement)] = "Appliquer le Placement";
    texts[static_cast<int>(TextId::PresetsRadiusRings)] = "R=%d anneaux=%d";
    texts[static_cast<int>(TextId::PresetsMu)] = "mu=%.3f";
    texts[static_cast<int>(TextId::PresetsSigma)] = "sigma=%.4f";
    
    // Simulation section
    texts[static_cast<int>(TextId::SimPaused)] = "PAUSE (Espace pour lancer, S pour étape)";
    texts[static_cast<int>(TextId::SimPausedLabel)] = "En Pause (Espace)";
    texts[static_cast<int>(TextId::SimHoldToStep)] = "Maintenir S pour avancer";
    texts[static_cast<int>(TextId::SimStepsPerFrame)] = "Étapes/Image";
    texts[static_cast<int>(TextId::SimStepsPerFrameTooltip)] = "Étapes de simulation par image.";
    texts[static_cast<int>(TextId::SimStep)] = "Étape";
    texts[static_cast<int>(TextId::SimStepFormat)] = "Étape : %d";
    texts[static_cast<int>(TextId::SimTime)] = "Temps";
    texts[static_cast<int>(TextId::SimTimeMs)] = "Sim : %.2f ms";
    
    // Growth function section
    texts[static_cast<int>(TextId::GrowthType)] = "Type de Croissance";
    texts[static_cast<int>(TextId::GrowthTypeTooltip)] = "Fonction de croissance.";
    texts[static_cast<int>(TextId::GrowthTypeLenia)] = "Lenia (Gaussien)";
    texts[static_cast<int>(TextId::GrowthTypeStep)] = "Fonction Échelon";
    texts[static_cast<int>(TextId::GrowthTypeGameOfLife)] = "Jeu de la Vie";
    texts[static_cast<int>(TextId::GrowthTypeSmoothLife)] = "SmoothLife";
    texts[static_cast<int>(TextId::GrowthTypePolynomial)] = "Polynomial";
    texts[static_cast<int>(TextId::GrowthTypeExponential)] = "Exponentiel";
    texts[static_cast<int>(TextId::GrowthTypeDoublePeak)] = "Double Pic";
    texts[static_cast<int>(TextId::GrowthTypeAsymptotic)] = "Asymptotique";
    texts[static_cast<int>(TextId::GrowthTypeSoftClip)] = "Soft Clip";
    texts[static_cast<int>(TextId::GrowthTypeLargerThanLife)] = "Larger Than Life";
    texts[static_cast<int>(TextId::GrowthTypeQuad4)] = "Quad4";
    texts[static_cast<int>(TextId::GrowthMu)] = "Mu (μ)";
    texts[static_cast<int>(TextId::GrowthMuTooltip)] = "Centre de croissance.";
    texts[static_cast<int>(TextId::GrowthSigma)] = "Sigma (σ)";
    texts[static_cast<int>(TextId::GrowthSigmaTooltip)] = "Largeur de croissance.";
    texts[static_cast<int>(TextId::GrowthDt)] = "Pas de Temps (dt)";
    texts[static_cast<int>(TextId::GrowthDtTooltip)] = "Taille du pas d'intégration.";
    texts[static_cast<int>(TextId::GrowthPlotGoLHint)] = "Jeu de la Vie B3/S23 (axe x : voisins 0-8)";
    texts[static_cast<int>(TextId::GrowthPlotAsymptoticHint)] = "Asymptotique  mu=%.4f sigma=%.4f";
    texts[static_cast<int>(TextId::GrowthPlotSoftClipHint)] = "SoftClip  mu=%.4f sigma=%.4f";
    texts[static_cast<int>(TextId::GrowthPlotLTLHint)] = "Larger-than-Life  mu=%.4f sigma=%.4f";
    texts[static_cast<int>(TextId::GrowthPlotDefaultHint)] = "Croissance(U)  mu=%.4f  sigma=%.4f";
    
    // Kernel section
    texts[static_cast<int>(TextId::KernelType)] = "Type de Noyau";
    texts[static_cast<int>(TextId::KernelTypeTooltip)] = "Forme du noyau de convolution.";
    texts[static_cast<int>(TextId::KernelPreset)] = "Préréglage de Noyau";
    texts[static_cast<int>(TextId::KernelRadius)] = "Rayon";
    texts[static_cast<int>(TextId::KernelRadiusTooltip)] = "Rayon du noyau en cellules.";
    texts[static_cast<int>(TextId::KernelRings)] = "Anneaux";
    texts[static_cast<int>(TextId::KernelRingsTooltip)] = "Nombre d'anneaux concentriques.";
    texts[static_cast<int>(TextId::KernelRingWeight)] = "Poids Anneau %d";
    texts[static_cast<int>(TextId::KernelRingWeightTooltip)] = "Poids pour l'anneau %d.";
    texts[static_cast<int>(TextId::KernelAdvanced)] = "Avancé";
    texts[static_cast<int>(TextId::KernelAnisotropy)] = "Anisotropie";
    texts[static_cast<int>(TextId::KernelAnisotropyTooltip)] = "Biais directionnel du noyau.";
    texts[static_cast<int>(TextId::KernelDirection)] = "Direction";
    texts[static_cast<int>(TextId::KernelDirectionTooltip)] = "Angle du biais en degrés.";
    texts[static_cast<int>(TextId::KernelTimeVarying)] = "Variable dans le Temps";
    texts[static_cast<int>(TextId::KernelTimeVaryingTooltip)] = "Modulation temporelle.";
    texts[static_cast<int>(TextId::KernelPulseFrequency)] = "Fréquence de Pulsation";
    texts[static_cast<int>(TextId::KernelPulseFrequencyTooltip)] = "Fréquence de pulsation.";
    texts[static_cast<int>(TextId::KernelModifier)] = "Modificateur";
    texts[static_cast<int>(TextId::KernelModifierTooltip)] = "Modifications supplémentaires.";
    texts[static_cast<int>(TextId::KernelModifierNone)] = "Aucun";
    texts[static_cast<int>(TextId::KernelModifierNegativeRing)] = "Anneau Négatif";
    texts[static_cast<int>(TextId::KernelShowPreview)] = "Aperçu du Noyau";
    texts[static_cast<int>(TextId::KernelPerRuleNote)] = "Note : Paramètres configurables par règle en mode Multi-Canal.";
    texts[static_cast<int>(TextId::KernelCrossSection)] = "Coupe du Noyau";
    texts[static_cast<int>(TextId::KernelCrossSectionWithSize)] = "Coupe du Noyau (%dx%d)";
    
    // Kernel types
    texts[static_cast<int>(TextId::KernelGaussianShell)] = "Shell Gaussien";
    texts[static_cast<int>(TextId::KernelBump4)] = "Bump4";
    texts[static_cast<int>(TextId::KernelMultiringGauss)] = "Multi-anneau Gaussien";
    texts[static_cast<int>(TextId::KernelMultiringBump4)] = "Multi-anneau Bump4";
    texts[static_cast<int>(TextId::KernelGameOfLife)] = "Jeu de la Vie";
    texts[static_cast<int>(TextId::KernelStepUnimodal)] = "Échelon (Unimodal)";
    texts[static_cast<int>(TextId::KernelCosineShell)] = "Shell Cosinus";
    texts[static_cast<int>(TextId::KernelMexicanHat)] = "Chapeau Mexicain";
    texts[static_cast<int>(TextId::KernelQuad4)] = "Quad4";
    texts[static_cast<int>(TextId::KernelMultiringQuad4)] = "Multi-anneau Quad4";
    texts[static_cast<int>(TextId::KernelCone)] = "Cône";
    texts[static_cast<int>(TextId::KernelTorusDualRing)] = "Tore (Double Anneau)";
    texts[static_cast<int>(TextId::KernelRingSharp)] = "Anneau (Net)";
    texts[static_cast<int>(TextId::KernelGaussianMixture)] = "Mélange Gaussien";
    texts[static_cast<int>(TextId::KernelSinc)] = "Sinc";
    texts[static_cast<int>(TextId::KernelWaveletRicker)] = "Ondelette (Ricker)";
    texts[static_cast<int>(TextId::KernelNegativeRing)] = "Anneau Négatif";
    
    // Multi-channel section
    texts[static_cast<int>(TextId::MultiChannels)] = "Canaux";
    texts[static_cast<int>(TextId::MultiChannelsTooltip)] = "Nombre de canaux d'état.";
    texts[static_cast<int>(TextId::MultiChannelsSingle)] = "1 (Simple)";
    texts[static_cast<int>(TextId::MultiChannelsRGB)] = "3 (RVB)";
    texts[static_cast<int>(TextId::MultiRules)] = "Règles";
    texts[static_cast<int>(TextId::MultiRulesCount)] = "Règles : %d";
    texts[static_cast<int>(TextId::MultiAddRule)] = "Ajouter Règle";
    texts[static_cast<int>(TextId::MultiAddRuleTooltip)] = "Ajouter une nouvelle règle.";
    texts[static_cast<int>(TextId::MultiRemoveRule)] = "Supprimer";
    texts[static_cast<int>(TextId::MultiRemoveRuleTooltip)] = "Supprimer cette règle.";
    texts[static_cast<int>(TextId::MultiChannelRouting)] = "Routage des Canaux";
    texts[static_cast<int>(TextId::MultiRule)] = "Règle %d";
    texts[static_cast<int>(TextId::MultiRuleSummary)] = "r%d m=%.3f s=%.4f h=%.2f";
    texts[static_cast<int>(TextId::MultiSourceChannel)] = "Canal Source";
    texts[static_cast<int>(TextId::MultiDestChannel)] = "Canal Destination";
    texts[static_cast<int>(TextId::MultiStrength)] = "Force";
    texts[static_cast<int>(TextId::MultiStrengthH)] = "Force (h)";
    texts[static_cast<int>(TextId::MultiRadiusFrac)] = "Fraction de Rayon";
    texts[static_cast<int>(TextId::MultiKernelLabel)] = "Noyau";
    texts[static_cast<int>(TextId::MultiGrowthLabel)] = "Croissance";
    
    // Display section
    texts[static_cast<int>(TextId::DisplayMode)] = "Mode d'Affichage";
    texts[static_cast<int>(TextId::DisplayModeTooltip)] = "Quoi visualiser.";
    texts[static_cast<int>(TextId::DisplayWorld)] = "Monde";
    texts[static_cast<int>(TextId::DisplayNeighborSums)] = "Sommes Voisinage";
    texts[static_cast<int>(TextId::DisplayGrowthValues)] = "Valeurs de Croissance";
    texts[static_cast<int>(TextId::DisplayKernel)] = "Noyau";
    texts[static_cast<int>(TextId::DisplayDelta)] = "Delta";
    texts[static_cast<int>(TextId::DisplayVectorField)] = "Champ Vectoriel";
    texts[static_cast<int>(TextId::DisplayContourLines)] = "Lignes de Contour";
    texts[static_cast<int>(TextId::DisplayHeatMap)] = "Carte de Chaleur";
    texts[static_cast<int>(TextId::DisplayActivityMap)] = "Carte d'Activité";
    texts[static_cast<int>(TextId::DisplayDifference)] = "Différence";
    texts[static_cast<int>(TextId::DisplayVectorScale)] = "Échelle Vecteur";
    texts[static_cast<int>(TextId::DisplayVectorScaleTooltip)] = "Échelle des flèches.";
    texts[static_cast<int>(TextId::DisplayVectorDensity)] = "Densité Vecteur";
    texts[static_cast<int>(TextId::DisplayVectorDensityTooltip)] = "Nombre de vecteurs.";
    texts[static_cast<int>(TextId::DisplayContourLevels)] = "Niveaux de Contour";
    texts[static_cast<int>(TextId::DisplayContourLevelsTooltip)] = "Nombre de lignes de contour.";
    texts[static_cast<int>(TextId::DisplayLineThickness)] = "Épaisseur de Ligne";
    texts[static_cast<int>(TextId::DisplayLineThicknessTooltip)] = "Épaisseur des contours.";
    texts[static_cast<int>(TextId::DisplayActivityDecay)] = "Décroissance Activité";
    texts[static_cast<int>(TextId::DisplayActivityDecayTooltip)] = "Vitesse de disparition.";
    texts[static_cast<int>(TextId::DisplayColormap)] = "Palette de Couleurs";
    texts[static_cast<int>(TextId::DisplayColormapTooltip)] = "Schéma de couleurs.";
    texts[static_cast<int>(TextId::DisplayColormapLenia)] = "Lenia";
    texts[static_cast<int>(TextId::DisplayColormapViridis)] = "Viridis";
    texts[static_cast<int>(TextId::DisplayColormapMagma)] = "Magma";
    texts[static_cast<int>(TextId::DisplayColormapInferno)] = "Inferno";
    texts[static_cast<int>(TextId::DisplayColormapPlasma)] = "Plasma";
    texts[static_cast<int>(TextId::DisplayColormapGrayscale)] = "Niveaux de Gris";
    texts[static_cast<int>(TextId::DisplayColormapGrayscaleInv)] = "Niveaux de Gris Inv.";
    texts[static_cast<int>(TextId::DisplayColormapJet)] = "Jet";
    texts[static_cast<int>(TextId::DisplayUseColormapMulti)] = "Palette Multi-Canal";
    texts[static_cast<int>(TextId::DisplayUseColormapMultiTooltip)] = "Appliquer la palette au multi-canal.";
    texts[static_cast<int>(TextId::DisplayBlendMode)] = "Mode de Fusion";
    texts[static_cast<int>(TextId::DisplayBlendModeTooltip)] = "Comment combiner les canaux.";
    texts[static_cast<int>(TextId::DisplayBlendLuminance)] = "Luminance";
    texts[static_cast<int>(TextId::DisplayBlendAverage)] = "Moyenne";
    texts[static_cast<int>(TextId::DisplayBlendMaxChannel)] = "Canal Max";
    texts[static_cast<int>(TextId::DisplayBlendMinChannel)] = "Canal Min";
    texts[static_cast<int>(TextId::DisplayBlendRedOnly)] = "Rouge Seul";
    texts[static_cast<int>(TextId::DisplayBlendGreenOnly)] = "Vert Seul";
    texts[static_cast<int>(TextId::DisplayBlendBlueOnly)] = "Bleu Seul";
    texts[static_cast<int>(TextId::DisplayChannelWeights)] = "Poids des Canaux";
    texts[static_cast<int>(TextId::DisplayChannelWeightsTooltip)] = "Poids pour le calcul.";
    texts[static_cast<int>(TextId::DisplayChannelWeightR)] = "Poids R";
    texts[static_cast<int>(TextId::DisplayChannelWeightG)] = "Poids V";
    texts[static_cast<int>(TextId::DisplayChannelWeightB)] = "Poids B";
    texts[static_cast<int>(TextId::DisplayResetWeights)] = "Réinitialiser Poids";
    texts[static_cast<int>(TextId::DisplayZoom)] = "Zoom";
    texts[static_cast<int>(TextId::DisplayZoomTooltip)] = "Niveau de grossissement.";
    texts[static_cast<int>(TextId::DisplayPanX)] = "Déplacement X";
    texts[static_cast<int>(TextId::DisplayPanXTooltip)] = "Décalage horizontal.";
    texts[static_cast<int>(TextId::DisplayPanY)] = "Déplacement Y";
    texts[static_cast<int>(TextId::DisplayPanYTooltip)] = "Décalage vertical.";
    texts[static_cast<int>(TextId::DisplayResetView)] = "Réinitialiser l'Affichage";
    texts[static_cast<int>(TextId::DisplayResetViewTooltip)] = "Réinitialiser zoom et déplacement.";
    texts[static_cast<int>(TextId::DisplayCenterView)] = "Centrer l'Affichage";
    texts[static_cast<int>(TextId::DisplayCenterViewTooltip)] = "Centrer sur la grille.";
    texts[static_cast<int>(TextId::DisplayBrightness)] = "Luminosité";
    texts[static_cast<int>(TextId::DisplayBrightnessTooltip)] = "Réglage de luminosité.";
    texts[static_cast<int>(TextId::DisplayContrast)] = "Contraste";
    texts[static_cast<int>(TextId::DisplayContrastTooltip)] = "Réglage de contraste.";
    texts[static_cast<int>(TextId::DisplayGamma)] = "Gamma";
    texts[static_cast<int>(TextId::DisplayGammaTooltip)] = "Correction gamma.";
    texts[static_cast<int>(TextId::DisplayFilterMode)] = "Mode de Filtre";
    texts[static_cast<int>(TextId::DisplayFilterModeTooltip)] = "Filtrage de texture.";
    texts[static_cast<int>(TextId::DisplayFilterBilinear)] = "Bilinéaire";
    texts[static_cast<int>(TextId::DisplayFilterNearest)] = "Plus Proche";
    texts[static_cast<int>(TextId::DisplayFilterSharpen)] = "Netteté";
    texts[static_cast<int>(TextId::DisplayEdgeDetect)] = "Détection de Bords";
    texts[static_cast<int>(TextId::DisplayEdgeDetectTooltip)] = "Mettre en évidence les bords.";
    texts[static_cast<int>(TextId::DisplayGlowSettings)] = "Paramètres de Lueur";
    texts[static_cast<int>(TextId::DisplayGlowStrength)] = "Force de Lueur";
    texts[static_cast<int>(TextId::DisplayGlowStrengthTooltip)] = "Intensité de la lueur.";
    texts[static_cast<int>(TextId::DisplayGlowTint)] = "Teinte de Lueur";
    texts[static_cast<int>(TextId::DisplayGlowTintTooltip)] = "Couleur de la lueur.";
    texts[static_cast<int>(TextId::DisplayGlowIntensity)] = "Intensité de Lueur";
    texts[static_cast<int>(TextId::DisplayGlowIntensityTooltip)] = "Luminosité de la lueur.";
    texts[static_cast<int>(TextId::DisplayCustomGradient)] = "Dégradé Personnalisé";
    texts[static_cast<int>(TextId::DisplayCustomGradientTooltip)] = "Couleurs personnalisées.";
    texts[static_cast<int>(TextId::DisplayGradientStops)] = "Points de Dégradé";
    texts[static_cast<int>(TextId::DisplayGradientStopsTooltip)] = "Nombre de points de couleur.";
    texts[static_cast<int>(TextId::DisplayGradientStopLabel)] = "Point %d";
    texts[static_cast<int>(TextId::DisplayGridOverlay)] = "Superposition Grille";
    texts[static_cast<int>(TextId::DisplayGridOverlayTooltip)] = "Afficher les lignes de grille.";
    texts[static_cast<int>(TextId::DisplayGridOpacity)] = "Opacité Grille";
    texts[static_cast<int>(TextId::DisplayGridOpacityTooltip)] = "Transparence des lignes.";
    texts[static_cast<int>(TextId::DisplayGridColor)] = "Couleur Grille";
    texts[static_cast<int>(TextId::DisplayGridColorTooltip)] = "Couleur des lignes.";
    texts[static_cast<int>(TextId::DisplayGridLineThickness)] = "Épaisseur Grille";
    texts[static_cast<int>(TextId::DisplayGridLineThicknessTooltip)] = "Largeur des lignes.";
    texts[static_cast<int>(TextId::DisplayGridSpacing)] = "Espacement Grille";
    texts[static_cast<int>(TextId::DisplayGridSpacingTooltip)] = "Distance entre les lignes.";
    texts[static_cast<int>(TextId::DisplayGridEveryCell)] = "Chaque Cellule";
    texts[static_cast<int>(TextId::DisplayGridCustomInterval)] = "Intervalle Personnalisé";
    texts[static_cast<int>(TextId::DisplayGridInterval)] = "Intervalle";
    texts[static_cast<int>(TextId::DisplayGridIntervalTooltip)] = "Espacement personnalisé.";
    texts[static_cast<int>(TextId::DisplayGridMajorLines)] = "Lignes Principales";
    texts[static_cast<int>(TextId::DisplayGridMajorLinesTooltip)] = "Lignes accentuées.";
    texts[static_cast<int>(TextId::DisplayGridMajorEvery)] = "Principal Tous les";
    texts[static_cast<int>(TextId::DisplayGridMajorEveryTooltip)] = "Intervalle des lignes principales.";
    texts[static_cast<int>(TextId::DisplayGridMajorOpacity)] = "Opacité Principale";
    texts[static_cast<int>(TextId::DisplayGridMajorOpacityTooltip)] = "Opacité des lignes principales.";
    texts[static_cast<int>(TextId::DisplayInvertColors)] = "Inverser les Couleurs";
    texts[static_cast<int>(TextId::DisplayInvertColorsTooltip)] = "Inverser toutes les couleurs.";
    texts[static_cast<int>(TextId::DisplayShowBoundary)] = "Afficher Bordure";
    texts[static_cast<int>(TextId::DisplayShowBoundaryTooltip)] = "Indicateur de bordure.";
    texts[static_cast<int>(TextId::DisplayBoundaryColor)] = "Couleur Bordure";
    texts[static_cast<int>(TextId::DisplayBoundaryOpacity)] = "Opacité Bordure";
    texts[static_cast<int>(TextId::DisplayBoundaryStyle)] = "Style Bordure";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleTooltip)] = "Style visuel de la bordure.";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleSolid)] = "Plein";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleDashed)] = "Tiret";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleDotted)] = "Pointillé";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleDouble)] = "Double";
    texts[static_cast<int>(TextId::DisplayBoundaryStyleGlow)] = "Lueur";
    texts[static_cast<int>(TextId::DisplayBoundaryWidth)] = "Largeur Bordure";
    texts[static_cast<int>(TextId::DisplayBoundaryWidthTooltip)] = "Largeur de la ligne.";
    texts[static_cast<int>(TextId::DisplayDashLength)] = "Longueur Tiret";
    texts[static_cast<int>(TextId::DisplayDashLengthTooltip)] = "Longueur des tirets.";
    texts[static_cast<int>(TextId::DisplayAnimateBoundary)] = "Animer Bordure";
    texts[static_cast<int>(TextId::DisplayAnimateBoundaryTooltip)] = "Animer l'indicateur.";
    texts[static_cast<int>(TextId::DisplayBGColor)] = "Couleur de Fond";
    texts[static_cast<int>(TextId::DisplayBGColorTooltip)] = "Couleur de l'arrière-plan.";
    texts[static_cast<int>(TextId::DisplayClipNullCells)] = "Écrêter Cellules Nulles";
    texts[static_cast<int>(TextId::DisplayClipNullCellsTooltip)] = "Traiter les petites valeurs comme zéro.";
    texts[static_cast<int>(TextId::DisplayClipThreshold)] = "Seuil d'Écrêtage";
    texts[static_cast<int>(TextId::DisplayClipThresholdTooltip)] = "Valeurs en dessous = zéro.";
    texts[static_cast<int>(TextId::DisplayColormapDeformation)] = "Déformation de Palette";
    texts[static_cast<int>(TextId::DisplayCmapOffset)] = "Décalage";
    texts[static_cast<int>(TextId::DisplayCmapOffsetTooltip)] = "Décaler la palette.";
    texts[static_cast<int>(TextId::DisplayRangeMin)] = "Min Plage";
    texts[static_cast<int>(TextId::DisplayRangeMinTooltip)] = "Valeur minimale mappée.";
    texts[static_cast<int>(TextId::DisplayRangeMax)] = "Max Plage";
    texts[static_cast<int>(TextId::DisplayRangeMaxTooltip)] = "Valeur maximale mappée.";
    texts[static_cast<int>(TextId::DisplayPowerCurve)] = "Courbe de Puissance";
    texts[static_cast<int>(TextId::DisplayPowerCurveTooltip)] = "Appliquer une courbe de puissance.";
    texts[static_cast<int>(TextId::DisplayHueShift)] = "Décalage Teinte";
    texts[static_cast<int>(TextId::DisplayHueShiftTooltip)] = "Faire tourner les couleurs.";
    texts[static_cast<int>(TextId::DisplaySaturation)] = "Saturation";
    texts[static_cast<int>(TextId::DisplaySaturationTooltip)] = "Échelle de saturation.";
    texts[static_cast<int>(TextId::DisplayReverseColormap)] = "Inverser Palette";
    texts[static_cast<int>(TextId::DisplayReverseColormapTooltip)] = "Inverser la direction.";
    texts[static_cast<int>(TextId::DisplayResetColormapDeformation)] = "Réinitialiser Déformation";
    texts[static_cast<int>(TextId::DisplayRGBChannelIntensity)] = "Intensité Canaux RVB";
    
    // Analysis section
    texts[static_cast<int>(TextId::AnalysisEnable)] = "Activer l'Analyse";
    texts[static_cast<int>(TextId::AnalysisEnableTooltip)] = "Calculer des statistiques en temps réel.";
    texts[static_cast<int>(TextId::AnalysisAutoPause)] = "Pause Auto";
    texts[static_cast<int>(TextId::AnalysisAutoPauseTooltip)] = "Pause automatique si vide ou stabilisé.";
    texts[static_cast<int>(TextId::AnalysisAliveThreshold)] = "Seuil Vivant";
    texts[static_cast<int>(TextId::AnalysisAliveThresholdTooltip)] = "Valeur minimale pour 'vivante'.";
    texts[static_cast<int>(TextId::AnalysisTotalMass)] = "Masse Totale : %.2f";
    texts[static_cast<int>(TextId::AnalysisAliveCells)] = "Cellules Vivantes : %d / %d (%.1f%%)";
    texts[static_cast<int>(TextId::AnalysisAverage)] = "Moyenne : %.4f";
    texts[static_cast<int>(TextId::AnalysisMinMax)] = "Min : %.4f  Max : %.4f";
    texts[static_cast<int>(TextId::AnalysisVariance)] = "Variance : %.6f";
    texts[static_cast<int>(TextId::AnalysisCentroid)] = "Centroïde : (%.1f, %.1f)";
    texts[static_cast<int>(TextId::AnalysisBounds)] = "Limites : (%.0f,%.0f)-(%.0f,%.0f)";
    texts[static_cast<int>(TextId::AnalysisStateEmpty)] = "État : VIDE";
    texts[static_cast<int>(TextId::AnalysisStateStabilized)] = "État : STABILISÉ";
    texts[static_cast<int>(TextId::AnalysisStatePeriodic)] = "État : PÉRIODIQUE (T=%d, conf=%.0f%%)";
    texts[static_cast<int>(TextId::AnalysisStateActive)] = "État : Actif";
    texts[static_cast<int>(TextId::AnalysisSpecies)] = "Espèces (est) : %d";
    texts[static_cast<int>(TextId::AnalysisSpeed)] = "Vitesse : %.3f cellules/étape";
    texts[static_cast<int>(TextId::AnalysisDirection)] = "Direction : %.1f deg";
    texts[static_cast<int>(TextId::AnalysisOrientation)] = "Orientation : %.1f deg";
    texts[static_cast<int>(TextId::AnalysisGraphs)] = "Graphiques";
    texts[static_cast<int>(TextId::AnalysisMass)] = "Masse";
    texts[static_cast<int>(TextId::AnalysisAlive)] = "Vivantes";
    texts[static_cast<int>(TextId::AnalysisCentroidGraph)] = "Centroïde";
    texts[static_cast<int>(TextId::AnalysisSpeedGraph)] = "Vitesse";
    texts[static_cast<int>(TextId::AnalysisDirectionGraph)] = "Direction";
    texts[static_cast<int>(TextId::AnalysisAliveCellsGraph)] = "Cellules Vivantes";
    texts[static_cast<int>(TextId::AnalysisCentroidXGraph)] = "Centroïde X";
    texts[static_cast<int>(TextId::AnalysisCentroidYGraph)] = "Centroïde Y";
    texts[static_cast<int>(TextId::AnalysisSpeedGraphTitle)] = "Vitesse";
    texts[static_cast<int>(TextId::AnalysisDirectionGraphTitle)] = "Direction";
    texts[static_cast<int>(TextId::AnalysisGraphXAxisStep)] = "étape";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisMass)] = "masse";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisCells)] = "cellules";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisX)] = "x";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisY)] = "y";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisPxPerSec)] = "px/s";
    texts[static_cast<int>(TextId::AnalysisGraphYAxisDeg)] = "deg";
    texts[static_cast<int>(TextId::AnalysisDisplayWindow)] = "Fenêtre d'Affichage";
    texts[static_cast<int>(TextId::AnalysisDisplayWindowTooltip)] = "Nombre d'étapes à afficher.";
    texts[static_cast<int>(TextId::AnalysisGraphHeight)] = "Hauteur Graphique";
    texts[static_cast<int>(TextId::AnalysisAutoYScale)] = "Échelle Y Auto";
    texts[static_cast<int>(TextId::AnalysisAutoYScaleTooltip)] = "Ajuster l'axe Y automatiquement.";
    
    // Accessibility section
    texts[static_cast<int>(TextId::AccessibilityLanguage)] = "Langue";
    texts[static_cast<int>(TextId::AccessibilityLanguageTooltip)] = "Sélectionner la langue de l'interface.";
    texts[static_cast<int>(TextId::AccessibilityEnglish)] = "English";
    texts[static_cast<int>(TextId::AccessibilityFrench)] = "Français";
    texts[static_cast<int>(TextId::AccessibilityUIScale)] = "Échelle IU";
    texts[static_cast<int>(TextId::AccessibilityUIScaleTooltip)] = "Agrandir l'interface.";
    texts[static_cast<int>(TextId::AccessibilityFontSize)] = "Taille de Police";
    texts[static_cast<int>(TextId::AccessibilityFontSizeTooltip)] = "Taille de police de base.";
    texts[static_cast<int>(TextId::AccessibilityHighContrast)] = "Contraste Élevé";
    texts[static_cast<int>(TextId::AccessibilityHighContrastTooltip)] = "Mode contraste élevé.";
    texts[static_cast<int>(TextId::AccessibilityReduceMotion)] = "Réduire les Mouvements";
    texts[static_cast<int>(TextId::AccessibilityReduceMotionTooltip)] = "Réduire les animations.";
    texts[static_cast<int>(TextId::AccessibilityKeyboardNav)] = "Navigation Clavier";
    texts[static_cast<int>(TextId::AccessibilityKeyboardNavTooltip)] = "Navigation clavier pour l'IU.";
    texts[static_cast<int>(TextId::AccessibilityFocusIndicators)] = "Indicateurs de Focus";
    texts[static_cast<int>(TextId::AccessibilityFocusIndicatorsTooltip)] = "Afficher les contours de focus.";
    texts[static_cast<int>(TextId::AccessibilityResetDefaults)] = "Réinitialiser par Défaut";
    texts[static_cast<int>(TextId::AccessibilityResetDefaultsTooltip)] = "Réinitialiser les paramètres.";
    texts[static_cast<int>(TextId::AccessibilitySystemDpiScale)] = "Échelle DPI : %.2fx";
    texts[static_cast<int>(TextId::AccessibilityEffectiveScale)] = "Échelle Effective : %.2fx";
    
    // Common
    texts[static_cast<int>(TextId::CommonYes)] = "Oui";
    texts[static_cast<int>(TextId::CommonNo)] = "Non";
    texts[static_cast<int>(TextId::CommonOK)] = "OK";
    texts[static_cast<int>(TextId::CommonCancel)] = "Annuler";
    texts[static_cast<int>(TextId::CommonApply)] = "Appliquer";
    texts[static_cast<int>(TextId::CommonReset)] = "Réinitialiser";
    texts[static_cast<int>(TextId::CommonDefault)] = "Par Défaut";
    texts[static_cast<int>(TextId::CommonEnabled)] = "Activé";
    texts[static_cast<int>(TextId::CommonDisabled)] = "Désactivé";
    texts[static_cast<int>(TextId::CommonOn)] = "Activé";
    texts[static_cast<int>(TextId::CommonOff)] = "Désactivé";
    texts[static_cast<int>(TextId::CommonAll)] = "Tout";
    texts[static_cast<int>(TextId::CommonNone)] = "Aucun";
    texts[static_cast<int>(TextId::CommonChannel)] = "Canal";
    texts[static_cast<int>(TextId::CommonRed)] = "Rouge";
    texts[static_cast<int>(TextId::CommonGreen)] = "Vert";
    texts[static_cast<int>(TextId::CommonBlue)] = "Bleu";
    texts[static_cast<int>(TextId::CommonAlpha)] = "Alpha";
}

} // namespace lenia
