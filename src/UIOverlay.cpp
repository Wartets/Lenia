#include "UIOverlay.hpp"
#include "LeniaEngine.hpp"
#include "Presets.hpp"
#include "AnalysisManager.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cfloat>
#include <fstream>
#include <functional>

namespace lenia {

static void pushSectionColor(int sectionIndex) {
    const ImVec4 tints[] = {
        {0.30f, 0.50f, 0.90f, 0.12f},
        {0.20f, 0.75f, 0.45f, 0.10f},
        {0.85f, 0.55f, 0.15f, 0.10f},
        {0.70f, 0.25f, 0.70f, 0.10f},
        {0.90f, 0.35f, 0.35f, 0.10f},
        {0.25f, 0.70f, 0.70f, 0.10f},
        {0.60f, 0.60f, 0.25f, 0.10f},
        {0.50f, 0.40f, 0.80f, 0.10f},
        {0.40f, 0.60f, 0.40f, 0.10f},
        {0.65f, 0.45f, 0.55f, 0.10f},
    };
    int idx = sectionIndex % 10;
    ImVec4 base = ImGui::GetStyleColorVec4(ImGuiCol_Header);
    ImVec4 hov  = ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered);
    ImVec4 act  = ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive);
    ImGui::PushStyleColor(ImGuiCol_Header,
        ImVec4(base.x + tints[idx].x * tints[idx].w, base.y + tints[idx].y * tints[idx].w,
               base.z + tints[idx].z * tints[idx].w, base.w));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
        ImVec4(hov.x + tints[idx].x * tints[idx].w * 1.5f, hov.y + tints[idx].y * tints[idx].w * 1.5f,
               hov.z + tints[idx].z * tints[idx].w * 1.5f, hov.w));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,
        ImVec4(act.x + tints[idx].x * tints[idx].w * 2.0f, act.y + tints[idx].y * tints[idx].w * 2.0f,
               act.z + tints[idx].z * tints[idx].w * 2.0f, act.w));
}

static void popSectionColor() {
    ImGui::PopStyleColor(3);
}

static ImVec2 s_lastSliderMin, s_lastSliderMax;

static void drawSliderMarkers(float sliderMin, float sliderMax,
                               const float* redMarkers, int numRed,
                               const float* greenMarkers, int numGreen) {
    ImVec2 itemMin = s_lastSliderMin;
    ImVec2 itemMax = s_lastSliderMax;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float grabPad = ImGui::GetStyle().FramePadding.x;
    float barLeft = itemMin.x + grabPad;
    float barRight = itemMax.x - grabPad;
    float barRange = barRight - barLeft;
    float range = sliderMax - sliderMin;
    if (range < 1e-9f) return;

    for (int i = 0; i < numRed; ++i) {
        float t = (redMarkers[i] - sliderMin) / range;
        if (t < 0.0f || t > 1.0f) continue;
        float x = barLeft + t * barRange;
        dl->AddLine(ImVec2(x, itemMin.y + 2.0f), ImVec2(x, itemMax.y - 2.0f),
                    IM_COL32(220, 50, 50, 200), 2.0f);
    }

    for (int i = 0; i < numGreen; ++i) {
        float t = (greenMarkers[i] - sliderMin) / range;
        if (t < 0.0f || t > 1.0f) continue;
        float x = barLeft + t * barRange;
        dl->AddLine(ImVec2(x, itemMin.y + 2.0f), ImVec2(x, itemMax.y - 2.0f),
                    IM_COL32(50, 200, 50, 180), 1.5f);
    }
}

static void drawSliderMarkersInt(int sliderMin, int sliderMax,
                                  const int* redMarkers, int numRed,
                                  const int* greenMarkers, int numGreen) {
    ImVec2 itemMin = s_lastSliderMin;
    ImVec2 itemMax = s_lastSliderMax;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float grabPad = ImGui::GetStyle().FramePadding.x;
    float barLeft = itemMin.x + grabPad;
    float barRight = itemMax.x - grabPad;
    float barRange = barRight - barLeft;
    float range = static_cast<float>(sliderMax - sliderMin);
    if (range < 1e-9f) return;

    for (int i = 0; i < numRed; ++i) {
        float t = static_cast<float>(redMarkers[i] - sliderMin) / range;
        if (t < 0.0f || t > 1.0f) continue;
        float x = barLeft + t * barRange;
        dl->AddLine(ImVec2(x, itemMin.y + 2.0f), ImVec2(x, itemMax.y - 2.0f),
                    IM_COL32(220, 50, 50, 200), 2.0f);
    }

    for (int i = 0; i < numGreen; ++i) {
        float t = static_cast<float>(greenMarkers[i] - sliderMin) / range;
        if (t < 0.0f || t > 1.0f) continue;
        float x = barLeft + t * barRange;
        dl->AddLine(ImVec2(x, itemMin.y + 2.0f), ImVec2(x, itemMax.y - 2.0f),
                    IM_COL32(50, 200, 50, 180), 1.5f);
    }
}

static void snapFloat(float& val, float sliderMin, float sliderMax,
                      const float* markers, int numMarkers, float snapThreshold = 0.008f) {
    float range = sliderMax - sliderMin;
    float threshold = range * snapThreshold;
    for (int i = 0; i < numMarkers; ++i) {
        if (std::abs(val - markers[i]) < threshold) {
            val = markers[i];
            return;
        }
    }
}

static void snapInt(int& val, int sliderMin, int sliderMax,
                    const int* markers, int numMarkers, float snapThreshold = 0.015f) {
    float range = static_cast<float>(sliderMax - sliderMin);
    float threshold = range * snapThreshold;
    for (int i = 0; i < numMarkers; ++i) {
        if (std::abs(static_cast<float>(val - markers[i])) < threshold) {
            val = markers[i];
            return;
        }
    }
}

static bool SliderFloatWithInput(const char* label, float* v, float vmin, float vmax,
                                 const char* fmt = "%.4f", float inputWidth = 70.0f) {
    float totalW = ImGui::GetContentRegionAvail().x;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float sliderW = totalW - inputWidth - spacing;
    if (sliderW < 50.0f) sliderW = 50.0f;
    if (sliderW > 300.0f) sliderW = 300.0f;

    ImGui::PushItemWidth(sliderW);
    bool changed = ImGui::SliderFloat(label, v, vmin, vmax, fmt);
    ImGui::PopItemWidth();
    s_lastSliderMin = ImGui::GetItemRectMin();
    s_lastSliderMax = ImGui::GetItemRectMax();

    ImGui::SameLine();
    char inputLabel[128];
    std::snprintf(inputLabel, sizeof(inputLabel), "##inp_%s", label);
    ImGui::PushItemWidth(inputWidth);
    if (ImGui::InputFloat(inputLabel, v, 0.0f, 0.0f, fmt)) {
        *v = std::clamp(*v, vmin, vmax);
        changed = true;
    }
    ImGui::PopItemWidth();
    return changed;
}

static bool SliderIntWithInput(const char* label, int* v, int vmin, int vmax,
                               float inputWidth = 55.0f) {
    float totalW = ImGui::GetContentRegionAvail().x;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float sliderW = totalW - inputWidth - spacing;
    if (sliderW < 50.0f) sliderW = 50.0f;
    if (sliderW > 300.0f) sliderW = 300.0f;

    ImGui::PushItemWidth(sliderW);
    bool changed = ImGui::SliderInt(label, v, vmin, vmax);
    ImGui::PopItemWidth();
    s_lastSliderMin = ImGui::GetItemRectMin();
    s_lastSliderMax = ImGui::GetItemRectMax();

    ImGui::SameLine();
    char inputLabel[128];
    std::snprintf(inputLabel, sizeof(inputLabel), "##inp_%s", label);
    ImGui::PushItemWidth(inputWidth);
    if (ImGui::InputInt(inputLabel, v, 0, 0)) {
        *v = std::clamp(*v, vmin, vmax);
        changed = true;
    }
    ImGui::PopItemWidth();
    return changed;
}

static void Tooltip(const char* text) {
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(300.0f);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool UIOverlay::sectionHeader(const char* label, int sectionIdx, bool defaultOpen) {
    ImGui::PushID(sectionIdx);
    
    bool isDetached = m_sectionDetached[sectionIdx];
    
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowOverlap;
    if (defaultOpen) flags |= ImGuiTreeNodeFlags_DefaultOpen;
    
    bool open = ImGui::CollapsingHeader(label, flags);
    
    ImGui::PopID();
    
    return open && !isDetached;
}

void UIOverlay::triggerPauseOverlay(bool isPaused) {
    m_pauseOverlayAlpha = 1.0f;
    m_pauseOverlayPlaying = !isPaused;
}

void UIOverlay::updatePauseOverlay(float deltaTime) {
    if (m_pauseOverlayAlpha > 0.0f) {
        m_pauseOverlayAlpha -= deltaTime * 1.2f;
        if (m_pauseOverlayAlpha < 0.0f) m_pauseOverlayAlpha = 0.0f;
    }
}

void UIOverlay::renderPauseOverlay(int windowW, int windowH) {
    if (m_pauseOverlayAlpha <= 0.0f) return;
    
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    
    float iconSize = 80.0f;
    float margin = 30.0f;
    float centerX = windowW - margin - iconSize * 0.5f;
    float centerY = margin + iconSize * 0.5f;
    
    ImU32 bgColor = IM_COL32(0, 0, 0, static_cast<int>(100 * m_pauseOverlayAlpha));
    ImU32 iconColor = IM_COL32(255, 255, 255, static_cast<int>(220 * m_pauseOverlayAlpha));
    
    dl->AddCircleFilled(ImVec2(centerX, centerY), iconSize * 0.55f, bgColor, 32);
    
    if (m_pauseOverlayPlaying) {
        ImVec2 p1(centerX - iconSize * 0.2f, centerY - iconSize * 0.3f);
        ImVec2 p2(centerX - iconSize * 0.2f, centerY + iconSize * 0.3f);
        ImVec2 p3(centerX + iconSize * 0.35f, centerY);
        dl->AddTriangleFilled(p1, p2, p3, iconColor);
    } else {
        float barW = iconSize * 0.12f;
        float barH = iconSize * 0.5f;
        float gap = iconSize * 0.12f;
        
        ImVec2 bar1Min(centerX - gap - barW, centerY - barH * 0.5f);
        ImVec2 bar1Max(centerX - gap, centerY + barH * 0.5f);
        ImVec2 bar2Min(centerX + gap, centerY - barH * 0.5f);
        ImVec2 bar2Max(centerX + gap + barW, centerY + barH * 0.5f);
        
        dl->AddRectFilled(bar1Min, bar1Max, iconColor, 3.0f);
        dl->AddRectFilled(bar2Min, bar2Max, iconColor, 3.0f);
    }
}

UIOverlay::~UIOverlay() {
    shutdown();
}

bool UIOverlay::init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding    = 4.0f;
    style.GrabRounding     = 3.0f;
    style.WindowRounding   = 6.0f;
    style.ScrollbarSize    = 14.0f;
    style.TabRounding      = 4.0f;
    style.IndentSpacing    = 16.0f;
    style.Colors[ImGuiCol_WindowBg]       = ImVec4(0.08f, 0.08f, 0.12f, 0.94f);
    style.Colors[ImGuiCol_TitleBg]        = ImVec4(0.06f, 0.06f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]  = ImVec4(0.12f, 0.12f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_FrameBg]        = ImVec4(0.14f, 0.14f, 0.20f, 0.80f);
    style.Colors[ImGuiCol_SliderGrab]     = ImVec4(0.40f, 0.55f, 0.80f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.65f, 0.95f, 1.00f);
    style.Colors[ImGuiCol_Button]         = ImVec4(0.18f, 0.22f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered]  = ImVec4(0.28f, 0.35f, 0.55f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]   = ImVec4(0.35f, 0.45f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_Separator]      = ImVec4(0.30f, 0.30f, 0.45f, 0.50f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    m_initialized = true;
    return true;
}

void UIOverlay::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIOverlay::drawGrowthPlot(LeniaParams& params) {
    constexpr int N = 200;
    float plotData[N];
    float xMin = 0.0f, xMax = 1.0f;

    if (params.growthType == 2) {
        xMin = 0.0f;
        xMax = 9.0f;
    }

    for (int i = 0; i < N; ++i) {
        float x = xMin + (xMax - xMin) * static_cast<float>(i) / static_cast<float>(N - 1);
        if (params.growthType == 0) {
            float d = (x - params.mu) / std::max(params.sigma, 0.001f);
            plotData[i] = 2.0f * std::exp(-0.5f * d * d) - 1.0f;
        } else if (params.growthType == 1) {
            float lo = params.mu - params.sigma;
            float hi = params.mu + params.sigma;
            plotData[i] = (x >= lo && x <= hi) ? 1.0f : -1.0f;
        } else if (params.growthType == 2) {
            float neighbors = x;
            bool alive = (neighbors >= 4.5f);
            if (!alive) {
                plotData[i] = (neighbors >= 2.5f && neighbors <= 3.5f) ? 1.0f : -1.0f;
            } else {
                plotData[i] = (neighbors >= 1.5f && neighbors <= 3.5f) ? 0.5f : -1.0f;
            }
        } else if (params.growthType == 3) {
            float bLo = params.mu - params.sigma * 3.0f;
            float bHi = params.mu - params.sigma;
            float dLo = params.mu + params.sigma;
            float dHi = params.mu + params.sigma * 3.0f;
            if (x >= bLo && x <= bHi)      plotData[i] = 0.8f;
            else if (x >= dLo && x <= dHi) plotData[i] = -0.8f;
            else                            plotData[i] = 0.0f;
        } else if (params.growthType == 4) {
            float d = (x - params.mu) / std::max(params.sigma, 0.001f);
            float v = 1.0f - d * d;
            plotData[i] = v > 0.0f ? v * v - 0.5f : -0.5f;
        } else if (params.growthType == 5) {
            float d = std::abs(x - params.mu) / std::max(params.sigma, 0.001f);
            plotData[i] = 2.0f * std::exp(-d) - 1.0f;
        } else if (params.growthType == 6) {
            float d1 = (x - params.mu * 0.7f) / std::max(params.sigma, 0.001f);
            float d2 = (x - params.mu * 1.3f) / std::max(params.sigma, 0.001f);
            float g1 = std::exp(-0.5f * d1 * d1);
            float g2 = std::exp(-0.5f * d2 * d2);
            plotData[i] = 2.0f * std::max(g1, g2) - 1.0f;
        } else if (params.growthType == 7) {
            float d = (x - params.mu) / std::max(params.sigma, 0.001f);
            float target = std::exp(-0.5f * d * d);
            plotData[i] = target - x;
        } else if (params.growthType == 8) {
            float d = (x - params.mu) / std::max(params.sigma, 0.001f);
            float g = 2.0f * std::exp(-0.5f * d * d) - 1.0f;
            float raw = x + params.dt * g;
            plotData[i] = 1.0f / (1.0f + std::exp(-4.0f * (raw - 0.5f)));
            plotData[i] = plotData[i] - x;
        } else if (params.growthType == 9) {
            float neighbors = x;
            float b1 = params.mu - params.sigma * 3.0f;
            float b2 = params.mu + params.sigma * 3.0f;
            float s1 = params.mu - params.sigma;
            float s2 = params.mu + params.sigma;
            bool alive = (neighbors >= 4.5f);
            if (!alive)
                plotData[i] = (neighbors >= b1 && neighbors <= b2) ? 1.0f : -1.0f;
            else
                plotData[i] = (neighbors >= s1 && neighbors <= s2) ? 0.5f : -1.0f;
        } else {
            plotData[i] = 0.0f;
        }
    }

    float plotH = 80.0f;
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSz(ImGui::GetContentRegionAvail().x, plotH);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSz.x, canvasPos.y + canvasSz.y),
                      IM_COL32(15, 15, 25, 200), 4.0f);

    float yZero = canvasPos.y + canvasSz.y * 0.5f;
    dl->AddLine(ImVec2(canvasPos.x, yZero),
                ImVec2(canvasPos.x + canvasSz.x, yZero),
                IM_COL32(80, 80, 100, 120), 1.0f);

    float muPx = canvasPos.x + (params.mu - xMin) / (xMax - xMin) * canvasSz.x;
    if (muPx >= canvasPos.x && muPx <= canvasPos.x + canvasSz.x) {
        dl->AddLine(ImVec2(muPx, canvasPos.y), ImVec2(muPx, canvasPos.y + canvasSz.y),
                    IM_COL32(255, 200, 80, 100), 1.0f);
    }

    ImU32 lineCol = IM_COL32(100, 200, 255, 230);
    ImU32 fillCol = IM_COL32(60, 140, 200, 50);

    for (int i = 0; i < N - 1; ++i) {
        float x0 = canvasPos.x + canvasSz.x * static_cast<float>(i) / static_cast<float>(N - 1);
        float x1 = canvasPos.x + canvasSz.x * static_cast<float>(i + 1) / static_cast<float>(N - 1);
        float y0 = canvasPos.y + canvasSz.y * (0.5f - plotData[i] * 0.45f);
        float y1 = canvasPos.y + canvasSz.y * (0.5f - plotData[i + 1] * 0.45f);

        dl->AddTriangleFilled(ImVec2(x0, yZero), ImVec2(x0, y0), ImVec2(x1, y1), fillCol);
        dl->AddTriangleFilled(ImVec2(x0, yZero), ImVec2(x1, y1), ImVec2(x1, yZero), fillCol);
        dl->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), lineCol, 2.0f);
    }

    dl->AddRect(canvasPos, ImVec2(canvasPos.x + canvasSz.x, canvasPos.y + canvasSz.y),
                IM_COL32(60, 60, 80, 180), 4.0f);

    ImGui::Dummy(canvasSz);
    if (params.growthType == 2) {
        ImGui::TextDisabled("Game of Life B3/S23 (x-axis: neighbor count 0-8)");
    } else if (params.growthType == 7) {
        ImGui::TextDisabled("Asymptotic target(U)-A  mu=%.4f sigma=%.4f", params.mu, params.sigma);
    } else if (params.growthType == 8) {
        ImGui::TextDisabled("SoftClip sigmoid  mu=%.4f sigma=%.4f", params.mu, params.sigma);
    } else if (params.growthType == 9) {
        ImGui::TextDisabled("Larger-than-Life  mu=%.4f sigma=%.4f", params.mu, params.sigma);
    } else {
        ImGui::TextDisabled("Growth(U)  mu=%.4f  sigma=%.4f", params.mu, params.sigma);
    }
}

void UIOverlay::drawKernelCrossSection(GLuint kernelTex, int kernelDiam) {
    if (kernelTex == 0 || kernelDiam <= 0) return;

    std::vector<float> pixels(kernelDiam * kernelDiam);
    glGetTextureImage(kernelTex, 0, GL_RED, GL_FLOAT,
                      static_cast<GLsizei>(pixels.size() * sizeof(float)), pixels.data());

    int mid = kernelDiam / 2;
    float minVal = 0.0f;
    float maxVal = 0.001f;
    for (int i = 0; i < kernelDiam; ++i) {
        float v = pixels[mid * kernelDiam + i];
        if (v > maxVal) maxVal = v;
        if (v < minVal) minVal = v;
    }

    float valRange = maxVal - minVal;
    if (valRange < 0.001f) valRange = 0.001f;

    int plotRes = std::max(kernelDiam, 128);

    float plotH = 70.0f;
    float pad = 2.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 sz(ImGui::GetContentRegionAvail().x, plotH);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    float plotTop = pos.y + pad;
    float plotBot = pos.y + sz.y - pad;
    float plotArea = plotBot - plotTop;

    float zeroY = plotTop + (maxVal / valRange) * plotArea;

    dl->AddRectFilled(pos, ImVec2(pos.x + sz.x, pos.y + sz.y),
                      IM_COL32(15, 15, 25, 200), 4.0f);

    if (minVal < -0.0001f) {
        dl->AddLine(ImVec2(pos.x, zeroY), ImVec2(pos.x + sz.x, zeroY),
                    IM_COL32(80, 80, 100, 160), 1.0f);
    }

    ImU32 barColPos = IM_COL32(120, 200, 120, 180);
    ImU32 barColNeg = IM_COL32(200, 100, 100, 180);
    ImU32 lineCol   = IM_COL32(180, 255, 180, 230);

    auto sampleRow = [&](float fIdx) -> float {
        int i0 = static_cast<int>(fIdx);
        int i1 = i0 + 1;
        if (i0 < 0) i0 = 0;
        if (i1 >= kernelDiam) i1 = kernelDiam - 1;
        if (i0 >= kernelDiam) i0 = kernelDiam - 1;
        float frac = fIdx - static_cast<float>(i0);
        float v0 = pixels[mid * kernelDiam + i0];
        float v1 = pixels[mid * kernelDiam + i1];
        return v0 + frac * (v1 - v0);
    };

    std::vector<float> sampledVals(plotRes);
    for (int i = 0; i < plotRes; ++i) {
        float fIdx = static_cast<float>(i) / static_cast<float>(plotRes) * static_cast<float>(kernelDiam);
        sampledVals[i] = sampleRow(fIdx);
    }

    float barW = sz.x / static_cast<float>(plotRes);
    for (int i = 0; i < plotRes; ++i) {
        float v = sampledVals[i];
        float x = pos.x + barW * i;
        float valY = plotTop + ((maxVal - v) / valRange) * plotArea;
        if (v > 0.0f) {
            float top = std::max(valY, plotTop);
            float bot = std::min(zeroY, plotBot);
            if (bot - top > 0.5f)
                dl->AddRectFilled(ImVec2(x, top), ImVec2(x + barW, bot), barColPos, 0.0f);
        } else if (v < 0.0f) {
            float top = std::max(zeroY, plotTop);
            float bot = std::min(valY, plotBot);
            if (bot - top > 0.5f)
                dl->AddRectFilled(ImVec2(x, top), ImVec2(x + barW, bot), barColNeg, 0.0f);
        }
    }

    for (int i = 0; i < plotRes - 1; ++i) {
        float v0 = sampledVals[i];
        float v1 = sampledVals[i + 1];
        float x0 = pos.x + barW * (i + 0.5f);
        float x1 = pos.x + barW * (i + 1.5f);
        float y0 = plotTop + ((maxVal - v0) / valRange) * plotArea;
        float y1 = plotTop + ((maxVal - v1) / valRange) * plotArea;
        y0 = std::clamp(y0, plotTop, plotBot);
        y1 = std::clamp(y1, plotTop, plotBot);
        dl->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), lineCol, 1.5f);
    }

    dl->AddRect(pos, ImVec2(pos.x + sz.x, pos.y + sz.y),
                IM_COL32(60, 60, 80, 180), 4.0f);

    ImGui::Dummy(sz);
    ImGui::TextDisabled("Kernel Cross-Section (%dx%d)", kernelDiam, kernelDiam);
}

static void viridisColor(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    float c0r=0.2777f,c0g=0.0054f,c0b=0.3340f;
    float c1r=0.1050f,c1g=0.4114f,c1b=0.5036f;
    float c2r=0.1270f,c2g=0.5660f,c2b=0.5506f;
    float c3r=0.2302f,c3g=0.6860f,c3b=0.5410f;
    float c4r=0.4775f,c4g=0.8212f,c4b=0.3180f;
    float c5r=0.9930f,c5g=0.9062f,c5b=0.1439f;
    float lr,lg,lb,hr,hg,hb; float f;
    if (t<0.2f){f=t/0.2f;lr=c0r;lg=c0g;lb=c0b;hr=c1r;hg=c1g;hb=c1b;}
    else if(t<0.4f){f=(t-0.2f)/0.2f;lr=c1r;lg=c1g;lb=c1b;hr=c2r;hg=c2g;hb=c2b;}
    else if(t<0.6f){f=(t-0.4f)/0.2f;lr=c2r;lg=c2g;lb=c2b;hr=c3r;hg=c3g;hb=c3b;}
    else if(t<0.8f){f=(t-0.6f)/0.2f;lr=c3r;lg=c3g;lb=c3b;hr=c4r;hg=c4g;hb=c4b;}
    else{f=(t-0.8f)/0.2f;lr=c4r;lg=c4g;lb=c4b;hr=c5r;hg=c5g;hb=c5b;}
    r=lr+f*(hr-lr); g=lg+f*(hg-lg); b=lb+f*(hb-lb);
}

static void magmaColor(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    float cs[][3]={{0.0015f,0.0005f,0.0139f},{0.2776f,0.0510f,0.3755f},{0.5756f,0.1476f,0.4526f},{0.8584f,0.3167f,0.3378f},{0.9824f,0.6004f,0.3595f},{0.9870f,0.9914f,0.7497f}};
    int seg=std::min(static_cast<int>(t*5.0f),4); float f=t*5.0f-seg;
    r=cs[seg][0]+f*(cs[seg+1][0]-cs[seg][0]); g=cs[seg][1]+f*(cs[seg+1][1]-cs[seg][1]); b=cs[seg][2]+f*(cs[seg+1][2]-cs[seg][2]);
}

static void infernoColor(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    float cs[][3]={{0.0015f,0.0005f,0.0139f},{0.2581f,0.0388f,0.4065f},{0.5783f,0.1481f,0.4040f},{0.8490f,0.2897f,0.2001f},{0.9882f,0.5766f,0.0399f},{0.9882f,0.9985f,0.6449f}};
    int seg=std::min(static_cast<int>(t*5.0f),4); float f=t*5.0f-seg;
    r=cs[seg][0]+f*(cs[seg+1][0]-cs[seg][0]); g=cs[seg][1]+f*(cs[seg+1][1]-cs[seg][1]); b=cs[seg][2]+f*(cs[seg+1][2]-cs[seg][2]);
}

static void plasmaColor(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    float cs[][3]={{0.0504f,0.0298f,0.5280f},{0.4177f,0.0056f,0.6582f},{0.6942f,0.1651f,0.5364f},{0.8810f,0.3924f,0.3267f},{0.9882f,0.6524f,0.0399f},{0.9400f,0.9752f,0.1313f}};
    int seg=std::min(static_cast<int>(t*5.0f),4); float f=t*5.0f-seg;
    r=cs[seg][0]+f*(cs[seg+1][0]-cs[seg][0]); g=cs[seg][1]+f*(cs[seg+1][1]-cs[seg][1]); b=cs[seg][2]+f*(cs[seg+1][2]-cs[seg][2]);
}

static void jetColor(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    r=std::clamp(1.5f-std::abs(t-0.75f)*4.0f,0.0f,1.0f);
    g=std::clamp(1.5f-std::abs(t-0.50f)*4.0f,0.0f,1.0f);
    b=std::clamp(1.5f-std::abs(t-0.25f)*4.0f,0.0f,1.0f);
}

static void leniaColor(float t, float& r, float& g, float& b) {
    t = std::clamp(t, 0.0f, 1.0f);
    struct CS { float t,r,g,b; };
    CS stops[]={
        {0.0f,0.0f,0.0f,0.05f},{0.15f,0.05f,0.02f,0.2f},{0.3f,0.1f,0.05f,0.4f},
        {0.4f,0.3f,0.05f,0.35f},{0.5f,0.6f,0.15f,0.1f},{0.6f,0.9f,0.4f,0.05f},
        {0.7f,1.0f,0.7f,0.1f},{0.8f,1.0f,0.9f,0.3f},{0.9f,1.0f,1.0f,0.6f},{1.0f,1.0f,1.0f,1.0f}
    };
    int idx=0;
    for(int s=0;s<9;++s){if(t>=stops[s].t&&t<=stops[s+1].t){idx=s;break;}}
    float range=stops[idx+1].t-stops[idx].t;
    float f=(range>0)?(t-stops[idx].t)/range:0.0f;
    f=f*f*(3.0f-2.0f*f);
    r=stops[idx].r+f*(stops[idx+1].r-stops[idx].r);
    g=stops[idx].g+f*(stops[idx+1].g-stops[idx].g);
    b=stops[idx].b+f*(stops[idx+1].b-stops[idx].b);
}

void UIOverlay::drawColorbar(const LeniaParams& params) {
    if (params.numChannels > 1 && !params.useColormapForMultichannel) {
        float barH = 18.0f;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float totalW = ImGui::GetContentRegionAvail().x;
        ImDrawList* dl = ImGui::GetWindowDrawList();

        float chH = barH / 3.0f;
        for (int ch = 0; ch < 3; ++ch) {
            float y0 = pos.y + ch * chH;
            constexpr int STEPS = 128;
            float stepW = totalW / STEPS;
            for (int i = 0; i < STEPS; ++i) {
                float raw = static_cast<float>(i) / (STEPS - 1);
                float t = raw;
                if (params.cmapReverse) t = 1.0f - t;
                t = params.cmapRange0 + t * (params.cmapRange1 - params.cmapRange0);
                t = std::pow(std::clamp(t, 0.0f, 1.0f), params.cmapPower);
                if (params.cmapOffset != 0.0f) t = std::fmod(t + params.cmapOffset, 1.0f);
                t = std::clamp(t, 0.0f, 1.0f);
                int rv = 0, gv = 0, bv = 0;
                if (ch == 0) rv = static_cast<int>(t * 255);
                else if (ch == 1) gv = static_cast<int>(t * 255);
                else bv = static_cast<int>(t * 255);
                dl->AddRectFilled(
                    ImVec2(pos.x + stepW * i, y0),
                    ImVec2(pos.x + stepW * (i + 1), y0 + chH),
                    IM_COL32(rv, gv, bv, 255));
            }
        }
        dl->AddRect(pos, ImVec2(pos.x + totalW, pos.y + barH), IM_COL32(80, 80, 100, 180), 2.0f);
        ImGui::Dummy(ImVec2(totalW, barH));
        ImGui::TextDisabled("RGB Channel Intensity");
        return;
    }

    float barH = 14.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float totalW = ImGui::GetContentRegionAvail().x;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    constexpr int STEPS = 128;
    float stepW = totalW / STEPS;
    
    bool isCustomColormap = (params.colormapMode >= 8);
    int customIdx = params.colormapMode - 8;
    bool hasCustomData = isCustomColormap && customIdx >= 0 && 
                         customIdx < static_cast<int>(m_customColormapData.size()) &&
                         !m_customColormapData[customIdx].empty();
    
    for (int i = 0; i < STEPS; ++i) {
        float raw = static_cast<float>(i) / (STEPS - 1);

        float t = raw;
        if (params.cmapReverse) t = 1.0f - t;
        t = params.cmapRange0 + t * (params.cmapRange1 - params.cmapRange0);
        t = std::pow(std::clamp(t, 0.0f, 1.0f), params.cmapPower);
        if (params.cmapOffset != 0.0f) t = std::fmod(t + params.cmapOffset, 1.0f);
        t = std::clamp(t, 0.0f, 1.0f);

        float cr, cg, cb;
        
        if (hasCustomData) {
            const auto& colors = m_customColormapData[customIdx];
            float fIdx = t * (colors.size() - 1);
            int idx0 = static_cast<int>(fIdx);
            int idx1 = std::min(idx0 + 1, static_cast<int>(colors.size()) - 1);
            float frac = fIdx - idx0;
            cr = colors[idx0][0] * (1.0f - frac) + colors[idx1][0] * frac;
            cg = colors[idx0][1] * (1.0f - frac) + colors[idx1][1] * frac;
            cb = colors[idx0][2] * (1.0f - frac) + colors[idx1][2] * frac;
        } else {
            switch (params.colormapMode) {
                case 1: viridisColor(t, cr, cg, cb); break;
                case 2: magmaColor(t, cr, cg, cb); break;
                case 3: infernoColor(t, cr, cg, cb); break;
                case 4: plasmaColor(t, cr, cg, cb); break;
                case 5: cr = cg = cb = t; break;
                case 6: cr = cg = cb = 1.0f - t; break;
                case 7: jetColor(t, cr, cg, cb); break;
                default: leniaColor(t, cr, cg, cb); break;
            }
        }

        if (params.cmapHueShift != 0.0f || params.cmapSaturation != 1.0f) {
            float maxC = std::max({cr, cg, cb});
            float minC = std::min({cr, cg, cb});
            float delta = maxC - minC;
            float h = 0.0f, s = 0.0f, v = maxC;
            if (delta > 0.0f) {
                s = delta / (v > 0.0f ? v : 1.0f);
                if (maxC == cr) h = std::fmod((cg - cb) / delta, 6.0f);
                else if (maxC == cg) h = (cb - cr) / delta + 2.0f;
                else h = (cr - cg) / delta + 4.0f;
                h /= 6.0f;
                if (h < 0.0f) h += 1.0f;
            }
            h = std::fmod(h + params.cmapHueShift, 1.0f);
            s = std::clamp(s * params.cmapSaturation, 0.0f, 1.0f);
            float c = v * s;
            float x2 = c * (1.0f - std::abs(std::fmod(h * 6.0f, 2.0f) - 1.0f));
            float m = v - c;
            int hi = static_cast<int>(h * 6.0f) % 6;
            switch (hi) {
                case 0: cr = c + m; cg = x2 + m; cb = m; break;
                case 1: cr = x2 + m; cg = c + m; cb = m; break;
                case 2: cr = m; cg = c + m; cb = x2 + m; break;
                case 3: cr = m; cg = x2 + m; cb = c + m; break;
                case 4: cr = x2 + m; cg = m; cb = c + m; break;
                default: cr = c + m; cg = m; cb = x2 + m; break;
            }
        }

        dl->AddRectFilled(
            ImVec2(pos.x + stepW * i, pos.y),
            ImVec2(pos.x + stepW * (i + 1), pos.y + barH),
            IM_COL32(static_cast<int>(std::clamp(cr, 0.0f, 1.0f) * 255),
                     static_cast<int>(std::clamp(cg, 0.0f, 1.0f) * 255),
                     static_cast<int>(std::clamp(cb, 0.0f, 1.0f) * 255), 255));
    }
    dl->AddRect(pos, ImVec2(pos.x + totalW, pos.y + barH), IM_COL32(80, 80, 100, 180), 2.0f);

    ImGui::Dummy(ImVec2(totalW, barH));
}

void UIOverlay::drawGraphWithAxes(const char* label, const float* data, int count, float yMin, float yMax,
                                   const char* xLabel, const char* yLabel, float height, unsigned int lineColor) {
    if (count < 2) return;

    float leftMargin = 50.0f;
    float bottomMargin = 22.0f;
    float rightMargin = 10.0f;
    float topMargin = 18.0f;

    float totalW = ImGui::GetContentRegionAvail().x;
    float plotW = totalW - leftMargin - rightMargin;
    float plotH = height - bottomMargin - topMargin;

    if (plotW < 20.0f || plotH < 20.0f) return;

    ImVec2 basePos = ImGui::GetCursorScreenPos();
    ImVec2 plotPos(basePos.x + leftMargin, basePos.y + topMargin);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(plotPos, ImVec2(plotPos.x + plotW, plotPos.y + plotH), IM_COL32(15, 15, 25, 220), 2.0f);
    dl->AddRect(plotPos, ImVec2(plotPos.x + plotW, plotPos.y + plotH), IM_COL32(60, 60, 80, 200), 2.0f);

    float range = yMax - yMin;
    if (range < 1e-9f) range = 1.0f;

    int numYTicks = 4;
    for (int i = 0; i <= numYTicks; ++i) {
        float t = static_cast<float>(i) / numYTicks;
        float yVal = yMin + t * range;
        float py = plotPos.y + plotH - t * plotH;
        dl->AddLine(ImVec2(plotPos.x, py), ImVec2(plotPos.x + plotW, py), IM_COL32(50, 50, 70, 100), 1.0f);
        dl->AddLine(ImVec2(plotPos.x - 4, py), ImVec2(plotPos.x, py), IM_COL32(100, 100, 120, 200), 1.0f);
        char buf[32];
        if (std::abs(yVal) >= 10000.0f) std::snprintf(buf, sizeof(buf), "%.0fk", yVal / 1000.0f);
        else if (std::abs(yVal) >= 100.0f) std::snprintf(buf, sizeof(buf), "%.0f", yVal);
        else if (std::abs(yVal) >= 1.0f) std::snprintf(buf, sizeof(buf), "%.1f", yVal);
        else std::snprintf(buf, sizeof(buf), "%.2f", yVal);
        ImVec2 txtSz = ImGui::CalcTextSize(buf);
        dl->AddText(ImVec2(plotPos.x - txtSz.x - 6, py - txtSz.y * 0.5f), IM_COL32(140, 140, 160, 220), buf);
    }

    int numXTicks = 4;
    for (int i = 0; i <= numXTicks; ++i) {
        float t = static_cast<float>(i) / numXTicks;
        float px = plotPos.x + t * plotW;
        int idx = static_cast<int>(t * (count - 1));
        dl->AddLine(ImVec2(px, plotPos.y), ImVec2(px, plotPos.y + plotH), IM_COL32(50, 50, 70, 100), 1.0f);
        dl->AddLine(ImVec2(px, plotPos.y + plotH), ImVec2(px, plotPos.y + plotH + 4), IM_COL32(100, 100, 120, 200), 1.0f);
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", idx);
        ImVec2 txtSz = ImGui::CalcTextSize(buf);
        dl->AddText(ImVec2(px - txtSz.x * 0.5f, plotPos.y + plotH + 6), IM_COL32(140, 140, 160, 200), buf);
    }

    for (int i = 0; i < count - 1; ++i) {
        float t0 = static_cast<float>(i) / (count - 1);
        float t1 = static_cast<float>(i + 1) / (count - 1);
        float v0 = std::clamp((data[i] - yMin) / range, 0.0f, 1.0f);
        float v1 = std::clamp((data[i + 1] - yMin) / range, 0.0f, 1.0f);
        float x0 = plotPos.x + t0 * plotW;
        float x1 = plotPos.x + t1 * plotW;
        float y0 = plotPos.y + plotH - v0 * plotH;
        float y1 = plotPos.y + plotH - v1 * plotH;
        dl->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), lineColor, 1.5f);
    }

    if (label && label[0]) {
        dl->AddText(ImVec2(plotPos.x + 4, basePos.y + 2), IM_COL32(180, 180, 200, 220), label);
    }
    if (yLabel && yLabel[0]) {
        ImVec2 txtSz = ImGui::CalcTextSize(yLabel);
        dl->AddText(ImVec2(basePos.x + 2, basePos.y + topMargin + plotH / 2 - txtSz.y / 2), IM_COL32(100, 150, 200, 180), yLabel);
    }
    if (xLabel && xLabel[0]) {
        ImVec2 txtSz = ImGui::CalcTextSize(xLabel);
        dl->AddText(ImVec2(plotPos.x + plotW / 2 - txtSz.x / 2, plotPos.y + plotH + 6), IM_COL32(100, 150, 200, 180), xLabel);
    }

    ImGui::Dummy(ImVec2(totalW, height));
}

void UIOverlay::drawPresetPreview(const Preset& preset, float size, const LeniaParams& params) {
    (void)params;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(10, 10, 18, 255), 3.0f);

    bool isMultichannel = (std::strcmp(preset.category, "Multichannel") == 0) || 
                          (std::strcmp(preset.category, "Multi-Kernel") == 0);
    bool hasSpecies = (preset.cellData != nullptr) || (preset.speciesFile != nullptr) || isMultichannel;

    if (hasSpecies && m_callbacks.getSpeciesPreviewData) {
        int presetIdx = -1;
        const auto& presets = getPresets();
        for (int i = 0; i < static_cast<int>(presets.size()); ++i) {
            if (&presets[i] == &preset) { presetIdx = i; break; }
        }

        if (presetIdx >= 0) {
            int rows = 0, cols = 0, channels = 0;
            std::vector<float> data = m_callbacks.getSpeciesPreviewData(presetIdx, rows, cols, channels);
            if (!data.empty() && rows > 0 && cols > 0) {
                float cellW = size / cols;
                float cellH = size / rows;
                for (int y = 0; y < rows; ++y) {
                    for (int x = 0; x < cols; ++x) {
                        int idx = (y * cols + x) * channels;
                        float r = 0, g = 0, b = 0;
                        if (channels >= 3) {
                            r = data[idx]; g = data[idx + 1]; b = data[idx + 2];
                        } else {
                            float v = std::clamp(data[idx], 0.0f, 1.0f);
                            r = g = b = v;
                        }
                        if (r > 0.01f || g > 0.01f || b > 0.01f) {
                            dl->AddRectFilled(
                                ImVec2(pos.x + x * cellW, pos.y + y * cellH),
                                ImVec2(pos.x + (x + 1) * cellW, pos.y + (y + 1) * cellH),
                                IM_COL32(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255), 255));
                        }
                    }
                }
            }
        }
    } else {
        float cx = pos.x + size * 0.5f;
        float cy = pos.y + size * 0.5f;
        float ringR = size * 0.38f;
        int numRings = std::clamp(preset.numRings, 1, 8);
        for (int rr = numRings - 1; rr >= 0; --rr) {
            float rFrac = (rr + 1.0f) / numRings;
            float w = preset.ringWeights[rr];
            int alpha = static_cast<int>(std::clamp(w, 0.0f, 1.0f) * 180 + 40);
            dl->AddCircle(ImVec2(cx, cy), ringR * rFrac, IM_COL32(100, 180, 255, alpha), 32, 2.0f);
        }
    }

    dl->AddRect(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(80, 80, 120, 200), 3.0f);
    ImGui::Dummy(ImVec2(size, size));
}

void UIOverlay::drawKernelPreview(GLuint kernelTex, int kernelDiam, float size) {
    if (kernelTex == 0 || kernelDiam <= 0) return;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(10, 10, 18, 255), 3.0f);

    std::vector<float> pixels(kernelDiam * kernelDiam);
    glGetTextureImage(kernelTex, 0, GL_RED, GL_FLOAT,
                      static_cast<GLsizei>(pixels.size() * sizeof(float)), pixels.data());

    float maxVal = 0.001f;
    for (int i = 0; i < kernelDiam * kernelDiam; ++i) {
        if (pixels[i] > maxVal) maxVal = pixels[i];
    }

    float cellW = size / kernelDiam;
    float cellH = size / kernelDiam;
    for (int y = 0; y < kernelDiam; ++y) {
        for (int x = 0; x < kernelDiam; ++x) {
            float v = pixels[y * kernelDiam + x] / maxVal;
            if (v > 0.01f) {
                int intensity = static_cast<int>(std::clamp(v, 0.0f, 1.0f) * 255);
                dl->AddRectFilled(
                    ImVec2(pos.x + x * cellW, pos.y + y * cellH),
                    ImVec2(pos.x + (x + 1) * cellW, pos.y + (y + 1) * cellH),
                    IM_COL32(intensity / 2, intensity, intensity / 2, 255));
            }
        }
    }

    dl->AddRect(pos, ImVec2(pos.x + size, pos.y + size), IM_COL32(80, 80, 120, 200), 3.0f);
    ImGui::Dummy(ImVec2(size, size));
}

void UIOverlay::render(LeniaParams& params, bool& paused, int& stepsPerFrame, bool& showUI,
                       const AnalysisData* analysis, const AnalysisManager* analysisMgr,
                       GLuint kernelTex, int kernelDiam, int stepCount, float simTimeMs,
                       int mouseGridX, int mouseGridY, float mouseValue, bool mouseInGrid) {
    if (!showUI) return;

    float fps = ImGui::GetIO().Framerate;
    m_frameTimeHistory[m_frameTimeHead] = 1000.0f / (fps > 0.0f ? fps : 1.0f);
    m_frameTimeHead = (m_frameTimeHead + 1) % 120;
    if (m_frameTimeCount < 120) m_frameTimeCount++;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(520, 920), ImGuiCond_FirstUseEver);

    ImGui::Begin("Lenia Explorer");

    if (paused != m_lastPausedState) {
        triggerPauseOverlay(paused);
        m_lastPausedState = paused;
    }

    int sec = 0;

    pushSectionColor(sec++);
    if (sectionHeader("Info", 0)) {
        if (mouseInGrid) {
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Cursor: (%d, %d)", mouseGridX, mouseGridY);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.5f, 1.0f), "Value: %.5f", mouseValue);
            ImGui::Separator();
        }

        ImGui::Text("Grid: %d x %d  |  Step: %d", params.gridW, params.gridH, stepCount);
        ImGui::Text("Channels: %d  |  Rules: %d", params.numChannels, params.numKernelRules);

        ImGui::Separator();
        if (ImGui::Checkbox("Show Console on Startup", &params.showConsoleOnStartup)) {
            std::ofstream cfg("lenia_config.txt");
            if (cfg.is_open()) {
                cfg << "showConsole=" << (params.showConsoleOnStartup ? "1" : "0") << "\n";
                cfg.close();
            }
        }
        Tooltip("If enabled, the console window will appear when starting the application.\nRequires restart to take effect.");

        ImGui::SeparatorText("Keybinds");
        ImGui::TextWrapped(
            "Space: Pause/Resume\n"
            "S: Single step | Hold S: Step @5fps\n"
            "Shift+S: Step @10fps\n"
            "R: Reset | C: Clear\n"
            "+/-: Zoom | Arrows: Pan\n"
            "Home: Reset View | Tab: Toggle UI\n"
            "1-5: Set steps/frame\n"
            "F11: Fullscreen | Esc: Quit");

        ImGui::Separator();
        if (ImGui::TreeNode("Theory")) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.95f, 1.0f));

            ImGui::SeparatorText("Lenia Fundamentals");
            ImGui::TextWrapped(
                "Lenia is a continuous cellular automaton system that generalizes discrete CA like "
                "Conway's Game of Life into a continuous domain. Unlike discrete CA with binary states "
                "and integer neighbor counts, Lenia uses continuous cell states in [0,1], continuous "
                "space via smooth kernels, and continuous time via differential integration."
            );
            ImGui::Spacing();
            ImGui::TextWrapped(
                "The fundamental equation governing Lenia is:\n"
                "  A(t+dt) = clip( A(t) + dt * G(K * A) )\n\n"
                "Where:\n"
                "  A(t) = cell state field at time t (values in [0,1])\n"
                "  K = convolution kernel (weighted neighborhood)\n"
                "  K * A = potential field (neighborhood sums)\n"
                "  G() = growth mapping function\n"
                "  dt = time step (integration rate)\n"
                "  clip() = clamps result to [0,1]"
            );

            ImGui::SeparatorText("Convolution Kernel");
            ImGui::TextWrapped(
                "The kernel K defines how neighbors influence each cell. It is typically radially "
                "symmetric and normalized (sums to 1). The kernel radius R determines the range of "
                "interaction - larger R creates larger, more complex patterns but requires more computation.\n\n"
                "Common kernel shapes:\n"
                "- Gaussian Shell: exp(-(r-peaks)^2/w^2), smooth bell-shaped rings\n"
                "- Bump4: (4r(1-r))^4, polynomial with compact support\n"
                "- Quad4: Polynomial kernel variant for specific dynamics\n"
                "- Multi-ring: Multiple concentric rings with independent weights (B values)\n\n"
                "The kernel is sampled on a (2R+1)x(2R+1) grid centered on each cell. The potential "
                "U at position (x,y) is computed as: U(x,y) = sum over neighbors of K(dx,dy) * A(x+dx,y+dy)"
            );

            ImGui::SeparatorText("Growth Function G(u)");
            ImGui::TextWrapped(
                "The growth function G maps potential U to a growth rate in [-1, +1]. This determines "
                "how cells respond to their neighborhood sum:\n\n"
                "- G(u) > 0: Cell value increases (growth/birth)\n"
                "- G(u) < 0: Cell value decreases (decay/death)\n"
                "- G(u) = 0: Cell remains stable\n\n"
                "Standard Lenia Growth (Gaussian):\n"
                "  G(u) = 2 * exp(-((u - mu)^2) / (2 * sigma^2)) - 1\n\n"
                "The parameters mu and sigma control:\n"
                "- mu (center): The ideal potential for maximum growth. Cells thrive when surrounded "
                "by approximately 'mu' worth of neighbors.\n"
                "- sigma (width): How tolerant growth is to deviation from mu. Small sigma = precise, "
                "fragile patterns. Large sigma = robust but less defined patterns.\n\n"
                "Growth Function Types:\n"
                "- Gaussian: Smooth bell curve, standard Lenia behavior\n"
                "- Step: Sharp threshold transitions, binary-like behavior\n"
                "- Game of Life: Discrete neighbor counting (B3/S23 rules)\n"
                "- SmoothLife: Smooth interpolation of GoL rules\n"
                "- Polynomial: G(u) = (u-mu)^n shaped curves\n"
                "- Exponential: Asymmetric decay characteristics\n"
                "- Double Peak: Two growth optima for complex dynamics\n"
                "- Mexican Hat: Center-surround pattern (DoG)\n"
                "- Asymptotic: Approaches limits smoothly\n"
                "- Soft Clip: Smooth saturation behavior"
            );

            ImGui::SeparatorText("Time Integration (dt)");
            ImGui::TextWrapped(
                "The time step dt controls how much change is applied per simulation step:\n\n"
                "- Small dt (0.01-0.1): Smooth, continuous evolution. Patterns move and morph gradually. "
                "Better for observing fine dynamics but slower overall evolution.\n"
                "- Medium dt (0.1-0.5): Standard Lenia range. Balance of smoothness and speed.\n"
                "- Large dt (0.5-1.0): Discrete-like behavior. Patterns jump between states. "
                "Necessary for GoL-style dynamics where dt=1.0 gives classic behavior.\n\n"
                "The dt also affects pattern stability - some configurations only work with specific dt "
                "values. Standard Orbium uses dt=0.1, while multi-channel aquarium species often use dt=0.5."
            );

            ImGui::SeparatorText("Multi-Channel Systems");
            ImGui::TextWrapped(
                "Multi-channel Lenia extends the system to multiple interacting fields (channels). "
                "Each channel is an independent state field that can influence other channels through "
                "kernel rules.\n\n"
                "A kernel rule defines:\n"
                "- Source Channel: Which channel is read by the kernel\n"
                "- Destination Channel: Which channel receives the growth\n"
                "- Strength (h): Coupling strength, can be negative for inhibition\n"
                "- Private mu/sigma: Rule-specific growth parameters\n"
                "- Private kernel: Rule-specific kernel shape and size\n\n"
                "This enables predator-prey dynamics, symbiotic relationships, and complex ecosystems "
                "where different 'species' (channel patterns) interact. Channel interactions can be:\n"
                "- Self-interaction (Ch0->Ch0): Standard single-channel behavior\n"
                "- Cross-activation (Ch0->Ch1 with h>0): Presence promotes growth\n"
                "- Cross-inhibition (Ch0->Ch1 with h<0): Presence suppresses growth"
            );

            ImGui::SeparatorText("Edge Conditions");
            ImGui::TextWrapped(
                "Edge conditions determine what happens at grid boundaries:\n\n"
                "- Periodic (Wrap): Edges connect to opposite sides, creating a toroidal surface. "
                "Patterns leaving one edge reappear on the other. Most common for exploring "
                "infinite-like behavior.\n\n"
                "- Clamp to Edge: Values at the boundary are extended beyond. Creates a 'wall' "
                "effect where patterns see the edge state repeated.\n\n"
                "- Mirror: Values are reflected at boundaries. Patterns see a mirror image of "
                "themselves at edges, creating reflection symmetry.\n\n"
                "Edge Fade controls the transition width for non-periodic modes, allowing smooth "
                "boundaries instead of sharp walls."
            );

            ImGui::SeparatorText("Walls");
            ImGui::TextWrapped(
                "Walls are persistent obstacles that affect simulation dynamics:\n\n"
                "- Wall Texture: A separate RGBA channel storing wall presence and properties\n"
                "- Wall Strength: How much the wall blocks or modifies cell activity\n"
                "- Wall Decay: Rate at which walls fade over time (0 = permanent)\n\n"
                "Wall Physics Types:\n"
                "- Block: Prevents cell activity in wall regions (multiplies by 0)\n"
                "- Reflect: Patterns bounce off walls (reverses growth direction)\n"
                "- Absorb: Walls absorb cell energy (negative growth in wall regions)\n"
                "- Dampen: Reduces but doesn't eliminate activity\n"
                "- None: Walls are visible but have no physical effect\n\n"
                "Walls can be drawn with the brush system using various shapes and patterns."
            );

            ImGui::SeparatorText("Pattern Characteristics");
            ImGui::TextWrapped(
                "Lenia can produce various pattern types:\n\n"
                "- Solitons (Gliders): Self-sustaining, moving structures that maintain their form. "
                "The famous Orbium is a soliton that glides smoothly across the grid.\n\n"
                "- Oscillators: Patterns that cycle through states while remaining stationary. "
                "Include pulsing, rotating, and complex periodic behaviors.\n\n"
                "- Still Lifes: Stable, unchanging patterns where G(U)=0 everywhere.\n\n"
                "- Chaotic/Turbulent: Unpredictable, ever-changing dynamics without stable structures.\n\n"
                "- Growing/Dying: Patterns that expand or contract. Some fill the grid, others fade away.\n\n"
                "Pattern survival depends on precise parameter tuning. Each species has a specific "
                "(mu, sigma, kernel) configuration that enables its existence."
            );

            ImGui::SeparatorText("Parameter Relationships");
            ImGui::TextWrapped(
                "Key parameter interactions:\n\n"
                "mu and Kernel: Higher mu values require denser neighborhoods or larger kernels "
                "to achieve the target potential. Species with high mu tend to be larger.\n\n"
                "sigma and Stability: Narrow sigma creates precise but fragile patterns that require "
                "exact conditions. Wide sigma produces robust but less defined structures.\n\n"
                "dt and Pattern Speed: Smaller dt makes patterns move slower per step but more "
                "smoothly. Some patterns only exist at specific dt values.\n\n"
                "Radius and Complexity: Larger radius allows more complex internal structure and "
                "larger patterns. Very small radius (R<6) limits pattern richness.\n\n"
                "Multi-ring Weights (B values): Control the relative influence of inner vs outer "
                "rings. Asymmetric B distributions create directional or complex behaviors."
            );

            ImGui::SeparatorText("Colormap & Visualization");
            ImGui::TextWrapped(
                "Display modes for understanding simulation state:\n\n"
                "- World View: Shows cell states with chosen colormap\n"
                "- Neighbor Sums: Visualizes potential field U (convolution result)\n"
                "- Growth Values: Shows G(U), the current growth rate field\n"
                "- Kernel: Displays the kernel shape being used\n"
                "- Delta: Shows change per step (A(t+dt) - A(t))\n\n"
                "Colormaps map scalar values to colors:\n"
                "- Perceptually uniform (Viridis, Magma, Inferno, Plasma): Equal steps look equal\n"
                "- Linear (Grayscale): Direct brightness mapping\n"
                "- Diverging (Jet): Distinct colors for high/low values\n\n"
                "For multi-channel, RGB channels are displayed directly as colors."
            );

            ImGui::PopStyleColor();
            ImGui::TreePop();
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader("Performance", 1, true)) {
        float avgFt = 0.0f, minFt = FLT_MAX, maxFt = 0.0f;
        for (int i = 0; i < m_frameTimeCount; ++i) {
            int idx = (m_frameTimeHead - m_frameTimeCount + i + 120) % 120;
            float ft = m_frameTimeHistory[idx];
            avgFt += ft;
            if (ft < minFt) minFt = ft;
            if (ft > maxFt) maxFt = ft;
        }
        avgFt /= std::max(1, m_frameTimeCount);

        ImVec4 fpsColor;
        if (fps >= 55.0f) fpsColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        else if (fps >= 30.0f) fpsColor = ImVec4(1.0f, 1.0f, 0.3f, 1.0f);
        else if (fps >= 15.0f) fpsColor = ImVec4(1.0f, 0.6f, 0.2f, 1.0f);
        else fpsColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);

        ImGui::TextColored(fpsColor, "FPS: %.1f", fps);
        Tooltip("Current frames per second.\nGreen: 55+ (excellent)\nYellow: 30-55 (good)\nOrange: 15-30 (acceptable)\nRed: <15 (slow)");
        ImGui::SameLine();
        ImGui::Text("| Frame: %.2f ms (avg)", avgFt);

        ImGui::Separator();
        ImGui::Text("Frame Time:");
        ImGui::SameLine(120);
        ImGui::TextDisabled("min=%.2f  avg=%.2f  max=%.2f ms", minFt, avgFt, maxFt);

        int totalCells = params.gridW * params.gridH;
        ImGui::Text("Grid Size:");
        ImGui::SameLine(120);
        if (totalCells >= 1000000)
            ImGui::Text("%d x %d = %.2fM cells", params.gridW, params.gridH, totalCells / 1000000.0f);
        else
            ImGui::Text("%d x %d = %.1fK cells", params.gridW, params.gridH, totalCells / 1000.0f);

        float simMsPerStep = simTimeMs / std::max(1, stepsPerFrame);
        ImGui::Text("Simulation:");
        ImGui::SameLine(120);
        ImGui::Text("%.2f ms/step  (%.2f ms total)", simMsPerStep, simTimeMs);

        float cellsPerSec = static_cast<float>(totalCells * stepsPerFrame) / std::max(0.001f, simTimeMs / 1000.0f);
        ImGui::Text("Throughput:");
        ImGui::SameLine(120);
        if (cellsPerSec >= 1e9f)
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "%.2f Gcells/s", cellsPerSec / 1e9f);
        else if (cellsPerSec >= 1e6f)
            ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), "%.1f Mcells/s", cellsPerSec / 1e6f);
        else
            ImGui::Text("%.0f Kcells/s", cellsPerSec / 1e3f);
        Tooltip("Processing throughput in cells updated per second.");

        int kernelCells = (params.radius * 2 + 1) * (params.radius * 2 + 1);
        long long opsPerStep = static_cast<long long>(totalCells) * kernelCells;
        ImGui::Text("Kernel Ops:");
        ImGui::SameLine(120);
        if (opsPerStep >= 1e9)
            ImGui::Text("%.2f Gops/step", opsPerStep / 1e9);
        else
            ImGui::Text("%.1f Mops/step", opsPerStep / 1e6);
        Tooltip("Kernel convolution operations per simulation step (cells x kernel size).");

        ImGui::Text("Kernel Size:");
        ImGui::SameLine(120);
        ImGui::Text("%dx%d = %d samples", params.radius * 2 + 1, params.radius * 2 + 1, kernelCells);

        ImGui::Text("Steps/Frame:");
        ImGui::SameLine(120);
        ImGui::Text("%d", stepsPerFrame);

        ImGui::Text("Total Steps:");
        ImGui::SameLine(120);
        ImGui::Text("%d", stepCount);

        ImGui::Separator();
        const char* perfLevel;
        ImVec4 perfColor;
        if (fps >= 55.0f && simTimeMs < 16.0f) {
            perfLevel = "Excellent"; perfColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        } else if (fps >= 30.0f && simTimeMs < 33.0f) {
            perfLevel = "Good"; perfColor = ImVec4(0.7f, 1.0f, 0.3f, 1.0f);
        } else if (fps >= 15.0f) {
            perfLevel = "Acceptable"; perfColor = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
        } else {
            perfLevel = "Slow"; perfColor = ImVec4(1.0f, 0.4f, 0.2f, 1.0f);
        }
        ImGui::TextColored(perfColor, "Performance: %s", perfLevel);
        Tooltip("Reduce grid size or kernel radius to improve performance.");

        if (m_frameTimeCount > 1) {
            float ftPlot[120];
            for (int i = 0; i < m_frameTimeCount; ++i) {
                int idx = (m_frameTimeHead - m_frameTimeCount + i + 120) % 120;
                ftPlot[i] = m_frameTimeHistory[idx];
            }
            drawGraphWithAxes("Frame Time", ftPlot, m_frameTimeCount, 0.0f, maxFt * 1.2f, "frames", "ms", 70.0f, IM_COL32(100, 200, 255, 220));
        }
        
        ImGui::Spacing();
        ImGui::Checkbox("Show Resource Monitor", &params.showResourceMonitor);
        if (params.showResourceMonitor) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Resource Usage:");
            
            if (params.gpuMemoryTotalMB > 0) {
                float memPercent = static_cast<float>(params.gpuMemoryUsedMB) / params.gpuMemoryTotalMB;
                ImVec4 memColor = (memPercent > 0.9f) ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) :
                                  (memPercent > 0.7f) ? ImVec4(1.0f, 0.8f, 0.3f, 1.0f) :
                                  ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
                ImGui::TextColored(memColor, "GPU Memory: %d / %d MB (%.0f%%)", 
                    params.gpuMemoryUsedMB, params.gpuMemoryTotalMB, memPercent * 100.0f);
                ImGui::ProgressBar(memPercent, ImVec2(-1, 8), "");
            } else {
                ImGui::TextDisabled("GPU Memory: N/A");
            }
            
            if (params.cpuMemoryUsedMB > 0.0f) {
                ImGui::Text("CPU Memory: %.1f MB", params.cpuMemoryUsedMB);
            }
            
            int gridMemBytes = params.gridW * params.gridH * (params.numChannels > 1 ? 16 : 4) * 2;
            int kernelMemBytes = (params.radius * 2) * (params.radius * 2) * 4;
            float totalTexMB = (gridMemBytes + kernelMemBytes) / (1024.0f * 1024.0f);
            ImGui::Text("Texture Memory: ~%.2f MB", totalTexMB);
            Tooltip("Estimated GPU memory for simulation textures.\n2x grid textures + kernel texture.");
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader("Grid", 2)) {
        bool gridDirty = false;
        int prevW = params.gridW, prevH = params.gridH;

        ImGui::Text("Size: %d x %d (%s cells)",
                    params.gridW, params.gridH,
                    params.gridW * params.gridH > 1000000 ?
                        (std::to_string(params.gridW * params.gridH / 1000000) + "M").c_str() :
                        (std::to_string(params.gridW * params.gridH / 1000) + "K").c_str());

        ImGui::InputInt("Width##grid",  &params.gridW, 64, 256);
        Tooltip("Grid width in cells. Larger grids allow more complex patterns but are slower. Must be >= 32.");
        ImGui::InputInt("Height##grid", &params.gridH, 64, 256);
        Tooltip("Grid height in cells. The grid wraps toroidally (edges connect).");
        params.gridW = std::max(32, params.gridW);
        params.gridH = std::max(32, params.gridH);
        if (params.gridW != prevW || params.gridH != prevH) gridDirty = true;

        if (gridDirty && m_callbacks.onGridResized)
            m_callbacks.onGridResized();

        ImGui::Separator();
        ImGui::Text("Transformations:");

        float btnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3) / 4.0f;

        if (ImGui::Button("<->##fliph", ImVec2(btnW, 24))) {
            if (m_callbacks.onFlipHorizontal)
                m_callbacks.onFlipHorizontal(true);
        }
        Tooltip("Flip horizontally (mirror left-right).");
        ImGui::SameLine();
        if (ImGui::Button("^v##flipv", ImVec2(btnW, 24))) {
            if (m_callbacks.onFlipVertical)
                m_callbacks.onFlipVertical(true);
        }
        Tooltip("Flip vertically (mirror top-bottom).");
        ImGui::SameLine();
        if (ImGui::Button("->|##rotcw", ImVec2(btnW, 24))) {
            if (m_callbacks.onRotateGrid)
                m_callbacks.onRotateGrid(1);
        }
        Tooltip("Rotate 90 degrees clockwise.");
        ImGui::SameLine();
        if (ImGui::Button("|<-##rotccw", ImVec2(btnW, 24))) {
            if (m_callbacks.onRotateGrid)
                m_callbacks.onRotateGrid(-1);
        }
        Tooltip("Rotate the grid 90 degrees counter-clockwise.");

        ImGui::Separator();
        ImGui::Text("Edge Conditions:");

        const char* edgeModes[] = {"Periodic (Wrap)", "Clamp to Edge", "Mirror"};

        ImGui::Combo("X Edge##edgex", &params.edgeModeX, edgeModes, IM_ARRAYSIZE(edgeModes));
        Tooltip("Horizontal edge behavior:\n- Periodic: Wraps around (toroidal)\n- Clamp: Uses edge values\n- Mirror: Reflects at boundaries");

        ImGui::Combo("Y Edge##edgey", &params.edgeModeY, edgeModes, IM_ARRAYSIZE(edgeModes));
        Tooltip("Vertical edge behavior:\n- Periodic: Wraps around (toroidal)\n- Clamp: Uses edge values\n- Mirror: Reflects at boundaries");

        if (params.edgeModeX != 0 || params.edgeModeY != 0) {
            ImGui::Separator();
            ImGui::Text("Edge Fade:");

            if (params.edgeModeX != 0) {
                ImGui::SliderFloat("X Fade##xfade", &params.edgeFadeX, 0.0f, 0.5f, "%.2f");
                Tooltip("Fade distance at horizontal edges (0 = hard edge, 0.5 = half grid).");
            }

            if (params.edgeModeY != 0) {
                ImGui::SliderFloat("Y Fade##yfade", &params.edgeFadeY, 0.0f, 0.5f, "%.2f");
                Tooltip("Fade distance at vertical edges (0 = hard edge, 0.5 = half grid).");
            }

            ImGui::Separator();
            const char* displayEdgeModes[] = {"Show Tiled", "Background Color", "Checker Pattern"};
            ImGui::Combo("Outside Display##dispedge", &params.displayEdgeMode, displayEdgeModes, IM_ARRAYSIZE(displayEdgeModes));
            Tooltip("How to display areas outside the grid:\n- Tiled: Repeats based on edge mode\n- Background: Shows background color\n- Checker: Shows a checker pattern");
        }

        ImGui::Spacing();
        ImGui::Separator();
        
        if (ImGui::CollapsingHeader("Infinite World Mode##infworld")) {
            ImGui::Checkbox("Enable Infinite World##infEnable", &params.infiniteWorldMode);
            Tooltip("Enable exploration of an infinite procedural world.\nUse mouse drag (middle-click or Ctrl+right-click) to pan.\nEdge conditions become periodic (wrapping).");

            if (params.infiniteWorldMode) {
                params.edgeModeX = 0;
                params.edgeModeY = 0;
                
                ImGui::Spacing();
                
                ImGui::TextColored(ImVec4(0.5f, 0.9f, 1.0f, 1.0f), "World Settings:");
                
                const char* chunkSizes[] = {"64x64", "128x128", "256x256", "512x512"};
                int chunkIdx = 0;
                if (params.chunkSize == 128) chunkIdx = 1;
                else if (params.chunkSize == 256) chunkIdx = 2;
                else if (params.chunkSize == 512) chunkIdx = 3;
                if (ImGui::Combo("Chunk Size##chunkSz", &chunkIdx, chunkSizes, 4)) {
                    int sizes[] = {64, 128, 256, 512};
                    params.chunkSize = sizes[chunkIdx];
                }
                Tooltip("Size of each world chunk in cells.");

                SliderIntWithInput("Load Radius##loadRad", &params.loadedChunksRadius, 1, 5);
                Tooltip("Number of chunks to keep loaded around the view center.");

                SliderIntWithInput("Max Chunks##maxCh", &params.maxLoadedChunks, 9, 81);
                Tooltip("Maximum number of chunks to keep in memory.");

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.9f, 1.0f, 1.0f), "Navigation:");
                
                ImGui::Text("Chunk Position: (%d, %d)", params.viewChunkX, params.viewChunkY);
                ImGui::Text("World Offset: (%.2f, %.2f)", params.panX, params.panY);
                
                float navBtnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;
                
                ImGui::Dummy(ImVec2(navBtnW, 0)); ImGui::SameLine();
                if (ImGui::Button("N##navN", ImVec2(navBtnW, 24))) {
                    params.viewChunkY++;
                    params.panY = 0.0f;
                }
                ImGui::SameLine(); ImGui::Dummy(ImVec2(navBtnW, 0));
                
                if (ImGui::Button("W##navW", ImVec2(navBtnW, 24))) {
                    params.viewChunkX--;
                    params.panX = 0.0f;
                }
                ImGui::SameLine();
                if (ImGui::Button("Home##navHome", ImVec2(navBtnW, 24))) {
                    params.viewChunkX = 0;
                    params.viewChunkY = 0;
                    params.panX = 0.0f;
                    params.panY = 0.0f;
                    params.zoom = 1.0f;
                }
                ImGui::SameLine();
                if (ImGui::Button("E##navE", ImVec2(navBtnW, 24))) {
                    params.viewChunkX++;
                    params.panX = 0.0f;
                }
                
                ImGui::Dummy(ImVec2(navBtnW, 0)); ImGui::SameLine();
                if (ImGui::Button("S##navS", ImVec2(navBtnW, 24))) {
                    params.viewChunkY--;
                    params.panY = 0.0f;
                }
                
                Tooltip("Navigate between chunks. Use mouse drag to pan within a chunk.");
                
                SliderFloatWithInput("Explore Speed##explSpd", &params.worldExploreSpeed, 0.1f, 5.0f, "%.1fx");
                Tooltip("Speed multiplier for keyboard navigation.");

                ImGui::Checkbox("Auto-Load Chunks##autoLoad", &params.autoLoadChunks);
                Tooltip("Automatically load new chunks as you explore.");

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.9f, 1.0f, 1.0f), "Display Options:");
                
                ImGui::Checkbox("Show Chunk Grid##showChGrid", &params.chunkBoundaryVisible);
                Tooltip("Display borders between chunks.");

                SliderFloatWithInput("Edge Fade##edgeFade", &params.chunkFadeDistance, 0.0f, 4.0f, "%.1f");
                Tooltip("Fade at world edges (0 = no fade).");

                const char* persistence[] = {"None (Clear)", "Preserve State", "Seed-Based"};
                ImGui::Combo("Persistence##persist", &params.chunkPersistence, persistence, 3);
                Tooltip("How chunk state is handled:\n- None: Chunks reset when unloaded\n- Preserve: Keeps state in memory\n- Seed-Based: Regenerates from seed");
                
                ImGui::Spacing();
                ImGui::TextDisabled("Tip: Middle-click or Ctrl+Right-click to pan");
                ImGui::TextDisabled("Scroll wheel to zoom");
            }
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader("Drawing Tools", 3)) {
        const char* toolModes[] = {"Brush (Living Cells)", "Obstacle (Barrier)"};
        int toolMode = params.wallEnabled ? 1 : 0;
        if (ImGui::Combo("Tool Mode", &toolMode, toolModes, 2)) {
            params.wallEnabled = (toolMode == 1);
            params.brushEnabled = true;
        }
        Tooltip("Brush paints cells that evolve with the simulation.\nObstacle creates barriers where cells are held at a fixed value.");

        ImGui::Checkbox("Enable Drawing", &params.brushEnabled);
        Tooltip("Enable or disable drawing on the simulation grid.");

        if (params.brushEnabled) {
            ImGui::Separator();
            
            if (params.wallEnabled) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
                ImGui::Text("OBSTACLE MODE ACTIVE");
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
                ImGui::Text("BRUSH MODE ACTIVE");
                ImGui::PopStyleColor();
            }

            if (ImGui::CollapsingHeader("Shape & Size", ImGuiTreeNodeFlags_DefaultOpen)) {
                const char* shapeNames[] = {"Circle", "Square", "Diamond", "Ring", "Star (5pt)", "Star (6pt)", "Hexagon", "Cross", "Plus", "Gaussian Blob", "Noise Disc", "Gradient Disc"};
                int shapeIdx = params.wallEnabled ? params.wallShape : params.brushShape;
                if (ImGui::Combo("Shape", &shapeIdx, shapeNames, IM_ARRAYSIZE(shapeNames))) {
                    if (params.wallEnabled)
                        params.wallShape = shapeIdx;
                    else
                        params.brushShape = shapeIdx;
                }
                Tooltip("Shape of the brush/obstacle.\n- Ring: Hollow circle\n- Star: 5 or 6 pointed star\n- Hexagon: Hexagonal shape\n- Cross/Plus: Cross patterns\n- Gaussian Blob: Soft falloff\n- Noise Disc: Random texture\n- Gradient Disc: Linear gradient");

                ImGui::SliderInt("Size", &params.brushSize, 1, 100);
                Tooltip("Size of the brush in cells.");
                
                params.wallThickness = static_cast<float>(params.brushSize);

                if (!params.wallEnabled) {
                    ImGui::SliderFloat("Falloff", &params.brushFalloff, 0.0f, 1.0f, "%.2f");
                    Tooltip("Edge softness. 0 = hard edge, 1 = smooth fade.");
                } else {
                    ImGui::SliderFloat("Falloff", &params.wallFalloff, 0.0f, 1.0f, "%.2f");
                    Tooltip("Edge softness for obstacle boundaries.");
                }
            }

            if (ImGui::CollapsingHeader("Draw Method", ImGuiTreeNodeFlags_DefaultOpen)) {
                const char* drawModeNames[] = {"Freehand", "Line", "Circle", "Rectangle"};
                ImGui::Combo("Draw Mode", &params.brushDrawMode, drawModeNames, IM_ARRAYSIZE(drawModeNames));
                Tooltip("Freehand: Click and drag to draw\nLine: Click start, release at end\nCircle: Click center, drag radius\nRectangle: Click corner, drag to opposite corner");

                if (params.brushDrawMode != 0) {
                    ImGui::Separator();
                    bool isDrawing = params.wallEnabled ? params.wallLineDrawing : params.brushLineDrawing;
                    if (isDrawing) {
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Drawing... (Right-click to cancel)");
                    } else {
                        ImGui::TextDisabled("Click on grid to start drawing");
                    }
                }
            }

            if (params.wallEnabled) {
                if (ImGui::CollapsingHeader("Obstacle Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::SliderFloat("Cell Value", &params.wallValue, 0.0f, 1.0f, "%.3f");
                    Tooltip("Fixed value for cells inside obstacles.\n0.0 = dead/empty (blocks life)\n1.0 = fully alive (creates permanent life)");

                    ImGui::Separator();
                    float wallColor[4] = {params.wallR, params.wallG, params.wallB, params.wallA};
                    if (ImGui::ColorEdit4("Display Color", wallColor)) {
                        params.wallR = wallColor[0];
                        params.wallG = wallColor[1];
                        params.wallB = wallColor[2];
                        params.wallA = wallColor[3];
                    }
                    Tooltip("Visual color of obstacles in the display.");

                    if (params.numChannels > 1) {
                        ImGui::Separator();
                        ImGui::Text("Affected Channels:");
                        ImGui::Checkbox("Ch0 (R)", &params.wallAffectsCh0);
                        ImGui::SameLine();
                        if (params.numChannels >= 2) {
                            ImGui::Checkbox("Ch1 (G)", &params.wallAffectsCh1);
                            ImGui::SameLine();
                        }
                        if (params.numChannels >= 3) {
                            ImGui::Checkbox("Ch2 (B)", &params.wallAffectsCh2);
                        }
                        Tooltip("Which channels the obstacle affects.");
                    }

                    const char* wallBlendNames[] = {"Replace", "Max", "Replace If Stronger", "Blend", "Erase"};
                    ImGui::Combo("Blend Mode", &params.wallBlendMode, wallBlendNames, IM_ARRAYSIZE(wallBlendNames));
                    Tooltip("Replace: Overwrite existing\nMax: Keep stronger value\nBlend: Smooth blend\nErase: Remove obstacles");
                }

                ImGui::Separator();
                if (ImGui::Button("Clear All Obstacles", ImVec2(-1, 0))) {
                    if (m_callbacks.onClearWalls)
                        m_callbacks.onClearWalls();
                }
                Tooltip("Remove all obstacles from the simulation.");
            } else {
                if (ImGui::CollapsingHeader("Brush Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                    const char* modeNames[] = {"Set", "Add", "Subtract", "Max", "Min", "Erase"};
                    ImGui::Combo("Paint Mode", &params.brushMode, modeNames, IM_ARRAYSIZE(modeNames));
                    Tooltip("Set: Replace cell value\nAdd: Add to existing\nSubtract: Subtract from existing\nMax/Min: Keep larger/smaller\nErase: Set to zero");

                    ImGui::SliderFloat("Value", &params.brushValue, 0.0f, 1.0f, "%.2f");
                    Tooltip("Cell value to paint.");

                    ImGui::SliderFloat("Strength", &params.brushStrength, 0.0f, 2.0f, "%.2f");
                    Tooltip("Intensity multiplier.");

                    if (params.numChannels > 1) {
                        ImGui::Separator();
                        const char* channelNames[] = {"Red (Ch0)", "Green (Ch1)", "Blue (Ch2)", "All Channels"};
                        int maxCh = (std::min)(params.numChannels, 3);
                        ImGui::Combo("Target Channel", &params.brushChannel, channelNames, maxCh + 1);
                        Tooltip("Which channel(s) to paint.");
                    }
                }

                if (ImGui::CollapsingHeader("Symmetry")) {
                    ImGui::Checkbox("Mirror X", &params.brushSymmetryX);
                    ImGui::SameLine();
                    ImGui::Checkbox("Mirror Y", &params.brushSymmetryY);
                    Tooltip("Mirror strokes across the grid center.");

                    ImGui::Checkbox("Radial Symmetry", &params.brushSymmetryRadial);
                    if (params.brushSymmetryRadial) {
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(80);
                        ImGui::SliderInt("##radialcount", &params.brushRadialCount, 2, 16);
                    }
                    Tooltip("Rotational symmetry around grid center.");
                }
            }

            if (ImGui::CollapsingHeader("Stroke Spacing##brushSpacingHeader")) {
                ImGui::SliderFloat("Brush Spacing##brushSpacingSlider", &params.brushSpacing, 0.1f, 5.0f, "%.1f");
                Tooltip("Distance between stroke applications when dragging.");

                ImGui::Checkbox("Smooth Interpolation##brushSmooth", &params.brushSmooth);
                Tooltip("Interpolate positions when moving quickly.");
            }
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader("Presets & Initialization", 4, true)) {
        const auto& presets = getPresets();
        const auto& categories = getPresetCategories();

        std::vector<const char*> catPtrs;
        for (const auto& c : categories)
            catPtrs.push_back(c.c_str());

        ImGui::Combo("Category", &m_selectedCategory, catPtrs.data(),
                     static_cast<int>(catPtrs.size()));

        ImGui::InputTextWithHint("##search", "Search presets...", m_presetSearchBuf, sizeof(m_presetSearchBuf));

        std::vector<int> filteredIndices;
        std::vector<std::string> filteredNames;
        std::string searchLower;
        for (char c : std::string(m_presetSearchBuf))
            searchLower += static_cast<char>(std::tolower(c));

        for (int i = 0; i < static_cast<int>(presets.size()); ++i) {
            bool catMatch = (m_selectedCategory == 0 ||
                (m_selectedCategory < static_cast<int>(categories.size()) &&
                 std::strcmp(presets[i].category, categories[m_selectedCategory].c_str()) == 0));

            std::string nameLower;
            for (char c : std::string(presets[i].name))
                nameLower += static_cast<char>(std::tolower(c));

            bool searchMatch = searchLower.empty() || nameLower.find(searchLower) != std::string::npos;

            if (catMatch && searchMatch) {
                filteredIndices.push_back(i);
                filteredNames.push_back(presets[i].name);
            }
        }

        int filteredSel = 0;
        for (int i = 0; i < static_cast<int>(filteredIndices.size()); ++i) {
            if (filteredIndices[i] == m_selectedPreset) { filteredSel = i; break; }
        }

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::BeginListBox("##presetlist", ImVec2(-1, 100))) {
            for (int i = 0; i < static_cast<int>(filteredNames.size()); ++i) {
                bool isSelected = (i == filteredSel);
                if (ImGui::Selectable(filteredNames[i].c_str(), isSelected)) {
                    filteredSel = i;
                    m_selectedPreset = filteredIndices[i];
                    if (m_callbacks.onPresetSelected)
                        m_callbacks.onPresetSelected(m_selectedPreset);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        if (m_selectedPreset >= 0 && m_selectedPreset < static_cast<int>(presets.size())) {
            const auto& cp = presets[m_selectedPreset];
            ImGui::Separator();
            ImGui::Text("Selected: %s", cp.name);

            float previewSize = 60.0f;
            ImGui::BeginGroup();
            ImGui::TextDisabled("Species");
            drawPresetPreview(cp, previewSize, params);
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::TextDisabled("Kernel");
            drawKernelPreview(kernelTex, kernelDiam, previewSize);
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            bool hasCells = (cp.cellData != nullptr) || (cp.speciesFile != nullptr);
            ImGui::TextDisabled("%s", cp.category);
            ImGui::TextDisabled("%s", hasCells ? "Species" : "Procedural");
            ImGui::TextDisabled("R=%d rings=%d", cp.radius, cp.numRings);
            ImGui::TextDisabled("mu=%.3f", cp.mu);
            ImGui::TextDisabled("sigma=%.4f", cp.sigma);
            ImGui::EndGroup();
        }

        ImGui::TextDisabled("%d presets (%d shown)",
            static_cast<int>(presets.size()), static_cast<int>(filteredIndices.size()));

        float btnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;
        if (ImGui::Button("Randomize", ImVec2(btnW, 28))) {
            if (m_callbacks.onRandomize) m_callbacks.onRandomize();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear", ImVec2(btnW, 28))) {
            if (m_callbacks.onClear) m_callbacks.onClear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Preset", ImVec2(btnW, 28))) {
            if (m_callbacks.onPresetSelected)
                m_callbacks.onPresetSelected(m_selectedPreset);
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Placement");

        const char* placementNames[] = {
            "Center", "Top-Left", "Top-Right", "Bottom-Left", "Bottom-Right",
            "Top", "Bottom", "Left", "Right", "Random", "Grid", "Two-Place", "Scatter"
        };
        ImGui::Combo("Placement", &params.placementMode, placementNames, 13);

        SliderIntWithInput("Count", &params.placementCount, 1, 50);

        SliderFloatWithInput("Scale", &params.placementScale, 0.1f, 3.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 3.0f, r, 1, nullptr, 0); snapFloat(params.placementScale, 0.1f, 3.0f, r, 1); }

        const char* rotNames[] = {"0 deg", "90 deg", "180 deg", "270 deg"};
        ImGui::Combo("Rotation", &params.placementRotation, rotNames, 4);

        SliderFloatWithInput("Margin", &params.placementMargin, 0.0f, 0.25f, "%.3f");

        if (params.placementMode >= 9) {
            ImGui::Checkbox("Random Flip", &params.placementRandomFlip);
        } else {
            ImGui::Checkbox("Flip Horizontal##placeFlipH", &params.placementFlipH);
            ImGui::SameLine();
            ImGui::Checkbox("Flip Vertical##placeFlipV", &params.placementFlipV);
        }

        if (params.placementCount > 1 && params.placementMode < 9) {
            SliderFloatWithInput("Place Spacing##placeSpacing", &params.placementSpacing, 0.01f, 0.5f, "%.3f");
        }

        if (params.placementMode == static_cast<int>(PlacementMode::Scatter)) {
            SliderIntWithInput("Min Separation##minSep", &params.placementMinSeparation, 0, 100);
        }

        ImGui::Checkbox("Clear Grid First##clearFirst", &params.placementClearFirst);

        ImGui::Spacing();
        if (ImGui::Button("Apply Placement", ImVec2(-1, 28))) {
            if (m_callbacks.onReset) m_callbacks.onReset();
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader("Simulation", 5, true)) {
        ImGui::Checkbox("Paused (Space)", &paused);
        ImGui::SameLine();
        ImGui::TextDisabled("Hold S to step");

        SliderIntWithInput("Steps/Frame", &stepsPerFrame, 1, 50);
        { int r[] = {1}; int g[] = {5, 10, 20}; drawSliderMarkersInt(1, 50, r, 1, g, 3); snapInt(stepsPerFrame, 1, 50, r, 1); snapInt(stepsPerFrame, 1, 50, g, 3); }
        Tooltip("Number of simulation steps per rendered frame. Higher values speed up evolution at the cost of frame rate. Keys 1-5 for quick set.");

        ImGui::Text("Step: %d", stepCount);
        ImGui::SameLine();
        ImGui::Text("Sim: %.2f ms", simTimeMs);
    }
    popSectionColor();
    pushSectionColor(sec++);
    if (sectionHeader("Growth Function", 6, true)) {
        const char* growthNames[] = {"Lenia (Gaussian)", "Step", "Game of Life", "SmoothLife", "Polynomial", "Exponential", "Double Peak", "Asymptotic", "Soft Clip", "Larger-than-Life", "Quad4 (Polynomial)"};
        ImGui::Combo("Growth Type", &params.growthType, growthNames, 11);
        Tooltip("Selects the mathematical function mapping convolution potential to growth rate. Gaussian is standard Lenia. Step creates sharp boundaries. Game of Life uses discrete neighbor counting.");

        SliderFloatWithInput("mu##growth", &params.mu, 0.001f, 1.0f);
        { float r[] = {0.15f}; float g[] = {0.29f, 0.35f}; drawSliderMarkers(0.001f, 1.0f, r, 1, g, 2); float all[] = {0.15f, 0.29f, 0.35f}; snapFloat(params.mu, 0.001f, 1.0f, all, 3); }
        Tooltip("Center of the growth function peak (mu). The potential value at which maximum growth occurs. Typical values: 0.15 for Orbium, 0.29 for Scutium, 0.35 for larger species.");

        SliderFloatWithInput("sigma##growth", &params.sigma, 0.001f, 0.5f);
        { float r[] = {0.017f}; float g[] = {0.015f, 0.045f}; drawSliderMarkers(0.001f, 0.5f, r, 1, g, 2); float all[] = {0.017f, 0.015f, 0.045f}; snapFloat(params.sigma, 0.001f, 0.5f, all, 3); }
        Tooltip("Width of the growth function peak (sigma). Narrower values create more selective growth. Small sigma = fragile but precise patterns. Large sigma = robust but blobby.");

        SliderFloatWithInput("dt##timestep", &params.dt, 0.001f, 2.0f);
        { float r[] = {0.25f}; float g[] = {0.1f, 0.5f, 1.0f}; drawSliderMarkers(0.001f, 2.0f, r, 1, g, 3); float all[] = {0.25f, 0.1f, 0.5f, 1.0f}; snapFloat(params.dt, 0.001f, 2.0f, all, 4); }
        Tooltip("Time step per simulation step. Smaller dt = smoother but slower evolution. 0.1 is standard Lenia, 0.5 for multi-channel aquarium, 1.0 for Game of Life.");

        ImGui::Spacing();
        drawGrowthPlot(params);
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader("Kernel", 7, true)) {
        bool kernelDirty = false;
        bool isMulti = (params.numChannels > 1);

        if (!isMulti) {
            const char* kernelNames[] = {"Gaussian Shell", "Bump4", "Multiring (Gauss)", "Multiring (Bump4)", "Game of Life", "Step Unimodal", "Cosine Shell", "Mexican Hat", "Quad4", "Multiring (Quad4)", "Cone", "Torus (Dual Ring)", "Ring (Sharp)", "Gaussian Mixture", "Sinc", "Wavelet (Ricker)", "Negative Ring"};
            if (ImGui::Combo("Kernel Type", &params.kernelType, kernelNames, 17))
                kernelDirty = true;
            Tooltip("Shape of the convolution kernel.\n- Gaussian Shell: Standard Lenia\n- Bump4: Bert Chan's exponential bump\n- Quad4: Polynomial (4r(1-r))^4\n- Cone: Linear falloff\n- Torus: Dual concentric rings\n- Ring: Sharp single ring\n- Gaussian Mixture: Three overlapping Gaussians\n- Sinc: sin(x)/x oscillating kernel\n- Wavelet (Ricker): Mexican hat wavelet\n- Negative Ring: Outer inhibitory ring");

            if (!m_kernelPresetNames.empty()) {
                auto kpGetter = [](void* data, int idx, const char** out) -> bool {
                    auto* names = static_cast<std::vector<std::string>*>(data);
                    if (idx < 0 || idx >= static_cast<int>(names->size())) return false;
                    *out = (*names)[idx].c_str();
                    return true;
                };
                int prevKP = m_selectedKernelPreset;
                if (ImGui::Combo("Kernel Preset", &m_selectedKernelPreset, kpGetter,
                                 &m_kernelPresetNames, static_cast<int>(m_kernelPresetNames.size()), 10)) {
                    if (m_selectedKernelPreset != prevKP && m_callbacks.onKernelPresetSelected)
                        m_callbacks.onKernelPresetSelected(m_selectedKernelPreset);
                }
            }
        }

        int prevR = params.radius;
        if (SliderIntWithInput("Radius (R)", &params.radius, 1, 128)) {
            kernelDirty = (params.radius != prevR);
        }
        { int r[] = {13}; int g[] = {10, 12, 18, 26, 52}; drawSliderMarkersInt(1, 128, r, 1, g, 5); snapInt(params.radius, 1, 128, r, 1); snapInt(params.radius, 1, 128, g, 5); }
        Tooltip("Kernel radius in cells. Determines the range of interaction. Larger radii create more complex and larger patterns but are slower. 13 is standard for Orbium, 10-12 for multi-channel.");
        if (params.radius != prevR) {
            kernelDirty = true;
            if (isMulti) {
                for (int r = 0; r < params.numKernelRules; ++r) {
                    if (m_callbacks.onRuleKernelChanged)
                        m_callbacks.onRuleKernelChanged(r);
                }
            }
        }

        if (!isMulti) {
            if (params.kernelType != 4) {
                int prevRings = params.numRings;
                if (SliderIntWithInput("Rings", &params.numRings, 1, 8)) {
                    if (params.numRings != prevRings) kernelDirty = true;
                }
                Tooltip("Number of concentric rings in the kernel. Each ring can have its own weight (B value). More rings enable richer interactions. Standard Lenia uses 1 ring.");

                if (params.numRings > 1) {
                    ImGui::Indent(10.0f);
                    for (int i = 0; i < params.numRings && i < 16; ++i) {
                        char label[32];
                        std::snprintf(label, sizeof(label), "B%d##ring", i);
                        if (ImGui::SliderFloat(label, &params.ringWeights[i], 0.0f, 1.0f, "%.3f"))
                            kernelDirty = true;
                        Tooltip("Weight of this ring in the kernel. Controls the relative contribution of this distance band to the convolution potential.");
                    }
                    ImGui::Unindent(10.0f);
                }
            }
            
            ImGui::Spacing();
            if (ImGui::CollapsingHeader("Advanced Kernel##advkernel")) {
                if (ImGui::SliderFloat("Anisotropy", &params.kernelAnisotropy, 0.0f, 1.0f, "%.2f")) {
                    kernelDirty = true;
                }
                Tooltip("Directional asymmetry strength. 0 = isotropic (symmetric), 1 = strongly directional.");
                
                if (params.kernelAnisotropy > 0.01f) {
                    if (ImGui::SliderFloat("Direction", &params.kernelAnisotropyAngle, 0.0f, 360.0f, "%.0f°")) {
                        kernelDirty = true;
                    }
                    Tooltip("Direction of anisotropy in degrees.");
                }
                
                ImGui::Checkbox("Time-Varying", &params.kernelTimeVarying);
                Tooltip("Enable kernel pulsing/breathing effect over time.");
                
                if (params.kernelTimeVarying) {
                    if (ImGui::SliderFloat("Pulse Frequency", &params.kernelPulseFrequency, 0.0f, 5.0f, "%.2f")) {
                        kernelDirty = true;
                    }
                    Tooltip("How fast the kernel pulses (cycles per time unit).");
                }
                
                const char* modifierNames[] = {"None", "Negative Outer Ring"};
                if (ImGui::Combo("Modifier", &params.kernelModifier, modifierNames, 2)) {
                    kernelDirty = true;
                }
                Tooltip("None: Standard kernel\nNegative Outer Ring: Adds inhibitory effect at outer edges");
            }
        }

        if (kernelDirty && m_callbacks.onKernelChanged)
            m_callbacks.onKernelChanged();

        ImGui::Checkbox("Show Kernel Preview", &params.showKernelPreview);
        if (params.showKernelPreview) {
            if (isMulti) {
                const char* chLabel[] = {"R", "G", "B"};
                for (int r = 0; r < params.numKernelRules; ++r) {
                    auto& rule = params.kernelRules[r];
                    int src = std::clamp(rule.sourceChannel, 0, params.numChannels - 1);
                    int dst = std::clamp(rule.destChannel, 0, params.numChannels - 1);
                    ImGui::TextDisabled("Rule %d: %s -> %s", r, chLabel[src], chLabel[dst]);
                    if (m_callbacks.getRuleKernelInfo) {
                        auto [tex, diam] = m_callbacks.getRuleKernelInfo(r);
                        if (tex > 0 && diam > 0)
                            drawKernelCrossSection(tex, diam);
                    }
                }
            } else if (kernelTex > 0 && kernelDiam > 0) {
                drawKernelCrossSection(kernelTex, kernelDiam);
            }
        }

        if (isMulti) {
            ImGui::TextDisabled("Per-rule kernel settings in Multi-Channel section.");
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader("Multi-Channel", 8)) {
        int prevCh = params.numChannels;
        const char* chNames[] = {"1 (Single)", "3 (RGB)"};
        int chIdx = (params.numChannels > 1) ? 1 : 0;
        if (ImGui::Combo("Channels", &chIdx, chNames, 2)) {
            int newCh = (chIdx == 1) ? 3 : 1;
            if (newCh != prevCh && m_callbacks.onChannelModeChanged) {
                m_callbacks.onChannelModeChanged(newCh);
            }
        }
        Tooltip("Number of independent state channels. Single = classic Lenia. RGB = 3 channels (R/G/B) with cross-channel kernel rules, used for multi-species like Aquarium.");

        if (params.numChannels > 1) {
            ImGui::TextColored(ImVec4(0.7f,0.9f,1.0f,1.0f), "Rules: %d", params.numKernelRules);
            ImGui::SameLine(0, 10);
            if (ImGui::SmallButton("+##addRule")) {
                if (params.numKernelRules < 16) {
                    auto& nr = params.kernelRules[params.numKernelRules];
                    nr = {};
                    nr.mu = 0.15f;
                    nr.sigma = 0.015f;
                    nr.growthStrength = 1.0f;
                    nr.radiusFraction = 1.0f;
                    nr.numRings = 1;
                    nr.ringWeights[0] = 1.0f;
                    nr.sourceChannel = 0;
                    nr.destChannel = 0;
                    nr.kernelType = 0;
                    nr.growthType = 0;
                    params.numKernelRules++;
                }
            }
            Tooltip("Add a new kernel rule.");
            ImGui::SameLine(0, 5);
            if (ImGui::SmallButton("-##removeRule")) {
                if (params.numKernelRules > 0)
                    params.numKernelRules--;
            }
            Tooltip("Remove the last kernel rule.");

            ImGui::Separator();

            const ImVec4 chColors[3] = {
                ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                ImVec4(0.35f, 1.0f, 0.35f, 1.0f),
                ImVec4(0.4f, 0.55f, 1.0f, 1.0f)
            };
            const char* chLabels[3] = {"R", "G", "B"};

            if (params.numKernelRules > 0) {
                ImGui::Text("Channel Routing:");
                ImGui::Indent(10.0f);
                for (int r = 0; r < params.numKernelRules && r < 16; ++r) {
                    auto& rule = params.kernelRules[r];
                    int s = std::clamp(rule.sourceChannel, 0, 2);
                    int d = std::clamp(rule.destChannel, 0, 2);
                    ImGui::TextColored(chColors[s], "%s", chLabels[s]);
                    ImGui::SameLine(0, 2);
                    ImGui::TextUnformatted("->");
                    ImGui::SameLine(0, 2);
                    ImGui::TextColored(chColors[d], "%s", chLabels[d]);
                    ImGui::SameLine(0, 8);
                    ImGui::TextDisabled("r%d m=%.3f s=%.4f h=%.2f", r, rule.mu, rule.sigma, rule.growthStrength);
                }
                ImGui::Unindent(10.0f);
                ImGui::Separator();
            }

            if (params.numKernelRules > 0) {
                ImGui::Indent(10.0f);
                for (int r = 0; r < params.numKernelRules && r < 16; ++r) {
                    auto& rule = params.kernelRules[r];
                    int s = std::clamp(rule.sourceChannel, 0, 2);
                    int d = std::clamp(rule.destChannel, 0, 2);

                    char hdr[96];
                    std::snprintf(hdr, sizeof(hdr), "Rule %d (%s -> %s) | m=%.3f s=%.4f h=%.1f##rule%d",
                                  r, chLabels[s], chLabels[d], rule.mu, rule.sigma, rule.growthStrength, r);
                    ImGui::PushID(r);
                    if (ImGui::TreeNode(hdr)) {
                        bool ruleKernelDirty = false;

                        char srcLabel[32], dstLabel[32];
                        std::snprintf(srcLabel, sizeof(srcLabel), "Source Channel##src%d", r);
                        std::snprintf(dstLabel, sizeof(dstLabel), "Dest Channel##dst%d", r);

                        ImGui::TextColored(chColors[s], "Source: %s (%d)", chLabels[s], rule.sourceChannel);
                        ImGui::SameLine(0, 20);
                        ImGui::TextColored(chColors[d], "Dest: %s (%d)", chLabels[d], rule.destChannel);

                        SliderIntWithInput(srcLabel, &rule.sourceChannel, 0, 2);
                        SliderIntWithInput(dstLabel, &rule.destChannel, 0, 2);

                        char muL[32], sigL[32], hL[32], rfL[32], rnL[32], ktL[32], gtL[32];
                        std::snprintf(muL, sizeof(muL), "mu##rmu%d", r);
                        std::snprintf(sigL, sizeof(sigL), "sigma##rsig%d", r);
                        std::snprintf(hL, sizeof(hL), "Strength (h)##rh%d", r);
                        std::snprintf(rfL, sizeof(rfL), "Radius Frac##rrf%d", r);
                        std::snprintf(rnL, sizeof(rnL), "Rings##rrn%d", r);
                        std::snprintf(ktL, sizeof(ktL), "Kernel##rkt%d", r);
                        std::snprintf(gtL, sizeof(gtL), "Growth##rgt%d", r);

                        SliderFloatWithInput(muL, &rule.mu, 0.001f, 1.0f, "%.4f");
                        SliderFloatWithInput(sigL, &rule.sigma, 0.001f, 0.5f, "%.4f");
                        SliderFloatWithInput(hL, &rule.growthStrength, -2.0f, 2.0f, "%.3f");
                        if (SliderFloatWithInput(rfL, &rule.radiusFraction, 0.1f, 2.0f, "%.3f"))
                            ruleKernelDirty = true;
                        if (SliderIntWithInput(rnL, &rule.numRings, 1, 8))
                            ruleKernelDirty = true;

                        const char* kNames[] = {"Gaussian", "Bump4", "Multi-Gauss", "Multi-Bump4", "GoL", "Step", "Cosine", "Mexican Hat", "Quad4", "Multi-Quad4"};
                        if (ImGui::Combo(ktL, &rule.kernelType, kNames, 10))
                            ruleKernelDirty = true;

                        const char* gNames[] = {"Lenia", "Step", "GoL", "SmoothLife", "Polynomial", "Exponential", "DoublePeak", "Asymptotic", "SoftClip", "LTL"};
                        ImGui::Combo(gtL, &rule.growthType, gNames, 10);

                        if (rule.numRings > 1) {
                            ImGui::Indent(5.0f);
                            for (int b = 0; b < rule.numRings && b < 16; ++b) {
                                char bl[32];
                                std::snprintf(bl, sizeof(bl), "B%d##rb%d_%d", b, r, b);
                                if (ImGui::SliderFloat(bl, &rule.ringWeights[b], 0.0f, 1.0f, "%.3f"))
                                    ruleKernelDirty = true;
                            }
                            ImGui::Unindent(5.0f);
                        }

                        if (ruleKernelDirty && m_callbacks.onRuleKernelChanged)
                            m_callbacks.onRuleKernelChanged(r);

                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                ImGui::Unindent(10.0f);
            }
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader("Display", 9)) {
        const char* dispModes[] = {
            "World", "Neighbor Sums", "Growth Values", "Kernel", "Delta (Change)",
            "Vector Field", "Contour Lines", "Heat Map", "Activity Map", "Difference"
        };
        ImGui::Combo("Display Mode", &params.displayMode, dispModes, 10);
        Tooltip("Visualization modes:\n"
                "World: Cell states\n"
                "Neighbor Sums: Convolution potentials\n"
                "Growth: Growth rate field\n"
                "Kernel: Convolution kernel\n"
                "Delta: Change per step\n"
                "Vector Field: Gradient directions\n"
                "Contour Lines: Density levels\n"
                "Heat Map: Intensity heatmap\n"
                "Activity Map: Recent changes\n"
                "Difference: Frame-to-frame changes");
        
        if (params.displayMode == 5) {
            ImGui::SliderFloat("Vector Scale", &params.vectorFieldScale, 0.1f, 5.0f, "%.1f");
            Tooltip("Scale of vector arrows.");
            ImGui::SliderInt("Vector Density", &params.vectorFieldDensity, 5, 50);
            Tooltip("Number of vectors per axis.");
        }
        
        if (params.displayMode == 6) {
            ImGui::SliderInt("Contour Levels", &params.contourLevels, 2, 30);
            Tooltip("Number of contour lines.");
            ImGui::SliderFloat("Line Thickness", &params.contourThickness, 0.5f, 3.0f, "%.1f");
            Tooltip("Thickness of contour lines.");
        }
        
        if (params.displayMode == 8) {
            ImGui::SliderFloat("Activity Decay", &params.activityDecay, 0.8f, 0.999f, "%.3f");
            Tooltip("How quickly activity fades (higher = longer trails).");
        }

        std::vector<std::string> cmapList = {
            "Lenia (Custom)", "Viridis", "Magma", "Inferno",
            "Plasma", "Grayscale", "Grayscale Inv.", "Jet"
        };
        for (auto& cn : m_customColormapNames)
            cmapList.push_back(cn);

        auto cmapGetter = [](void* data, int idx, const char** out) -> bool {
            auto* list = static_cast<std::vector<std::string>*>(data);
            if (idx < 0 || idx >= static_cast<int>(list->size())) return false;
            *out = (*list)[idx].c_str();
            return true;
        };
        ImGui::Combo("Colormap", &params.colormapMode, cmapGetter,
                     &cmapList, static_cast<int>(cmapList.size()));
        Tooltip("Color mapping applied to single-channel state values. In RGB multi-channel mode, raw channel values are used as R/G/B directly.");

        drawColorbar(params);

        if (params.numChannels > 1) {
            ImGui::Separator();
            ImGui::Checkbox("Use Colormap for Multichannel##useCmapMC", &params.useColormapForMultichannel);
            Tooltip("Convert RGB channels to a single luminance value and apply the colormap.");
            
            if (params.useColormapForMultichannel) {
                const char* blendModes[] = {"Luminance (Weighted)", "Average", "Max Channel", "Min Channel", "Red Only", "Green Only", "Blue Only"};
                ImGui::Combo("Blend Mode##mcBlend", &params.multiChannelBlend, blendModes, 7);
                Tooltip("How to combine RGB channels into a single value for colormap:\n- Luminance: Standard perceptual weighting\n- Average: Equal weights\n- Max/Min: Brightest/darkest channel\n- Single: Use one channel only");
                
                if (params.multiChannelBlend == 0) {
                    ImGui::Text("Channel Weights:");
                    ImGui::SliderFloat("R Weight##wR", &params.channelWeightR, 0.0f, 2.0f, "%.2f");
                    ImGui::SliderFloat("G Weight##wG", &params.channelWeightG, 0.0f, 2.0f, "%.2f");
                    ImGui::SliderFloat("B Weight##wB", &params.channelWeightB, 0.0f, 2.0f, "%.2f");
                    Tooltip("Custom weights for luminance calculation. Standard is R=0.299, G=0.587, B=0.114");
                    if (ImGui::Button("Reset Weights##resetW")) {
                        params.channelWeightR = 0.299f;
                        params.channelWeightG = 0.587f;
                        params.channelWeightB = 0.114f;
                    }
                }
            }
        }

        ImGui::Separator();
        SliderFloatWithInput("Zoom (+/-)", &params.zoom, 0.1f, 20.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 20.0f, r, 1, nullptr, 0); }
        Tooltip("Viewport zoom level. Use +/- keys or mouse scroll wheel. 1.0 = 1:1 pixel mapping.");
        SliderFloatWithInput("Pan X", &params.panX, -2.0f, 2.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(-2.0f, 2.0f, r, 1, nullptr, 0); snapFloat(params.panX, -2.0f, 2.0f, r, 1); }
        Tooltip("Horizontal viewport offset. 0 = centered. Use arrow keys to pan.");
        SliderFloatWithInput("Pan Y", &params.panY, -2.0f, 2.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(-2.0f, 2.0f, r, 1, nullptr, 0); snapFloat(params.panY, -2.0f, 2.0f, r, 1); }
        Tooltip("Vertical viewport offset. 0 = centered. Use arrow keys to pan.");

        float halfBtnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;
        if (ImGui::Button("Reset View (Home)", ImVec2(halfBtnW, 22))) {
            params.zoom = 1.0f;
            params.panX = 0.0f;
            params.panY = 0.0f;
        }
        Tooltip("Reset zoom to 1.0 and center the view.");
        ImGui::SameLine();
        if (ImGui::Button("Center View", ImVec2(halfBtnW, 22))) {
            params.panX = 0.0f;
            params.panY = 0.0f;
        }
        Tooltip("Center the view without changing zoom.");

        ImGui::Separator();
        SliderFloatWithInput("Brightness", &params.brightness, 0.0f, 1.5f, "%.2f");
        { float r[] = {0.5f}; drawSliderMarkers(0.0f, 1.5f, r, 1, nullptr, 0); snapFloat(params.brightness, 0.0f, 1.5f, r, 1); }
        Tooltip("Additive brightness offset applied after colormap. 0.5 is the default.");
        SliderFloatWithInput("Contrast", &params.contrast, 0.1f, 5.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 5.0f, r, 1, nullptr, 0); snapFloat(params.contrast, 0.1f, 5.0f, r, 1); }
        Tooltip("Multiplicative contrast scaling. 1.0 = no change. Higher values increase the difference between bright and dark areas.");
        SliderFloatWithInput("Gamma", &params.gamma, 0.1f, 5.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 5.0f, r, 1, nullptr, 0); snapFloat(params.gamma, 0.1f, 5.0f, r, 1); }
        Tooltip("Gamma correction exponent. 1.0 = linear. <1 brightens midtones, >1 darkens midtones.");

        ImGui::Separator();
        const char* filterNames[] = {"Bilinear", "Nearest", "Sharpen"};
        ImGui::Combo("Filter Mode", &params.filterMode, filterNames, 3);
        Tooltip("Texture filtering mode. Bilinear = smooth interpolation. Nearest = pixelated look. Sharpen = enhanced edges.");
        SliderFloatWithInput("Edge Detect", &params.edgeStrength, 0.0f, 1.0f, "%.2f");
        { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.edgeStrength, 0.0f, 1.0f, r, 1); }
        Tooltip("Strength of Sobel edge detection overlay. 0 = off. Highlights boundaries between high and low values.");
        
        if (ImGui::CollapsingHeader("Glow Settings")) {
            SliderFloatWithInput("Glow Strength", &params.glowStrength, 0.0f, 1.0f, "%.2f");
            { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.glowStrength, 0.0f, 1.0f, r, 1); }
            Tooltip("Bloom/glow effect intensity. 0 = off.");
            
            if (params.glowStrength > 0.0f) {
                float gc[3] = {params.glowR, params.glowG, params.glowB};
                if (ImGui::ColorEdit3("Glow Tint", gc)) {
                    params.glowR = gc[0];
                    params.glowG = gc[1];
                    params.glowB = gc[2];
                }
                Tooltip("Color tint for the glow effect.");
                
                SliderFloatWithInput("Glow Intensity", &params.glowIntensity, 0.5f, 3.0f, "%.2f");
                { float r[] = {1.0f}; drawSliderMarkers(0.5f, 3.0f, r, 1, nullptr, 0); snapFloat(params.glowIntensity, 0.5f, 3.0f, r, 1); }
                Tooltip("Multiplier for glow brightness.");
            }
        }
        
        if (ImGui::CollapsingHeader("Custom Gradient")) {
            ImGui::SliderInt("Gradient Stops", &params.gradientStops, 2, 5);
            Tooltip("Number of color stops in the gradient mapping.");
            
            for (int i = 0; i < params.gradientStops; ++i) {
                char label[32];
                std::snprintf(label, sizeof(label), "Stop %d", i + 1);
                float* col = &params.gradientColors[i * 3];
                ImGui::ColorEdit3(label, col, ImGuiColorEditFlags_NoInputs);
                if (i < params.gradientStops - 1) ImGui::SameLine();
            }
            Tooltip("Colors for custom gradient mapping. Applied when using custom colormap.");
        }

        ImGui::Separator();
        ImGui::Checkbox("Grid Overlay", &params.showGrid);
        Tooltip("Draw a grid overlay on the simulation view at cell boundaries (visible when zoomed in).");
        if (params.showGrid) {
            SliderFloatWithInput("Grid Opacity", &params.gridOpacity, 0.0f, 1.0f, "%.2f");
            Tooltip("Opacity of the grid overlay lines.");

            float glc[3] = {params.gridLineR, params.gridLineG, params.gridLineB};
            if (ImGui::ColorEdit3("Grid Color", glc)) {
                params.gridLineR = glc[0]; params.gridLineG = glc[1]; params.gridLineB = glc[2];
            }
            Tooltip("Color of grid overlay lines.");

            SliderFloatWithInput("Line Thickness", &params.gridLineThickness, 0.1f, 5.0f, "%.1f");
            { float r[] = {1.0f}; drawSliderMarkers(0.1f, 5.0f, r, 1, nullptr, 0); snapFloat(params.gridLineThickness, 0.1f, 5.0f, r, 1); }
            Tooltip("Thickness of grid lines. 1.0 = standard width.");

            const char* spacingModes[] = {"Every Cell", "Custom Interval"};
            ImGui::Combo("Grid Spacing", &params.gridSpacingMode, spacingModes, 2);
            Tooltip("Every Cell: lines at each cell boundary.\nCustom Interval: lines every N cells.");

            if (params.gridSpacingMode == 1) {
                SliderIntWithInput("Interval", &params.gridCustomSpacing, 1, 100);
                Tooltip("Draw grid lines every N cells.");
            }

            ImGui::Checkbox("Major Lines", &params.gridMajorLines);
            Tooltip("Draw thicker accent lines at regular intervals.");
            if (params.gridMajorLines) {
                SliderIntWithInput("Major Every", &params.gridMajorEvery, 2, 50);
                Tooltip("Draw a major grid line every N minor intervals.");
                SliderFloatWithInput("Major Opacity", &params.gridMajorOpacity, 0.0f, 1.0f, "%.2f");
                Tooltip("Opacity of major grid lines.");
            }
        }

        ImGui::Separator();
        ImGui::Checkbox("Invert Colors", &params.invertColors);
        Tooltip("Invert all output colors (1 - color).");

        ImGui::Checkbox("Show Boundary", &params.showBoundary);
        Tooltip("Draw lines at the edges of the simulation grid.");
        if (params.showBoundary) {
            float bc[3] = {params.boundaryR, params.boundaryG, params.boundaryB};
            if (ImGui::ColorEdit3("Boundary Color##bcolor", bc)) {
                params.boundaryR = bc[0]; params.boundaryG = bc[1]; params.boundaryB = bc[2];
            }
            SliderFloatWithInput("Boundary Opacity##bopacity", &params.boundaryOpacity, 0.0f, 1.0f, "%.2f");
            
            const char* boundaryStyles[] = {"Solid", "Dashed", "Dotted", "Double", "Glow"};
            ImGui::Combo("Boundary Style##bstyle", &params.boundaryStyle, boundaryStyles, 5);
            Tooltip("Style of the boundary lines.");
            
            SliderFloatWithInput("Boundary Width##bwidth", &params.boundaryThickness, 0.5f, 10.0f, "%.1f");
            Tooltip("Thickness of boundary lines.");
            
            if (params.boundaryStyle == 1 || params.boundaryStyle == 2) {
                SliderFloatWithInput("Dash Length##bdash", &params.boundaryDashLength, 2.0f, 30.0f, "%.0f");
                Tooltip("Length of dashes/dots.");
            }
            
            ImGui::Checkbox("Animate Boundary##banim", &params.boundaryAnimate);
            Tooltip("Animate the boundary with a marching ants effect.");
        }

        float bg[3] = {params.bgR, params.bgG, params.bgB};
        if (ImGui::ColorEdit3("BG Color", bg)) {
            params.bgR = bg[0]; params.bgG = bg[1]; params.bgB = bg[2];
        }
        Tooltip("Background color visible when null cells are clipped or outside the grid.");

        ImGui::Checkbox("Clip Null Cells", &params.clipToZero);
        Tooltip("Replace cells below the threshold with the background color instead of colormapping them.");
        if (params.clipToZero) {
            SliderFloatWithInput("Clip Threshold", &params.clipThreshold, 0.0001f, 0.1f, "%.4f");
            Tooltip("Cells with values below this threshold will show the background color.");
        }

        ImGui::SeparatorText("Colormap Deformation");

        SliderFloatWithInput("Cmap Offset", &params.cmapOffset, 0.0f, 1.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.cmapOffset, 0.0f, 1.0f, r, 1); }
        Tooltip("Cyclically shift the colormap lookup. 0 = no shift, 0.5 = half rotation.");

        SliderFloatWithInput("Range Min", &params.cmapRange0, 0.0f, 1.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.cmapRange0, 0.0f, 1.0f, r, 1); }
        Tooltip("Remap: values below this are clamped to the start of the colormap.");

        SliderFloatWithInput("Range Max", &params.cmapRange1, 0.0f, 1.0f, "%.3f");
        { float r[] = {1.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.cmapRange1, 0.0f, 1.0f, r, 1); }
        Tooltip("Remap: values above this are clamped to the end of the colormap.");

        SliderFloatWithInput("Power Curve", &params.cmapPower, 0.1f, 5.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 5.0f, r, 1, nullptr, 0); snapFloat(params.cmapPower, 0.1f, 5.0f, r, 1); }
        Tooltip("Apply a power curve to the colormap input. 1.0 = linear. <1 = brighter midtones, >1 = darker midtones.");

        SliderFloatWithInput("Hue Shift", &params.cmapHueShift, 0.0f, 1.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.cmapHueShift, 0.0f, 1.0f, r, 1); }
        Tooltip("Rotate the hue of the output color in HSV space.");

        SliderFloatWithInput("Saturation", &params.cmapSaturation, 0.0f, 3.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.0f, 3.0f, r, 1, nullptr, 0); snapFloat(params.cmapSaturation, 0.0f, 3.0f, r, 1); }
        Tooltip("Scale saturation of the output color. 0 = grayscale, 1 = original, >1 = oversaturated.");

        ImGui::Checkbox("Reverse Colormap", &params.cmapReverse);
        Tooltip("Reverse the direction of the colormap lookup (1 becomes 0 and vice versa).");

        if (ImGui::Button("Reset Colormap Deformation", ImVec2(-1, 22))) {
            params.cmapOffset = 0.0f;
            params.cmapRange0 = 0.0f;
            params.cmapRange1 = 1.0f;
            params.cmapPower = 1.0f;
            params.cmapHueShift = 0.0f;
            params.cmapSaturation = 1.0f;
            params.cmapReverse = false;
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader("Analysis", 10)) {
        ImGui::Checkbox("Enable Analysis", &params.showAnalysis);
        Tooltip("Compute live statistics about the simulation state using a GPU analysis shader.");
        ImGui::SameLine();
        ImGui::Checkbox("Auto-Pause", &params.autoPause);
        Tooltip("Automatically pause when the simulation is detected as empty or stabilized.");

        SliderFloatWithInput("Alive Threshold", &params.analysisThreshold, 0.0001f, 0.5f, "%.4f");
        Tooltip("Minimum cell value to be counted as 'alive'. Used for alive cell count, stabilization, and empty detection.");

        if (analysis && params.showAnalysis) {
            ImGui::Separator();
            ImGui::Text("Total Mass: %.2f", analysis->totalMass);
            ImGui::Text("Alive Cells: %d / %d (%.1f%%)",
                        analysis->aliveCount, analysis->totalPixels,
                        analysis->totalPixels > 0
                            ? 100.0f * analysis->aliveCount / analysis->totalPixels
                            : 0.0f);
            ImGui::Text("Average: %.4f", analysis->avgVal);
            ImGui::Text("Min: %.4f  Max: %.4f", analysis->minVal, analysis->maxVal);
            ImGui::Text("Variance: %.6f", analysis->variance);
            ImGui::Text("Centroid: (%.1f, %.1f)", analysis->centroidX, analysis->centroidY);
            ImGui::Text("Bounds: (%.0f,%.0f)-(%.0f,%.0f)",
                        analysis->boundMinX, analysis->boundMinY,
                        analysis->boundMaxX, analysis->boundMaxY);

            if (analysisMgr) {
                ImGui::Separator();
                if (analysisMgr->isEmpty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "State: EMPTY");
                } else if (analysisMgr->isStabilized()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "State: STABILIZED");
                } else if (analysisMgr->isPeriodic()) {
                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f),
                        "State: PERIODIC (T=%d, conf=%.0f%%)",
                        analysisMgr->detectedPeriod(),
                        analysisMgr->periodConfidence() * 100.0f);
                } else {
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "State: Active");
                }

                ImGui::Separator();
                ImGui::Text("Species (est): %d", analysisMgr->speciesCount());
                ImGui::Text("Speed: %.3f cells/step", analysisMgr->movementSpeed());
                ImGui::Text("Direction: %.1f deg", analysisMgr->movementDirection());
                ImGui::Text("Orientation: %.1f deg", analysisMgr->orientation());
            }

            if (analysisMgr && analysisMgr->historyCount() > 1) {
                ImGui::Separator();
                ImGui::Text("Graphs");

                ImGui::Checkbox("Mass", &params.showMassGraph);
                ImGui::SameLine();
                ImGui::Checkbox("Alive", &params.showAliveGraph);
                ImGui::SameLine();
                ImGui::Checkbox("Centroid", &params.showCentroidGraph);
                ImGui::Checkbox("Speed", &params.showSpeedGraph);
                ImGui::SameLine();
                ImGui::Checkbox("Direction", &params.showDirectionGraph);

                SliderIntWithInput("Display Window", &params.graphTimeWindow, 0, AnalysisManager::HISTORY_SIZE);
                Tooltip("Number of history steps to display. 0 = show all available data.");

                SliderFloatWithInput("Graph Height", &params.graphHeight, 50.0f, 200.0f, "%.0f");

                ImGui::Checkbox("Auto Y Scale", &params.graphAutoScale);
                Tooltip("Automatically scale Y axis to fit visible data.");

                int fullCount = analysisMgr->historyCount();
                int head = analysisMgr->historyHead();
                int dispCount = (params.graphTimeWindow > 0 && params.graphTimeWindow < fullCount)
                                ? params.graphTimeWindow : fullCount;
                int startOff = fullCount - dispCount;

                if (params.showMassGraph) {
                    float massPlot[512];
                    float mMin = FLT_MAX, mMax = -FLT_MAX;
                    for (int i = 0; i < dispCount; ++i) {
                        int idx = (head - fullCount + startOff + i + AnalysisManager::HISTORY_SIZE) % AnalysisManager::HISTORY_SIZE;
                        massPlot[i] = analysisMgr->massHistory(idx);
                        if (massPlot[i] < mMin) mMin = massPlot[i];
                        if (massPlot[i] > mMax) mMax = massPlot[i];
                    }
                    float yMin = 0.0f;
                    float yMax = mMax > 0.0f ? mMax * 1.1f : 1.0f;
                    if (!params.graphAutoScale && params.graphMassMax > 0.0f) {
                        yMax = params.graphMassMax;
                    } else if (mMax > 0.0f) {
                        params.graphMassMax = mMax * 1.1f;
                    }

                    drawGraphWithAxes("Mass", massPlot, dispCount, yMin, yMax, "step", "mass", params.graphHeight, IM_COL32(100, 220, 150, 230));

                    if (analysisMgr->isPeriodic()) {
                        ImVec2 pMin = ImGui::GetItemRectMin();
                        ImVec2 pMax = ImGui::GetItemRectMax();
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        float plotW = pMax.x - pMin.x;
                        int period = analysisMgr->detectedPeriod();
                        if (period > 0 && dispCount > 0) {
                            for (int t = period; t < dispCount; t += period) {
                                float fx = pMin.x + (static_cast<float>(t) / static_cast<float>(dispCount)) * plotW;
                                dl->AddLine(ImVec2(fx, pMin.y), ImVec2(fx, pMax.y),
                                            IM_COL32(100, 180, 255, 80), 1.0f);
                            }
                        }
                    }
                }

                if (params.showAliveGraph) {
                    float alivePlot[512];
                    float aMin = FLT_MAX, aMax = -FLT_MAX;
                    for (int i = 0; i < dispCount; ++i) {
                        int idx = (head - fullCount + startOff + i + AnalysisManager::HISTORY_SIZE) % AnalysisManager::HISTORY_SIZE;
                        alivePlot[i] = static_cast<float>(analysisMgr->aliveHistory(idx));
                        if (alivePlot[i] < aMin) aMin = alivePlot[i];
                        if (alivePlot[i] > aMax) aMax = alivePlot[i];
                    }
                    float yMin = 0.0f;
                    float yMax = aMax > 0.0f ? aMax * 1.1f : 1.0f;
                    if (!params.graphAutoScale && params.graphAliveMax > 0.0f) {
                        yMax = params.graphAliveMax;
                    } else if (aMax > 0.0f) {
                        params.graphAliveMax = aMax * 1.1f;
                    }

                    drawGraphWithAxes("Alive Cells", alivePlot, dispCount, yMin, yMax, "step", "cells", params.graphHeight, IM_COL32(220, 180, 100, 230));
                }

                if (params.showCentroidGraph) {
                    float cxPlot[512], cyPlot[512];
                    for (int i = 0; i < dispCount; ++i) {
                        int idx = (head - fullCount + startOff + i + AnalysisManager::HISTORY_SIZE) % AnalysisManager::HISTORY_SIZE;
                        cxPlot[i] = analysisMgr->centroidXHistory(idx);
                        cyPlot[i] = analysisMgr->centroidYHistory(idx);
                    }
                    drawGraphWithAxes("Centroid X", cxPlot, dispCount, 0.0f, static_cast<float>(params.gridW), "step", "x", params.graphHeight, IM_COL32(150, 200, 255, 230));
                    drawGraphWithAxes("Centroid Y", cyPlot, dispCount, 0.0f, static_cast<float>(params.gridH), "step", "y", params.graphHeight, IM_COL32(255, 150, 200, 230));
                }

                if (params.showSpeedGraph) {
                    float spdPlot[512];
                    float sMin = FLT_MAX, sMax = -FLT_MAX;
                    for (int i = 0; i < dispCount; ++i) {
                        int idx = (head - fullCount + startOff + i + AnalysisManager::HISTORY_SIZE) % AnalysisManager::HISTORY_SIZE;
                        spdPlot[i] = analysisMgr->speedHistory(idx);
                        if (spdPlot[i] < sMin) sMin = spdPlot[i];
                        if (spdPlot[i] > sMax) sMax = spdPlot[i];
                    }
                    float yMax = sMax > 0.001f ? sMax * 1.1f : 1.0f;
                    drawGraphWithAxes("Speed", spdPlot, dispCount, 0.0f, yMax, "step", "px/s", params.graphHeight, IM_COL32(255, 180, 100, 230));
                }

                if (params.showDirectionGraph) {
                    float dirPlot[512];
                    for (int i = 0; i < dispCount; ++i) {
                        int idx = (head - fullCount + startOff + i + AnalysisManager::HISTORY_SIZE) % AnalysisManager::HISTORY_SIZE;
                        dirPlot[i] = analysisMgr->directionHistory(idx);
                    }
                    drawGraphWithAxes("Direction", dirPlot, dispCount, -180.0f, 180.0f, "step", "deg", params.graphHeight, IM_COL32(200, 150, 255, 230));
                }
            }
        }
    }
    popSectionColor();

    ImGui::End();
}

void UIOverlay::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIOverlay::shutdown() {
    if (!m_initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

}
