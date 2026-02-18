#include "UIOverlay.hpp"
#include "LeniaEngine.hpp"
#include "Presets.hpp"
#include "AnalysisManager.hpp"
#include "Localization.hpp"
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
    (void)windowH;
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
    m_window = window;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    float xscale = 1.0f, yscale = 1.0f;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor) {
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    }
    m_dpiScale = xscale;
    
    loadAccessibilitySettings();
    applyKeyboardNavigationSettings();
    
    float effectiveScale = m_dpiScale * m_accessibilitySettings.uiScale;
    
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding    = 4.0f;
    style.GrabRounding     = 3.0f;
    style.WindowRounding   = 6.0f;
    style.ScrollbarSize    = 14.0f * effectiveScale;
    style.TabRounding      = 4.0f;
    style.IndentSpacing    = 16.0f;
    style.WindowBorderHoverPadding = std::max(1.0f, style.WindowBorderHoverPadding);
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
    
    style.ScaleAllSizes(effectiveScale);
    style.WindowBorderHoverPadding = std::max(1.0f, style.WindowBorderHoverPadding);
    m_lastStyleScale = effectiveScale;
    
    float fontSize = m_accessibilitySettings.fontSize * effectiveScale;
    io.Fonts->Clear();
    io.Fonts->AddFontDefault();
    m_defaultFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", fontSize);
    if (!m_defaultFont) {
        ImFontConfig config;
        config.SizePixels = fontSize;
        m_defaultFont = io.Fonts->AddFontDefault(&config);
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    
    if (m_accessibilitySettings.highContrast) {
        applyHighContrastTheme(true);
    }
    
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
        ImGui::TextDisabled(TR(GrowthPlotGoLHint));
    } else if (params.growthType == 7) {
        ImGui::TextDisabled(TR(GrowthPlotAsymptoticHint), params.mu, params.sigma);
    } else if (params.growthType == 8) {
        ImGui::TextDisabled(TR(GrowthPlotSoftClipHint), params.mu, params.sigma);
    } else if (params.growthType == 9) {
        ImGui::TextDisabled(TR(GrowthPlotLTLHint), params.mu, params.sigma);
    } else {
        ImGui::TextDisabled(TR(GrowthPlotDefaultHint), params.mu, params.sigma);
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
    ImGui::TextDisabled(TR(KernelCrossSectionWithSize), kernelDiam, kernelDiam);
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
        ImGui::TextDisabled(TR(DisplayRGBChannelIntensity));
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

    ImGui::Begin(TR(AppTitle));

    if (paused != m_lastPausedState) {
        triggerPauseOverlay(paused);
        m_lastPausedState = paused;
    }

    int sec = 0;

    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionInfo), 0)) {
        if (mouseInGrid) {
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), TR(InfoCursor), mouseGridX, mouseGridY);
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.5f, 1.0f), TR(InfoValue), mouseValue);
            ImGui::Separator();
        }

        ImGui::Text(TR(InfoGrid), params.gridW, params.gridH, stepCount);
        ImGui::Text(TR(InfoChannels), params.numChannels, params.numKernelRules);

        ImGui::Separator();
        if (ImGui::Checkbox(TR(InfoShowConsoleStartup), &params.showConsoleOnStartup)) {
            std::ofstream cfg("lenia_config.txt");
            if (cfg.is_open()) {
                cfg << "showConsole=" << (params.showConsoleOnStartup ? "1" : "0") << "\n";
                cfg.close();
            }
        }
        Tooltip(TR(InfoShowConsoleTooltip));

        ImGui::SeparatorText(TR(KeybindsHeader));
        ImGui::TextWrapped(TR(KeybindsText));

        ImGui::Separator();
        if (ImGui::TreeNode(TR(TheoryHeader))) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.95f, 1.0f));

            ImGui::SeparatorText(TR(TheoryFundamentals));
            ImGui::TextWrapped(TR(TheoryFundamentalsText));
            ImGui::Spacing();
            ImGui::TextWrapped(TR(TheoryEquation));

            ImGui::SeparatorText(TR(TheoryKernel));
            ImGui::TextWrapped(TR(TheoryKernelText));

            ImGui::SeparatorText(TR(TheoryGrowthFunction));
            ImGui::TextWrapped(TR(TheoryGrowthFunctionText));

            ImGui::SeparatorText(TR(TheoryTimeIntegration));
            ImGui::TextWrapped(TR(TheoryTimeIntegrationText));

            ImGui::SeparatorText(TR(TheoryMultiChannel));
            ImGui::TextWrapped(TR(TheoryMultiChannelText));

            ImGui::SeparatorText(TR(TheoryEdgeConditions));
            ImGui::TextWrapped(TR(TheoryEdgeConditionsText));

            ImGui::SeparatorText(TR(TheoryWalls));
            ImGui::TextWrapped(TR(TheoryWallsText));

            ImGui::SeparatorText(TR(TheoryPatternCharacteristics));
            ImGui::TextWrapped(TR(TheoryPatternCharacteristicsText));

            ImGui::SeparatorText(TR(TheoryParameterRelationships));
            ImGui::TextWrapped(TR(TheoryParameterRelationshipsText));

            ImGui::SeparatorText(TR(TheoryColormapVisualization));
            ImGui::TextWrapped(TR(TheoryColormapVisualizationText));

            ImGui::PopStyleColor();
            ImGui::TreePop();
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionPerformance), 1, true)) {
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

        ImGui::TextColored(fpsColor, TR(PerfFPS), fps);
        Tooltip(TR(PerfFPSTooltip));
        ImGui::SameLine();
        ImGui::Text(TR(PerfFrameTime), avgFt);

        ImGui::Separator();
        ImGui::Text(TR(PerfFrameTimeLabel));
        ImGui::SameLine(120);
        ImGui::TextDisabled(TR(PerfFrameTimeStats), minFt, avgFt, maxFt);

        int totalCells = params.gridW * params.gridH;
        ImGui::Text(TR(PerfGridSize));
        ImGui::SameLine(120);
        if (totalCells >= 1000000)
            ImGui::Text(TR(PerfGridSizeCellsM), params.gridW, params.gridH, totalCells / 1000000.0f);
        else
            ImGui::Text(TR(PerfGridSizeCellsK), params.gridW, params.gridH, totalCells / 1000.0f);

        float simMsPerStep = simTimeMs / std::max(1, stepsPerFrame);
        ImGui::Text(TR(PerfSimulation));
        ImGui::SameLine(120);
        ImGui::Text(TR(PerfSimTimeStep), simMsPerStep, simTimeMs);

        float cellsPerSec = static_cast<float>(totalCells * stepsPerFrame) / std::max(0.001f, simTimeMs / 1000.0f);
        ImGui::Text(TR(PerfThroughput));
        ImGui::SameLine(120);
        if (cellsPerSec >= 1e9f)
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), TR(PerfThroughputG), cellsPerSec / 1e9f);
        else if (cellsPerSec >= 1e6f)
            ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), TR(PerfThroughputM), cellsPerSec / 1e6f);
        else
            ImGui::Text(TR(PerfThroughputK), cellsPerSec / 1e3f);
        Tooltip(TR(PerfThroughputTooltip));

        int kernelCells = (params.radius * 2 + 1) * (params.radius * 2 + 1);
        long long opsPerStep = static_cast<long long>(totalCells) * kernelCells;
        ImGui::Text(TR(PerfKernelOps));
        ImGui::SameLine(120);
        if (opsPerStep >= 1e9)
            ImGui::Text(TR(PerfKernelOpsG), opsPerStep / 1e9);
        else
            ImGui::Text(TR(PerfKernelOpsM), opsPerStep / 1e6);
        Tooltip(TR(PerfKernelOpsTooltip));

        ImGui::Text(TR(PerfKernelSize));
        ImGui::SameLine(120);
        ImGui::Text(TR(PerfKernelSizeSamples), params.radius * 2 + 1, params.radius * 2 + 1, kernelCells);

        ImGui::Text(TR(PerfStepsFrame));
        ImGui::SameLine(120);
        ImGui::Text("%d", stepsPerFrame);

        ImGui::Text(TR(PerfTotalSteps));
        ImGui::SameLine(120);
        ImGui::Text("%d", stepCount);

        ImGui::Separator();
        const char* perfLevel;
        ImVec4 perfColor;
        if (fps >= 55.0f && simTimeMs < 16.0f) {
            perfLevel = TR(PerfExcellent); perfColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        } else if (fps >= 30.0f && simTimeMs < 33.0f) {
            perfLevel = TR(PerfGood); perfColor = ImVec4(0.7f, 1.0f, 0.3f, 1.0f);
        } else if (fps >= 15.0f) {
            perfLevel = TR(PerfAcceptable); perfColor = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
        } else {
            perfLevel = TR(PerfSlow); perfColor = ImVec4(1.0f, 0.4f, 0.2f, 1.0f);
        }
        ImGui::TextColored(perfColor, TR(PerfPerformance), perfLevel);
        Tooltip(TR(PerfPerformanceTooltip));

        if (m_frameTimeCount > 1) {
            float ftPlot[120];
            for (int i = 0; i < m_frameTimeCount; ++i) {
                int idx = (m_frameTimeHead - m_frameTimeCount + i + 120) % 120;
                ftPlot[i] = m_frameTimeHistory[idx];
            }
            drawGraphWithAxes(TR(PerfFrameTimeGraphTitle), ftPlot, m_frameTimeCount, 0.0f, maxFt * 1.2f,
                              TR(PerfFrameTimeGraphXLabel), TR(PerfFrameTimeGraphYLabel), 70.0f,
                              IM_COL32(100, 200, 255, 220));
        }
        
        ImGui::Spacing();
        ImGui::Checkbox(TR(PerfShowResourceMonitor), &params.showResourceMonitor);
        if (params.showResourceMonitor) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), TR(PerfResourceUsage));
            
            if (params.gpuMemoryTotalMB > 0) {
                float memPercent = static_cast<float>(params.gpuMemoryUsedMB) / params.gpuMemoryTotalMB;
                ImVec4 memColor = (memPercent > 0.9f) ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) :
                                  (memPercent > 0.7f) ? ImVec4(1.0f, 0.8f, 0.3f, 1.0f) :
                                  ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
                ImGui::TextColored(memColor, TR(PerfGPUMemory), 
                    params.gpuMemoryUsedMB, params.gpuMemoryTotalMB, memPercent * 100.0f);
                ImGui::ProgressBar(memPercent, ImVec2(-1, 8), "");
            } else {
                ImGui::TextDisabled(TR(PerfGPUMemoryNA));
            }
            
            if (params.cpuMemoryUsedMB > 0.0f) {
                ImGui::Text(TR(PerfCPUMemory), params.cpuMemoryUsedMB);
            }
            
            int gridMemBytes = params.gridW * params.gridH * (params.numChannels > 1 ? 16 : 4) * 2;
            int kernelMemBytes = (params.radius * 2) * (params.radius * 2) * 4;
            float totalTexMB = (gridMemBytes + kernelMemBytes) / (1024.0f * 1024.0f);
            ImGui::Text(TR(PerfTextureMemory), totalTexMB);
            Tooltip(TR(PerfTextureMemoryTooltip));
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionGrid), 2)) {
        bool gridDirty = false;
        int prevW = params.gridW, prevH = params.gridH;

        ImGui::Text(TR(GridSize),
                    params.gridW, params.gridH,
                    params.gridW * params.gridH > 1000000 ?
                        (std::to_string(params.gridW * params.gridH / 1000000) + "M").c_str() :
                        (std::to_string(params.gridW * params.gridH / 1000) + "K").c_str());

        std::string widthLabel = std::string(TR(GridWidth)) + "##grid";
        std::string heightLabel = std::string(TR(GridHeight)) + "##grid";
        ImGui::InputInt(widthLabel.c_str(),  &params.gridW, 64, 256);
        Tooltip(TR(GridWidthTooltip));
        ImGui::InputInt(heightLabel.c_str(), &params.gridH, 64, 256);
        Tooltip(TR(GridHeightTooltip));
        params.gridW = std::max(32, params.gridW);
        params.gridH = std::max(32, params.gridH);
        if (params.gridW != prevW || params.gridH != prevH) gridDirty = true;

        if (gridDirty && m_callbacks.onGridResized)
            m_callbacks.onGridResized();

        ImGui::Separator();
        ImGui::Text(TR(GridTransformations));

        float btnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 3) / 4.0f;

        std::string flipHLabel = std::string(TR(GridFlipHorizontal)) + "##fliph";
        std::string flipVLabel = std::string(TR(GridFlipVertical)) + "##flipv";
        std::string rotCWLabel = std::string(TR(GridRotateCW)) + "##rotcw";
        std::string rotCCWLabel = std::string(TR(GridRotateCCW)) + "##rotccw";
        if (ImGui::Button(flipHLabel.c_str(), ImVec2(btnW, 24))) {
            if (m_callbacks.onFlipHorizontal)
                m_callbacks.onFlipHorizontal(true);
        }
        Tooltip(TR(GridFlipHorizontalTooltip));
        ImGui::SameLine();
        if (ImGui::Button(flipVLabel.c_str(), ImVec2(btnW, 24))) {
            if (m_callbacks.onFlipVertical)
                m_callbacks.onFlipVertical(true);
        }
        Tooltip(TR(GridFlipVerticalTooltip));
        ImGui::SameLine();
        if (ImGui::Button(rotCWLabel.c_str(), ImVec2(btnW, 24))) {
            if (m_callbacks.onRotateGrid)
                m_callbacks.onRotateGrid(1);
        }
        Tooltip(TR(GridRotateCWTooltip));
        ImGui::SameLine();
        if (ImGui::Button(rotCCWLabel.c_str(), ImVec2(btnW, 24))) {
            if (m_callbacks.onRotateGrid)
                m_callbacks.onRotateGrid(-1);
        }
        Tooltip(TR(GridRotateCCWTooltip));

        ImGui::Separator();
        ImGui::Text(TR(GridEdgeConditions));

        const char* edgeModes[] = {TR(GridEdgePeriodic), TR(GridEdgeClamp), TR(GridEdgeMirror)};

        std::string edgeXLabel = std::string(TR(GridEdgeModeX)) + "##edgex";
        std::string edgeYLabel = std::string(TR(GridEdgeModeY)) + "##edgey";
        ImGui::Combo(edgeXLabel.c_str(), &params.edgeModeX, edgeModes, IM_ARRAYSIZE(edgeModes));
        Tooltip(TR(GridEdgeModeXTooltip));

        ImGui::Combo(edgeYLabel.c_str(), &params.edgeModeY, edgeModes, IM_ARRAYSIZE(edgeModes));
        Tooltip(TR(GridEdgeModeYTooltip));

        if (params.edgeModeX != 0 || params.edgeModeY != 0) {
            ImGui::Separator();
            ImGui::Text(TR(GridEdgeFade));

            if (params.edgeModeX != 0) {
                std::string fadeXLabel = std::string(TR(GridEdgeFadeX)) + "##xfade";
                ImGui::SliderFloat(fadeXLabel.c_str(), &params.edgeFadeX, 0.0f, 0.5f, "%.2f");
                Tooltip(TR(GridEdgeFadeXTooltip));
            }

            if (params.edgeModeY != 0) {
                std::string fadeYLabel = std::string(TR(GridEdgeFadeY)) + "##yfade";
                ImGui::SliderFloat(fadeYLabel.c_str(), &params.edgeFadeY, 0.0f, 0.5f, "%.2f");
                Tooltip(TR(GridEdgeFadeYTooltip));
            }

            ImGui::Separator();
            const char* displayEdgeModes[] = {TR(GridShowTiled), TR(GridBackgroundColor), TR(GridCheckerPattern)};
            std::string outsideLabel = std::string(TR(GridOutsideDisplay)) + "##dispedge";
            ImGui::Combo(outsideLabel.c_str(), &params.displayEdgeMode, displayEdgeModes, IM_ARRAYSIZE(displayEdgeModes));
            Tooltip(TR(GridOutsideDisplayTooltip));
        }

        ImGui::Spacing();
        ImGui::Separator();
        
        std::string infWorldLabel = std::string(TR(InfiniteWorldMode)) + "##infworld";
        if (ImGui::CollapsingHeader(infWorldLabel.c_str())) {
            std::string infEnableLabel = std::string(TR(InfiniteWorldEnable)) + "##infEnable";
            ImGui::Checkbox(infEnableLabel.c_str(), &params.infiniteWorldMode);
            Tooltip(TR(InfiniteWorldEnableTooltip));

            if (params.infiniteWorldMode) {
                params.edgeModeX = 0;
                params.edgeModeY = 0;
                
                ImGui::Spacing();
                
                ImGui::TextColored(ImVec4(0.5f, 0.9f, 1.0f, 1.0f), TR(InfiniteWorldSettings));
                
                const char* chunkSizes[] = {"64x64", "128x128", "256x256", "512x512"};
                int chunkIdx = 0;
                if (params.chunkSize == 128) chunkIdx = 1;
                else if (params.chunkSize == 256) chunkIdx = 2;
                else if (params.chunkSize == 512) chunkIdx = 3;
                std::string chunkSizeLabel = std::string(TR(InfiniteChunkSize)) + "##chunkSz";
                if (ImGui::Combo(chunkSizeLabel.c_str(), &chunkIdx, chunkSizes, 4)) {
                    int sizes[] = {64, 128, 256, 512};
                    params.chunkSize = sizes[chunkIdx];
                }
                Tooltip(TR(InfiniteChunkSizeTooltip));

                SliderIntWithInput((std::string(TR(InfiniteLoadRadius)) + "##loadRad").c_str(),
                                   &params.loadedChunksRadius, 1, 5);
                Tooltip(TR(InfiniteLoadRadiusTooltip));

                SliderIntWithInput((std::string(TR(InfiniteMaxChunks)) + "##maxCh").c_str(),
                                   &params.maxLoadedChunks, 9, 81);
                Tooltip(TR(InfiniteMaxChunksTooltip));

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.9f, 1.0f, 1.0f), TR(InfiniteNavigation));
                
                ImGui::Text(TR(InfiniteChunkPosition), params.viewChunkX, params.viewChunkY);
                ImGui::Text(TR(InfiniteWorldOffset), params.panX, params.panY);
                
                float navBtnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;
                
                ImGui::Dummy(ImVec2(navBtnW, 0)); ImGui::SameLine();
                std::string navNLabel = std::string(TR(InfiniteNavNorth)) + "##navN";
                if (ImGui::Button(navNLabel.c_str(), ImVec2(navBtnW, 24))) {
                    params.viewChunkY++;
                    params.panY = 0.0f;
                }
                ImGui::SameLine(); ImGui::Dummy(ImVec2(navBtnW, 0));
                
                std::string navWLabel = std::string(TR(InfiniteNavWest)) + "##navW";
                if (ImGui::Button(navWLabel.c_str(), ImVec2(navBtnW, 24))) {
                    params.viewChunkX--;
                    params.panX = 0.0f;
                }
                ImGui::SameLine();
                std::string navHomeLabel = std::string(TR(InfiniteHome)) + "##navHome";
                if (ImGui::Button(navHomeLabel.c_str(), ImVec2(navBtnW, 24))) {
                    params.viewChunkX = 0;
                    params.viewChunkY = 0;
                    params.panX = 0.0f;
                    params.panY = 0.0f;
                    params.zoom = 1.0f;
                }
                ImGui::SameLine();
                std::string navELabel = std::string(TR(InfiniteNavEast)) + "##navE";
                if (ImGui::Button(navELabel.c_str(), ImVec2(navBtnW, 24))) {
                    params.viewChunkX++;
                    params.panX = 0.0f;
                }
                
                ImGui::Dummy(ImVec2(navBtnW, 0)); ImGui::SameLine();
                std::string navSLabel = std::string(TR(InfiniteNavSouth)) + "##navS";
                if (ImGui::Button(navSLabel.c_str(), ImVec2(navBtnW, 24))) {
                    params.viewChunkY--;
                    params.panY = 0.0f;
                }
                
                Tooltip(TR(InfiniteNavigationTooltip));
                
                SliderFloatWithInput((std::string(TR(InfiniteExploreSpeed)) + "##explSpd").c_str(),
                                    &params.worldExploreSpeed, 0.1f, 5.0f, "%.1fx");
                Tooltip(TR(InfiniteExploreSpeedTooltip));

                std::string autoLoadLabel = std::string(TR(InfiniteAutoLoad)) + "##autoLoad";
                ImGui::Checkbox(autoLoadLabel.c_str(), &params.autoLoadChunks);
                Tooltip(TR(InfiniteAutoLoadTooltip));

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.9f, 1.0f, 1.0f), TR(InfiniteDisplayOptions));
                
                std::string showChunkGridLabel = std::string(TR(InfiniteShowChunkGrid)) + "##showChGrid";
                ImGui::Checkbox(showChunkGridLabel.c_str(), &params.chunkBoundaryVisible);
                Tooltip(TR(InfiniteShowChunkGridTooltip));

                SliderFloatWithInput((std::string(TR(InfiniteEdgeFade)) + "##edgeFade").c_str(),
                                    &params.chunkFadeDistance, 0.0f, 4.0f, "%.1f");
                Tooltip(TR(InfiniteEdgeFadeTooltip));

                const char* persistence[] = {TR(InfinitePersistenceNone), TR(InfinitePersistencePreserve), TR(InfinitePersistenceSeed)};
                std::string persistLabel = std::string(TR(InfinitePersistence)) + "##persist";
                ImGui::Combo(persistLabel.c_str(), &params.chunkPersistence, persistence, 3);
                Tooltip(TR(InfinitePersistenceTooltip));
                
                ImGui::Spacing();
                ImGui::TextDisabled(TR(InfinitePanTip));
                ImGui::TextDisabled(TR(InfiniteScrollTip));
            }
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionDrawingTools), 3)) {
        const char* toolModes[] = {TR(DrawToolBrush), TR(DrawToolObstacle)};
        int toolMode = params.wallEnabled ? 1 : 0;
        if (ImGui::Combo(TR(DrawToolMode), &toolMode, toolModes, 2)) {
            params.wallEnabled = (toolMode == 1);
            params.brushEnabled = true;
        }
        Tooltip(TR(DrawToolModeTooltip));

        ImGui::Checkbox(TR(DrawEnableDrawing), &params.brushEnabled);
        Tooltip(TR(DrawEnableDrawingTooltip));

        if (params.brushEnabled) {
            ImGui::Separator();
            
            if (params.wallEnabled) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
                ImGui::Text(TR(DrawObstacleModeActive));
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
                ImGui::Text(TR(DrawBrushModeActive));
                ImGui::PopStyleColor();
            }

            if (ImGui::CollapsingHeader(TR(DrawShapeSize), ImGuiTreeNodeFlags_DefaultOpen)) {
                const char* shapeNames[] = {
                    TR(DrawShapeCircle), TR(DrawShapeSquare), TR(DrawShapeDiamond), TR(DrawShapeRing),
                    TR(DrawShapeStar5), TR(DrawShapeStar6), TR(DrawShapeHexagon), TR(DrawShapeCross),
                    TR(DrawShapePlus), TR(DrawShapeGaussian), TR(DrawShapeNoiseDisc), TR(DrawShapeGradientDisc)
                };
                int shapeIdx = params.wallEnabled ? params.wallShape : params.brushShape;
                if (ImGui::Combo(TR(DrawShape), &shapeIdx, shapeNames, IM_ARRAYSIZE(shapeNames))) {
                    if (params.wallEnabled)
                        params.wallShape = shapeIdx;
                    else
                        params.brushShape = shapeIdx;
                }
                Tooltip(TR(DrawShapeTooltip));

                ImGui::SliderInt(TR(DrawSize), &params.brushSize, 1, 100);
                Tooltip(TR(DrawSizeTooltip));
                
                params.wallThickness = static_cast<float>(params.brushSize);

                if (!params.wallEnabled) {
                    ImGui::SliderFloat(TR(DrawFalloff), &params.brushFalloff, 0.0f, 1.0f, "%.2f");
                    Tooltip(TR(DrawFalloffTooltip));
                } else {
                    ImGui::SliderFloat(TR(DrawFalloff), &params.wallFalloff, 0.0f, 1.0f, "%.2f");
                    Tooltip(TR(DrawFalloffTooltip));
                }
            }

            if (ImGui::CollapsingHeader(TR(DrawMethod), ImGuiTreeNodeFlags_DefaultOpen)) {
                const char* drawModeNames[] = {TR(DrawModeFreehand), TR(DrawModeLine), TR(DrawModeCircle), TR(DrawModeRectangle)};
                ImGui::Combo(TR(DrawMethod), &params.brushDrawMode, drawModeNames, IM_ARRAYSIZE(drawModeNames));
                Tooltip(TR(DrawModeTooltip));

                if (params.brushDrawMode != 0) {
                    ImGui::Separator();
                    bool isDrawing = params.wallEnabled ? params.wallLineDrawing : params.brushLineDrawing;
                    if (isDrawing) {
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), TR(DrawDrawing));
                    } else {
                        ImGui::TextDisabled(TR(DrawClickToStart));
                    }
                }
            }

            if (params.wallEnabled) {
                if (ImGui::CollapsingHeader(TR(DrawObstacleSettings), ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::SliderFloat(TR(DrawCellValue), &params.wallValue, 0.0f, 1.0f, "%.3f");
                    Tooltip(TR(DrawCellValueTooltip));

                    ImGui::Separator();
                    float wallColor[4] = {params.wallR, params.wallG, params.wallB, params.wallA};
                    if (ImGui::ColorEdit4(TR(DrawDisplayColor), wallColor)) {
                        params.wallR = wallColor[0];
                        params.wallG = wallColor[1];
                        params.wallB = wallColor[2];
                        params.wallA = wallColor[3];
                    }
                    Tooltip(TR(DrawDisplayColorTooltip));

                    if (params.numChannels > 1) {
                        ImGui::Separator();
                        ImGui::Text(TR(DrawAffectedChannels));
                        std::string ch0Label = std::string(TR(CommonRed)) + " (Ch0)";
                        ImGui::Checkbox(ch0Label.c_str(), &params.wallAffectsCh0);
                        ImGui::SameLine();
                        if (params.numChannels >= 2) {
                            std::string ch1Label = std::string(TR(CommonGreen)) + " (Ch1)";
                            ImGui::Checkbox(ch1Label.c_str(), &params.wallAffectsCh1);
                            ImGui::SameLine();
                        }
                        if (params.numChannels >= 3) {
                            std::string ch2Label = std::string(TR(CommonBlue)) + " (Ch2)";
                            ImGui::Checkbox(ch2Label.c_str(), &params.wallAffectsCh2);
                        }
                        Tooltip(TR(DrawAffectedChannelsTooltip));
                    }

                    const char* wallBlendNames[] = {TR(DrawBlendReplace), TR(DrawBlendMax), TR(DrawBlendReplaceStronger), TR(DrawBlendBlend), TR(DrawBlendErase)};
                    ImGui::Combo(TR(DrawBlendMode), &params.wallBlendMode, wallBlendNames, IM_ARRAYSIZE(wallBlendNames));
                    Tooltip(TR(DrawBlendModeTooltip));
                }

                ImGui::Separator();
                if (ImGui::Button(TR(DrawClearAllObstacles), ImVec2(-1, 0))) {
                    if (m_callbacks.onClearWalls)
                        m_callbacks.onClearWalls();
                }
                Tooltip(TR(DrawClearAllObstaclesTooltip));
            } else {
                if (ImGui::CollapsingHeader(TR(DrawBrushSettings), ImGuiTreeNodeFlags_DefaultOpen)) {
                    const char* modeNames[] = {TR(DrawPaintModeSet), TR(DrawPaintModeAdd), TR(DrawPaintModeSubtract), TR(DrawPaintModeMax), TR(DrawPaintModeMin), TR(DrawPaintModeErase)};
                    ImGui::Combo(TR(DrawPaintMode), &params.brushMode, modeNames, IM_ARRAYSIZE(modeNames));
                    Tooltip(TR(DrawPaintModeTooltip));

                    ImGui::SliderFloat(TR(DrawBrushValue), &params.brushValue, 0.0f, 1.0f, "%.2f");
                    Tooltip(TR(DrawBrushValueTooltip));

                    ImGui::SliderFloat(TR(DrawStrength), &params.brushStrength, 0.0f, 2.0f, "%.2f");
                    Tooltip(TR(DrawStrengthTooltip));

                    if (params.numChannels > 1) {
                        ImGui::Separator();
                        std::string chR = std::string(TR(CommonRed)) + " (Ch0)";
                        std::string chG = std::string(TR(CommonGreen)) + " (Ch1)";
                        std::string chB = std::string(TR(CommonBlue)) + " (Ch2)";
                        std::string chAll = std::string(TR(CommonAll)) + " " + TR(CommonChannel) + "s";
                        const char* channelNames[] = {chR.c_str(), chG.c_str(), chB.c_str(), chAll.c_str()};
                        int maxCh = (std::min)(params.numChannels, 3);
                        ImGui::Combo(TR(DrawTargetChannel), &params.brushChannel, channelNames, maxCh + 1);
                        Tooltip(TR(DrawTargetChannelTooltip));
                    }
                }

                if (ImGui::CollapsingHeader(TR(DrawSymmetry))) {
                    ImGui::Checkbox(TR(DrawMirrorX), &params.brushSymmetryX);
                    ImGui::SameLine();
                    ImGui::Checkbox(TR(DrawMirrorY), &params.brushSymmetryY);
                    Tooltip(TR(DrawMirrorTooltip));

                    ImGui::Checkbox(TR(DrawRadialSymmetry), &params.brushSymmetryRadial);
                    if (params.brushSymmetryRadial) {
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(80);
                        ImGui::SliderInt("##radialcount", &params.brushRadialCount, 2, 16);
                    }
                    Tooltip(TR(DrawRadialSymmetryTooltip));
                }
            }

            std::string spacingHeader = std::string(TR(DrawStrokeSpacing)) + "##brushSpacingHeader";
            if (ImGui::CollapsingHeader(spacingHeader.c_str())) {
                std::string spacingSlider = std::string(TR(DrawBrushSpacing)) + "##brushSpacingSlider";
                ImGui::SliderFloat(spacingSlider.c_str(), &params.brushSpacing, 0.1f, 5.0f, "%.1f");
                Tooltip(TR(DrawBrushSpacingTooltip));

                std::string smoothLabel = std::string(TR(DrawSmoothInterpolation)) + "##brushSmooth";
                ImGui::Checkbox(smoothLabel.c_str(), &params.brushSmooth);
                Tooltip(TR(DrawSmoothInterpolationTooltip));
            }
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionPresetsInit), 4, true)) {
        const auto& presets = getPresets();
        const auto& categories = getPresetCategories();

        std::vector<const char*> catPtrs;
        for (const auto& c : categories)
            catPtrs.push_back(c.c_str());

        ImGui::Combo(TR(PresetsCategory), &m_selectedCategory, catPtrs.data(),
                     static_cast<int>(catPtrs.size()));

        ImGui::InputTextWithHint("##search", TR(PresetsSearchHint), m_presetSearchBuf, sizeof(m_presetSearchBuf));

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
            ImGui::Text(TR(PresetsSelected), cp.name);

            float previewSize = 60.0f;
            ImGui::BeginGroup();
            ImGui::TextDisabled(TR(PresetsSpecies));
            drawPresetPreview(cp, previewSize, params);
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::TextDisabled(TR(PresetsKernel));
            drawKernelPreview(kernelTex, kernelDiam, previewSize);
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();
            bool hasCells = (cp.cellData != nullptr) || (cp.speciesFile != nullptr);
            ImGui::TextDisabled("%s", cp.category);
            ImGui::TextDisabled("%s", hasCells ? TR(PresetsSpecies) : TR(PresetsProcedural));
            ImGui::TextDisabled(TR(PresetsRadiusRings), cp.radius, cp.numRings);
            ImGui::TextDisabled(TR(PresetsMu), cp.mu);
            ImGui::TextDisabled(TR(PresetsSigma), cp.sigma);
            ImGui::EndGroup();
        }

        ImGui::TextDisabled(TR(PresetsCountShown),
            static_cast<int>(presets.size()), static_cast<int>(filteredIndices.size()));

        float btnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;
        if (ImGui::Button(TR(PresetsRandomize), ImVec2(btnW, 28))) {
            if (m_callbacks.onRandomize) m_callbacks.onRandomize();
        }
        ImGui::SameLine();
        if (ImGui::Button(TR(PresetsClear), ImVec2(btnW, 28))) {
            if (m_callbacks.onClear) m_callbacks.onClear();
        }
        ImGui::SameLine();
        if (ImGui::Button(TR(PresetsResetPreset), ImVec2(btnW, 28))) {
            if (m_callbacks.onPresetSelected)
                m_callbacks.onPresetSelected(m_selectedPreset);
        }

        ImGui::Spacing();
        ImGui::SeparatorText(TR(PresetsPlacement));

        const char* placementNames[] = {
            TR(PresetsPlacementCenter), TR(PresetsPlacementTopLeft), TR(PresetsPlacementTopRight),
            TR(PresetsPlacementBottomLeft), TR(PresetsPlacementBottomRight),
            TR(PresetsPlacementTop), TR(PresetsPlacementBottom), TR(PresetsPlacementLeft),
            TR(PresetsPlacementRight), TR(PresetsPlacementRandom), TR(PresetsPlacementGrid),
            TR(PresetsPlacementTwoPlace), TR(PresetsPlacementScatter)
        };
        ImGui::Combo(TR(PresetsPlacement), &params.placementMode, placementNames, 13);

        SliderIntWithInput(TR(PresetsCount), &params.placementCount, 1, 50);

        SliderFloatWithInput(TR(PresetsScale), &params.placementScale, 0.1f, 3.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 3.0f, r, 1, nullptr, 0); snapFloat(params.placementScale, 0.1f, 3.0f, r, 1); }

        const char* rotNames[] = {TR(PresetsRotation0), TR(PresetsRotation90), TR(PresetsRotation180), TR(PresetsRotation270)};
        ImGui::Combo(TR(PresetsRotation), &params.placementRotation, rotNames, 4);

        SliderFloatWithInput(TR(PresetsMargin), &params.placementMargin, 0.0f, 0.25f, "%.3f");

        if (params.placementMode >= 9) {
            ImGui::Checkbox(TR(PresetsRandomFlip), &params.placementRandomFlip);
        } else {
            std::string flipHLabel = std::string(TR(PresetsFlipHorizontal)) + "##placeFlipH";
            ImGui::Checkbox(flipHLabel.c_str(), &params.placementFlipH);
            ImGui::SameLine();
            std::string flipVLabel = std::string(TR(PresetsFlipVertical)) + "##placeFlipV";
            ImGui::Checkbox(flipVLabel.c_str(), &params.placementFlipV);
        }

        if (params.placementCount > 1 && params.placementMode < 9) {
            std::string spacingLabel = std::string(TR(PresetsPlaceSpacing)) + "##placeSpacing";
            SliderFloatWithInput(spacingLabel.c_str(), &params.placementSpacing, 0.01f, 0.5f, "%.3f");
        }

        if (params.placementMode == static_cast<int>(PlacementMode::Scatter)) {
            std::string minSepLabel = std::string(TR(PresetsMinSeparation)) + "##minSep";
            SliderIntWithInput(minSepLabel.c_str(), &params.placementMinSeparation, 0, 100);
        }

        std::string clearFirstLabel = std::string(TR(PresetsClearGridFirst)) + "##clearFirst";
        ImGui::Checkbox(clearFirstLabel.c_str(), &params.placementClearFirst);

        ImGui::Spacing();
        if (ImGui::Button(TR(PresetsApplyPlacement), ImVec2(-1, 28))) {
            if (m_callbacks.onReset) m_callbacks.onReset();
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionSimulation), 5, true)) {
        ImGui::Checkbox(TR(SimPausedLabel), &paused);
        ImGui::SameLine();
        ImGui::TextDisabled(TR(SimHoldToStep));

        SliderIntWithInput(TR(SimStepsPerFrame), &stepsPerFrame, 1, 50);
        { int r[] = {1}; int g[] = {5, 10, 20}; drawSliderMarkersInt(1, 50, r, 1, g, 3); snapInt(stepsPerFrame, 1, 50, r, 1); snapInt(stepsPerFrame, 1, 50, g, 3); }
        Tooltip(TR(SimStepsPerFrameTooltip));

        ImGui::Text(TR(SimStepFormat), stepCount);
        ImGui::SameLine();
        ImGui::Text(TR(SimTimeMs), simTimeMs);
    }
    popSectionColor();
    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionGrowthFunction), 6, true)) {
        const char* growthNames[] = {
            TR(GrowthTypeLenia), TR(GrowthTypeStep), TR(GrowthTypeGameOfLife), TR(GrowthTypeSmoothLife),
            TR(GrowthTypePolynomial), TR(GrowthTypeExponential), TR(GrowthTypeDoublePeak), TR(GrowthTypeAsymptotic),
            TR(GrowthTypeSoftClip), TR(GrowthTypeLargerThanLife), TR(GrowthTypeQuad4)
        };
        ImGui::Combo(TR(GrowthType), &params.growthType, growthNames, 11);
        Tooltip(TR(GrowthTypeTooltip));

        std::string muLabel = std::string(TR(GrowthMu)) + "##growth";
        SliderFloatWithInput(muLabel.c_str(), &params.mu, 0.001f, 1.0f);
        { float r[] = {0.15f}; float g[] = {0.29f, 0.35f}; drawSliderMarkers(0.001f, 1.0f, r, 1, g, 2); float all[] = {0.15f, 0.29f, 0.35f}; snapFloat(params.mu, 0.001f, 1.0f, all, 3); }
        Tooltip(TR(GrowthMuTooltip));

        std::string sigmaLabel = std::string(TR(GrowthSigma)) + "##growth";
        SliderFloatWithInput(sigmaLabel.c_str(), &params.sigma, 0.001f, 0.5f);
        { float r[] = {0.017f}; float g[] = {0.015f, 0.045f}; drawSliderMarkers(0.001f, 0.5f, r, 1, g, 2); float all[] = {0.017f, 0.015f, 0.045f}; snapFloat(params.sigma, 0.001f, 0.5f, all, 3); }
        Tooltip(TR(GrowthSigmaTooltip));

        std::string dtLabel = std::string(TR(GrowthDt)) + "##timestep";
        SliderFloatWithInput(dtLabel.c_str(), &params.dt, 0.001f, 2.0f);
        { float r[] = {0.25f}; float g[] = {0.1f, 0.5f, 1.0f}; drawSliderMarkers(0.001f, 2.0f, r, 1, g, 3); float all[] = {0.25f, 0.1f, 0.5f, 1.0f}; snapFloat(params.dt, 0.001f, 2.0f, all, 4); }
        Tooltip(TR(GrowthDtTooltip));

        ImGui::Spacing();
        drawGrowthPlot(params);
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionKernel), 7, true)) {
        bool kernelDirty = false;
        bool isMulti = (params.numChannels > 1);

        if (!isMulti) {
            const char* kernelNames[] = {
                TR(KernelGaussianShell), TR(KernelBump4), TR(KernelMultiringGauss), TR(KernelMultiringBump4),
                TR(KernelGameOfLife), TR(KernelStepUnimodal), TR(KernelCosineShell), TR(KernelMexicanHat),
                TR(KernelQuad4), TR(KernelMultiringQuad4), TR(KernelCone), TR(KernelTorusDualRing),
                TR(KernelRingSharp), TR(KernelGaussianMixture), TR(KernelSinc), TR(KernelWaveletRicker),
                TR(KernelNegativeRing)
            };
            if (ImGui::Combo(TR(KernelType), &params.kernelType, kernelNames, 17))
                kernelDirty = true;
            Tooltip(TR(KernelTypeTooltip));

            if (!m_kernelPresetNames.empty()) {
                auto kpGetter = [](void* data, int idx, const char** out) -> bool {
                    auto* names = static_cast<std::vector<std::string>*>(data);
                    if (idx < 0 || idx >= static_cast<int>(names->size())) return false;
                    *out = (*names)[idx].c_str();
                    return true;
                };
                int prevKP = m_selectedKernelPreset;
                if (ImGui::Combo(TR(KernelPreset), &m_selectedKernelPreset, kpGetter,
                                 &m_kernelPresetNames, static_cast<int>(m_kernelPresetNames.size()), 10)) {
                    if (m_selectedKernelPreset != prevKP && m_callbacks.onKernelPresetSelected)
                        m_callbacks.onKernelPresetSelected(m_selectedKernelPreset);
                }
            }
        }

        int prevR = params.radius;
        std::string radiusLabel = std::string(TR(KernelRadius)) + " (R)";
        if (SliderIntWithInput(radiusLabel.c_str(), &params.radius, 1, 128)) {
            kernelDirty = (params.radius != prevR);
        }
        { int r[] = {13}; int g[] = {10, 12, 18, 26, 52}; drawSliderMarkersInt(1, 128, r, 1, g, 5); snapInt(params.radius, 1, 128, r, 1); snapInt(params.radius, 1, 128, g, 5); }
        Tooltip(TR(KernelRadiusTooltip));
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
                if (SliderIntWithInput(TR(KernelRings), &params.numRings, 1, 8)) {
                    if (params.numRings != prevRings) kernelDirty = true;
                }
                Tooltip(TR(KernelRingsTooltip));

                if (params.numRings > 1) {
                    ImGui::Indent(10.0f);
                    for (int i = 0; i < params.numRings && i < 16; ++i) {
                        char label[32];
                        std::snprintf(label, sizeof(label), TR(KernelRingWeight), i);
                        std::string ringLabel = std::string(label) + "##ring" + std::to_string(i);
                        if (ImGui::SliderFloat(ringLabel.c_str(), &params.ringWeights[i], 0.0f, 1.0f, "%.3f"))
                            kernelDirty = true;
                        char tooltipBuf[64];
                        std::snprintf(tooltipBuf, sizeof(tooltipBuf), TR(KernelRingWeightTooltip), i);
                        Tooltip(tooltipBuf);
                    }
                    ImGui::Unindent(10.0f);
                }
            }
            
            ImGui::Spacing();
            std::string advLabel = std::string(TR(KernelAdvanced)) + "##advkernel";
            if (ImGui::CollapsingHeader(advLabel.c_str())) {
                if (ImGui::SliderFloat(TR(KernelAnisotropy), &params.kernelAnisotropy, 0.0f, 1.0f, "%.2f")) {
                    kernelDirty = true;
                }
                Tooltip(TR(KernelAnisotropyTooltip));
                
                if (params.kernelAnisotropy > 0.01f) {
                    if (ImGui::SliderFloat(TR(KernelDirection), &params.kernelAnisotropyAngle, 0.0f, 360.0f, "%.0f°")) {
                        kernelDirty = true;
                    }
                    Tooltip(TR(KernelDirectionTooltip));
                }
                
                ImGui::Checkbox(TR(KernelTimeVarying), &params.kernelTimeVarying);
                Tooltip(TR(KernelTimeVaryingTooltip));
                
                if (params.kernelTimeVarying) {
                    if (ImGui::SliderFloat(TR(KernelPulseFrequency), &params.kernelPulseFrequency, 0.0f, 5.0f, "%.2f")) {
                        kernelDirty = true;
                    }
                    Tooltip(TR(KernelPulseFrequencyTooltip));
                }
                
                const char* modifierNames[] = {TR(KernelModifierNone), TR(KernelModifierNegativeRing)};
                if (ImGui::Combo(TR(KernelModifier), &params.kernelModifier, modifierNames, 2)) {
                    kernelDirty = true;
                }
                Tooltip(TR(KernelModifierTooltip));
            }
        }

        if (kernelDirty && m_callbacks.onKernelChanged)
            m_callbacks.onKernelChanged();

        ImGui::Checkbox(TR(KernelShowPreview), &params.showKernelPreview);
        if (params.showKernelPreview) {
            if (isMulti) {
                const char* chLabel[] = {"R", "G", "B"};
                for (int r = 0; r < params.numKernelRules; ++r) {
                    auto& rule = params.kernelRules[r];
                    int src = std::clamp(rule.sourceChannel, 0, params.numChannels - 1);
                    int dst = std::clamp(rule.destChannel, 0, params.numChannels - 1);
                    ImGui::TextDisabled(TR(MultiRule), r);
                    ImGui::SameLine(0, 6);
                    ImGui::TextDisabled("%s -> %s", chLabel[src], chLabel[dst]);
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
            ImGui::TextDisabled(TR(KernelPerRuleNote));
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionMultiChannel), 8)) {
        int prevCh = params.numChannels;
        const char* chNames[] = {TR(MultiChannelsSingle), TR(MultiChannelsRGB)};
        int chIdx = (params.numChannels > 1) ? 1 : 0;
        if (ImGui::Combo(TR(MultiChannels), &chIdx, chNames, 2)) {
            int newCh = (chIdx == 1) ? 3 : 1;
            if (newCh != prevCh && m_callbacks.onChannelModeChanged) {
                m_callbacks.onChannelModeChanged(newCh);
            }
        }
        Tooltip(TR(MultiChannelsTooltip));

        if (params.numChannels > 1) {
            ImGui::TextColored(ImVec4(0.7f,0.9f,1.0f,1.0f), TR(MultiRulesCount), params.numKernelRules);
            ImGui::SameLine(0, 10);
            if (ImGui::SmallButton("+##addRule")) {
                if (params.numKernelRules < 16) {
                    auto& nr = params.kernelRules[params.numKernelRules];
                    nr = {};
                    nr.mu = 0.15f;
                    nr.sigma = 0.015f;
                    nr.growthStrength = 1.0f;
                    nr.numRings = 1;
                    nr.ringWeights[0] = 1.0f;
                    nr.sourceChannel = 0;
                    nr.destChannel = 0;
                    nr.kernelType = 0;
                    nr.growthType = 0;
                    params.numKernelRules++;
                }
            }
            Tooltip(TR(MultiAddRuleTooltip));
            ImGui::SameLine(0, 5);
            if (ImGui::SmallButton("-##removeRule")) {
                if (params.numKernelRules > 0)
                    params.numKernelRules--;
            }
            Tooltip(TR(MultiRemoveRuleTooltip));

            ImGui::Separator();

            const ImVec4 chColors[3] = {
                ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
                ImVec4(0.35f, 1.0f, 0.35f, 1.0f),
                ImVec4(0.4f, 0.55f, 1.0f, 1.0f)
            };
            const char* chLabels[3] = {"R", "G", "B"};

            if (params.numKernelRules > 0) {
                ImGui::Text(TR(MultiChannelRouting));
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
                    std::snprintf(hdr, sizeof(hdr), TR(MultiRule), r);
                    std::string hdrStr = std::string(hdr) + " (" + chLabels[s] + " -> " + chLabels[d] + ") | m=" +
                                         std::to_string(rule.mu) + " s=" + std::to_string(rule.sigma) +
                                         " h=" + std::to_string(rule.growthStrength) + "##rule" + std::to_string(r);
                    ImGui::PushID(r);
                    if (ImGui::TreeNode(hdrStr.c_str())) {
                        bool ruleKernelDirty = false;

                        char srcLabel[32], dstLabel[32];
                        std::snprintf(srcLabel, sizeof(srcLabel), "%s##src%d", TR(MultiSourceChannel), r);
                        std::snprintf(dstLabel, sizeof(dstLabel), "%s##dst%d", TR(MultiDestChannel), r);

                        ImGui::TextColored(chColors[s], "%s: %s (%d)", TR(MultiSourceChannel), chLabels[s], rule.sourceChannel);
                        ImGui::SameLine(0, 20);
                        ImGui::TextColored(chColors[d], "%s: %s (%d)", TR(MultiDestChannel), chLabels[d], rule.destChannel);

                        SliderIntWithInput(srcLabel, &rule.sourceChannel, 0, 2);
                        SliderIntWithInput(dstLabel, &rule.destChannel, 0, 2);

                        char muL[32], sigL[32], hL[32], rfL[32], rnL[32], ktL[32], gtL[32];
                        std::snprintf(muL, sizeof(muL), "mu##rmu%d", r);
                        std::snprintf(sigL, sizeof(sigL), "sigma##rsig%d", r);
                        std::snprintf(hL, sizeof(hL), "%s##rh%d", TR(MultiStrengthH), r);
                        std::snprintf(rfL, sizeof(rfL), "%s##rrf%d", TR(MultiRadiusFrac), r);
                        std::snprintf(rnL, sizeof(rnL), "%s##rrn%d", TR(KernelRings), r);
                        std::snprintf(ktL, sizeof(ktL), "%s##rkt%d", TR(MultiKernelLabel), r);
                        std::snprintf(gtL, sizeof(gtL), "%s##rgt%d", TR(MultiGrowthLabel), r);

                        SliderFloatWithInput(muL, &rule.mu, 0.001f, 1.0f, "%.4f");
                        SliderFloatWithInput(sigL, &rule.sigma, 0.001f, 0.5f, "%.4f");
                        SliderFloatWithInput(hL, &rule.growthStrength, -2.0f, 2.0f, "%.3f");
                        if (SliderFloatWithInput(rfL, &rule.radiusFraction, 0.1f, 2.0f, "%.3f"))
                            ruleKernelDirty = true;
                        if (SliderIntWithInput(rnL, &rule.numRings, 1, 8))
                            ruleKernelDirty = true;

                        const char* kNames[] = {
                            TR(KernelGaussianShell), TR(KernelBump4), TR(KernelMultiringGauss), TR(KernelMultiringBump4),
                            TR(KernelGameOfLife), TR(KernelStepUnimodal), TR(KernelCosineShell), TR(KernelMexicanHat),
                            TR(KernelQuad4), TR(KernelMultiringQuad4)
                        };
                        if (ImGui::Combo(ktL, &rule.kernelType, kNames, 10))
                            ruleKernelDirty = true;

                        const char* gNames[] = {
                            TR(GrowthTypeLenia), TR(GrowthTypeStep), TR(GrowthTypeGameOfLife), TR(GrowthTypeSmoothLife),
                            TR(GrowthTypePolynomial), TR(GrowthTypeExponential), TR(GrowthTypeDoublePeak), TR(GrowthTypeAsymptotic),
                            TR(GrowthTypeSoftClip), TR(GrowthTypeLargerThanLife)
                        };
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
    if (sectionHeader(TR(SectionDisplay), 9)) {
        const char* dispModes[] = {
            TR(DisplayWorld), TR(DisplayNeighborSums), TR(DisplayGrowthValues), TR(DisplayKernel), TR(DisplayDelta),
            TR(DisplayVectorField), TR(DisplayContourLines), TR(DisplayHeatMap), TR(DisplayActivityMap), TR(DisplayDifference)
        };
        ImGui::Combo(TR(DisplayMode), &params.displayMode, dispModes, 10);
        Tooltip(TR(DisplayModeTooltip));
        
        if (params.displayMode == 5) {
            ImGui::SliderFloat(TR(DisplayVectorScale), &params.vectorFieldScale, 0.1f, 5.0f, "%.1f");
            Tooltip(TR(DisplayVectorScaleTooltip));
            ImGui::SliderInt(TR(DisplayVectorDensity), &params.vectorFieldDensity, 5, 50);
            Tooltip(TR(DisplayVectorDensityTooltip));
        }
        
        if (params.displayMode == 6) {
            ImGui::SliderInt(TR(DisplayContourLevels), &params.contourLevels, 2, 30);
            Tooltip(TR(DisplayContourLevelsTooltip));
            ImGui::SliderFloat(TR(DisplayLineThickness), &params.contourThickness, 0.5f, 3.0f, "%.1f");
            Tooltip(TR(DisplayLineThicknessTooltip));
        }
        
        if (params.displayMode == 8) {
            ImGui::SliderFloat(TR(DisplayActivityDecay), &params.activityDecay, 0.8f, 0.999f, "%.3f");
            Tooltip(TR(DisplayActivityDecayTooltip));
        }

        std::vector<std::string> cmapList = {
            TR(DisplayColormapLenia), TR(DisplayColormapViridis), TR(DisplayColormapMagma), TR(DisplayColormapInferno),
            TR(DisplayColormapPlasma), TR(DisplayColormapGrayscale), TR(DisplayColormapGrayscaleInv), TR(DisplayColormapJet)
        };
        for (auto& cn : m_customColormapNames)
            cmapList.push_back(cn);

        auto cmapGetter = [](void* data, int idx, const char** out) -> bool {
            auto* list = static_cast<std::vector<std::string>*>(data);
            if (idx < 0 || idx >= static_cast<int>(list->size())) return false;
            *out = (*list)[idx].c_str();
            return true;
        };
        ImGui::Combo(TR(DisplayColormap), &params.colormapMode, cmapGetter,
                     &cmapList, static_cast<int>(cmapList.size()));
        Tooltip(TR(DisplayColormapTooltip));

        drawColorbar(params);

        if (params.numChannels > 1) {
            ImGui::Separator();
            std::string useCmapLabel = std::string(TR(DisplayUseColormapMulti)) + "##useCmapMC";
            ImGui::Checkbox(useCmapLabel.c_str(), &params.useColormapForMultichannel);
            Tooltip(TR(DisplayUseColormapMultiTooltip));
            
            if (params.useColormapForMultichannel) {
                const char* blendModes[] = {TR(DisplayBlendLuminance), TR(DisplayBlendAverage), TR(DisplayBlendMaxChannel), TR(DisplayBlendMinChannel), TR(DisplayBlendRedOnly), TR(DisplayBlendGreenOnly), TR(DisplayBlendBlueOnly)};
                ImGui::Combo(TR(DisplayBlendMode), &params.multiChannelBlend, blendModes, 7);
                Tooltip(TR(DisplayBlendModeTooltip));
                
                if (params.multiChannelBlend == 0) {
                    ImGui::Text(TR(DisplayChannelWeights));
                    std::string wRLabel = std::string(TR(DisplayChannelWeightR)) + "##wR";
                    std::string wGLabel = std::string(TR(DisplayChannelWeightG)) + "##wG";
                    std::string wBLabel = std::string(TR(DisplayChannelWeightB)) + "##wB";
                    ImGui::SliderFloat(wRLabel.c_str(), &params.channelWeightR, 0.0f, 2.0f, "%.2f");
                    ImGui::SliderFloat(wGLabel.c_str(), &params.channelWeightG, 0.0f, 2.0f, "%.2f");
                    ImGui::SliderFloat(wBLabel.c_str(), &params.channelWeightB, 0.0f, 2.0f, "%.2f");
                    Tooltip(TR(DisplayChannelWeightsTooltip));
                    if (ImGui::Button(TR(DisplayResetWeights))) {
                        params.channelWeightR = 0.299f;
                        params.channelWeightG = 0.587f;
                        params.channelWeightB = 0.114f;
                    }
                }
            }
        }

        ImGui::Separator();
        std::string zoomLabel = std::string(TR(DisplayZoom)) + " (+/-)";
        SliderFloatWithInput(zoomLabel.c_str(), &params.zoom, 0.1f, 20.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 20.0f, r, 1, nullptr, 0); }
        Tooltip(TR(DisplayZoomTooltip));
        SliderFloatWithInput(TR(DisplayPanX), &params.panX, -2.0f, 2.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(-2.0f, 2.0f, r, 1, nullptr, 0); snapFloat(params.panX, -2.0f, 2.0f, r, 1); }
        Tooltip(TR(DisplayPanXTooltip));
        SliderFloatWithInput(TR(DisplayPanY), &params.panY, -2.0f, 2.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(-2.0f, 2.0f, r, 1, nullptr, 0); snapFloat(params.panY, -2.0f, 2.0f, r, 1); }
        Tooltip(TR(DisplayPanYTooltip));

        float halfBtnW = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;
        std::string resetViewLabel = std::string(TR(DisplayResetView)) + " (Home)";
        if (ImGui::Button(resetViewLabel.c_str(), ImVec2(halfBtnW, 22))) {
            params.zoom = 1.0f;
            params.panX = 0.0f;
            params.panY = 0.0f;
        }
        Tooltip(TR(DisplayResetViewTooltip));
        ImGui::SameLine();
        if (ImGui::Button(TR(DisplayCenterView), ImVec2(halfBtnW, 22))) {
            params.panX = 0.0f;
            params.panY = 0.0f;
        }
        Tooltip(TR(DisplayCenterViewTooltip));

        ImGui::Separator();
        SliderFloatWithInput(TR(DisplayBrightness), &params.brightness, 0.0f, 1.5f, "%.2f");
        { float r[] = {0.5f}; drawSliderMarkers(0.0f, 1.5f, r, 1, nullptr, 0); snapFloat(params.brightness, 0.0f, 1.5f, r, 1); }
        Tooltip(TR(DisplayBrightnessTooltip));
        SliderFloatWithInput(TR(DisplayContrast), &params.contrast, 0.1f, 5.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 5.0f, r, 1, nullptr, 0); snapFloat(params.contrast, 0.1f, 5.0f, r, 1); }
        Tooltip(TR(DisplayContrastTooltip));
        SliderFloatWithInput(TR(DisplayGamma), &params.gamma, 0.1f, 5.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 5.0f, r, 1, nullptr, 0); snapFloat(params.gamma, 0.1f, 5.0f, r, 1); }
        Tooltip(TR(DisplayGammaTooltip));

        ImGui::Separator();
        const char* filterNames[] = {TR(DisplayFilterBilinear), TR(DisplayFilterNearest), TR(DisplayFilterSharpen)};
        ImGui::Combo(TR(DisplayFilterMode), &params.filterMode, filterNames, 3);
        Tooltip(TR(DisplayFilterModeTooltip));
        SliderFloatWithInput(TR(DisplayEdgeDetect), &params.edgeStrength, 0.0f, 1.0f, "%.2f");
        { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.edgeStrength, 0.0f, 1.0f, r, 1); }
        Tooltip(TR(DisplayEdgeDetectTooltip));
        
        if (ImGui::CollapsingHeader(TR(DisplayGlowSettings))) {
            SliderFloatWithInput(TR(DisplayGlowStrength), &params.glowStrength, 0.0f, 1.0f, "%.2f");
            { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.glowStrength, 0.0f, 1.0f, r, 1); }
            Tooltip(TR(DisplayGlowStrengthTooltip));
            
            if (params.glowStrength > 0.0f) {
                float gc[3] = {params.glowR, params.glowG, params.glowB};
                if (ImGui::ColorEdit3(TR(DisplayGlowTint), gc)) {
                    params.glowR = gc[0];
                    params.glowG = gc[1];
                    params.glowB = gc[2];
                }
                Tooltip(TR(DisplayGlowTintTooltip));
                
                SliderFloatWithInput(TR(DisplayGlowIntensity), &params.glowIntensity, 0.5f, 3.0f, "%.2f");
                { float r[] = {1.0f}; drawSliderMarkers(0.5f, 3.0f, r, 1, nullptr, 0); snapFloat(params.glowIntensity, 0.5f, 3.0f, r, 1); }
                Tooltip(TR(DisplayGlowIntensityTooltip));
            }
        }
        
        if (ImGui::CollapsingHeader(TR(DisplayCustomGradient))) {
            ImGui::SliderInt(TR(DisplayGradientStops), &params.gradientStops, 2, 5);
            Tooltip(TR(DisplayGradientStopsTooltip));
            
            for (int i = 0; i < params.gradientStops; ++i) {
                char label[32];
                std::snprintf(label, sizeof(label), TR(DisplayGradientStopLabel), i + 1);
                float* col = &params.gradientColors[i * 3];
                ImGui::ColorEdit3(label, col, ImGuiColorEditFlags_NoInputs);
                if (i < params.gradientStops - 1) ImGui::SameLine();
            }
            Tooltip(TR(DisplayCustomGradientTooltip));
        }

        ImGui::Separator();
        ImGui::Checkbox(TR(DisplayGridOverlay), &params.showGrid);
        Tooltip(TR(DisplayGridOverlayTooltip));
        if (params.showGrid) {
            SliderFloatWithInput(TR(DisplayGridOpacity), &params.gridOpacity, 0.0f, 1.0f, "%.2f");
            Tooltip(TR(DisplayGridOpacityTooltip));

            float glc[3] = {params.gridLineR, params.gridLineG, params.gridLineB};
            if (ImGui::ColorEdit3(TR(DisplayGridColor), glc)) {
                params.gridLineR = glc[0]; params.gridLineG = glc[1]; params.gridLineB = glc[2];
            }
            Tooltip(TR(DisplayGridColorTooltip));

            SliderFloatWithInput(TR(DisplayGridLineThickness), &params.gridLineThickness, 0.1f, 5.0f, "%.1f");
            { float r[] = {1.0f}; drawSliderMarkers(0.1f, 5.0f, r, 1, nullptr, 0); snapFloat(params.gridLineThickness, 0.1f, 5.0f, r, 1); }
            Tooltip(TR(DisplayGridLineThicknessTooltip));

            const char* spacingModes[] = {TR(DisplayGridEveryCell), TR(DisplayGridCustomInterval)};
            ImGui::Combo(TR(DisplayGridSpacing), &params.gridSpacingMode, spacingModes, 2);
            Tooltip(TR(DisplayGridSpacingTooltip));

            if (params.gridSpacingMode == 1) {
                SliderIntWithInput(TR(DisplayGridInterval), &params.gridCustomSpacing, 1, 100);
                Tooltip(TR(DisplayGridIntervalTooltip));
            }

            ImGui::Checkbox(TR(DisplayGridMajorLines), &params.gridMajorLines);
            Tooltip(TR(DisplayGridMajorLinesTooltip));
            if (params.gridMajorLines) {
                SliderIntWithInput(TR(DisplayGridMajorEvery), &params.gridMajorEvery, 2, 50);
                Tooltip(TR(DisplayGridMajorEveryTooltip));
                SliderFloatWithInput(TR(DisplayGridMajorOpacity), &params.gridMajorOpacity, 0.0f, 1.0f, "%.2f");
                Tooltip(TR(DisplayGridMajorOpacityTooltip));
            }
        }

        ImGui::Separator();
        ImGui::Checkbox(TR(DisplayInvertColors), &params.invertColors);
        Tooltip(TR(DisplayInvertColorsTooltip));

        ImGui::Checkbox(TR(DisplayShowBoundary), &params.showBoundary);
        Tooltip(TR(DisplayShowBoundaryTooltip));
        if (params.showBoundary) {
            float bc[3] = {params.boundaryR, params.boundaryG, params.boundaryB};
            std::string boundaryColorLabel = std::string(TR(DisplayBoundaryColor)) + "##bcolor";
            if (ImGui::ColorEdit3(boundaryColorLabel.c_str(), bc)) {
                params.boundaryR = bc[0]; params.boundaryG = bc[1]; params.boundaryB = bc[2];
            }
            std::string boundaryOpacityLabel = std::string(TR(DisplayBoundaryOpacity)) + "##bopacity";
            SliderFloatWithInput(boundaryOpacityLabel.c_str(), &params.boundaryOpacity, 0.0f, 1.0f, "%.2f");
            
            const char* boundaryStyles[] = {TR(DisplayBoundaryStyleSolid), TR(DisplayBoundaryStyleDashed), TR(DisplayBoundaryStyleDotted), TR(DisplayBoundaryStyleDouble), TR(DisplayBoundaryStyleGlow)};
            std::string boundaryStyleLabel = std::string(TR(DisplayBoundaryStyle)) + "##bstyle";
            ImGui::Combo(boundaryStyleLabel.c_str(), &params.boundaryStyle, boundaryStyles, 5);
            Tooltip(TR(DisplayBoundaryStyleTooltip));
            
            std::string boundaryWidthLabel = std::string(TR(DisplayBoundaryWidth)) + "##bwidth";
            SliderFloatWithInput(boundaryWidthLabel.c_str(), &params.boundaryThickness, 0.5f, 10.0f, "%.1f");
            Tooltip(TR(DisplayBoundaryWidthTooltip));
            
            if (params.boundaryStyle == 1 || params.boundaryStyle == 2) {
                std::string dashLabel = std::string(TR(DisplayDashLength)) + "##bdash";
                SliderFloatWithInput(dashLabel.c_str(), &params.boundaryDashLength, 2.0f, 30.0f, "%.0f");
                Tooltip(TR(DisplayDashLengthTooltip));
            }
            
            std::string animateLabel = std::string(TR(DisplayAnimateBoundary)) + "##banim";
            ImGui::Checkbox(animateLabel.c_str(), &params.boundaryAnimate);
            Tooltip(TR(DisplayAnimateBoundaryTooltip));
        }

        float bg[3] = {params.bgR, params.bgG, params.bgB};
        if (ImGui::ColorEdit3(TR(DisplayBGColor), bg)) {
            params.bgR = bg[0]; params.bgG = bg[1]; params.bgB = bg[2];
        }
        Tooltip(TR(DisplayBGColorTooltip));

        ImGui::Checkbox(TR(DisplayClipNullCells), &params.clipToZero);
        Tooltip(TR(DisplayClipNullCellsTooltip));
        if (params.clipToZero) {
            SliderFloatWithInput(TR(DisplayClipThreshold), &params.clipThreshold, 0.0001f, 0.1f, "%.4f");
            Tooltip(TR(DisplayClipThresholdTooltip));
        }

        ImGui::SeparatorText(TR(DisplayColormapDeformation));

        SliderFloatWithInput(TR(DisplayCmapOffset), &params.cmapOffset, 0.0f, 1.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.cmapOffset, 0.0f, 1.0f, r, 1); }
        Tooltip(TR(DisplayCmapOffsetTooltip));

        SliderFloatWithInput(TR(DisplayRangeMin), &params.cmapRange0, 0.0f, 1.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.cmapRange0, 0.0f, 1.0f, r, 1); }
        Tooltip(TR(DisplayRangeMinTooltip));

        SliderFloatWithInput(TR(DisplayRangeMax), &params.cmapRange1, 0.0f, 1.0f, "%.3f");
        { float r[] = {1.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.cmapRange1, 0.0f, 1.0f, r, 1); }
        Tooltip(TR(DisplayRangeMaxTooltip));

        SliderFloatWithInput(TR(DisplayPowerCurve), &params.cmapPower, 0.1f, 5.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.1f, 5.0f, r, 1, nullptr, 0); snapFloat(params.cmapPower, 0.1f, 5.0f, r, 1); }
        Tooltip(TR(DisplayPowerCurveTooltip));

        SliderFloatWithInput(TR(DisplayHueShift), &params.cmapHueShift, 0.0f, 1.0f, "%.3f");
        { float r[] = {0.0f}; drawSliderMarkers(0.0f, 1.0f, r, 1, nullptr, 0); snapFloat(params.cmapHueShift, 0.0f, 1.0f, r, 1); }
        Tooltip(TR(DisplayHueShiftTooltip));

        SliderFloatWithInput(TR(DisplaySaturation), &params.cmapSaturation, 0.0f, 3.0f, "%.2f");
        { float r[] = {1.0f}; drawSliderMarkers(0.0f, 3.0f, r, 1, nullptr, 0); snapFloat(params.cmapSaturation, 0.0f, 3.0f, r, 1); }
        Tooltip(TR(DisplaySaturationTooltip));

        ImGui::Checkbox(TR(DisplayReverseColormap), &params.cmapReverse);
        Tooltip(TR(DisplayReverseColormapTooltip));

        if (ImGui::Button(TR(DisplayResetColormapDeformation), ImVec2(-1, 22))) {
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
    if (sectionHeader(TR(SectionAnalysis), 10)) {
        ImGui::Checkbox(TR(AnalysisEnable), &params.showAnalysis);
        Tooltip(TR(AnalysisEnableTooltip));
        ImGui::SameLine();
        ImGui::Checkbox(TR(AnalysisAutoPause), &params.autoPause);
        Tooltip(TR(AnalysisAutoPauseTooltip));

        SliderFloatWithInput(TR(AnalysisAliveThreshold), &params.analysisThreshold, 0.0001f, 0.5f, "%.4f");
        Tooltip(TR(AnalysisAliveThresholdTooltip));

        if (analysis && params.showAnalysis) {
            ImGui::Separator();
            ImGui::Text(TR(AnalysisTotalMass), analysis->totalMass);
            ImGui::Text(TR(AnalysisAliveCells),
                        analysis->aliveCount, analysis->totalPixels,
                        analysis->totalPixels > 0
                            ? 100.0f * analysis->aliveCount / analysis->totalPixels
                            : 0.0f);
            ImGui::Text(TR(AnalysisAverage), analysis->avgVal);
            ImGui::Text(TR(AnalysisMinMax), analysis->minVal, analysis->maxVal);
            ImGui::Text(TR(AnalysisVariance), analysis->variance);
            ImGui::Text(TR(AnalysisCentroid), analysis->centroidX, analysis->centroidY);
            ImGui::Text(TR(AnalysisBounds),
                        analysis->boundMinX, analysis->boundMinY,
                        analysis->boundMaxX, analysis->boundMaxY);

            if (analysisMgr) {
                ImGui::Separator();
                if (analysisMgr->isEmpty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), TR(AnalysisStateEmpty));
                } else if (analysisMgr->isStabilized()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), TR(AnalysisStateStabilized));
                } else if (analysisMgr->isPeriodic()) {
                    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f),
                        TR(AnalysisStatePeriodic),
                        analysisMgr->detectedPeriod(),
                        analysisMgr->periodConfidence() * 100.0f);
                } else {
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), TR(AnalysisStateActive));
                }

                ImGui::Separator();
                ImGui::Text(TR(AnalysisSpecies), analysisMgr->speciesCount());
                ImGui::Text(TR(AnalysisSpeed), analysisMgr->movementSpeed());
                ImGui::Text(TR(AnalysisDirection), analysisMgr->movementDirection());
                ImGui::Text(TR(AnalysisOrientation), analysisMgr->orientation());
            }

            if (analysisMgr && analysisMgr->historyCount() > 1) {
                ImGui::Separator();
                ImGui::Text(TR(AnalysisGraphs));

                ImGui::Checkbox(TR(AnalysisMass), &params.showMassGraph);
                ImGui::SameLine();
                ImGui::Checkbox(TR(AnalysisAlive), &params.showAliveGraph);
                ImGui::SameLine();
                ImGui::Checkbox(TR(AnalysisCentroidGraph), &params.showCentroidGraph);
                ImGui::Checkbox(TR(AnalysisSpeedGraph), &params.showSpeedGraph);
                ImGui::SameLine();
                ImGui::Checkbox(TR(AnalysisDirectionGraph), &params.showDirectionGraph);

                SliderIntWithInput(TR(AnalysisDisplayWindow), &params.graphTimeWindow, 0, AnalysisManager::HISTORY_SIZE);
                Tooltip(TR(AnalysisDisplayWindowTooltip));

                SliderFloatWithInput(TR(AnalysisGraphHeight), &params.graphHeight, 50.0f, 200.0f, "%.0f");

                ImGui::Checkbox(TR(AnalysisAutoYScale), &params.graphAutoScale);
                Tooltip(TR(AnalysisAutoYScaleTooltip));

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

                    drawGraphWithAxes(TR(AnalysisMass), massPlot, dispCount, yMin, yMax,
                                      TR(AnalysisGraphXAxisStep), TR(AnalysisGraphYAxisMass), params.graphHeight,
                                      IM_COL32(100, 220, 150, 230));

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

                    drawGraphWithAxes(TR(AnalysisAliveCellsGraph), alivePlot, dispCount, yMin, yMax,
                                      TR(AnalysisGraphXAxisStep), TR(AnalysisGraphYAxisCells), params.graphHeight,
                                      IM_COL32(220, 180, 100, 230));
                }

                if (params.showCentroidGraph) {
                    float cxPlot[512], cyPlot[512];
                    for (int i = 0; i < dispCount; ++i) {
                        int idx = (head - fullCount + startOff + i + AnalysisManager::HISTORY_SIZE) % AnalysisManager::HISTORY_SIZE;
                        cxPlot[i] = analysisMgr->centroidXHistory(idx);
                        cyPlot[i] = analysisMgr->centroidYHistory(idx);
                    }
                    drawGraphWithAxes(TR(AnalysisCentroidXGraph), cxPlot, dispCount, 0.0f, static_cast<float>(params.gridW),
                                      TR(AnalysisGraphXAxisStep), TR(AnalysisGraphYAxisX), params.graphHeight, IM_COL32(150, 200, 255, 230));
                    drawGraphWithAxes(TR(AnalysisCentroidYGraph), cyPlot, dispCount, 0.0f, static_cast<float>(params.gridH),
                                      TR(AnalysisGraphXAxisStep), TR(AnalysisGraphYAxisY), params.graphHeight, IM_COL32(255, 150, 200, 230));
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
                    drawGraphWithAxes(TR(AnalysisSpeedGraphTitle), spdPlot, dispCount, 0.0f, yMax,
                                      TR(AnalysisGraphXAxisStep), TR(AnalysisGraphYAxisPxPerSec), params.graphHeight,
                                      IM_COL32(255, 180, 100, 230));
                }

                if (params.showDirectionGraph) {
                    float dirPlot[512];
                    for (int i = 0; i < dispCount; ++i) {
                        int idx = (head - fullCount + startOff + i + AnalysisManager::HISTORY_SIZE) % AnalysisManager::HISTORY_SIZE;
                        dirPlot[i] = analysisMgr->directionHistory(idx);
                    }
                    drawGraphWithAxes(TR(AnalysisDirectionGraphTitle), dirPlot, dispCount, -180.0f, 180.0f,
                                      TR(AnalysisGraphXAxisStep), TR(AnalysisGraphYAxisDeg), params.graphHeight,
                                      IM_COL32(200, 150, 255, 230));
                }
            }
        }
    }
    popSectionColor();

    pushSectionColor(sec++);
    if (sectionHeader(TR(SectionAccessibility), 11)) {
        renderAccessibilitySection();
    }
    popSectionColor();

    ImGui::End();
}

void UIOverlay::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    if (m_needsStyleUpdate) {
        updateStyle();
    }
    
    if (m_needsFontRebuild) {
        rebuildFonts();
    }
}

void UIOverlay::shutdown() {
    if (!m_initialized) return;
    saveAccessibilitySettings();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_initialized = false;
}

void UIOverlay::setAccessibilitySettings(const AccessibilitySettings& settings) {
    bool scaleChanged = (m_accessibilitySettings.uiScale != settings.uiScale);
    bool fontChanged = (m_accessibilitySettings.fontSize != settings.fontSize);
    bool contrastChanged = (m_accessibilitySettings.highContrast != settings.highContrast);
    bool navChanged = (m_accessibilitySettings.keyboardNavEnabled != settings.keyboardNavEnabled) ||
                      (m_accessibilitySettings.showFocusIndicators != settings.showFocusIndicators);
    
    m_accessibilitySettings = settings;
    
    if (scaleChanged) {
        applyUIScale(settings.uiScale);
    }
    if (fontChanged) {
        applyFontSize(settings.fontSize);
    }
    if (contrastChanged) {
        applyHighContrastTheme(settings.highContrast);
    }
    if (navChanged) {
        applyKeyboardNavigationSettings();
    }
    
    saveAccessibilitySettings();
}

void UIOverlay::applyUIScale(float scale) {
    scale = std::clamp(scale, AccessibilitySettings::MIN_UI_SCALE, AccessibilitySettings::MAX_UI_SCALE);
    m_accessibilitySettings.uiScale = scale;
    m_needsStyleUpdate = true;
}

void UIOverlay::applyFontSize(float size) {
    size = std::clamp(size, AccessibilitySettings::MIN_FONT_SIZE, AccessibilitySettings::MAX_FONT_SIZE);
    m_accessibilitySettings.fontSize = size;
    m_needsFontRebuild = true;
}

void UIOverlay::updateStyle() {
    if (!m_needsStyleUpdate) return;
    m_needsStyleUpdate = false;
    
    float effectiveScale = m_dpiScale * m_accessibilitySettings.uiScale;
    
    if (std::abs(effectiveScale - m_lastStyleScale) < 0.001f) {
        return;
    }
    
    float ratio = (m_lastStyleScale > 0.0f) ? (effectiveScale / m_lastStyleScale) : 1.0f;
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowPadding = ImVec2(style.WindowPadding.x * ratio, style.WindowPadding.y * ratio);
    style.WindowBorderSize = std::max(0.0f, style.WindowBorderSize * ratio);
    style.WindowMinSize = ImVec2(style.WindowMinSize.x * ratio, style.WindowMinSize.y * ratio);
    
    style.FramePadding = ImVec2(style.FramePadding.x * ratio, style.FramePadding.y * ratio);
    style.FrameBorderSize = std::max(0.0f, style.FrameBorderSize * ratio);
    
    style.ItemSpacing = ImVec2(style.ItemSpacing.x * ratio, style.ItemSpacing.y * ratio);
    style.ItemInnerSpacing = ImVec2(style.ItemInnerSpacing.x * ratio, style.ItemInnerSpacing.y * ratio);
    style.CellPadding = ImVec2(style.CellPadding.x * ratio, style.CellPadding.y * ratio);
    
    style.IndentSpacing = style.IndentSpacing * ratio;
    style.ColumnsMinSpacing = style.ColumnsMinSpacing * ratio;
    style.ScrollbarSize = std::max(1.0f, style.ScrollbarSize * ratio);
    
    style.GrabMinSize = std::max(1.0f, style.GrabMinSize * ratio);
    style.GrabRounding = std::max(0.0f, style.GrabRounding * ratio);
    
    style.TabBorderSize = std::max(0.0f, style.TabBorderSize * ratio);
    style.TabRounding = std::max(0.0f, style.TabRounding * ratio);
    
    style.WindowRounding = std::max(0.0f, style.WindowRounding * ratio);
    style.ChildRounding = std::max(0.0f, style.ChildRounding * ratio);
    style.FrameRounding = std::max(0.0f, style.FrameRounding * ratio);
    style.PopupRounding = std::max(0.0f, style.PopupRounding * ratio);
    
    style.SeparatorTextPadding = ImVec2(style.SeparatorTextPadding.x * ratio, style.SeparatorTextPadding.y * ratio);
    style.SeparatorTextBorderSize = std::max(0.0f, style.SeparatorTextBorderSize * ratio);
    
    style.DisplaySafeAreaPadding = ImVec2(style.DisplaySafeAreaPadding.x * ratio, style.DisplaySafeAreaPadding.y * ratio);
    
    style.WindowBorderHoverPadding = std::max(1.0f, style.WindowBorderHoverPadding * ratio);
    
    m_lastStyleScale = effectiveScale;
}

void UIOverlay::rebuildFonts() {
    if (!m_needsFontRebuild) return;
    
    float effectiveScale = m_dpiScale * m_accessibilitySettings.uiScale;
    float fontSize = m_accessibilitySettings.fontSize * effectiveScale;
    
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    
    io.Fonts->Clear();
    io.Fonts->AddFontDefault();
    
    m_defaultFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", fontSize);
    if (!m_defaultFont) {
        ImFontConfig config;
        config.SizePixels = fontSize;
        m_defaultFont = io.Fonts->AddFontDefault(&config);
    }
    
    io.Fonts->Build();
    
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    
    io.FontDefault = m_defaultFont;
    
    m_needsFontRebuild = false;
}

void UIOverlay::applyKeyboardNavigationSettings() {
    ImGuiIO& io = ImGui::GetIO();
    if (m_accessibilitySettings.keyboardNavEnabled) {
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    } else {
        io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    if (m_accessibilitySettings.showFocusIndicators) {
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 1.00f, 0.20f, 1.00f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 0.20f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.35f);
    } else {
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    }
}

void UIOverlay::initHighContrastStyle() {
    // High contrast theme is applied on-demand in applyHighContrastTheme
}

void UIOverlay::applyHighContrastTheme(bool enable) {
    m_accessibilitySettings.highContrast = enable;
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    if (enable) {
        // Apply high contrast colors - white on black with yellow highlights
        style.Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
        style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        style.Colors[ImGuiCol_CheckMark]             = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab]            = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(1.00f, 1.00f, 0.00f, 0.30f);
        style.FrameBorderSize = 1.0f;
        style.WindowBorderSize = 1.0f;
    } else {
        // Restore normal colors
        style.Colors[ImGuiCol_Text]                  = ImVec4(0.95f, 0.95f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.10f, 0.12f, 0.18f, 0.95f);
        style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.08f, 0.10f, 0.16f, 0.90f);
        style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.14f, 0.14f, 0.20f, 0.80f);
        style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.45f, 0.70f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.40f, 0.55f, 0.80f, 1.00f);
        style.Colors[ImGuiCol_Button]                = ImVec4(0.18f, 0.22f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.28f, 0.35f, 0.55f, 1.00f);
        style.FrameBorderSize = 0.0f;
        style.WindowBorderSize = 0.0f;
    }
}

void UIOverlay::saveAccessibilitySettings() const {
    std::ofstream file("lenia_accessibility.cfg");
    if (file.is_open()) {
        file << "uiScale=" << m_accessibilitySettings.uiScale << "\n";
        file << "fontSize=" << m_accessibilitySettings.fontSize << "\n";
        file << "highContrast=" << (m_accessibilitySettings.highContrast ? 1 : 0) << "\n";
        file << "reduceMotion=" << (m_accessibilitySettings.reduceMotion ? 1 : 0) << "\n";
        file << "keyboardNavEnabled=" << (m_accessibilitySettings.keyboardNavEnabled ? 1 : 0) << "\n";
        file << "showFocusIndicators=" << (m_accessibilitySettings.showFocusIndicators ? 1 : 0) << "\n";
        file << "invertColors=" << (m_accessibilitySettings.invertColors ? 1 : 0) << "\n";
        file << "cursorSize=" << m_accessibilitySettings.cursorSize << "\n";
        file << "language=" << static_cast<int>(Localization::instance().getLanguage()) << "\n";
    }
}

void UIOverlay::loadAccessibilitySettings() {
    std::ifstream file("lenia_accessibility.cfg");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;
            
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);
            
            if (key == "uiScale") {
                m_accessibilitySettings.uiScale = std::stof(value);
            } else if (key == "fontSize") {
                m_accessibilitySettings.fontSize = std::stof(value);
            } else if (key == "highContrast") {
                m_accessibilitySettings.highContrast = (value == "1");
            } else if (key == "reduceMotion") {
                m_accessibilitySettings.reduceMotion = (value == "1");
            } else if (key == "keyboardNavEnabled") {
                m_accessibilitySettings.keyboardNavEnabled = (value == "1");
            } else if (key == "showFocusIndicators") {
                m_accessibilitySettings.showFocusIndicators = (value == "1");
            } else if (key == "invertColors") {
                m_accessibilitySettings.invertColors = (value == "1");
            } else if (key == "cursorSize") {
                m_accessibilitySettings.cursorSize = std::stof(value);
            } else if (key == "language") {
                int langVal = std::stoi(value);
                if (langVal >= 0 && langVal <= 1) {
                    Localization::instance().setLanguage(static_cast<Language>(langVal));
                }
            }
        }
        
        m_accessibilitySettings.uiScale = std::clamp(m_accessibilitySettings.uiScale, 
            AccessibilitySettings::MIN_UI_SCALE, AccessibilitySettings::MAX_UI_SCALE);
        m_accessibilitySettings.fontSize = std::clamp(m_accessibilitySettings.fontSize,
            AccessibilitySettings::MIN_FONT_SIZE, AccessibilitySettings::MAX_FONT_SIZE);
    }
}

void UIOverlay::renderAccessibilitySection() {
    auto& loc = Localization::instance();
    
    ImGui::Text("%s", TR(AccessibilityLanguage));
    
    auto languages = loc.getAvailableLanguages();
    int currentLangIdx = static_cast<int>(loc.getLanguage());
    
    std::vector<const char*> langNames;
    for (auto lang : languages) {
        langNames.push_back(loc.getLanguageName(lang));
    }
    
    if (ImGui::Combo("##language", &currentLangIdx, langNames.data(), static_cast<int>(langNames.size()))) {
        loc.setLanguage(static_cast<Language>(currentLangIdx));
        saveAccessibilitySettings();
    }
    Tooltip(TR(AccessibilityLanguageTooltip));
    
    ImGui::Separator();
    
    ImGui::Text("%s", TR(AccessibilityUIScale));
    float uiScale = m_accessibilitySettings.uiScale;
    if (ImGui::SliderFloat("##uiscale", &uiScale, AccessibilitySettings::MIN_UI_SCALE, 
                           AccessibilitySettings::MAX_UI_SCALE, "%.2fx")) {
        applyUIScale(uiScale);
        saveAccessibilitySettings();
    }
    Tooltip(TR(AccessibilityUIScaleTooltip));
    
    ImGui::Text("%s", TR(AccessibilityFontSize));
    float fontSize = m_accessibilitySettings.fontSize;
    if (ImGui::SliderFloat("##fontsize", &fontSize, AccessibilitySettings::MIN_FONT_SIZE,
                           AccessibilitySettings::MAX_FONT_SIZE, "%.0f px")) {
        applyFontSize(fontSize);
        saveAccessibilitySettings();
    }
    Tooltip(TR(AccessibilityFontSizeTooltip));
    
    ImGui::Separator();
    
    bool highContrast = m_accessibilitySettings.highContrast;
    if (ImGui::Checkbox(TR(AccessibilityHighContrast), &highContrast)) {
        applyHighContrastTheme(highContrast);
        saveAccessibilitySettings();
    }
    Tooltip(TR(AccessibilityHighContrastTooltip));
    
    if (ImGui::Checkbox(TR(AccessibilityReduceMotion), &m_accessibilitySettings.reduceMotion)) {
        saveAccessibilitySettings();
    }
    Tooltip(TR(AccessibilityReduceMotionTooltip));
    
    bool keyboardNavEnabled = m_accessibilitySettings.keyboardNavEnabled;
    if (ImGui::Checkbox(TR(AccessibilityKeyboardNav), &keyboardNavEnabled)) {
        m_accessibilitySettings.keyboardNavEnabled = keyboardNavEnabled;
        applyKeyboardNavigationSettings();
        saveAccessibilitySettings();
    }
    Tooltip(TR(AccessibilityKeyboardNavTooltip));

    bool showFocusIndicators = m_accessibilitySettings.showFocusIndicators;
    if (ImGui::Checkbox(TR(AccessibilityFocusIndicators), &showFocusIndicators)) {
        m_accessibilitySettings.showFocusIndicators = showFocusIndicators;
        applyKeyboardNavigationSettings();
        saveAccessibilitySettings();
    }
    Tooltip(TR(AccessibilityFocusIndicatorsTooltip));
    
    ImGui::Separator();
    
    if (ImGui::Button(TR(AccessibilityResetDefaults), ImVec2(-1, 28))) {
        m_accessibilitySettings.reset();
        loc.setLanguage(Language::English);
        applyUIScale(m_accessibilitySettings.uiScale);
        applyFontSize(m_accessibilitySettings.fontSize);
        applyHighContrastTheme(false);
        applyKeyboardNavigationSettings();
        saveAccessibilitySettings();
    }
    Tooltip(TR(AccessibilityResetDefaultsTooltip));
    
    ImGui::Spacing();
    ImGui::TextDisabled("%s: %.2fx", TR(AccessibilitySystemDpiScale), m_dpiScale);
    ImGui::TextDisabled("%s: %.2fx", TR(AccessibilityEffectiveScale), m_dpiScale * m_accessibilitySettings.uiScale);
}

}
