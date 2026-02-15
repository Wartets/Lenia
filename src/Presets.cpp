#include "Presets.hpp"
#include "PresetData.inc"
#include "AnimalData.inc"
#include <vector>
#include <string>
#include <cstring>

namespace lenia {

static std::vector<Preset> s_presets = {
    {"Orbium Unicaudatus", "Lenia Species",
     0.15f, 0.015f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Orbium_unicaudatus2.npy", PlacementMode::TwoPlace, false, 0, 0, nullptr},

    {"Gyropteron Arcus", "Lenia Species",
     0.283f, 0.0481f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Gyropteron_arcus.npy", PlacementMode::TopLeft, true, 0, 0, nullptr},

    {"Scutium Solidus", "Lenia Species",
     0.29f, 0.045f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Scutium_solidus.npy", PlacementMode::TopLeft, false, 0, 0, nullptr},

    {"Hydrogeminium Natans", "Multiring",
     0.26f, 0.036f, 0.1f, 18, 3, {0.5f, 1.0f, 0.667f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Hydrogeminium.npy", PlacementMode::TopLeft, false, 0, 0, nullptr},

    {"Small Bug", "Lenia Species",
     0.31f, 0.048f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "SmallBug.npy", PlacementMode::TopLeft, false, 0, 0, nullptr},

    {"Compilation", "Lenia Species",
     0.337f, 0.057f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Compilation.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Trihelicium Pachus", "Lenia Species",
     0.46f, 0.119f, 0.1f, 13, 1, {1.0f}, KernelType::StepUnimodal, GrowthType::Step,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "TriheliciumPachus.npy", PlacementMode::TopLeft, false, 0, 0, nullptr},

    {"Discutium Pachus", "Lenia Species",
     0.545f, 0.186f, 0.1f, 13, 1, {1.0f}, KernelType::StepUnimodal, GrowthType::Step,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "DiscutiumPachus.npy", PlacementMode::TopLeft, false, 0, 0, nullptr},

    {"Circogeminium Ventilans", "Multiring",
     0.29f, 0.035f, 0.1f, 45, 3, {1.0f, 1.0f, 1.0f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "BigCircogeminiumVentilans.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Gyrogeminium Serratus", "Multiring",
     0.27f, 0.04f, 0.1f, 36, 3, {0.5f, 1.0f, 0.5f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "GyrogeminiumSerratus.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Triorbium", "Lenia Species",
     0.114f, 0.0115f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Triorbium.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Decascutium", "Lenia Species",
     0.48f, 0.108f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Decascutium.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Catenoscutium Bidirectus", "Lenia Species",
     0.29f, 0.043f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "CatenoscutiumBidirectus.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Vagopteron", "Lenia Species",
     0.218f, 0.0351f, 0.1f, 25, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Vagopteron.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"H. Serratus Liquefaciens", "Multiring",
     0.34f, 0.051f, 0.1f, 20, 3, {0.75f, 1.0f, 1.0f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "HeptapteryxSerratusLiquefaciens.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Hexacaudopteryx", "Lenia Species",
     0.35f, 0.048f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Hexacaudopteryx.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Catenopteryx Cyclon", "Lenia Species",
     0.34f, 0.045f, 0.2f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "CatenopteryxCyclon.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"C. Cyclon Scutoides", "Lenia Species",
     0.38f, 0.07f, 0.2f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "CatenopteryxCyclonScutoides.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"C. Bispirae Scutoides", "Lenia Species",
     0.407f, 0.0806f, 0.1f, 13, 1, {1.0f}, KernelType::Bump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "CatenoheliciumBispiraeScutoides.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Decadentium Volubilis", "Multiring",
     0.15f, 0.014f, 0.1f, 36, 4, {0.667f, 1.0f, 0.667f, 0.333f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "DecadentiumVolubilis.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Aerogeminium Quietus", "Multiring",
     0.3f, 0.048f, 0.1f, 18, 3, {1.0f, 1.0f, 1.0f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "AerogeminiumQuietus.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Hydrogeminium Natans 2", "Multiring",
     0.26f, 0.036f, 0.1f, 36, 3, {1.0f, 1.0f, 1.0f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "HydrogeminiumNatans2.npy", PlacementMode::BottomLeft, false, 0, 0, nullptr},

    {"Glider Gun", "Game of Life",
     0.35f, 0.07f, 1.0f, 1, 1, {1.0f}, KernelType::GameOfLife, GrowthType::GameOfLife,
     InitMode::Species, 0.0f, 0.0f, 100, 100,
     "GliderGun.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Weekender", "Game of Life",
     0.35f, 0.07f, 1.0f, 1, 1, {1.0f}, KernelType::GameOfLife, GrowthType::GameOfLife,
     InitMode::Species, 0.0f, 0.0f, 100, 100,
     "Weekender.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Space Filler", "Game of Life",
     0.35f, 0.07f, 1.0f, 1, 1, {1.0f}, KernelType::GameOfLife, GrowthType::GameOfLife,
     InitMode::Species, 0.0f, 0.0f, 256, 256,
     "SpaceFiller.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Pufferfish", "Game of Life",
     0.35f, 0.07f, 1.0f, 1, 1, {1.0f}, KernelType::GameOfLife, GrowthType::GameOfLife,
     InitMode::Species, 0.0f, 0.0f, 200, 200,
     "Pufferfish.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"R-pentomino", "Game of Life",
     0.35f, 0.07f, 1.0f, 1, 1, {1.0f}, KernelType::GameOfLife, GrowthType::GameOfLife,
     InitMode::Species, 0.0f, 0.0f, 200, 200,
     "R-pentomino.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Hexastrium", "Multiring",
     0.2f, 0.024f, 0.1f, 96, 3, {1.0f, 0.0833f, 1.0f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Hexastrium.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Fish", "Multiring",
     0.156f, 0.0118f, 0.1f, 10, 3, {0.5f, 1.0f, 0.667f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Fish.npy", PlacementMode::Scatter, false, 0, 0, nullptr},

    {"Dodecadentium Nausia", "Multiring",
     0.27f, 0.033f, 0.1f, 54, 3, {0.667f, 1.0f, 0.333f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "DodecadentiumNausia.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Dodecafolium Ventilans", "Multiring",
     0.23f, 0.019f, 0.1f, 72, 4, {0.5f, 0.583f, 0.75f, 1.0f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "DodecafoliumVentilans.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Wanderer", "Multiring",
     0.1f, 0.005f, 0.1f, 13, 2, {0.75f, 1.0f}, KernelType::MultiringBump4, GrowthType::Asymptotic,
     InitMode::Species, 0.0f, 0.0f, 350, 350,
     "Wanderer.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Bosco", "Larger-than-Life",
     0.4198f, 0.5556f, 1.0f, 5, 1, {1.0f}, KernelType::StepUnimodal, GrowthType::LargerThanLife,
     InitMode::Species, 0.4198f, 0.716f, 200, 200,
     "Bosco.npy", PlacementMode::Center, false, 0, 0, nullptr},

    {"Aquarium (Self-Replicating)", "Multichannel",
     0.15f, 0.015f, 0.5f, 12, 3, {1.0f}, KernelType::MultiringGauss, GrowthType::Lenia,
     InitMode::Species, 0.0f, 0.0f, 256, 256,
     nullptr, PlacementMode::Center, false, 0, 0, nullptr},

    {"Aquarium (Swarm)", "Multichannel",
     0.15f, 0.015f, 0.5f, 12, 3, {1.0f}, KernelType::MultiringGauss, GrowthType::Lenia,
     InitMode::Species, 0.0f, 0.0f, 256, 256,
     nullptr, PlacementMode::Center, false, 0, 0, nullptr},

    {"Emitter (Glider Gun)", "Multichannel",
     0.15f, 0.015f, 0.5f, 13, 1, {1.0f}, KernelType::GaussianShell, GrowthType::Lenia,
     InitMode::Species, 0.0f, 0.0f, 256, 256,
     nullptr, PlacementMode::Center, false, 0, 0, nullptr},

    {"Pacman", "Multichannel",
     0.15f, 0.015f, 0.5f, 13, 1, {1.0f}, KernelType::GaussianShell, GrowthType::Lenia,
     InitMode::Species, 0.0f, 0.0f, 256, 256,
     nullptr, PlacementMode::Center, false, 0, 0, nullptr},

    {"Fish (Multi-Kernel)", "Multi-Kernel",
     0.156f, 0.0118f, 0.2f, 10, 3, {0.5f, 1.0f, 0.667f}, KernelType::MultiringBump4, GrowthType::Quad4,
     InitMode::Species, 0.0f, 0.0f, 256, 256,
     nullptr, PlacementMode::Center, false, 0, 0, nullptr},
};

static bool s_presetsBuilt = false;

static void buildUnifiedPresets() {
    if (s_presetsBuilt) return;
    s_presetsBuilt = true;

    for (int i = 0; i < ANIMAL_COUNT; ++i) {
        const auto& a = s_animals[i];
        Preset p{};
        p.name = a.name;
        p.category = a.family;
        p.mu = a.mu;
        p.sigma = a.sigma;
        p.dt = a.dt;
        p.radius = a.radius;
        p.numRings = a.numRings;
        for (int j = 0; j < 5 && j < a.numRings; ++j)
            p.ringWeights[j] = a.ringWeights[j];
        p.kernelType = static_cast<KernelType>(a.kernelType);
        p.growthType = static_cast<GrowthType>(a.growthType);
        p.initMode = InitMode::Species;
        p.initParam1 = 0.0f;
        p.initParam2 = 0.0f;
        p.gridW = 350;
        p.gridH = 350;
        p.speciesFile = nullptr;
        p.placement = PlacementMode::Center;
        p.flipInit = false;
        p.cellRows = a.cellRows;
        p.cellCols = a.cellCols;
        p.cellData = a.cells;
        s_presets.push_back(p);
    }
}

const std::vector<Preset>& getPresets() {
    buildUnifiedPresets();
    return s_presets;
}

static std::vector<std::string> s_categories;
static bool s_categoriesBuilt = false;

static void buildCategories() {
    if (s_categoriesBuilt) return;
    s_categoriesBuilt = true;
    buildUnifiedPresets();
    s_categories.push_back("All");
    for (const auto& p : s_presets) {
        bool found = false;
        for (const auto& c : s_categories) {
            if (c == p.category) { found = true; break; }
        }
        if (!found) s_categories.emplace_back(p.category);
    }
}

const std::vector<std::string>& getPresetCategories() {
    buildCategories();
    return s_categories;
}

static std::vector<KernelPreset> s_kernelPresets = {
    {"Gaussian Shell (default)", 0, 1, {1.0f}, 13},
    {"Bump4 (Bert Chan)", 1, 1, {1.0f}, 13},
    {"Step Unimodal", 5, 1, {1.0f}, 13},
    {"Game of Life (3x3)", 4, 1, {1.0f}, 1},
    {"Multiring 3-ring equal", 2, 3, {0.333f, 0.333f, 0.333f}, 26},
    {"Multiring 3-ring 1/2,1,2/3", 2, 3, {0.5f, 1.0f, 0.667f}, 18},
    {"Multiring 3-ring 1,1,1", 2, 3, {1.0f, 1.0f, 1.0f}, 45},
    {"Multiring 3-ring 1/2,1,1/2", 2, 3, {0.5f, 1.0f, 0.5f}, 36},
    {"Multiring 4-ring 2/3,1,2/3,1/3", 2, 4, {0.667f, 1.0f, 0.667f, 0.333f}, 72},
    {"Multiring 4-ring 1/2,7/12,3/4,1", 2, 4, {0.5f, 0.583f, 0.75f, 1.0f}, 72},
    {"Multiring 3-ring bump4", 3, 3, {0.333f, 0.333f, 0.333f}, 26},
    {"Large Gaussian R=96", 0, 1, {1.0f}, 96},
    {"Hexastrium-like 1,1/12,1", 2, 3, {1.0f, 0.0833f, 1.0f}, 96},
    {"Small R=10 fish-like", 2, 3, {0.5f, 1.0f, 0.667f}, 10},
};

const std::vector<KernelPreset>& getKernelPresets() {
    return s_kernelPresets;
}

static const float s_VT049W_ch0[18][24] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.04f,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0.49f,1.0f,0,0.03f,0.49f,0.49f,0.28f,0.16f,0.03f,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0.6f,0.47f,0.31f,0.58f,0.51f,0.35f,0.28f,0.22f,0,0,0,0,0},
    {0,0,0,0,0,0,0.15f,0.32f,0.17f,0.61f,0.97f,0.29f,0.67f,0.59f,0.88f,1.0f,0.92f,0.8f,0.61f,0.42f,0.19f,0,0,0},
    {0,0,0,0,0,0,0,0.25f,0.64f,0.26f,0.92f,0.04f,0.24f,0.97f,1.0f,1.0f,1.0f,1.0f,0.97f,0.71f,0.33f,0.12f,0,0},
    {0,0,0,0,0,0,0,0.38f,0.84f,0.99f,0.78f,0.67f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.95f,0.62f,0.37f,0,0},
    {0,0,0,0,0.04f,0.11f,0,0.69f,0.75f,0.75f,0.91f,1.0f,1.0f,0.89f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.81f,0.42f,0.07f,0},
    {0,0,0,0,0.44f,0.63f,0.04f,0,0,0,0.11f,0.14f,0,0.05f,0.64f,1.0f,1.0f,1.0f,1.0f,1.0f,0.92f,0.56f,0.23f,0},
    {0,0,0,0,0.11f,0.36f,0.35f,0.2f,0,0,0,0,0,0,0.63f,1.0f,1.0f,1.0f,1.0f,1.0f,0.96f,0.49f,0.26f,0},
    {0,0,0,0,0,0.4f,0.37f,0.18f,0,0,0,0,0,0.04f,0.41f,0.52f,0.67f,0.82f,1.0f,1.0f,0.91f,0.4f,0.23f,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.04f,0,0.05f,0.45f,0.89f,1.0f,0.66f,0.35f,0.09f,0},
    {0,0,0.22f,0,0,0,0.05f,0.36f,0.6f,0.13f,0.02f,0.04f,0.24f,0.34f,0.1f,0,0.04f,0.62f,1.0f,1.0f,0.44f,0.25f,0,0},
    {0,0,0,0.43f,0.53f,0.58f,0.78f,0.9f,0.96f,1.0f,1.0f,1.0f,1.0f,0.71f,0.46f,0.51f,0.81f,1.0f,1.0f,0.93f,0.19f,0.06f,0,0},
    {0,0,0,0,0.23f,0.26f,0.37f,0.51f,0.71f,0.89f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.42f,0.06f,0,0,0},
    {0,0,0,0,0.03f,0,0,0.11f,0.35f,0.62f,0.81f,0.93f,1.0f,1.0f,1.0f,1.0f,1.0f,0.64f,0.15f,0,0,0,0,0},
    {0,0,0,0,0,0,0.06f,0.1f,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0.05f,0.09f,0.05f,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
static const float s_VT049W_ch1[18][24] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.02f,0.28f,0.42f,0.44f,0.34f,0.18f,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.34f,1.0f,1.0f,1.0f,1.0f,1.0f,0.91f,0.52f,0.14f,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0.01f,0.17f,0.75f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.93f,0.35f,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.22f,0.92f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.59f,0.09f},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.75f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.71f,0.16f},
    {0,0,0,0,0,0,0,0,0,0,0,0,0.01f,0.67f,0.83f,0.85f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.68f,0.17f},
    {0,0,0,0,0,0,0,0,0,0,0,0,0.21f,0.04f,0.12f,0.58f,0.95f,1.0f,1.0f,1.0f,1.0f,1.0f,0.57f,0.13f},
    {0,0,0,0,0,0,0,0,0,0,0,0.07f,0,0,0,0.2f,0.64f,0.96f,1.0f,1.0f,1.0f,0.9f,0.24f,0.01f},
    {0,0,0,0,0,0,0,0,0,0,0.13f,0.29f,0,0,0,0.25f,0.9f,1.0f,1.0f,1.0f,1.0f,0.45f,0.05f,0},
    {0,0,0,0,0,0,0,0,0,0,0.13f,0.31f,0.07f,0,0.46f,0.96f,1.0f,1.0f,1.0f,1.0f,0.51f,0.12f,0,0},
    {0,0,0,0,0,0,0,0,0.26f,0.82f,1.0f,0.95f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.3f,0.05f,0,0,0},
    {0,0,0,0,0,0,0,0,0.28f,0.74f,1.0f,0.95f,0.87f,1.0f,1.0f,1.0f,1.0f,1.0f,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0.07f,0.69f,1.0f,1.0f,1.0f,1.0f,1.0f,0.96f,0.25f,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0.4f,0.72f,0.9f,0.83f,0.7f,0.56f,0.43f,0.14f,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
static const float s_VT049W_ch2[18][24] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0.04f,0.25f,0.37f,0.44f,0.37f,0.24f,0.11f,0.04f,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0.19f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.75f,0.4f,0.15f,0,0,0,0},
    {0,0,0,0,0,0,0,0,0.14f,0.48f,0.83f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.4f,0,0,0,0},
    {0,0,0,0,0,0,0,0,0.62f,0.78f,0.94f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.64f,0,0,0,0},
    {0,0,0,0,0,0,0,0.02f,0.65f,0.98f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.78f,0,0,0,0},
    {0,0,0,0,0,0,0,0.15f,0.48f,0.93f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.79f,0.05f,0,0,0},
    {0,0,0,0,0,0,0.33f,0.56f,0.8f,1.0f,1.0f,1.0f,0.37f,0.6f,0.94f,1.0f,1.0f,1.0f,1.0f,0.68f,0.05f,0,0,0},
    {0,0,0,0,0.35f,0.51f,0.76f,0.89f,1.0f,1.0f,0.72f,0.15f,0,0.29f,0.57f,0.69f,0.86f,1.0f,0.92f,0.49f,0,0,0,0},
    {0,0,0,0,0,0.38f,0.86f,1.0f,1.0f,0.96f,0.31f,0,0,0,0,0.02f,0.2f,0.52f,0.37f,0.11f,0,0,0,0},
    {0,0,0.01f,0,0,0.07f,0.75f,1.0f,1.0f,1.0f,0.48f,0.03f,0,0,0,0,0,0.18f,0.07f,0,0,0,0,0},
    {0,0.11f,0.09f,0.22f,0.15f,0.32f,0.71f,0.94f,1.0f,1.0f,0.97f,0.54f,0.12f,0.02f,0,0,0,0,0,0,0,0,0,0},
    {0.06f,0.33f,0.47f,0.51f,0.58f,0.77f,0.95f,1.0f,1.0f,1.0f,1.0f,0.62f,0.12f,0,0,0,0,0,0,0,0,0,0,0},
    {0.04f,0.4f,0.69f,0.88f,0.95f,1.0f,1.0f,1.0f,1.0f,1.0f,0.93f,0.68f,0.22f,0.02f,0,0,0.01f,0,0,0,0,0,0,0},
    {0,0.39f,0.69f,0.91f,1.0f,1.0f,1.0f,1.0f,1.0f,0.85f,0.52f,0.35f,0.24f,0.17f,0.07f,0,0,0,0,0,0,0,0,0},
    {0,0,0.29f,0.82f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,0.67f,0.29f,0.02f,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0.2f,0.51f,0.77f,0.96f,0.93f,0.71f,0.4f,0.16f,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0.08f,0.07f,0.03f,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

static const MultiChannelPresetRule s_VT049W_rules[15] = {
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.272f, 0.0595f, 0.138f, 0.91f, 1, 0, 0, 1, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.349f, 0.1585f, 0.48f,  0.62f, 1, 0, 0, 1, 10},
    {{1,0.25f,0,0,0,0,0,0,0,0,0,0,0,0,0,0},        0.2f,   0.0332f, 0.284f, 0.5f,  2, 0, 0, 3, 10},
    {{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.114f, 0.0528f, 0.256f, 0.97f, 2, 1, 1, 3, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.447f, 0.0777f, 0.5f,   0.72f, 1, 1, 1, 1, 10},
    {{0.8333f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.247f, 0.0342f, 0.622f, 0.8f,  2, 1, 1, 3, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.21f,  0.0617f, 0.35f,  0.96f, 1, 2, 2, 1, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.462f, 0.1192f, 0.218f, 0.56f, 1, 2, 2, 1, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.446f, 0.1793f, 0.556f, 0.78f, 1, 2, 2, 1, 10},
    {{0.9167f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.327f, 0.1408f, 0.344f, 0.79f, 2, 0, 1, 3, 10},
    {{0.75f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},        0.476f, 0.0995f, 0.456f, 0.5f,  2, 0, 2, 3, 10},
    {{0.9167f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.379f, 0.0697f, 0.67f,  0.72f, 2, 1, 0, 3, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.262f, 0.0877f, 0.42f,  0.68f, 1, 1, 2, 1, 10},
    {{0.1667f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.412f, 0.1101f, 0.43f,  0.82f, 2, 2, 0, 3, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.201f, 0.0786f, 0.278f, 0.82f, 1, 2, 1, 1, 10},
};

static const float s_5N7KKM_ch0[20][20] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0.12f,0,0,0,0,0,0,0},
    {0,0,0,0,0.49f,0,0,0,0,0,0,0.23f,0.47f,0.31f,0.93f,0.75f,0,0,0,0},
    {0,0,0,0.23f,0,0,0.65f,0.68f,0.12f,0,0,0,0.02f,0.40f,0.82f,0.86f,0,0.19f,0,0},
    {0,0,0.01f,0.01f,0.77f,1.00f,0.98f,1.00f,0.97f,0.85f,0.70f,0.55f,0.12f,0.15f,0.01f,0,0,0.95f,0,0},
    {0,0,0,0.66f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.74f,0.76f,0.27f,0,0,0.18f,0.59f,0.31f,0},
    {0,0.04f,0.08f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.88f,0.68f,0.07f,0,0,0,0,0,0},
    {0,0,0.29f,1.00f,1.00f,1.00f,1.00f,0.90f,1.00f,0.92f,0.58f,0.84f,0.89f,0.39f,0,0,0.04f,1.00f,0,0},
    {0,0.06f,0.27f,1.00f,1.00f,1.00f,0.82f,0.39f,0,0,0,0.12f,0.87f,0.70f,0.58f,0.04f,0.40f,1.00f,0.35f,0},
    {0,0.21f,0.38f,1.00f,1.00f,0.66f,0,0,0,0,0,0,1.00f,0.79f,0.74f,0.16f,0.31f,0.42f,0,0},
    {0,0.26f,0.50f,1.00f,1.00f,0.46f,0,0,0,0,0,0.40f,1.00f,1.00f,0.71f,0.16f,0,0.22f,0,0},
    {0,0.14f,0.48f,1.00f,1.00f,0.77f,0,0,0,0,0,1.00f,1.00f,1.00f,0.79f,0,0,0,0,0},
    {0,0,0.16f,1.00f,1.00f,1.00f,0.19f,0,0,0.09f,0.20f,0.57f,1.00f,1.00f,0.74f,0,0,0,0,0},
    {0,0,0,0.59f,1.00f,1.00f,0.85f,0.75f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.47f,0,0,0,0,0},
    {0,0,0,0,0.95f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.96f,0.44f,0,0,0,0,0},
    {0,0,0,0,0.25f,0.79f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.96f,0,0,0,0,0,0},
    {0,0,0,0,0.04f,0.06f,0.26f,0.61f,1.00f,1.00f,1.00f,1.00f,1.00f,0,0,0.32f,0,0,0,0},
    {0,0,0,0,0,0,0.15f,0,0.02f,0.23f,0.24f,0.05f,0,0,0.25f,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0.02f,0.04f,0,0,0.08f,0,0,0,0,0,0,0}
};
static const float s_5N7KKM_ch1[20][20] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0.03f,0.43f,0,0,0,0,0,0,0,0,0,0},
    {0,0,0.14f,0.47f,0,0,0.27f,0.92f,0.87f,0.70f,0,0,0,0,0,0,0,0,0,0},
    {0,0,0.86f,1.00f,0.66f,1.00f,1.00f,1.00f,1.00f,0.33f,0,0,0,0,0,0,0,0,0,0},
    {0,0,1.00f,1.00f,1.00f,1.00f,1.00f,0.13f,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1.00f,1.00f,1.00f,1.00f,1.00f,0,0.22f,0.30f,0,0,0,0,0,0,0,0,0,0},
    {0,0,0.76f,1.00f,1.00f,1.00f,1.00f,1.00f,0.83f,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0.71f,1.00f,1.00f,1.00f,1.00f,0.77f,0.81f,0.75f,0,0,0,0,0,0,0,0,0,0},
    {0,0,0.69f,1.00f,1.00f,1.00f,0.88f,0.24f,0.35f,0.62f,0.35f,0.09f,0,0,0,0,0,0,0,0},
    {0,0,0.29f,1.00f,1.00f,1.00f,0.98f,0.38f,0.13f,0.65f,0.88f,0.32f,0,0,0,0,0,0,0,0},
    {0,0,0,0.09f,1.00f,1.00f,1.00f,0.93f,0.77f,0.88f,0.24f,0.03f,0.69f,1.00f,0.78f,0,0,0,0,0},
    {0,0,0,0,0.44f,1.00f,1.00f,1.00f,1.00f,1.00f,0.76f,0.83f,1.00f,0.92f,0.17f,0,0,0,0,0},
    {0,0,0,0,0,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0,0,0,0,0,0,0},
    {0,0,0,0,0,0.36f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.12f,0,0,0,0,0,0,0},
    {0,0,0,0,0,0.06f,0.39f,0.79f,1.00f,1.00f,1.00f,0.48f,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0.16f,0.59f,1.00f,1.00f,1.00f,0.13f,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0.04f,0.16f,0.02f,0,0,0,0,0,0,0,0}
};
static const float s_5N7KKM_ch2[20][20] = {
    {0,0,0,0,0,0,0,0,0,0.51f,0.46f,0.26f,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0.38f,1.00f,1.00f,0.96f,0.85f,0.57f,0,0,0,0,0,0,0},
    {0,0,0,0,0,0.06f,0.63f,1.00f,1.00f,1.00f,1.00f,0.96f,0.70f,0.08f,0,0,0,0,0,0},
    {0,0,0,0,0.01f,0.36f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.82f,0.49f,0,0,0,0,0,0},
    {0,0,0,0,0.05f,0.50f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.99f,0.65f,0.04f,0,0,0,0},
    {0,0,0.01f,0,0.08f,0.52f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.93f,0,0,0,0,0},
    {0,0,0.03f,0,0.09f,0.49f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.58f,0.04f,0,0,0,0,0},
    {0,0,0.02f,0,0.08f,0.50f,1.00f,1.00f,1.00f,1.00f,1.00f,0.81f,0,0,0,0,0.08f,0.01f,0,0},
    {0,0,0,0,0.04f,0.47f,1.00f,1.00f,1.00f,1.00f,0.40f,0,0,0,0,0.77f,0.85f,0.35f,0,0},
    {0,0,0,0,0.03f,0.41f,1.00f,1.00f,1.00f,1.00f,0.20f,0,0,0.01f,1.00f,1.00f,1.00f,0.55f,0,0},
    {0,0,0,0,0,0.30f,0.98f,1.00f,1.00f,1.00f,0.68f,0.09f,0.26f,1.00f,1.00f,1.00f,1.00f,0.70f,0.19f,0},
    {0,0,0,0,0,0.13f,0.55f,0.95f,1.00f,1.00f,1.00f,0.89f,1.00f,1.00f,1.00f,1.00f,1.00f,0.85f,0.67f,0.24f},
    {0,0,0,0,0,0.02f,0.31f,0.63f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.88f,0.45f},
    {0,0,0,0,0,0,0.12f,0.44f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.91f,0.36f},
    {0,0,0,0,0,0,0,0.16f,0.48f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.86f,0.06f},
    {0,0,0,0,0,0,0,0,0.13f,0.32f,0.70f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,0.28f,0},
    {0,0,0,0,0,0,0,0,0,0.06f,0.18f,0.32f,0.57f,0.90f,1.00f,1.00f,1.00f,0.08f,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0.04f,0.12f,0.25f,0.39f,0.31f,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0.02f,0,0,0,0,0,0,0,0,0}
};

static const MultiChannelPresetRule s_5N7KKM_rules[15] = {
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.22f,  0.0628f, 0.174f, 0.87f, 1, 0, 0, 1, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.351f, 0.1539f, 0.46f,  0.52f, 1, 0, 0, 1, 10},
    {{1,0.25f,0,0,0,0,0,0,0,0,0,0,0,0,0,0},        0.177f, 0.0333f, 0.31f,  0.58f, 2, 0, 0, 3, 10},
    {{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.126f, 0.0525f, 0.242f, 0.89f, 2, 1, 1, 3, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.437f, 0.0797f, 0.508f, 0.78f, 1, 1, 1, 1, 10},
    {{0.75f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},        0.234f, 0.0369f, 0.566f, 0.79f, 2, 1, 1, 3, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.179f, 0.0653f, 0.406f, 1.0f,  1, 2, 2, 1, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.489f, 0.1213f, 0.27f,  0.64f, 1, 2, 2, 1, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.419f, 0.1775f, 0.588f, 0.96f, 1, 2, 2, 1, 10},
    {{0.9167f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.341f, 0.1388f, 0.294f, 0.66f, 2, 0, 1, 3, 10},
    {{0.75f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},        0.469f, 0.1054f, 0.388f, 0.69f, 2, 0, 2, 3, 10},
    {{1,0.9167f,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.369f, 0.0721f, 0.62f,  0.61f, 2, 1, 0, 3, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.219f, 0.0898f, 0.348f, 0.81f, 1, 1, 2, 1, 10},
    {{0.1667f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.385f, 0.1102f, 0.436f, 0.81f, 2, 2, 0, 3, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.208f, 0.0749f, 0.39f,  0.71f, 1, 2, 1, 1, 10},
};

static const MultiChannelPresetRule s_emitter_rules[15] = {
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.184f, 0.0632f, 0.076f, 0.56f, 1, 0, 0, 0, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.1f,   0.1511f, 0.516f, 0.76f, 1, 0, 0, 0, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.246f, 0.047f,  0.554f, 0.5f,  1, 0, 0, 0, 0},
    {{0.0833f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.1f,   0.0553f, 0.294f, 0.84f, 2, 1, 1, 2, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.324f, 0.0782f, 0.594f, 0.97f, 1, 1, 1, 0, 0},
    {{0.8333f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.229f, 0.0321f, 0.612f, 0.98f, 2, 1, 1, 2, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.29f,  0.0713f, 0.396f, 0.87f, 1, 2, 2, 0, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.484f, 0.1343f, 0.244f, 0.96f, 1, 2, 2, 0, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.592f, 0.1807f, 0.562f, 0.93f, 1, 2, 2, 0, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.398f, 0.1411f, 0.36f,  0.89f, 1, 0, 1, 0, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.388f, 0.1144f, 0.192f, 0.67f, 1, 0, 2, 0, 0},
    {{1,0.9167f,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.312f, 0.0697f, 0.462f, 0.58f, 3, 1, 0, 2, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.327f, 0.1036f, 0.608f, 1.0f,  1, 1, 2, 0, 0},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.471f, 0.1176f, 0.394f, 0.8f,  1, 2, 0, 0, 0},
    {{1,0.0833f,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.1f,   0.0573f, 0.14f,  0.62f, 2, 2, 1, 2, 0},
};

static const MultiChannelPresetRule s_pacman_rules[15] = {
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.362f, 0.0404f, 0.17f,  0.57f, 2, 0, 0, 2, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.257f, 0.1469f, 0.41f,  0.5f,  1, 0, 0, 0, 8},
    {{1,0.25f,0,0,0,0,0,0,0,0,0,0,0,0,0,0},        0.247f, 0.0245f, 0.72f,  0.8f,  2, 0, 0, 2, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.199f, 0.0575f, 0.24f,  0.91f, 1, 1, 1, 0, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.288f, 0.0699f, 0.34f,  0.67f, 1, 1, 1, 0, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.26f,  0.0346f, 0.74f,  1.0f,  1, 1, 1, 0, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.108f, 0.0786f, 0.5f,   0.89f, 1, 2, 2, 0, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.597f, 0.1136f, 0.18f,  0.55f, 2, 2, 2, 2, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.476f, 0.1894f, 0.35f,  0.59f, 2, 2, 2, 2, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.481f, 0.143f,  0.53f,  0.98f, 1, 0, 1, 0, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.343f, 0.0914f, 0.17f,  0.62f, 1, 0, 2, 0, 8},
    {{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.197f, 0.0732f, 0.55f,  0.57f, 2, 1, 0, 2, 8},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.247f, 0.1089f, 0.2f,   0.6f,  1, 1, 2, 0, 8},
    {{0.25f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},        0.393f, 0.1117f, 0.42f,  0.78f, 3, 2, 0, 2, 8},
    {{1,0.0833f,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.179f, 0.0906f, 0.23f,  0.84f, 2, 2, 1, 2, 8},
};

static const MultiChannelPresetRule s_fish_rules[3] = {
    {{0.5f,1.0f,0.667f,0,0,0,0,0,0,0,0,0,0,0,0,0}, 0.156f, 0.0118f, 1.0f,  1.0f,  3, 0, 0, 3, 10},
    {{0.0833f,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},      0.193f, 0.049f,  1.0f,  1.0f,  2, 0, 0, 3, 10},
    {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            0.342f, 0.0891f, 1.0f,  1.0f,  1, 0, 0, 1, 10},
};

static std::vector<MultiChannelPreset> s_multiChannelPresets = {
    {
        "Aquarium (Self-Replicating)", "Multichannel",
        12, 0.15f, 256, 256, 3, 15,
        {},
        18, 24,
        &s_VT049W_ch0[0][0], &s_VT049W_ch1[0][0], &s_VT049W_ch2[0][0]
    },
    {
        "Aquarium (Swarm)", "Multichannel",
        12, 0.2f, 256, 256, 3, 15,
        {},
        20, 20,
        &s_5N7KKM_ch0[0][0], &s_5N7KKM_ch1[0][0], &s_5N7KKM_ch2[0][0]
    },
    {
        "Emitter (Glider Gun)", "Multichannel",
        13, 0.26f, 256, 256, 3, 15,
        {},
        29, 28,
        &s_emitter_ch0[0][0], &s_emitter_ch1[0][0], &s_emitter_ch2[0][0]
    },
    {
        "Pacman", "Multichannel",
        13, 0.6f, 256, 256, 3, 15,
        {},
        39, 45,
        &s_pacman_ch0[0][0], &s_pacman_ch1[0][0], &s_pacman_ch2[0][0]
    },
    {
        "Fish (Multi-Kernel)", "Multi-Kernel",
        10, 0.03f, 256, 256, 3, 3,
        {},
        21, 22,
        &s_fish_ch0[0][0], nullptr, nullptr
    },
};

static bool s_mcPresetsInitialized = false;

static void initMultiChannelPresets() {
    if (s_mcPresetsInitialized) return;
    s_mcPresetsInitialized = true;

    for (int i = 0; i < 15; ++i)
        s_multiChannelPresets[0].rules[i] = s_VT049W_rules[i];

    for (int i = 0; i < 15; ++i)
        s_multiChannelPresets[1].rules[i] = s_5N7KKM_rules[i];

    for (int i = 0; i < 15; ++i)
        s_multiChannelPresets[2].rules[i] = s_emitter_rules[i];

    for (int i = 0; i < 15; ++i)
        s_multiChannelPresets[3].rules[i] = s_pacman_rules[i];

    for (int i = 0; i < 3; ++i)
        s_multiChannelPresets[4].rules[i] = s_fish_rules[i];
}

const std::vector<MultiChannelPreset>& getMultiChannelPresets() {
    initMultiChannelPresets();
    return s_multiChannelPresets;
}

}
