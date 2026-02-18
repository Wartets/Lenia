/**
 * @file LeniaEngine.cpp
 * @brief Implementation of Lenia cellular automaton simulation engine.
 * 
 * The core simulation loop implements the Lenia update equation:
 *   A^(t+dt) = clip[A^t + dt * G(K * A^t), 0, 1]
 * 
 * Where:
 * - A is the cell state grid (values 0 to 1)
 * - K is the convolution kernel (neighborhood weighting)
 * - G is the growth function (maps neighborhood sum to growth rate)
 * - dt is the time step (controls simulation speed)
 * 
 * @see https://arxiv.org/abs/1812.05433 - Original Lenia paper
 */

#include "LeniaEngine.hpp"
#include "Presets.hpp"
#include "Utils/GLUtils.hpp"
#include "Utils/Logger.hpp"
#include "Utils/NpyLoader.hpp"
#include <random>
#include <chrono>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace lenia {

LeniaEngine::~LeniaEngine() {
    if (m_simUBO)   glDeleteBuffers(1, &m_simUBO);
    if (m_multiUBO) glDeleteBuffers(1, &m_multiUBO);
    if (m_noiseUBO) glDeleteBuffers(1, &m_noiseUBO);
    if (m_stateSampler)  glDeleteSamplers(1, &m_stateSampler);
    if (m_kernelSampler) glDeleteSamplers(1, &m_kernelSampler);
    if (m_debugSampler)  glDeleteSamplers(1, &m_debugSampler);
    if (m_neighborSumsTex) glDeleteTextures(1, &m_neighborSumsTex);
    if (m_growthTex)       glDeleteTextures(1, &m_growthTex);
}

bool LeniaEngine::init(const std::string& assetDir) {
    std::string shaderDir = assetDir + "/shaders/";
    LOG_INFO("Loading shaders from: %s", shaderDir.c_str());

    m_initDir = "Initialisation";

    if (!m_kernelMgr.init(shaderDir + "kernel_gen.comp")) {
        LOG_ERROR("Failed to load kernel_gen.comp"); return false;
    }
    for (int i = 0; i < 16; ++i) {
        if (!m_ruleKernels[i].init(shaderDir + "kernel_gen.comp")) {
            LOG_ERROR("Failed to init rule kernel %d", i); return false;
        }
    }
    if (!m_simShader.loadCompute(shaderDir + "sim_spatial.comp")) {
        LOG_ERROR("Failed to load sim_spatial.comp"); return false;
    }
    if (!m_multiChannelShader.loadCompute(shaderDir + "sim_multichannel.comp")) {
        LOG_ERROR("Failed to load sim_multichannel.comp"); return false;
    }
    if (!m_noiseShader.loadCompute(shaderDir + "sim_noise.comp")) {
        LOG_ERROR("Failed to load sim_noise.comp"); return false;
    }
    if (!m_renderer.init(shaderDir + "display.vert", shaderDir + "display.frag")) {
        LOG_ERROR("Failed to load display shaders"); return false;
    }
    if (!m_analysisMgr.init(shaderDir + "analysis.comp")) {
        LOG_ERROR("Failed to load analysis.comp"); return false;
    }

    LOG_INFO("All shaders loaded successfully.");
    createUBOs();

    glCreateSamplers(1, &m_stateSampler);
    glSamplerParameteri(m_stateSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(m_stateSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(m_stateSampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(m_stateSampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glCreateSamplers(1, &m_kernelSampler);
    glSamplerParameteri(m_kernelSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(m_kernelSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(m_kernelSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_kernelSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateSamplers(1, &m_debugSampler);
    glSamplerParameteri(m_debugSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(m_debugSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(m_debugSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_debugSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    LeniaParams defaults;
    m_state.init(defaults.gridW, defaults.gridH);
    applyPreset(0, defaults);

    return true;
}

void LeniaEngine::createUBOs() {
    glCreateBuffers(1, &m_simUBO);
    glNamedBufferStorage(m_simUBO, sizeof(GPUSimParams), nullptr, GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &m_multiUBO);
    glNamedBufferStorage(m_multiUBO, sizeof(GPUMultiChannelParams), nullptr, GL_DYNAMIC_STORAGE_BIT);

    glCreateBuffers(1, &m_noiseUBO);
    glNamedBufferStorage(m_noiseUBO, sizeof(GPUNoiseParams), nullptr, GL_DYNAMIC_STORAGE_BIT);
}

/**
 * @brief Run one or more simulation steps.
 * 
 * Each step:
 * 1. Binds current state and kernel textures
 * 2. Runs compute shader to convolve and apply growth function
 * 3. Writes result to next state texture
 * 4. Swaps buffers for next iteration
 * 
 * @param params Simulation parameters (mu, sigma, dt, etc.)
 * @param steps Number of simulation steps to run
 */
void LeniaEngine::update(const LeniaParams& params, int steps) {
    // Configure texture wrapping based on edge mode
    // 0 = Periodic (wrap), 1 = Clamp, 2 = Mirror
    GLenum wrapX = (params.edgeModeX == 0) ? GL_REPEAT : 
                   (params.edgeModeX == 2) ? GL_MIRRORED_REPEAT : GL_CLAMP_TO_EDGE;
    GLenum wrapY = (params.edgeModeY == 0) ? GL_REPEAT : 
                   (params.edgeModeY == 2) ? GL_MIRRORED_REPEAT : GL_CLAMP_TO_EDGE;
    glSamplerParameteri(m_stateSampler, GL_TEXTURE_WRAP_S, wrapX);
    glSamplerParameteri(m_stateSampler, GL_TEXTURE_WRAP_T, wrapY);

    // Prepare GPU parameters structure
    GPUSimParams gpu{};
    gpu.gridW       = m_state.width();
    gpu.gridH       = m_state.height();
    gpu.radius      = params.radius;
    gpu.dt          = params.dt;      // Time step for integration
    gpu.mu          = params.mu;      // Growth function center
    gpu.sigma       = params.sigma;   // Growth function width
    gpu.growthType  = params.growthType;
    gpu.param1      = params.noiseParam1;
    gpu.param2      = params.noiseParam2;
    gpu.wallValue   = params.wallValue;
    gpu.wallEnabled = (m_wallTex != 0) ? 1 : 0;

    glNamedBufferSubData(m_simUBO, 0, sizeof(GPUSimParams), &gpu);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_simUBO);

    m_simShader.use();

    bool wantDebug = (params.displayMode != 0);
    if (wantDebug)
        ensureDebugTextures(m_state.width(), m_state.height());

    for (int i = 0; i < steps; ++i) {
        glBindTextureUnit(0, m_state.currentTexture());
        glBindSampler(0, m_stateSampler);
        glBindImageTexture(1, m_state.nextTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        glBindTextureUnit(2, m_kernelMgr.texture());
        glBindSampler(2, m_kernelSampler);
        
        // Bind wall texture
        if (m_wallTex != 0) {
            glBindTextureUnit(3, m_wallTex);
        }

        if (wantDebug) {
            glBindImageTexture(4, m_neighborSumsTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glBindImageTexture(5, m_growthTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        }

        dispatchCompute2D(m_state.width(), m_state.height());
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

        m_state.swap();
        
        enforceObstacles(params);
        
        m_stepCount++;
    }

    glBindSampler(0, 0);
    glBindSampler(2, 0);
}

void LeniaEngine::render(int viewportW, int viewportH, const LeniaParams& params) {
    GLuint tex = m_state.currentTexture();
    if (params.displayMode == 1 || params.displayMode == 2) {
        ensureDebugTextures(m_state.width(), m_state.height());
        tex = (params.displayMode == 1) ? m_neighborSumsTex : m_growthTex;
    } else if (params.displayMode == 3) {
        tex = m_kernelMgr.texture();
    }
    m_renderer.draw(tex, viewportW, viewportH, params);
}

void LeniaEngine::reset(const LeniaParams& params) {
    if (params.numChannels > 1) {
        const auto& mcPresets = getMultiChannelPresets();
        int mcIdx = static_cast<int>(params.noiseParam4);
        if (mcIdx >= 0 && mcIdx < static_cast<int>(mcPresets.size())) {
            const auto& mcp = mcPresets[mcIdx];
            if (mcp.cellsCh0 && mcp.cellRows > 0 && mcp.cellCols > 0) {
                loadMultiChannelCellData(mcp, params);
                return;
            }
        }
        if (params.placementClearFirst)
            m_state.clear();
        randomizeGrid(params);
        return;
    }

    if (params.noiseMode == static_cast<int>(InitMode::Species)) {
        if (params.placementClearFirst)
            m_state.clear();

        const auto& presets = getPresets();
        int presetIdx = static_cast<int>(params.noiseParam3);
        if (presetIdx >= 0 && presetIdx < static_cast<int>(presets.size())) {
            const auto& preset = presets[presetIdx];
            if (preset.cellData && preset.cellRows > 0 && preset.cellCols > 0) {
                loadCellData(preset.cellData, preset.cellRows, preset.cellCols, params);
                return;
            }
        }

        loadSpeciesAndPlace(params);
        return;
    }

    auto seed = static_cast<uint32_t>(
        std::chrono::steady_clock::now().time_since_epoch().count() & 0xFFFFFFFF);

    GPUNoiseParams gpu{};
    gpu.gridW  = m_state.width();
    gpu.gridH  = m_state.height();
    gpu.mode   = params.noiseMode;
    gpu.seed   = seed;
    gpu.param1 = params.noiseParam1;
    gpu.param2 = params.noiseParam2;
    gpu.param3 = params.noiseParam3;
    gpu.param4 = params.noiseParam4;

    glNamedBufferSubData(m_noiseUBO, 0, sizeof(GPUNoiseParams), &gpu);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_noiseUBO);

    m_noiseShader.use();
    glBindImageTexture(0, m_state.currentTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    dispatchCompute2D(m_state.width(), m_state.height());
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void LeniaEngine::loadSpeciesAndPlace(const LeniaParams& params) {
    const auto& presets = getPresets();
    int idx = -1;
    for (int i = 0; i < static_cast<int>(presets.size()); ++i) {
        if (presets[i].speciesFile && params.noiseParam3 == static_cast<float>(i)) {
            idx = i;
            break;
        }
    }

    const char* speciesFile = nullptr;
    PlacementMode placement = static_cast<PlacementMode>(params.placementMode);
    bool flipH = params.placementFlipH;
    bool flipV = params.placementFlipV;
    bool randomFlip = params.placementRandomFlip;
    int count = std::max(1, params.placementCount);
    float spacing = params.placementSpacing;
    float margin = params.placementMargin;
    float scale = params.placementScale;
    int rotation = params.placementRotation;
    int minSep = params.placementMinSeparation;

    if (idx >= 0) {
        speciesFile = presets[idx].speciesFile;
        if (params.placementMode == 0 && presets[idx].placement != PlacementMode::Center) {
            placement = presets[idx].placement;
        }
        if (presets[idx].flipInit && !flipH && !flipV) {
            flipV = true;
        }
    }

    if (!speciesFile) {
        GPUNoiseParams gpu{};
        gpu.gridW = m_state.width();
        gpu.gridH = m_state.height();
        gpu.mode = 1;
        gpu.seed = 42;
        glNamedBufferSubData(m_noiseUBO, 0, sizeof(GPUNoiseParams), &gpu);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_noiseUBO);
        m_noiseShader.use();
        glBindImageTexture(0, m_state.currentTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        dispatchCompute2D(m_state.width(), m_state.height());
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        return;
    }

    std::string path = m_initDir + "/" + speciesFile;
    NpyArray arr;
    if (!loadNpy(path, arr)) {
        LOG_ERROR("Failed to load species file: %s", path.c_str());
        return;
    }

    int gw = m_state.width();
    int gh = m_state.height();

    auto applyFlips = [&](std::vector<float>& data, int rows, int cols, bool fh, bool fv) {
        if (fv) {
            for (int r = 0; r < rows / 2; ++r)
                for (int c = 0; c < cols; ++c)
                    std::swap(data[r * cols + c], data[(rows - 1 - r) * cols + c]);
        }
        if (fh) {
            for (int r = 0; r < rows; ++r)
                for (int c = 0; c < cols / 2; ++c)
                    std::swap(data[r * cols + c], data[r * cols + (cols - 1 - c)]);
        }
    };

    auto rotate90 = [](const std::vector<float>& data, int& rows, int& cols) {
        std::vector<float> rotated(rows * cols);
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                rotated[c * rows + (rows - 1 - r)] = data[r * cols + c];
        int tmp = rows;
        rows = cols;
        cols = tmp;
        return rotated;
    };

    for (int r = 0; r < rotation; ++r) {
        arr.data = rotate90(arr.data, arr.rows, arr.cols);
    }

    if (scale != 1.0f && scale > 0.0f) {
        int newRows = std::max(1, static_cast<int>(arr.rows * scale));
        int newCols = std::max(1, static_cast<int>(arr.cols * scale));
        std::vector<float> scaled(newRows * newCols);
        for (int r = 0; r < newRows; ++r) {
            for (int c = 0; c < newCols; ++c) {
                float srcR = static_cast<float>(r) / scale;
                float srcC = static_cast<float>(c) / scale;
                int sr = std::clamp(static_cast<int>(srcR), 0, arr.rows - 1);
                int sc = std::clamp(static_cast<int>(srcC), 0, arr.cols - 1);
                scaled[r * newCols + c] = arr.data[sr * arr.cols + sc];
            }
        }
        arr.data = std::move(scaled);
        arr.rows = newRows;
        arr.cols = newCols;
    }

    if (flipH || flipV) {
        applyFlips(arr.data, arr.rows, arr.cols, flipH, flipV);
    }

    int marginPxX = static_cast<int>(margin * gw);
    int marginPxY = static_cast<int>(margin * gh);

    auto placeOnGrid = [&](int dstX, int dstY, const std::vector<float>& src, int srcCols, int srcRows) {
        int x0 = dstX;
        int y0 = dstY;
        int w = srcCols;
        int h = srcRows;
        if (x0 < 0) { w += x0; x0 = 0; }
        if (y0 < 0) { h += y0; y0 = 0; }
        if (x0 + w > gw) w = gw - x0;
        if (y0 + h > gh) h = gh - y0;
        if (w <= 0 || h <= 0) return;

        int srcOffX = x0 - dstX;
        int srcOffY = y0 - dstY;
        std::vector<float> region(w * h);
        for (int r = 0; r < h; ++r)
            for (int c = 0; c < w; ++c)
                region[r * w + c] = src[(r + srcOffY) * srcCols + (c + srcOffX)];

        m_state.uploadRegion(x0, y0, w, h, region.data());
    };

    std::mt19937 rng(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()));

    auto placeSingle = [&](int dstX, int dstY) {
        if (randomFlip && count > 1) {
            std::vector<float> copy = arr.data;
            applyFlips(copy, arr.rows, arr.cols, rng() % 2 == 0, rng() % 2 == 0);
            placeOnGrid(dstX, dstY, copy, arr.cols, arr.rows);
        } else {
            placeOnGrid(dstX, dstY, arr.data, arr.cols, arr.rows);
        }
    };

    auto getPosition = [&](PlacementMode pm) -> std::pair<int, int> {
        switch (pm) {
            case PlacementMode::Center:
                return {(gw - arr.cols) / 2, (gh - arr.rows) / 2};
            case PlacementMode::TopLeft:
                return {marginPxX, marginPxY};
            case PlacementMode::TopRight:
                return {gw - marginPxX - arr.cols, marginPxY};
            case PlacementMode::BottomLeft:
                return {marginPxX, gh - marginPxY - arr.rows};
            case PlacementMode::BottomRight:
                return {gw - marginPxX - arr.cols, gh - marginPxY - arr.rows};
            case PlacementMode::Top:
                return {(gw - arr.cols) / 2, marginPxY};
            case PlacementMode::Bottom:
                return {(gw - arr.cols) / 2, gh - marginPxY - arr.rows};
            case PlacementMode::Left:
                return {marginPxX, (gh - arr.rows) / 2};
            case PlacementMode::Right:
                return {gw - marginPxX - arr.cols, (gh - arr.rows) / 2};
            default:
                return {(gw - arr.cols) / 2, (gh - arr.rows) / 2};
        }
    };

    switch (placement) {
        case PlacementMode::Center:
        case PlacementMode::TopLeft:
        case PlacementMode::TopRight:
        case PlacementMode::BottomLeft:
        case PlacementMode::BottomRight:
        case PlacementMode::Top:
        case PlacementMode::Bottom:
        case PlacementMode::Left:
        case PlacementMode::Right: {
            if (count == 1) {
                auto [px, py] = getPosition(placement);
                placeSingle(std::max(0, px), std::max(0, py));
            } else {
                auto [bx, by] = getPosition(placement);
                int spacePx = static_cast<int>(spacing * static_cast<float>(std::min(gw, gh)));
                for (int i = 0; i < count; ++i) {
                    int ox = bx + i * spacePx;
                    int oy = by;
                    if (ox + arr.cols > gw) {
                        ox = bx;
                        oy += (i * spacePx);
                    }
                    placeSingle(std::max(0, ox), std::max(0, oy));
                }
            }
            break;
        }
        case PlacementMode::Random: {
            int rangeX = std::max(1, gw - arr.cols - 2 * marginPxX);
            int rangeY = std::max(1, gh - arr.rows - 2 * marginPxY);
            for (int i = 0; i < count; ++i) {
                int x = marginPxX + static_cast<int>(rng() % static_cast<unsigned>(rangeX));
                int y = marginPxY + static_cast<int>(rng() % static_cast<unsigned>(rangeY));
                std::vector<float> copy = arr.data;
                if (randomFlip) {
                    applyFlips(copy, arr.rows, arr.cols, rng() % 2 == 0, rng() % 2 == 0);
                }
                placeOnGrid(x, y, copy, arr.cols, arr.rows);
            }
            break;
        }
        case PlacementMode::Grid: {
            int side = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(count))));
            int cellW = gw / side;
            int cellH = gh / side;
            int placed = 0;
            for (int gy2 = 0; gy2 < side && placed < count; ++gy2) {
                for (int gx2 = 0; gx2 < side && placed < count; ++gx2) {
                    int cx = gx2 * cellW + (cellW - arr.cols) / 2;
                    int cy = gy2 * cellH + (cellH - arr.rows) / 2;
                    std::vector<float> copy = arr.data;
                    if (randomFlip) {
                        applyFlips(copy, arr.rows, arr.cols, rng() % 2 == 0, rng() % 2 == 0);
                    }
                    placeOnGrid(std::max(0, cx), std::max(0, cy), copy, arr.cols, arr.rows);
                    placed++;
                }
            }
            break;
        }
        case PlacementMode::TwoPlace: {
            auto [p1x, p1y] = getPosition(PlacementMode::TopLeft);
            placeSingle(p1x, p1y);
            auto [p2x, p2y] = getPosition(PlacementMode::BottomRight);
            if (p2x > 0 && p2y > 0) {
                std::vector<float> copy = arr.data;
                if (randomFlip) {
                    applyFlips(copy, arr.rows, arr.cols, rng() % 2 == 0, rng() % 2 == 0);
                }
                placeOnGrid(p2x, p2y, copy, arr.cols, arr.rows);
            }
            break;
        }
        case PlacementMode::Scatter: {
            struct PlacedRect { int x, y, w, h; };
            std::vector<PlacedRect> placed;
            int rangeX = std::max(1, gw - arr.cols - 2 * marginPxX);
            int rangeY = std::max(1, gh - arr.rows - 2 * marginPxY);
            int maxAttempts = count * 200;
            int attempts = 0;
            while (static_cast<int>(placed.size()) < count && attempts < maxAttempts) {
                attempts++;
                int x = marginPxX + static_cast<int>(rng() % static_cast<unsigned>(rangeX));
                int y = marginPxY + static_cast<int>(rng() % static_cast<unsigned>(rangeY));
                bool overlaps = false;
                for (auto& pr : placed) {
                    int sepX = std::abs((x + arr.cols / 2) - (pr.x + pr.w / 2)) - (arr.cols + pr.w) / 2;
                    int sepY = std::abs((y + arr.rows / 2) - (pr.y + pr.h / 2)) - (arr.rows + pr.h) / 2;
                    int sep = std::max(sepX, sepY);
                    if (sep < minSep) {
                        overlaps = true;
                        break;
                    }
                }
                if (overlaps) continue;
                std::vector<float> copy = arr.data;
                if (randomFlip) {
                    applyFlips(copy, arr.rows, arr.cols, rng() % 2 == 0, rng() % 2 == 0);
                }
                placeOnGrid(x, y, copy, arr.cols, arr.rows);
                placed.push_back({x, y, arr.cols, arr.rows});
            }
            break;
        }
    }
}

void LeniaEngine::clear() {
    m_state.clear();
}

void LeniaEngine::randomizeGrid(const LeniaParams& params) {
    bool isBinary = (params.growthType == static_cast<int>(GrowthType::GameOfLife) ||
                     params.growthType == static_cast<int>(GrowthType::LargerThanLife));

    if (params.numChannels > 1) {
        int gw = m_state.width();
        int gh = m_state.height();
        std::mt19937 rng(static_cast<unsigned>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::vector<float> data(gw * gh * 4);
        for (int i = 0; i < gw * gh; ++i) {
            data[i * 4 + 0] = isBinary ? (dist(rng) > 0.5f ? 1.0f : 0.0f) : dist(rng);
            data[i * 4 + 1] = isBinary ? (dist(rng) > 0.5f ? 1.0f : 0.0f) : dist(rng);
            data[i * 4 + 2] = isBinary ? (dist(rng) > 0.5f ? 1.0f : 0.0f) : dist(rng);
            data[i * 4 + 3] = 1.0f;
        }
        m_state.uploadRegionRGBA(0, 0, gw, gh, data.data());
        return;
    }

    auto seed = static_cast<uint32_t>(
        std::chrono::steady_clock::now().time_since_epoch().count() & 0xFFFFFFFF);

    GPUNoiseParams gpu{};
    gpu.gridW  = m_state.width();
    gpu.gridH  = m_state.height();
    gpu.mode   = isBinary ? 7 : 0;
    gpu.seed   = seed;
    gpu.param1 = 0.0f;
    gpu.param2 = 0.0f;
    gpu.param3 = 0.0f;
    gpu.param4 = 0.0f;

    glNamedBufferSubData(m_noiseUBO, 0, sizeof(GPUNoiseParams), &gpu);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_noiseUBO);

    m_noiseShader.use();
    glBindImageTexture(0, m_state.currentTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    dispatchCompute2D(m_state.width(), m_state.height());
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void LeniaEngine::loadCellData(const float* data, int rows, int cols, const LeniaParams& params) {
    if (!data || rows <= 0 || cols <= 0) return;

    int gw = m_state.width();
    int gh = m_state.height();

    PlacementMode placement = static_cast<PlacementMode>(params.placementMode);
    bool flipH = params.placementFlipH;
    bool flipV = params.placementFlipV;
    bool randomFlip = params.placementRandomFlip;
    int count = std::max(1, params.placementCount);
    float margin = params.placementMargin;
    float scale = params.placementScale;
    int rotation = params.placementRotation;
    int minSep = params.placementMinSeparation;

    int arrRows = rows;
    int arrCols = cols;
    std::vector<float> arrData(data, data + rows * cols);

    auto applyFlips = [](std::vector<float>& d, int r, int c, bool fh, bool fv) {
        if (fv) {
            for (int row = 0; row < r / 2; ++row)
                for (int col = 0; col < c; ++col)
                    std::swap(d[row * c + col], d[(r - 1 - row) * c + col]);
        }
        if (fh) {
            for (int row = 0; row < r; ++row)
                for (int col = 0; col < c / 2; ++col)
                    std::swap(d[row * c + col], d[row * c + (c - 1 - col)]);
        }
    };

    auto rotate90 = [](const std::vector<float>& d, int& r, int& c) {
        std::vector<float> rotated(r * c);
        for (int row = 0; row < r; ++row)
            for (int col = 0; col < c; ++col)
                rotated[col * r + (r - 1 - row)] = d[row * c + col];
        int tmp = r;
        r = c;
        c = tmp;
        return rotated;
    };

    for (int r = 0; r < rotation; ++r)
        arrData = rotate90(arrData, arrRows, arrCols);

    if (scale != 1.0f && scale > 0.0f) {
        int newRows = std::max(1, static_cast<int>(arrRows * scale));
        int newCols = std::max(1, static_cast<int>(arrCols * scale));
        std::vector<float> scaled(newRows * newCols);
        for (int r = 0; r < newRows; ++r) {
            for (int c = 0; c < newCols; ++c) {
                float srcR = static_cast<float>(r) / scale;
                float srcC = static_cast<float>(c) / scale;
                int sr = std::clamp(static_cast<int>(srcR), 0, arrRows - 1);
                int sc = std::clamp(static_cast<int>(srcC), 0, arrCols - 1);
                scaled[r * newCols + c] = arrData[sr * arrCols + sc];
            }
        }
        arrData = std::move(scaled);
        arrRows = newRows;
        arrCols = newCols;
    }

    if (flipH || flipV)
        applyFlips(arrData, arrRows, arrCols, flipH, flipV);

    int marginPxX = static_cast<int>(margin * gw);
    int marginPxY = static_cast<int>(margin * gh);

    auto placeOnGrid = [&](int dstX, int dstY, const std::vector<float>& src, int srcCols, int srcRows) {
        int x0 = dstX;
        int y0 = dstY;
        int w = srcCols;
        int h = srcRows;
        if (x0 < 0) { w += x0; x0 = 0; }
        if (y0 < 0) { h += y0; y0 = 0; }
        if (x0 + w > gw) w = gw - x0;
        if (y0 + h > gh) h = gh - y0;
        if (w <= 0 || h <= 0) return;

        int srcOffX = x0 - dstX;
        int srcOffY = y0 - dstY;
        std::vector<float> region(w * h);
        for (int r = 0; r < h; ++r)
            for (int c = 0; c < w; ++c)
                region[r * w + c] = src[(r + srcOffY) * srcCols + (c + srcOffX)];

        m_state.uploadRegion(x0, y0, w, h, region.data());
    };

    std::mt19937 rng(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()));

    auto placeSingle = [&](int dstX, int dstY) {
        if (randomFlip && count > 1) {
            std::vector<float> copy = arrData;
            applyFlips(copy, arrRows, arrCols, rng() % 2 == 0, rng() % 2 == 0);
            placeOnGrid(dstX, dstY, copy, arrCols, arrRows);
        } else {
            placeOnGrid(dstX, dstY, arrData, arrCols, arrRows);
        }
    };

    auto getPosition = [&](PlacementMode pm) -> std::pair<int, int> {
        switch (pm) {
            case PlacementMode::Center:
                return {(gw - arrCols) / 2, (gh - arrRows) / 2};
            case PlacementMode::TopLeft:
                return {marginPxX, marginPxY};
            case PlacementMode::TopRight:
                return {gw - marginPxX - arrCols, marginPxY};
            case PlacementMode::BottomLeft:
                return {marginPxX, gh - marginPxY - arrRows};
            case PlacementMode::BottomRight:
                return {gw - marginPxX - arrCols, gh - marginPxY - arrRows};
            case PlacementMode::Top:
                return {(gw - arrCols) / 2, marginPxY};
            case PlacementMode::Bottom:
                return {(gw - arrCols) / 2, gh - marginPxY - arrRows};
            case PlacementMode::Left:
                return {marginPxX, (gh - arrRows) / 2};
            case PlacementMode::Right:
                return {gw - marginPxX - arrCols, (gh - arrRows) / 2};
            default:
                return {(gw - arrCols) / 2, (gh - arrRows) / 2};
        }
    };

    switch (placement) {
        case PlacementMode::Center:
        case PlacementMode::TopLeft:
        case PlacementMode::TopRight:
        case PlacementMode::BottomLeft:
        case PlacementMode::BottomRight:
        case PlacementMode::Top:
        case PlacementMode::Bottom:
        case PlacementMode::Left:
        case PlacementMode::Right: {
            if (count == 1) {
                auto [px, py] = getPosition(placement);
                placeSingle(std::max(0, px), std::max(0, py));
            } else {
                auto [bx, by] = getPosition(placement);
                int spacePx = static_cast<int>(params.placementSpacing * static_cast<float>(std::min(gw, gh)));
                for (int i = 0; i < count; ++i) {
                    int ox = bx + i * spacePx;
                    int oy = by;
                    if (ox + arrCols > gw) {
                        ox = bx;
                        oy += (i * spacePx);
                    }
                    placeSingle(std::max(0, ox), std::max(0, oy));
                }
            }
            break;
        }
        case PlacementMode::Random: {
            int rangeX = std::max(1, gw - arrCols - 2 * marginPxX);
            int rangeY = std::max(1, gh - arrRows - 2 * marginPxY);
            for (int i = 0; i < count; ++i) {
                int x = marginPxX + static_cast<int>(rng() % static_cast<unsigned>(rangeX));
                int y = marginPxY + static_cast<int>(rng() % static_cast<unsigned>(rangeY));
                std::vector<float> copy = arrData;
                if (randomFlip)
                    applyFlips(copy, arrRows, arrCols, rng() % 2 == 0, rng() % 2 == 0);
                placeOnGrid(x, y, copy, arrCols, arrRows);
            }
            break;
        }
        case PlacementMode::Grid: {
            int side = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(count))));
            int cellW = gw / side;
            int cellH = gh / side;
            int placed2 = 0;
            for (int gy2 = 0; gy2 < side && placed2 < count; ++gy2) {
                for (int gx2 = 0; gx2 < side && placed2 < count; ++gx2) {
                    int cx = gx2 * cellW + (cellW - arrCols) / 2;
                    int cy = gy2 * cellH + (cellH - arrRows) / 2;
                    std::vector<float> copy = arrData;
                    if (randomFlip)
                        applyFlips(copy, arrRows, arrCols, rng() % 2 == 0, rng() % 2 == 0);
                    placeOnGrid(std::max(0, cx), std::max(0, cy), copy, arrCols, arrRows);
                    placed2++;
                }
            }
            break;
        }
        case PlacementMode::TwoPlace: {
            auto [p1x, p1y] = getPosition(PlacementMode::TopLeft);
            placeSingle(p1x, p1y);
            auto [p2x, p2y] = getPosition(PlacementMode::BottomRight);
            if (p2x > 0 && p2y > 0) {
                std::vector<float> copy = arrData;
                if (randomFlip)
                    applyFlips(copy, arrRows, arrCols, rng() % 2 == 0, rng() % 2 == 0);
                placeOnGrid(p2x, p2y, copy, arrCols, arrRows);
            }
            break;
        }
        case PlacementMode::Scatter: {
            struct PlacedRect { int x, y, w, h; };
            std::vector<PlacedRect> placedRects;
            int rangeX = std::max(1, gw - arrCols - 2 * marginPxX);
            int rangeY = std::max(1, gh - arrRows - 2 * marginPxY);
            int maxAttempts = count * 200;
            int attempts = 0;
            while (static_cast<int>(placedRects.size()) < count && attempts < maxAttempts) {
                attempts++;
                int x = marginPxX + static_cast<int>(rng() % static_cast<unsigned>(rangeX));
                int y = marginPxY + static_cast<int>(rng() % static_cast<unsigned>(rangeY));
                bool overlaps = false;
                for (auto& pr : placedRects) {
                    int sepX = std::abs((x + arrCols / 2) - (pr.x + pr.w / 2)) - (arrCols + pr.w) / 2;
                    int sepY = std::abs((y + arrRows / 2) - (pr.y + pr.h / 2)) - (arrRows + pr.h) / 2;
                    int sep = std::max(sepX, sepY);
                    if (sep < minSep) { overlaps = true; break; }
                }
                if (overlaps) continue;
                std::vector<float> copy = arrData;
                if (randomFlip)
                    applyFlips(copy, arrRows, arrCols, rng() % 2 == 0, rng() % 2 == 0);
                placeOnGrid(x, y, copy, arrCols, arrRows);
                placedRects.push_back({x, y, arrCols, arrRows});
            }
            break;
        }
    }
}

void LeniaEngine::loadMultiChannelCellData(const MultiChannelPreset& mcp, const LeniaParams& params) {
    int gw = m_state.width();
    int gh = m_state.height();

    if (params.placementClearFirst)
        m_state.clear();

    PlacementMode placement = static_cast<PlacementMode>(params.placementMode);
    bool flipH = params.placementFlipH;
    bool flipV = params.placementFlipV;
    bool randomFlip = params.placementRandomFlip;
    int count = std::max(1, params.placementCount);
    float margin = params.placementMargin;
    float scale = params.placementScale;
    int rotation = params.placementRotation;
    int minSep = params.placementMinSeparation;

    int arrRows = mcp.cellRows;
    int arrCols = mcp.cellCols;
    int numPixels = arrRows * arrCols;

    std::vector<float> ch0(numPixels, 0.0f);
    std::vector<float> ch1(numPixels, 0.0f);
    std::vector<float> ch2(numPixels, 0.0f);
    if (mcp.cellsCh0) std::copy(mcp.cellsCh0, mcp.cellsCh0 + numPixels, ch0.begin());
    if (mcp.cellsCh1) std::copy(mcp.cellsCh1, mcp.cellsCh1 + numPixels, ch1.begin());
    if (mcp.cellsCh2) std::copy(mcp.cellsCh2, mcp.cellsCh2 + numPixels, ch2.begin());

    auto applyFlipsVec = [](std::vector<float>& d, int r, int c, bool fh, bool fv) {
        if (fv) {
            for (int row = 0; row < r / 2; ++row)
                for (int col = 0; col < c; ++col)
                    std::swap(d[row * c + col], d[(r - 1 - row) * c + col]);
        }
        if (fh) {
            for (int row = 0; row < r; ++row)
                for (int col = 0; col < c / 2; ++col)
                    std::swap(d[row * c + col], d[row * c + (c - 1 - col)]);
        }
    };

    auto rotate90Vec = [](const std::vector<float>& d, int r, int c) {
        std::vector<float> rotated(r * c);
        for (int row = 0; row < r; ++row)
            for (int col = 0; col < c; ++col)
                rotated[col * r + (r - 1 - row)] = d[row * c + col];
        return rotated;
    };

    for (int r = 0; r < rotation; ++r) {
        ch0 = rotate90Vec(ch0, arrRows, arrCols);
        ch1 = rotate90Vec(ch1, arrRows, arrCols);
        ch2 = rotate90Vec(ch2, arrRows, arrCols);
        int tmp = arrRows;
        arrRows = arrCols;
        arrCols = tmp;
    }

    if (scale != 1.0f && scale > 0.0f) {
        int newRows = std::max(1, static_cast<int>(arrRows * scale));
        int newCols = std::max(1, static_cast<int>(arrCols * scale));
        auto scaleChannel = [&](const std::vector<float>& src) {
            std::vector<float> dst(newRows * newCols);
            for (int r = 0; r < newRows; ++r)
                for (int c = 0; c < newCols; ++c) {
                    int sr = std::clamp(static_cast<int>(static_cast<float>(r) / scale), 0, arrRows - 1);
                    int sc = std::clamp(static_cast<int>(static_cast<float>(c) / scale), 0, arrCols - 1);
                    dst[r * newCols + c] = src[sr * arrCols + sc];
                }
            return dst;
        };
        ch0 = scaleChannel(ch0);
        ch1 = scaleChannel(ch1);
        ch2 = scaleChannel(ch2);
        arrRows = newRows;
        arrCols = newCols;
    }

    if (flipH || flipV) {
        applyFlipsVec(ch0, arrRows, arrCols, flipH, flipV);
        applyFlipsVec(ch1, arrRows, arrCols, flipH, flipV);
        applyFlipsVec(ch2, arrRows, arrCols, flipH, flipV);
    }

    int marginPxX = static_cast<int>(margin * gw);
    int marginPxY = static_cast<int>(margin * gh);

    auto placeRGBA = [&](int dstX, int dstY, const std::vector<float>& c0,
                         const std::vector<float>& c1, const std::vector<float>& c2,
                         int srcCols, int srcRows) {
        int x0 = dstX, y0 = dstY, w = srcCols, h = srcRows;
        if (x0 < 0) { w += x0; x0 = 0; }
        if (y0 < 0) { h += y0; y0 = 0; }
        if (x0 + w > gw) w = gw - x0;
        if (y0 + h > gh) h = gh - y0;
        if (w <= 0 || h <= 0) return;

        int srcOffX = x0 - dstX;
        int srcOffY = y0 - dstY;
        std::vector<float> region(w * h * 4);
        for (int r = 0; r < h; ++r)
            for (int c2i = 0; c2i < w; ++c2i) {
                int si = (r + srcOffY) * srcCols + (c2i + srcOffX);
                int di = (r * w + c2i) * 4;
                region[di + 0] = c0[si];
                region[di + 1] = c1[si];
                region[di + 2] = c2[si];
                region[di + 3] = 1.0f;
            }
        m_state.uploadRegionRGBA(x0, y0, w, h, region.data());
    };

    std::mt19937 rng(static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count()));

    auto placeSingle = [&](int dstX, int dstY) {
        if (randomFlip && count > 1) {
            auto c0 = ch0, c1 = ch1, c2 = ch2;
            bool fh = rng() % 2 == 0, fv = rng() % 2 == 0;
            applyFlipsVec(c0, arrRows, arrCols, fh, fv);
            applyFlipsVec(c1, arrRows, arrCols, fh, fv);
            applyFlipsVec(c2, arrRows, arrCols, fh, fv);
            placeRGBA(dstX, dstY, c0, c1, c2, arrCols, arrRows);
        } else {
            placeRGBA(dstX, dstY, ch0, ch1, ch2, arrCols, arrRows);
        }
    };

    auto getPosition = [&](PlacementMode pm) -> std::pair<int, int> {
        switch (pm) {
            case PlacementMode::Center:      return {(gw - arrCols) / 2, (gh - arrRows) / 2};
            case PlacementMode::TopLeft:     return {marginPxX, marginPxY};
            case PlacementMode::TopRight:    return {gw - marginPxX - arrCols, marginPxY};
            case PlacementMode::BottomLeft:  return {marginPxX, gh - marginPxY - arrRows};
            case PlacementMode::BottomRight: return {gw - marginPxX - arrCols, gh - marginPxY - arrRows};
            case PlacementMode::Top:         return {(gw - arrCols) / 2, marginPxY};
            case PlacementMode::Bottom:      return {(gw - arrCols) / 2, gh - marginPxY - arrRows};
            case PlacementMode::Left:        return {marginPxX, (gh - arrRows) / 2};
            case PlacementMode::Right:       return {gw - marginPxX - arrCols, (gh - arrRows) / 2};
            default:                         return {(gw - arrCols) / 2, (gh - arrRows) / 2};
        }
    };

    switch (placement) {
        case PlacementMode::Center:
        case PlacementMode::TopLeft:
        case PlacementMode::TopRight:
        case PlacementMode::BottomLeft:
        case PlacementMode::BottomRight:
        case PlacementMode::Top:
        case PlacementMode::Bottom:
        case PlacementMode::Left:
        case PlacementMode::Right: {
            if (count == 1) {
                auto [px, py] = getPosition(placement);
                placeSingle(std::max(0, px), std::max(0, py));
            } else {
                auto [bx, by] = getPosition(placement);
                int spacePx = static_cast<int>(params.placementSpacing * static_cast<float>(std::min(gw, gh)));
                for (int i = 0; i < count; ++i) {
                    int ox = bx + i * spacePx;
                    int oy = by;
                    if (ox + arrCols > gw) { ox = bx; oy += (i * spacePx); }
                    placeSingle(std::max(0, ox), std::max(0, oy));
                }
            }
            break;
        }
        case PlacementMode::Random: {
            int rangeX = std::max(1, gw - arrCols - 2 * marginPxX);
            int rangeY = std::max(1, gh - arrRows - 2 * marginPxY);
            for (int i = 0; i < count; ++i) {
                int x = marginPxX + static_cast<int>(rng() % static_cast<unsigned>(rangeX));
                int y = marginPxY + static_cast<int>(rng() % static_cast<unsigned>(rangeY));
                auto c0 = ch0, c1 = ch1, c2 = ch2;
                if (randomFlip) {
                    bool fh = rng() % 2 == 0, fv = rng() % 2 == 0;
                    applyFlipsVec(c0, arrRows, arrCols, fh, fv);
                    applyFlipsVec(c1, arrRows, arrCols, fh, fv);
                    applyFlipsVec(c2, arrRows, arrCols, fh, fv);
                }
                placeRGBA(x, y, c0, c1, c2, arrCols, arrRows);
            }
            break;
        }
        case PlacementMode::Grid: {
            int side = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(count))));
            int cellW = gw / side;
            int cellH = gh / side;
            int placed = 0;
            for (int gy2 = 0; gy2 < side && placed < count; ++gy2) {
                for (int gx2 = 0; gx2 < side && placed < count; ++gx2) {
                    int cx = gx2 * cellW + (cellW - arrCols) / 2;
                    int cy = gy2 * cellH + (cellH - arrRows) / 2;
                    auto c0 = ch0, c1 = ch1, c2 = ch2;
                    if (randomFlip) {
                        bool fh = rng() % 2 == 0, fv = rng() % 2 == 0;
                        applyFlipsVec(c0, arrRows, arrCols, fh, fv);
                        applyFlipsVec(c1, arrRows, arrCols, fh, fv);
                        applyFlipsVec(c2, arrRows, arrCols, fh, fv);
                    }
                    placeRGBA(std::max(0, cx), std::max(0, cy), c0, c1, c2, arrCols, arrRows);
                    placed++;
                }
            }
            break;
        }
        case PlacementMode::TwoPlace: {
            auto [p1x, p1y] = getPosition(PlacementMode::TopLeft);
            placeSingle(p1x, p1y);
            auto [p2x, p2y] = getPosition(PlacementMode::BottomRight);
            if (p2x > 0 && p2y > 0) {
                auto c0 = ch0, c1 = ch1, c2 = ch2;
                if (randomFlip) {
                    bool fh = rng() % 2 == 0, fv = rng() % 2 == 0;
                    applyFlipsVec(c0, arrRows, arrCols, fh, fv);
                    applyFlipsVec(c1, arrRows, arrCols, fh, fv);
                    applyFlipsVec(c2, arrRows, arrCols, fh, fv);
                }
                placeRGBA(p2x, p2y, c0, c1, c2, arrCols, arrRows);
            }
            break;
        }
        case PlacementMode::Scatter: {
            struct PlacedRect { int x, y, w, h; };
            std::vector<PlacedRect> placedRects;
            int rangeX = std::max(1, gw - arrCols - 2 * marginPxX);
            int rangeY = std::max(1, gh - arrRows - 2 * marginPxY);
            int maxAttempts = count * 200;
            int attempts = 0;
            while (static_cast<int>(placedRects.size()) < count && attempts < maxAttempts) {
                attempts++;
                int x = marginPxX + static_cast<int>(rng() % static_cast<unsigned>(rangeX));
                int y = marginPxY + static_cast<int>(rng() % static_cast<unsigned>(rangeY));
                bool overlaps = false;
                for (auto& pr : placedRects) {
                    int sepX = std::abs((x + arrCols / 2) - (pr.x + pr.w / 2)) - (arrCols + pr.w) / 2;
                    int sepY = std::abs((y + arrRows / 2) - (pr.y + pr.h / 2)) - (arrRows + pr.h) / 2;
                    int sep = std::max(sepX, sepY);
                    if (sep < minSep) { overlaps = true; break; }
                }
                if (overlaps) continue;
                auto c0 = ch0, c1 = ch1, c2 = ch2;
                if (randomFlip) {
                    bool fh = rng() % 2 == 0, fv = rng() % 2 == 0;
                    applyFlipsVec(c0, arrRows, arrCols, fh, fv);
                    applyFlipsVec(c1, arrRows, arrCols, fh, fv);
                    applyFlipsVec(c2, arrRows, arrCols, fh, fv);
                }
                placeRGBA(x, y, c0, c1, c2, arrCols, arrRows);
                placedRects.push_back({x, y, arrCols, arrRows});
            }
            break;
        }
    }
}

void LeniaEngine::regenerateKernel(const LeniaParams& params) {
    KernelConfig cfg;
    cfg.radius              = params.radius;
    cfg.numRings            = params.numRings;
    cfg.kernelType          = params.kernelType;
    cfg.kernelModifier      = params.kernelModifier;
    cfg.anisotropyStrength  = params.kernelAnisotropy;
    cfg.anisotropyAngle     = params.kernelAnisotropyAngle;
    cfg.timePhase           = 0.0f;
    cfg.pulseFrequency      = params.kernelTimeVarying ? params.kernelPulseFrequency : 0.0f;
    for (int i = 0; i < 16; ++i)
        cfg.ringWeights[i] = params.ringWeights[i];
    m_kernelMgr.generate(cfg);
}

void LeniaEngine::regenerateRuleKernel(int ruleIndex, const LeniaParams& params) {
    if (ruleIndex < 0 || ruleIndex >= params.numKernelRules) return;
    const auto& rule = params.kernelRules[ruleIndex];
    int ruleR = std::max(1, static_cast<int>(params.radius * rule.radiusFraction));
    KernelConfig cfg;
    cfg.radius = ruleR;
    cfg.numRings = rule.numRings;
    cfg.kernelType = rule.kernelType;
    for (int j = 0; j < 16; ++j)
        cfg.ringWeights[j] = rule.ringWeights[j];
    m_ruleKernels[ruleIndex].generate(cfg);
}

void LeniaEngine::resizeGrid(const LeniaParams& params) {
    m_state.resize(params.gridW, params.gridH);
}

void LeniaEngine::applyPreset(int index, LeniaParams& params) {
    const auto& presets = getPresets();
    if (index < 0 || index >= static_cast<int>(presets.size())) return;

    const Preset& p = presets[index];
    params.mu         = p.mu;
    params.sigma      = p.sigma;
    params.dt         = p.dt;
    params.radius     = p.radius;
    params.numRings   = p.numRings;
    params.kernelType = static_cast<int>(p.kernelType);
    params.growthType = static_cast<int>(p.growthType);
    for (int i = 0; i < 16; ++i)
        params.ringWeights[i] = p.ringWeights[i];
    params.noiseMode   = static_cast<int>(p.initMode);
    params.noiseParam1 = p.initParam1;
    params.noiseParam2 = p.initParam2;
    params.noiseParam3 = static_cast<float>(index);
    params.noiseParam4 = 0.0f;

    if (std::strcmp(p.category, "Multichannel") == 0 || std::strcmp(p.category, "Multi-Kernel") == 0) {
        const auto& mcPresets = getMultiChannelPresets();
        int mcIdx = -1;
        for (int i = 0; i < static_cast<int>(mcPresets.size()); ++i) {
            if (std::strcmp(mcPresets[i].name, p.name) == 0) {
                mcIdx = i;
                break;
            }
        }
        if (mcIdx < 0) mcIdx = 0;

        const auto& mcp = mcPresets[mcIdx];
        params.numChannels = mcp.numChannels;
        if (params.numChannels > 1) {
            m_state.init(p.gridW, p.gridH, GL_RGBA32F);
        } else {
            m_state.init(p.gridW, p.gridH, GL_R32F);
        }
        params.gridW = p.gridW;
        params.gridH = p.gridH;
        params.radius = mcp.radius;
        params.dt = mcp.dt;
        params.numKernelRules = mcp.numRules;
        params.noiseParam4 = static_cast<float>(mcIdx);
        for (int i = 0; i < mcp.numRules; ++i) {
            auto& kr = params.kernelRules[i];
            const auto& mr = mcp.rules[i];
            kr.mu = mr.mu;
            kr.sigma = mr.sigma;
            kr.growthStrength = mr.growthStrength;
            kr.radiusFraction = mr.radiusFraction;
            kr.sourceChannel = mr.sourceChannel;
            kr.destChannel = mr.destChannel;
            kr.numRings = mr.numRings;
            kr.kernelType = mr.kernelType;
            kr.growthType = mr.growthType;
            for (int j = 0; j < 16; ++j)
                kr.ringWeights[j] = mr.ringWeights[j];
        }
        regenerateKernel(params);
        for (int i = 0; i < params.numKernelRules; ++i) {
            auto& rule = params.kernelRules[i];
            int ruleR = std::max(1, static_cast<int>(params.radius * rule.radiusFraction));
            KernelConfig cfg;
            cfg.radius = ruleR;
            cfg.numRings = rule.numRings;
            cfg.kernelType = rule.kernelType;
            for (int j = 0; j < 16; ++j)
                cfg.ringWeights[j] = rule.ringWeights[j];
            m_ruleKernels[i].generate(cfg);
        }
        return;
    }

    params.numChannels = 1;
    params.numKernelRules = 0;

    if (p.gridW != params.gridW || p.gridH != params.gridH) {
        params.gridW = p.gridW;
        params.gridH = p.gridH;
        resizeGrid(params);
    }

    if (m_state.format() != GL_R32F) {
        m_state.init(params.gridW, params.gridH, GL_R32F);
    }

    regenerateKernel(params);
}

void LeniaEngine::applyKernelPreset(int index, LeniaParams& params) {
    const auto& kp = getKernelPresets();
    if (index < 0 || index >= static_cast<int>(kp.size())) return;

    const KernelPreset& k = kp[index];
    params.kernelType = k.kernelType;
    params.numRings = k.numRings;
    params.radius = k.radius;
    for (int i = 0; i < 16; ++i)
        params.ringWeights[i] = k.ringWeights[i];

    regenerateKernel(params);
}

void LeniaEngine::runAnalysis(float threshold) {
    m_analysisMgr.analyze(m_state.currentTexture(), m_state.width(), m_state.height(), threshold);
}

void LeniaEngine::updateMultiChannel(const LeniaParams& params, int steps) {
    GLenum wrapX = (params.edgeModeX == 0) ? GL_REPEAT : 
                   (params.edgeModeX == 2) ? GL_MIRRORED_REPEAT : GL_CLAMP_TO_EDGE;
    GLenum wrapY = (params.edgeModeY == 0) ? GL_REPEAT : 
                   (params.edgeModeY == 2) ? GL_MIRRORED_REPEAT : GL_CLAMP_TO_EDGE;
    glSamplerParameteri(m_stateSampler, GL_TEXTURE_WRAP_S, wrapX);
    glSamplerParameteri(m_stateSampler, GL_TEXTURE_WRAP_T, wrapY);

    m_multiChannelShader.use();
    ensureDebugTextures(m_state.width(), m_state.height());

    glBindSampler(0, m_stateSampler);
    glBindSampler(3, m_stateSampler);
    glBindSampler(6, m_debugSampler);
    glBindSampler(7, m_debugSampler);

    for (int s = 0; s < steps; ++s) {
        glCopyImageSubData(
            m_state.currentTexture(), GL_TEXTURE_2D, 0, 0, 0, 0,
            m_state.nextTexture(), GL_TEXTURE_2D, 0, 0, 0, 0,
            m_state.width(), m_state.height(), 1);

        float zero[4] = {0, 0, 0, 0};
        glClearTexImage(m_neighborSumsTex, 0, GL_RGBA, GL_FLOAT, zero);
        glClearTexImage(m_growthTex, 0, GL_RGBA, GL_FLOAT, zero);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glBindTextureUnit(0, m_state.currentTexture());
        glBindTextureUnit(6, m_neighborSumsTex);
        glBindTextureUnit(7, m_growthTex);

        for (int r = 0; r < params.numKernelRules; ++r) {
            const auto& rule = params.kernelRules[r];
            int ruleRadius = std::max(1, static_cast<int>(params.radius * rule.radiusFraction));

            GPUMultiChannelParams gpu{};
            gpu.gridW = m_state.width();
            gpu.gridH = m_state.height();
            gpu.radius = ruleRadius;
            gpu.dt = params.dt;
            gpu.mu = rule.mu;
            gpu.sigma = rule.sigma;
            gpu.growthType = rule.growthType;
            gpu.sourceChannel = rule.sourceChannel;
            gpu.destChannel = rule.destChannel;
            gpu.growthStrength = rule.growthStrength;
            gpu.rulePass = r;
            gpu.numRules = params.numKernelRules;

            glNamedBufferSubData(m_multiUBO, 0, sizeof(GPUMultiChannelParams), &gpu);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_multiUBO);

            glBindTextureUnit(3, m_state.nextTexture());
            glBindImageTexture(1, m_state.nextTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glBindTextureUnit(2, m_ruleKernels[r].texture());
            glBindSampler(2, m_kernelSampler);
            glBindImageTexture(4, m_neighborSumsTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glBindImageTexture(5, m_growthTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

            dispatchCompute2D(m_state.width(), m_state.height());
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        }

        m_state.swap();
        
        enforceObstacles(params);
        
        m_stepCount++;
    }

    glBindSampler(0, 0);
    glBindSampler(2, 0);
    glBindSampler(3, 0);
    glBindSampler(6, 0);
    glBindSampler(7, 0);
}

void LeniaEngine::switchChannelMode(LeniaParams& params, int numChannels) {
    params.numChannels = numChannels;
    GLenum fmt = (numChannels > 1) ? GL_RGBA32F : GL_R32F;
    if (m_state.format() != fmt) {
        m_state.init(params.gridW, params.gridH, fmt);
    }
}

void LeniaEngine::ensureDebugTextures(int w, int h) {
    if (m_debugTexW == w && m_debugTexH == h && m_neighborSumsTex && m_growthTex) return;
    if (m_neighborSumsTex) glDeleteTextures(1, &m_neighborSumsTex);
    if (m_growthTex) glDeleteTextures(1, &m_growthTex);

    auto makeTex = [&]() -> GLuint {
        GLuint tex;
        glCreateTextures(GL_TEXTURE_2D, 1, &tex);
        glTextureStorage2D(tex, 1, GL_RGBA32F, w, h);
        return tex;
    };

    m_neighborSumsTex = makeTex();
    m_growthTex = makeTex();
    m_debugTexW = w;
    m_debugTexH = h;
}

void LeniaEngine::flipGridHorizontal() {
    int w = m_state.width();
    int h = m_state.height();
    bool isRGB = (m_state.format() == GL_RGBA32F);
    int components = isRGB ? 4 : 1;
    std::vector<float> pixels(w * h * components);

    glGetTextureImage(m_state.currentTexture(), 0, isRGB ? GL_RGBA : GL_RED, GL_FLOAT,
                      static_cast<GLsizei>(pixels.size() * sizeof(float)), pixels.data());

    std::vector<float> flipped(pixels.size());
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int srcIdx = (y * w + x) * components;
            int dstIdx = (y * w + (w - 1 - x)) * components;
            for (int c = 0; c < components; ++c)
                flipped[dstIdx + c] = pixels[srcIdx + c];
        }
    }

    glTextureSubImage2D(m_state.currentTexture(), 0, 0, 0, w, h,
                        isRGB ? GL_RGBA : GL_RED, GL_FLOAT, flipped.data());
}

void LeniaEngine::flipGridVertical() {
    int w = m_state.width();
    int h = m_state.height();
    bool isRGB = (m_state.format() == GL_RGBA32F);
    int components = isRGB ? 4 : 1;
    std::vector<float> pixels(w * h * components);

    glGetTextureImage(m_state.currentTexture(), 0, isRGB ? GL_RGBA : GL_RED, GL_FLOAT,
                      static_cast<GLsizei>(pixels.size() * sizeof(float)), pixels.data());

    std::vector<float> flipped(pixels.size());
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int srcIdx = (y * w + x) * components;
            int dstIdx = ((h - 1 - y) * w + x) * components;
            for (int c = 0; c < components; ++c)
                flipped[dstIdx + c] = pixels[srcIdx + c];
        }
    }

    glTextureSubImage2D(m_state.currentTexture(), 0, 0, 0, w, h,
                        isRGB ? GL_RGBA : GL_RED, GL_FLOAT, flipped.data());
}

void LeniaEngine::rotateGrid(int direction, LeniaParams& params) {
    int w = m_state.width();
    int h = m_state.height();
    bool isRGB = (m_state.format() == GL_RGBA32F);
    int components = isRGB ? 4 : 1;
    std::vector<float> pixels(w * h * components);

    glGetTextureImage(m_state.currentTexture(), 0, isRGB ? GL_RGBA : GL_RED, GL_FLOAT,
                      static_cast<GLsizei>(pixels.size() * sizeof(float)), pixels.data());

    int newW = h;
    int newH = w;
    std::vector<float> rotated(newW * newH * components);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int srcIdx = (y * w + x) * components;
            int newX, newY;
            if (direction > 0) {
                newX = h - 1 - y;
                newY = x;
            } else {
                newX = y;
                newY = w - 1 - x;
            }
            int dstIdx = (newY * newW + newX) * components;
            for (int c = 0; c < components; ++c)
                rotated[dstIdx + c] = pixels[srcIdx + c];
        }
    }

    if (newW != w || newH != h) {
        params.gridW = newW;
        params.gridH = newH;
        m_state.init(newW, newH, m_state.format());
    }

    glTextureSubImage2D(m_state.currentTexture(), 0, 0, 0, newW, newH,
                        isRGB ? GL_RGBA : GL_RED, GL_FLOAT, rotated.data());
}

float LeniaEngine::getCellValue(int x, int y) const {
    int w = m_state.width();
    int h = m_state.height();
    if (x < 0 || x >= w || y < 0 || y >= h) return 0.0f;

    bool isRGB = (m_state.format() == GL_RGBA32F);
    int components = isRGB ? 4 : 1;
    int idx = (y * w + x) * components;
    std::vector<float> pixels(w * h * components);

    glGetTextureImage(m_state.currentTexture(), 0, isRGB ? GL_RGBA : GL_RED, GL_FLOAT,
                      static_cast<GLsizei>(pixels.size() * sizeof(float)), pixels.data());

    if (isRGB) {
        return (pixels[idx] + pixels[idx+1] + pixels[idx+2]) / 3.0f;
    }
    return pixels[idx];
}

void LeniaEngine::applyBrush(int cx, int cy, const LeniaParams& params) {
    int w = m_state.width();
    int h = m_state.height();

    auto wrapX = [&](int x) -> int {
        if (params.edgeModeX == 0) {
            x = x % w;
            if (x < 0) x += w;
            return x;
        }
        return (x >= 0 && x < w) ? x : -1;
    };
    auto wrapY = [&](int y) -> int {
        if (params.edgeModeY == 0) {
            y = y % h;
            if (y < 0) y += h;
            return y;
        }
        return (y >= 0 && y < h) ? y : -1;
    };

    int wcx = wrapX(cx);
    int wcy = wrapY(cy);
    if (wcx < 0 || wcy < 0) return;

    bool isRGB = (m_state.format() == GL_RGBA32F);
    int components = isRGB ? 4 : 1;
    std::vector<float> pixels(w * h * components);

    glGetTextureImage(m_state.currentTexture(), 0, isRGB ? GL_RGBA : GL_RED, GL_FLOAT,
                      static_cast<GLsizei>(pixels.size() * sizeof(float)), pixels.data());

    int radius = params.brushSize;
    float strength = params.brushStrength;
    float falloff = params.brushFalloff;
    float value = params.brushValue;
    int shape = params.brushShape;
    int mode = params.brushMode;
    int channel = params.brushChannel;
    int blendMode = params.brushBlendMode;
    int pattern = params.brushPattern;
    float density = params.brushDensity;
    float noiseAmt = params.brushNoiseAmount;
    float jitter = params.brushJitter;
    float rotation = params.brushRotation * 3.14159265f / 180.0f;

    std::mt19937 rng(static_cast<unsigned>(cx * 1000 + cy));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    auto applySymmetry = [&](int px, int py, auto func) {
        func(px, py);
        if (params.brushSymmetryX) func(w - 1 - px, py);
        if (params.brushSymmetryY) func(px, h - 1 - py);
        if (params.brushSymmetryX && params.brushSymmetryY) func(w - 1 - px, h - 1 - py);
        if (params.brushSymmetryRadial) {
            float fcx = w / 2.0f;
            float fcy = h / 2.0f;
            float dx = px - fcx;
            float dy = py - fcy;
            for (int i = 1; i < params.brushRadialCount; ++i) {
                float angle = 2.0f * 3.14159265f * i / params.brushRadialCount;
                float cosA = std::cos(angle);
                float sinA = std::sin(angle);
                int rx = static_cast<int>(fcx + dx * cosA - dy * sinA);
                int ry = static_cast<int>(fcy + dx * sinA + dy * cosA);
                func(rx, ry);
            }
        }
    };

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            float rdx = dx, rdy = dy;
            if (rotation != 0.0f) {
                float cosR = std::cos(-rotation);
                float sinR = std::sin(-rotation);
                rdx = dx * cosR - dy * sinR;
                rdy = dx * sinR + dy * cosR;
            }

            float dist2 = rdx * rdx + rdy * rdy;
            float maxDist2 = static_cast<float>(radius * radius);
            float brushMask = 0.0f;
            float r = std::sqrt(dist2);

            switch (shape) {
                case 0: // Circle
                    brushMask = (dist2 <= maxDist2) ? 1.0f : 0.0f; 
                    break;
                case 1: // Square
                    brushMask = (std::abs(rdx) <= radius && std::abs(rdy) <= radius) ? 1.0f : 0.0f; 
                    break;
                case 2: // Diamond
                    brushMask = (std::abs(rdx) + std::abs(rdy) <= radius) ? 1.0f : 0.0f; 
                    break;
                case 3: { // Ring
                    brushMask = (r >= radius * 0.7f && r <= radius) ? 1.0f : 0.0f;
                } break;
                case 4: { // Star (5pt)
                    float angle = std::atan2(rdy, rdx);
                    float star = radius * (0.4f + 0.6f * std::abs(std::cos(angle * 2.5f)));
                    brushMask = (r <= star) ? 1.0f : 0.0f;
                } break;
                case 5: { // Star (6pt)
                    float angle = std::atan2(rdy, rdx);
                    float star = radius * (0.4f + 0.6f * std::abs(std::cos(angle * 3.0f)));
                    brushMask = (r <= star) ? 1.0f : 0.0f;
                } break;
                case 6: { // Hexagon
                    float angle = std::atan2(std::abs(rdy), std::abs(rdx));
                    float hexR = radius / std::max(std::cos(std::fmod(angle, 1.047197f) - 0.523599f), 0.001f);
                    brushMask = (r <= hexR) ? 1.0f : 0.0f;
                } break;
                case 7: { // Cross (X shape)
                    float crossW = radius * 0.3f;
                    brushMask = ((std::abs(std::abs(rdx) - std::abs(rdy)) <= crossW) && r <= radius) ? 1.0f : 0.0f;
                } break;
                case 8: { // Plus (+)
                    float armW = radius * 0.3f;
                    brushMask = ((std::abs(rdx) <= armW || std::abs(rdy) <= armW) && r <= radius) ? 1.0f : 0.0f;
                } break;
                case 9: { // Gaussian Blob
                    brushMask = std::exp(-3.0f * dist2 / maxDist2);
                } break;
                case 10: { // Noise Disc
                    if (dist2 <= maxDist2) {
                        int px = static_cast<int>(rdx + radius);
                        int py = static_cast<int>(rdy + radius);
                        unsigned int seed = static_cast<unsigned int>(px * 7919 + py * 7907 + cx * 7901 + cy * 7883);
                        seed = (seed ^ 61) ^ (seed >> 16);
                        seed *= 9;
                        seed ^= seed >> 4;
                        seed *= 0x27d4eb2d;
                        seed ^= seed >> 15;
                        brushMask = (seed & 0xFF) / 255.0f;
                    }
                } break;
                case 11: { // Gradient Disc
                    if (dist2 <= maxDist2) {
                        brushMask = 1.0f - r / static_cast<float>(radius);
                    }
                } break;
                default: brushMask = (dist2 <= maxDist2) ? 1.0f : 0.0f; break;
            }

            if (brushMask <= 0.0f) continue;

            float distFrac = std::sqrt(dist2) / std::max(1.0f, static_cast<float>(radius));
            float falloffMult = 1.0f - falloff * distFrac * distFrac;
            falloffMult = std::max(0.0f, falloffMult);

            float patternMult = 1.0f;
            switch (pattern) {
                case 1: patternMult = ((static_cast<int>(rdx + radius) + static_cast<int>(rdy + radius)) % 2 == 0) ? 1.0f : 0.0f; break;
                case 2: patternMult = (static_cast<int>(rdx + rdy + radius * 2) % 4 < 2) ? 1.0f : 0.0f; break;
                case 3: patternMult = (std::sin(rdx * 0.5f) * std::sin(rdy * 0.5f) > 0.0f) ? 1.0f : 0.5f; break;
                case 4: patternMult = 0.5f + 0.5f * std::sin(std::sqrt(dist2) * 0.5f); break;
                case 5: patternMult = dist(rng); break;
                case 6: {
                    float angle = std::atan2(rdy, rdx);
                    int seg = static_cast<int>((angle + 3.14159265f) / (6.28318f / 8.0f)) % 2;
                    patternMult = seg == 0 ? 1.0f : 0.5f;
                } break;
                default: break;
            }

            if (density < 1.0f && dist(rng) > density) continue;

            float noiseMod = 1.0f;
            if (noiseAmt > 0.0f) {
                noiseMod = 1.0f - noiseAmt + noiseAmt * dist(rng) * 2.0f;
            }

            float finalStrength = strength * brushMask * falloffMult * patternMult * noiseMod;
            if (finalStrength <= 0.0f) continue;

            int jdx = dx, jdy = dy;
            if (jitter > 0.0f) {
                jdx += static_cast<int>((dist(rng) - 0.5f) * jitter * 2.0f);
                jdy += static_cast<int>((dist(rng) - 0.5f) * jitter * 2.0f);
            }

            auto paintPixel = [&](int px, int py) {
                int wpx = wrapX(px);
                int wpy = wrapY(py);
                if (wpx < 0 || wpy < 0) return;
                int idx = (wpy * w + wpx) * components;

                auto blend = [&](int comp) {
                    float current = pixels[idx + comp];
                    float target = value;
                    float result = current;

                    switch (mode) {
                        case 0: result = target; break;
                        case 1: result = current + target * finalStrength; break;
                        case 2: result = current - target * finalStrength; break;
                        case 3: result = current * (1.0f - finalStrength) + target * finalStrength; break;
                        case 4: result = std::max(current, target * finalStrength); break;
                        case 5: result = std::min(current, 1.0f - target * finalStrength); break;
                        case 6: result = std::abs(current - target * finalStrength); break;
                        case 7: result = 1.0f - current; break;
                        case 8: result = current * target * finalStrength; break;
                        case 9: result = std::sqrt(current * target * finalStrength); break;
                        case 10: result = 0.0f; break;
                        default: result = current + (target - current) * finalStrength; break;
                    }

                    switch (blendMode) {
                        case 1: result = std::max(current, result); break;
                        case 2: result = std::min(current, result); break;
                        case 3: result = current + result - current * result; break;
                        case 4: result = current * result; break;
                        case 5: result = std::abs(current - result); break;
                        default: break;
                    }

                    pixels[idx + comp] = std::clamp(result, 0.0f, 1.0f);
                };

                if (isRGB) {
                    if (channel == 0 || channel == 4) blend(0);
                    if (channel == 1 || channel == 4) blend(1);
                    if (channel == 2 || channel == 4) blend(2);
                } else {
                    blend(0);
                }
            };

            applySymmetry(cx + jdx, cy + jdy, paintPixel);
        }
    }

    glTextureSubImage2D(m_state.currentTexture(), 0, 0, 0, w, h,
                        isRGB ? GL_RGBA : GL_RED, GL_FLOAT, pixels.data());
}

void LeniaEngine::applyBrushLine(int x0, int y0, int x1, int y1, const LeniaParams& params) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    int x = x0, y = y0;
    float spacing = params.brushSpacing * params.brushSize;
    if (spacing < 1.0f) spacing = 1.0f;
    float traveled = 0.0f;

    while (true) {
        if (traveled >= spacing || (x == x0 && y == y0)) {
            applyBrush(x, y, params);
            traveled = 0.0f;
        }

        if (x == x1 && y == y1) break;

        int e2 = 2 * err;
        float step = 0.0f;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
            step += 1.0f;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
            step += 1.0f;
        }
        traveled += std::sqrt(step);
    }
}

void LeniaEngine::applyBrushCurve(const std::vector<std::pair<int,int>>& points, const LeniaParams& params) {
    if (points.size() < 2) return;

    float spacing = params.brushSpacing * params.brushSize;
    if (spacing < 1.0f) spacing = 1.0f;

    for (size_t i = 1; i < points.size(); ++i) {
        int x0 = points[i-1].first;
        int y0 = points[i-1].second;
        int x1 = points[i].first;
        int y1 = points[i].second;

        float segLen = std::sqrt(static_cast<float>((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0)));
        if (segLen < 0.5f) continue;

        int steps = static_cast<int>(segLen / spacing) + 1;
        for (int s = 0; s <= steps; ++s) {
            float t = static_cast<float>(s) / steps;
            int px = static_cast<int>(x0 + (x1 - x0) * t);
            int py = static_cast<int>(y0 + (y1 - y0) * t);
            applyBrush(px, py, params);
        }
    }
}

void LeniaEngine::applyWall(int cx, int cy, const LeniaParams& params) {
    int w = m_state.width();
    int h = m_state.height();

    if (m_wallTex == 0) {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_wallTex);
        glTextureStorage2D(m_wallTex, 1, GL_RGBA32F, w, h);
        std::vector<float> zeros(w * h * 4, 0.0f);
        glTextureSubImage2D(m_wallTex, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, zeros.data());
    }

    GLint texW, texH;
    glGetTextureLevelParameteriv(m_wallTex, 0, GL_TEXTURE_WIDTH, &texW);
    glGetTextureLevelParameteriv(m_wallTex, 0, GL_TEXTURE_HEIGHT, &texH);
    if (texW != w || texH != h) {
        glDeleteTextures(1, &m_wallTex);
        glCreateTextures(GL_TEXTURE_2D, 1, &m_wallTex);
        glTextureStorage2D(m_wallTex, 1, GL_RGBA32F, w, h);
        std::vector<float> zeros(w * h * 4, 0.0f);
        glTextureSubImage2D(m_wallTex, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, zeros.data());
    }

    auto wrapX = [&](int x) -> int {
        if (params.edgeModeX == 0) {
            x = x % w;
            if (x < 0) x += w;
            return x;
        }
        return (x >= 0 && x < w) ? x : -1;
    };
    auto wrapY = [&](int y) -> int {
        if (params.edgeModeY == 0) {
            y = y % h;
            if (y < 0) y += h;
            return y;
        }
        return (y >= 0 && y < h) ? y : -1;
    };

    int wcx = wrapX(cx);
    int wcy = wrapY(cy);
    if (wcx < 0 || wcy < 0) return;

    std::vector<float> wallPixels(w * h * 4);
    glGetTextureImage(m_wallTex, 0, GL_RGBA, GL_FLOAT,
                      static_cast<GLsizei>(wallPixels.size() * sizeof(float)), wallPixels.data());

    int radius = static_cast<int>(params.wallThickness);
    float thickness = params.wallThickness;
    float falloff = params.wallFalloff;

    std::mt19937 rng(static_cast<unsigned>(cx * 1000 + cy));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            float dist2 = static_cast<float>(dx * dx + dy * dy);
            float maxDist2 = thickness * thickness;
            float wallMask = 0.0f;

            switch (params.wallShape) {
                case 0: wallMask = (dist2 <= maxDist2) ? 1.0f : 0.0f; break;
                case 1: wallMask = (std::abs(dx) <= thickness && std::abs(dy) <= thickness) ? 1.0f : 0.0f; break;
                case 2: wallMask = (std::abs(dx) + std::abs(dy) <= thickness) ? 1.0f : 0.0f; break;
                default: wallMask = (dist2 <= maxDist2) ? 1.0f : 0.0f; break;
            }

            if (wallMask <= 0.0f) continue;

            float distFrac = std::sqrt(dist2) / std::max(1.0f, thickness);
            float falloffMult = 1.0f - falloff * distFrac * distFrac;
            falloffMult = std::max(0.0f, falloffMult);

            float noiseMod = 1.0f;
            if (params.wallNoiseAmount > 0.0f) {
                noiseMod = 1.0f - params.wallNoiseAmount + params.wallNoiseAmount * dist(rng) * 2.0f;
            }

            float strength = wallMask * falloffMult * noiseMod;
            if (strength <= 0.0f) continue;

            int px = wrapX(cx + dx);
            int py = wrapY(cy + dy);
            if (px < 0 || py < 0) continue;

            int idx = (py * w + px) * 4;

            float newR = params.wallR;
            float newG = params.wallG;
            float newB = params.wallB;
            float newA = params.wallA * strength;

            switch (params.wallBlendMode) {
                case 0:
                    wallPixels[idx + 0] = newR;
                    wallPixels[idx + 1] = newG;
                    wallPixels[idx + 2] = newB;
                    wallPixels[idx + 3] = std::max(wallPixels[idx + 3], newA);
                    break;
                case 1:
                    wallPixels[idx + 0] = std::max(wallPixels[idx + 0], newR);
                    wallPixels[idx + 1] = std::max(wallPixels[idx + 1], newG);
                    wallPixels[idx + 2] = std::max(wallPixels[idx + 2], newB);
                    wallPixels[idx + 3] = std::max(wallPixels[idx + 3], newA);
                    break;
                case 2:
                    if (newA > wallPixels[idx + 3]) {
                        wallPixels[idx + 0] = newR;
                        wallPixels[idx + 1] = newG;
                        wallPixels[idx + 2] = newB;
                        wallPixels[idx + 3] = newA;
                    }
                    break;
                case 3: {
                    float a = wallPixels[idx + 3];
                    float blend = newA * strength;
                    wallPixels[idx + 0] = wallPixels[idx + 0] * (1.0f - blend) + newR * blend;
                    wallPixels[idx + 1] = wallPixels[idx + 1] * (1.0f - blend) + newG * blend;
                    wallPixels[idx + 2] = wallPixels[idx + 2] * (1.0f - blend) + newB * blend;
                    wallPixels[idx + 3] = std::max(a, newA);
                } break;
                case 4:
                    wallPixels[idx + 0] = 0.0f;
                    wallPixels[idx + 1] = 0.0f;
                    wallPixels[idx + 2] = 0.0f;
                    wallPixels[idx + 3] = 0.0f;
                    break;
                default:
                    wallPixels[idx + 0] = newR;
                    wallPixels[idx + 1] = newG;
                    wallPixels[idx + 2] = newB;
                    wallPixels[idx + 3] = newA;
                    break;
            }
        }
    }

    glTextureSubImage2D(m_wallTex, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, wallPixels.data());

    if (params.wallSolid && params.wallValue != 0.0f) {
        bool isRGB = (m_state.format() == GL_RGBA32F);
        int components = isRGB ? 4 : 1;
        std::vector<float> statePixels(w * h * components);
        glGetTextureImage(m_state.currentTexture(), 0, isRGB ? GL_RGBA : GL_RED, GL_FLOAT,
                          static_cast<GLsizei>(statePixels.size() * sizeof(float)), statePixels.data());

        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                float dist2 = static_cast<float>(dx * dx + dy * dy);
                float maxDist2 = thickness * thickness;
                if (dist2 > maxDist2) continue;

                int px = wrapX(cx + dx);
                int py = wrapY(cy + dy);
                if (px < 0 || py < 0) continue;

                int wallIdx = (py * w + px) * 4;
                if (wallPixels[wallIdx + 3] < 0.01f) continue;

                int stateIdx = (py * w + px) * components;
                float wallEffect = params.wallValue < 0.0f ? 0.0f : params.wallValue;

                if (isRGB) {
                    if (params.wallAffectsAllChannels || params.wallChannel == 0)
                        statePixels[stateIdx + 0] = wallEffect;
                    if (params.wallAffectsAllChannels || params.wallChannel == 1)
                        statePixels[stateIdx + 1] = wallEffect;
                    if (params.wallAffectsAllChannels || params.wallChannel == 2)
                        statePixels[stateIdx + 2] = wallEffect;
                } else {
                    statePixels[stateIdx] = wallEffect;
                }
            }
        }

        glTextureSubImage2D(m_state.currentTexture(), 0, 0, 0, w, h,
                            isRGB ? GL_RGBA : GL_RED, GL_FLOAT, statePixels.data());
    }
}

void LeniaEngine::applyWallLine(int x0, int y0, int x1, int y1, const LeniaParams& params) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    int x = x0, y = y0;
    float spacing = std::max(1.0f, params.wallThickness * 0.5f);
    float traveled = 0.0f;

    while (true) {
        if (traveled >= spacing || (x == x0 && y == y0)) {
            applyWall(x, y, params);
            traveled = 0.0f;
        }

        if (x == x1 && y == y1) break;

        int e2 = 2 * err;
        float step = 0.0f;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
            step += 1.0f;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
            step += 1.0f;
        }
        traveled += std::sqrt(step);
    }
}

void LeniaEngine::applyWallCurve(const std::vector<std::pair<int,int>>& points, const LeniaParams& params) {
    if (points.size() < 2) return;

    float spacing = std::max(1.0f, params.wallThickness * 0.5f);

    for (size_t i = 1; i < points.size(); ++i) {
        int x0 = points[i-1].first;
        int y0 = points[i-1].second;
        int x1 = points[i].first;
        int y1 = points[i].second;

        float segLen = std::sqrt(static_cast<float>((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0)));
        if (segLen < 0.5f) continue;

        int steps = static_cast<int>(segLen / spacing) + 1;
        for (int s = 0; s <= steps; ++s) {
            float t = static_cast<float>(s) / steps;
            int px = static_cast<int>(x0 + (x1 - x0) * t);
            int py = static_cast<int>(y0 + (y1 - y0) * t);
            applyWall(px, py, params);
        }
    }
}

void LeniaEngine::clearWalls() {
    if (m_wallTex != 0) {
        GLint w, h;
        glGetTextureLevelParameteriv(m_wallTex, 0, GL_TEXTURE_WIDTH, &w);
        glGetTextureLevelParameteriv(m_wallTex, 0, GL_TEXTURE_HEIGHT, &h);
        std::vector<float> zeros(w * h * 4, 0.0f);
        glTextureSubImage2D(m_wallTex, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, zeros.data());
    }
}

void LeniaEngine::enforceObstacles(const LeniaParams& params) {
    if (m_wallTex == 0) return;
    
    int w = m_state.width();
    int h = m_state.height();
    
    GLint texW, texH;
    glGetTextureLevelParameteriv(m_wallTex, 0, GL_TEXTURE_WIDTH, &texW);
    glGetTextureLevelParameteriv(m_wallTex, 0, GL_TEXTURE_HEIGHT, &texH);
    if (texW != w || texH != h) return;
    
    std::vector<float> wallPixels(w * h * 4);
    glGetTextureImage(m_wallTex, 0, GL_RGBA, GL_FLOAT,
                      static_cast<GLsizei>(wallPixels.size() * sizeof(float)), wallPixels.data());
    
    bool hasWalls = false;
    for (int i = 0; i < w * h; ++i) {
        if (wallPixels[i * 4 + 3] > 0.01f) {
            hasWalls = true;
            break;
        }
    }
    if (!hasWalls) return;
    
    bool isRGB = (m_state.format() == GL_RGBA32F);
    int components = isRGB ? 4 : 1;
    std::vector<float> statePixels(w * h * components);
    glGetTextureImage(m_state.currentTexture(), 0, isRGB ? GL_RGBA : GL_RED, GL_FLOAT,
                      static_cast<GLsizei>(statePixels.size() * sizeof(float)), statePixels.data());
    
    bool modified = false;
    float obstacleValue = params.wallValue;
    
    for (int i = 0; i < w * h; ++i) {
        float wallAlpha = wallPixels[i * 4 + 3];
        if (wallAlpha < 0.01f) continue;
        
        int stateIdx = i * components;
        
        if (isRGB) {
            if (params.wallAffectsAllChannels || params.wallAffectsCh0) {
                statePixels[stateIdx + 0] = obstacleValue;
            }
            if ((params.wallAffectsAllChannels || params.wallAffectsCh1) && params.numChannels >= 2) {
                statePixels[stateIdx + 1] = obstacleValue;
            }
            if ((params.wallAffectsAllChannels || params.wallAffectsCh2) && params.numChannels >= 3) {
                statePixels[stateIdx + 2] = obstacleValue;
            }
        } else {
            statePixels[stateIdx] = obstacleValue;
        }
        modified = true;
    }
    
    if (modified) {
        glTextureSubImage2D(m_state.currentTexture(), 0, 0, 0, w, h,
                            isRGB ? GL_RGBA : GL_RED, GL_FLOAT, statePixels.data());
    }
}

}
