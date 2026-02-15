#include "Application.hpp"
#include "LeniaEngine.hpp"
#include "Presets.hpp"
#include "Utils/Logger.hpp"
#include <glad/glad.h>
#include "Utils/GLUtils.hpp"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <imgui.h>
#include <chrono>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace lenia {

static bool loadShowConsoleConfig() {
    std::ifstream cfg("lenia_config.txt");
    if (!cfg.is_open()) return true;
    std::string line;
    while (std::getline(cfg, line)) {
        if (line.find("showConsole=") == 0) {
            return line.substr(12) == "1";
        }
    }
    return true;
}

static void glfwErrorCallback(int code, const char* description) {
    LOG_ERROR("GLFW error %d: %s", code, description);
}

Application::~Application() {
    m_ui.shutdown();
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool Application::init(int width, int height, const std::string& title) {
    if (!initWindow(width, height, title)) return false;
    if (!initGL()) return false;

    if (!m_engine.init("assets")) {
        LOG_ERROR("Engine initialisation failed.");
        return false;
    }

    if (!m_ui.init(m_window)) {
        LOG_ERROR("UI initialisation failed.");
        return false;
    }

    const auto& presets = getPresets();
    std::vector<std::string> names;
    names.reserve(presets.size());
    for (const auto& p : presets) names.emplace_back(p.name);
    m_ui.setPresetNames(names);

    m_callbacks = {
        .onReset = [this]() {
            m_engine.reset(m_params);
        },
        .onClear = [this]() {
            m_engine.clear();
        },
        .onRandomize = [this]() {
            m_engine.randomizeGrid(m_params);
        },
        .onKernelChanged = [this]() {
            m_engine.regenerateKernel(m_params);
        },
        .onGridResized = [this]() {
            m_engine.resizeGrid(m_params);
            m_engine.regenerateKernel(m_params);
        },
        .onPresetSelected = [this](int idx) {
            m_engine.applyPreset(idx, m_params);
            m_stepsPerFrame = 7;
            m_engine.reset(m_params);
        },
        .onKernelPresetSelected = [this](int idx) {
            m_engine.applyKernelPreset(idx, m_params);
        },
        .onChannelModeChanged = [this](int numCh) {
            m_engine.switchChannelMode(m_params, numCh);
        },
        .onRuleKernelChanged = [this](int ruleIdx) {
            m_engine.regenerateRuleKernel(ruleIdx, m_params);
        },
        .getRuleKernelInfo = [this](int ruleIdx) -> std::pair<GLuint,int> {
            return { m_engine.ruleKernelTexture(ruleIdx), m_engine.ruleKernelDiameter(ruleIdx) };
        },
        .onFlipHorizontal = [this](bool) {
            m_engine.flipGridHorizontal();
        },
        .onFlipVertical = [this](bool) {
            m_engine.flipGridVertical();
        },
        .onRotateGrid = [this](int direction) {
            m_engine.rotateGrid(direction, m_params);
        },
        .getCellValue = [this](int x, int y) -> float {
            return m_engine.getCellValue(x, y);
        },
        .getSpeciesPreviewData = [this](int presetIdx, int& rows, int& cols, int& channels) -> std::vector<float> {
            const auto& presets = getPresets();
            const auto& mcPresets = getMultiChannelPresets();
            if (presetIdx < 0 || presetIdx >= static_cast<int>(presets.size()))
                return {};
            const auto& p = presets[presetIdx];
            bool isMC = (std::strcmp(p.category, "Multichannel") == 0);
            if (isMC) {
                for (const auto& mcp : mcPresets) {
                    if (std::strcmp(mcp.name, p.name) == 0 && mcp.cellsCh0) {
                        rows = mcp.cellRows;
                        cols = mcp.cellCols;
                        channels = (std::min)(mcp.numChannels, 3);
                        int total = rows * cols * channels;
                        std::vector<float> data(total, 0.0f);
                        const float* ch[3] = {mcp.cellsCh0, mcp.cellsCh1, mcp.cellsCh2};
                        for (int i = 0; i < rows * cols; ++i) {
                            for (int c = 0; c < channels; ++c)
                                data[i * channels + c] = ch[c] ? ch[c][i] : 0.0f;
                        }
                        return data;
                    }
                }
            }
            if (p.cellData && p.cellRows > 0 && p.cellCols > 0) {
                rows = p.cellRows;
                cols = p.cellCols;
                channels = 1;
                return std::vector<float>(p.cellData, p.cellData + rows * cols);
            }
            return {};
        },
        .onBrushApply = [this](int x, int y, const LeniaParams& params) {
            m_engine.applyBrush(x, y, params);
        },
        .onBrushLine = [this](int x0, int y0, int x1, int y1, const LeniaParams& params) {
            m_engine.applyBrushLine(x0, y0, x1, y1, params);
        },
        .onBrushCurve = [this](const std::vector<std::pair<int,int>>& points, const LeniaParams& params) {
            m_engine.applyBrushCurve(points, params);
        },
        .onWallApply = [this](int x, int y, const LeniaParams& params) {
            m_engine.applyWall(x, y, params);
        },
        .onWallLine = [this](int x0, int y0, int x1, int y1, const LeniaParams& params) {
            m_engine.applyWallLine(x0, y0, x1, y1, params);
        },
        .onWallCurve = [this](const std::vector<std::pair<int,int>>& points, const LeniaParams& params) {
            m_engine.applyWallCurve(points, params);
        },
        .onClearWalls = [this]() {
            m_engine.clearWalls();
        },
    };
    m_ui.setCallbacks(m_callbacks);

    const auto& kPresets = getKernelPresets();
    std::vector<std::string> kpNames;
    kpNames.reserve(kPresets.size());
    for (const auto& kp : kPresets) kpNames.emplace_back(kp.name);
    m_ui.setKernelPresetNames(kpNames);

    int defaultPreset = 0;
    for (int i = 0; i < static_cast<int>(presets.size()); ++i) {
        if (std::strcmp(presets[i].name, "Emitter (Glider Gun)") == 0) {
            defaultPreset = i;
            break;
        }
    }

    m_engine.applyPreset(defaultPreset, m_params);
    m_params.placementMode = static_cast<int>(PlacementMode::Scatter);
    m_params.placementCount = 6;
    m_params.showConsoleOnStartup = loadShowConsoleConfig();
    m_stepsPerFrame = 8;
    m_engine.reset(m_params);
    m_paused = true;

    m_ui.setSelectedPreset(defaultPreset);
    const auto& cats = getPresetCategories();
    for (int i = 0; i < static_cast<int>(cats.size()); ++i) {
        if (cats[i] == "Multichannel") { m_ui.setSelectedCategory(i); break; }
    }

    m_engine.loadCustomColormaps("colormap");
    m_ui.setCustomColormapNames(m_engine.customColormapNames());

    LOG_INFO("Application initialised successfully.");
    return true;
}

void Application::run() {
    LOG_INFO("Entering main loop.");
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        processInput();

        bool doSim = !m_paused;

        if (m_paused) {
            bool sDown = glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS;

            if (m_singleStepRequested) {
                doSim = true;
                m_singleStepRequested = false;
            } else if (sDown && !m_sKeyWasDown) {
                m_lastStepTime = glfwGetTime();
            } else if (sDown && m_sKeyWasDown) {
                double now = glfwGetTime();
                bool shiftHeld = glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                 glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
                double interval = shiftHeld ? 0.1 : 0.2;
                if (now - m_lastStepTime >= interval) {
                    doSim = true;
                    m_lastStepTime = now;
                }
            }

            m_sKeyWasDown = sDown;
        }

        if (doSim) {
            int steps = m_stepsPerFrame;
            auto t0 = std::chrono::high_resolution_clock::now();
            if (m_params.numKernelRules > 0) {
                m_engine.updateMultiChannel(m_params, steps);
            } else {
                m_engine.update(m_params, steps);
            }
            auto t1 = std::chrono::high_resolution_clock::now();
            m_simTimeMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
        }

        if (m_params.showAnalysis && doSim) {
            m_engine.runAnalysis(m_params.analysisThreshold);
            if (!m_paused && m_params.autoPause) {
                const auto& amgr = m_engine.analysisMgr();
                if (amgr.isEmpty() || amgr.isStabilized()) {
                    m_paused = true;
                    m_ui.triggerPauseOverlay(true);
                }
            }
        }

        m_engine.render(m_windowW, m_windowH, m_params);

        int mouseGridX = -1, mouseGridY = -1;
        float mouseValue = 0.0f;
        bool mouseInGrid = false;
        double mx, my;
        glfwGetCursorPos(m_window, &mx, &my);
        if (!ImGui::GetIO().WantCaptureMouse) {
            float aspect = static_cast<float>(m_windowW) / static_cast<float>(m_windowH);
            float gridAspect = static_cast<float>(m_params.gridW) / static_cast<float>(m_params.gridH);
            float scaleX, scaleY;
            if (aspect > gridAspect) {
                scaleY = m_params.zoom;
                scaleX = scaleY * gridAspect / aspect;
            } else {
                scaleX = m_params.zoom;
                scaleY = scaleX * aspect / gridAspect;
            }
            float ndcX = (static_cast<float>(mx) / m_windowW) * 2.0f - 1.0f;
            float ndcY = 1.0f - (static_cast<float>(my) / m_windowH) * 2.0f;
            float uvX = ((ndcX - m_params.panX * scaleX) / scaleX + 1.0f) * 0.5f;
            float uvY = ((ndcY - m_params.panY * scaleY) / scaleY + 1.0f) * 0.5f;

            float rawGridX = uvX * m_params.gridW;
            float rawGridY = uvY * m_params.gridH;

            auto wrapCoord = [](int c, int size, int mode) -> int {
                if (mode == 0) {
                    c = c % size;
                    if (c < 0) c += size;
                    return c;
                }
                if (c < 0 || c >= size) return -1;
                return c;
            };

            int wrappedX = wrapCoord(static_cast<int>(std::floor(rawGridX)), m_params.gridW, m_params.edgeModeX);
            int wrappedY = wrapCoord(static_cast<int>(std::floor(rawGridY)), m_params.gridH, m_params.edgeModeY);

            if (wrappedX >= 0 && wrappedY >= 0) {
                mouseGridX = wrappedX;
                mouseGridY = wrappedY;
                mouseInGrid = true;
                mouseValue = m_engine.getCellValue(mouseGridX, mouseGridY);
            }

            bool canInteract = (m_params.edgeModeX == 0 && m_params.edgeModeY == 0) ||
                               (wrappedX >= 0 && wrappedY >= 0);

            int currentX = static_cast<int>(std::floor(rawGridX));
            int currentY = static_cast<int>(std::floor(rawGridY));
            if (m_params.edgeModeX == 0) {
                currentX = currentX % m_params.gridW;
                if (currentX < 0) currentX += m_params.gridW;
            }
            if (m_params.edgeModeY == 0) {
                currentY = currentY % m_params.gridH;
                if (currentY < 0) currentY += m_params.gridH;
            }

            bool leftPressed = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
            bool leftReleased = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE;
            bool rightPressed = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

            auto generateArcCurve = [](int cx, int cy, float radius, float startAngle, float endAngle, int numPoints) {
                std::vector<std::pair<int,int>> points;
                for (int i = 0; i <= numPoints; ++i) {
                    float t = static_cast<float>(i) / numPoints;
                    float angle = startAngle + (endAngle - startAngle) * t;
                    int px = static_cast<int>(cx + radius * std::cos(angle));
                    int py = static_cast<int>(cy + radius * std::sin(angle));
                    points.emplace_back(px, py);
                }
                return points;
            };

            bool isWallMode = m_params.wallEnabled;
            
            auto applyPoint = [&](int x, int y) {
                if (isWallMode) {
                    if (m_callbacks.onWallApply) m_callbacks.onWallApply(x, y, m_params);
                } else {
                    if (m_callbacks.onBrushApply) m_callbacks.onBrushApply(x, y, m_params);
                }
            };
            
            auto applyLine = [&](int x0, int y0, int x1, int y1) {
                if (isWallMode) {
                    if (m_callbacks.onWallLine) m_callbacks.onWallLine(x0, y0, x1, y1, m_params);
                } else {
                    if (m_callbacks.onBrushLine) m_callbacks.onBrushLine(x0, y0, x1, y1, m_params);
                }
            };
            
            auto applyCurve = [&](const std::vector<std::pair<int,int>>& pts) {
                if (isWallMode) {
                    if (m_callbacks.onWallCurve) m_callbacks.onWallCurve(pts, m_params);
                } else {
                    if (m_callbacks.onBrushCurve) m_callbacks.onBrushCurve(pts, m_params);
                }
            };

            if (m_params.brushEnabled && canInteract) {
                int drawMode = m_params.brushDrawMode;

                if (drawMode == 0) {
                    if (leftPressed) {
                        float dx = currentX - m_lastBrushX;
                        float dy = currentY - m_lastBrushY;
                        if (m_params.edgeModeX == 0 && std::abs(dx) > m_params.gridW / 2) {
                            dx = (dx > 0) ? dx - m_params.gridW : dx + m_params.gridW;
                        }
                        if (m_params.edgeModeY == 0 && std::abs(dy) > m_params.gridH / 2) {
                            dy = (dy > 0) ? dy - m_params.gridH : dy + m_params.gridH;
                        }
                        float dist = std::sqrt(dx * dx + dy * dy);
                        float spacing = m_params.brushSpacing * m_params.brushSize;
                        if (spacing < 1.0f) spacing = 1.0f;

                        if (m_lastBrushX < 0 || dist >= spacing) {
                            if (m_params.brushSmooth && m_lastBrushX >= 0 && dist > spacing) {
                                int steps = static_cast<int>(dist / spacing);
                                for (int i = 0; i <= steps; ++i) {
                                    float t = static_cast<float>(i) / steps;
                                    int bx = static_cast<int>(m_lastBrushX + dx * t);
                                    int by = static_cast<int>(m_lastBrushY + dy * t);
                                    if (m_params.edgeModeX == 0) {
                                        bx = bx % m_params.gridW;
                                        if (bx < 0) bx += m_params.gridW;
                                    }
                                    if (m_params.edgeModeY == 0) {
                                        by = by % m_params.gridH;
                                        if (by < 0) by += m_params.gridH;
                                    }
                                    if (bx >= 0 && bx < m_params.gridW && by >= 0 && by < m_params.gridH) {
                                        applyPoint(bx, by);
                                    }
                                }
                            } else {
                                if (currentX >= 0 && currentX < m_params.gridW && currentY >= 0 && currentY < m_params.gridH) {
                                    applyPoint(currentX, currentY);
                                }
                            }
                            m_lastBrushX = currentX;
                            m_lastBrushY = currentY;
                        }
                    } else if (leftReleased) {
                        m_lastBrushX = -1;
                        m_lastBrushY = -1;
                    }
                } else if (drawMode == 1) {
                    if (leftPressed && !m_lineDrawing) {
                        m_lineStartX = currentX;
                        m_lineStartY = currentY;
                        m_lineDrawing = true;
                    } else if (leftReleased && m_lineDrawing) {
                        applyLine(m_lineStartX, m_lineStartY, currentX, currentY);
                        m_lineDrawing = false;
                        m_lineStartX = -1;
                        m_lineStartY = -1;
                    }
                    m_params.brushLineStartX = m_lineStartX;
                    m_params.brushLineStartY = m_lineStartY;
                    m_params.brushLineEndX = currentX;
                    m_params.brushLineEndY = currentY;
                    m_params.brushLineDrawing = m_lineDrawing;
                    m_params.wallLineDrawing = m_lineDrawing;
                    m_params.wallStartX = m_lineStartX;
                    m_params.wallStartY = m_lineStartY;
                    m_params.wallEndX = currentX;
                    m_params.wallEndY = currentY;
                } else if (drawMode == 2) {
                    if (leftPressed && !m_lineDrawing) {
                        m_lineStartX = currentX;
                        m_lineStartY = currentY;
                        m_lineDrawing = true;
                    } else if (leftReleased && m_lineDrawing) {
                        float radius = std::sqrt(static_cast<float>((currentX - m_lineStartX) * (currentX - m_lineStartX) +
                                                                    (currentY - m_lineStartY) * (currentY - m_lineStartY)));
                        auto points = generateArcCurve(m_lineStartX, m_lineStartY, radius, 0.0f, 6.28318f, 64);
                        applyCurve(points);
                        m_lineDrawing = false;
                        m_lineStartX = -1;
                    }
                    m_params.brushLineStartX = m_lineStartX;
                    m_params.brushLineStartY = m_lineStartY;
                    m_params.brushLineEndX = currentX;
                    m_params.brushLineEndY = currentY;
                    m_params.brushLineDrawing = m_lineDrawing;
                    m_params.wallLineDrawing = m_lineDrawing;
                    m_params.wallStartX = m_lineStartX;
                    m_params.wallStartY = m_lineStartY;
                    m_params.wallEndX = currentX;
                    m_params.wallEndY = currentY;
                } else if (drawMode == 3) {
                    if (leftPressed && !m_lineDrawing) {
                        m_lineStartX = currentX;
                        m_lineStartY = currentY;
                        m_lineDrawing = true;
                    } else if (leftReleased && m_lineDrawing) {
                        std::vector<std::pair<int,int>> points;
                        int x0 = m_lineStartX, y0 = m_lineStartY;
                        int x1 = currentX, y1 = currentY;
                        points.emplace_back(x0, y0);
                        points.emplace_back(x1, y0);
                        points.emplace_back(x1, y1);
                        points.emplace_back(x0, y1);
                        points.emplace_back(x0, y0);
                        applyCurve(points);
                        m_lineDrawing = false;
                        m_lineStartX = -1;
                    }
                    m_params.brushLineStartX = m_lineStartX;
                    m_params.brushLineStartY = m_lineStartY;
                    m_params.brushLineEndX = currentX;
                    m_params.brushLineEndY = currentY;
                    m_params.brushLineDrawing = m_lineDrawing;
                    m_params.wallLineDrawing = m_lineDrawing;
                    m_params.wallStartX = m_lineStartX;
                    m_params.wallStartY = m_lineStartY;
                    m_params.wallEndX = currentX;
                    m_params.wallEndY = currentY;
                }

                if (rightPressed) {
                    m_lineDrawing = false;
                    m_curveCtrl1Set = false;
                    m_curveCtrl2Set = false;
                    m_lineStartX = -1;
                    m_params.brushLineDrawing = false;
                    m_params.wallLineDrawing = false;
                }
            }

            if (!m_params.brushEnabled) {
                if (leftReleased) {
                    m_lastBrushX = -1;
                    m_lastBrushY = -1;
                }
            }
        }

        float currentFps = ImGui::GetIO().Framerate;
        float dt = 1000.0f / (currentFps > 0.0f ? currentFps : 60.0f) / 1000.0f;
        m_ui.updatePauseOverlay(dt);

        // Query GPU memory info (NVIDIA extension)
        if (m_params.showResourceMonitor) {
            // GL_NVX_gpu_memory_info constants
            constexpr int GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX = 0x9047;
            constexpr int GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX = 0x9049;
            
            GLint totalMem = 0, availMem = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &totalMem);
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availMem);
            
            if (totalMem > 0) {
                m_params.gpuMemoryTotalMB = totalMem / 1024;
                m_params.gpuMemoryUsedMB = (totalMem - availMem) / 1024;
            }
        }

        m_ui.beginFrame();
        m_ui.render(m_params, m_paused, m_stepsPerFrame, m_showUI,
                    &m_engine.analysisData(), &m_engine.analysisMgr(),
                    m_engine.kernelTexture(), m_engine.kernelDiameter(),
                    m_engine.stepCount(), m_simTimeMs,
                    mouseGridX, mouseGridY, mouseValue, mouseInGrid);
        m_ui.renderPauseOverlay(m_windowW, m_windowH);
        m_ui.endFrame();

        glfwSwapBuffers(m_window);
    }
    LOG_INFO("Main loop ended.");
}

void Application::processInput() {
    ImGuiIO& io = ImGui::GetIO();
    bool uiFocused = io.WantCaptureKeyboard;

    float panSpeed = 0.01f / m_params.zoom;
    if (!uiFocused) {
        if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
            m_params.panX -= panSpeed;
        if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            m_params.panX += panSpeed;
        if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
            m_params.panY += panSpeed;
        if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
            m_params.panY -= panSpeed;
    }
}

void Application::toggleFullscreen() {
    if (!m_fullscreen) {
        glfwGetWindowPos(m_window, &m_savedWinX, &m_savedWinY);
        glfwGetWindowSize(m_window, &m_savedWinW, &m_savedWinH);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        m_fullscreen = true;
    } else {
        glfwSetWindowMonitor(m_window, nullptr, m_savedWinX, m_savedWinY, m_savedWinW, m_savedWinH, 0);
        m_fullscreen = false;
    }
}

static GLFWwindow* tryCreateWindow(int major, int minor,
                                   int w, int h, const char* title, bool debug) {
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (debug)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    return glfwCreateWindow(w, h, title, nullptr, nullptr);
}

bool Application::initWindow(int width, int height, const std::string& title) {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        LOG_FATAL("GLFW initialisation failed.");
        return false;
    }
    LOG_INFO("GLFW %s initialised.", glfwGetVersionString());

    struct { int major, minor; } versions[] = {
        {4, 6}, {4, 5}, {4, 3}
    };

    for (auto [maj, min] : versions) {
        LOG_INFO("Trying OpenGL %d.%d Core (debug=true)...", maj, min);
        m_window = tryCreateWindow(maj, min, width, height, title.c_str(), true);
        if (m_window) break;

        LOG_WARN("OpenGL %d.%d with debug context failed, retrying without debug...", maj, min);
        m_window = tryCreateWindow(maj, min, width, height, title.c_str(), false);
        if (m_window) break;
    }

    if (!m_window) {
        LOG_FATAL("Could not create an OpenGL 4.3+ window on any GPU.");
        LOG_FATAL("Check your GPU drivers support OpenGL 4.3 or higher.");
        glfwTerminate();
        return false;
    }

    int iconW = 0, iconH = 0, iconChannels = 0;
    unsigned char* iconData = nullptr;
    
    const char* iconPaths[] = {
        "assets/icon.png",
        "../assets/icon.png",
        "icon.png"
    };
    
    for (const char* path : iconPaths) {
        iconData = stbi_load(path, &iconW, &iconH, &iconChannels, 4);
        if (iconData && iconW > 0 && iconH > 0) {
            LOG_INFO("Loaded window icon from %s (%dx%d)", path, iconW, iconH);
            break;
        }
    }
    
    if (iconData && iconW > 0 && iconH > 0) {
        GLFWimage icon;
        icon.width = iconW;
        icon.height = iconH;
        icon.pixels = iconData;
        glfwSetWindowIcon(m_window, 1, &icon);
        stbi_image_free(iconData);
    } else {
        LOG_WARN("Could not load assets/icon.png, using procedural icon");
        static const int iconWidth = 64;
        static const int iconHeight = 64;
        static unsigned char iconPixels[iconWidth * iconHeight * 4];
        for (int y = 0; y < iconHeight; ++y) {
            for (int x = 0; x < iconWidth; ++x) {
                int idx = (y * iconWidth + x) * 4;
                float fx = (x - iconWidth / 2.0f) / (iconWidth / 2.0f);
                float fy = (y - iconHeight / 2.0f) / (iconHeight / 2.0f);
                float d1 = std::sqrt(std::pow(fx + 0.15f, 2) + std::pow(fy + 0.1f, 2));
                float d2 = std::sqrt(std::pow(fx - 0.20f, 2) + std::pow(fy - 0.25f, 2));
                float d3 = std::sqrt(std::pow(fx + 0.25f, 2) + std::pow(fy - 0.30f, 2));
                float v = 1.1f * std::exp(-7.0f * d1 * d1) + 0.6f * std::exp(-12.0f * d2 * d2) + 0.4f * std::exp(-18.0f * d3 * d3);
                float ring = 0.25f * std::exp(-35.0f * std::pow(std::sqrt(fx*fx + fy*fy) - 0.5f, 2));
                float val = std::clamp(v + ring, 0.0f, 1.2f);
                int r = static_cast<int>(130 * std::pow(val, 3.0f));
                int g = static_cast<int>(255 * std::pow(val, 1.5f));
                int b = static_cast<int>(255 * std::sqrt(val));
                iconPixels[idx + 0] = static_cast<unsigned char>(std::clamp(r, 0, 255));
                iconPixels[idx + 1] = static_cast<unsigned char>(std::clamp(g, 0, 255));
                iconPixels[idx + 2] = static_cast<unsigned char>(std::clamp(b, 0, 255));
                float alpha = val > 0.1f ? (val > 1.0f ? 1.0f : val) : 0.0f;
                iconPixels[idx + 3] = static_cast<unsigned char>(alpha * 255);
            }
        }
        GLFWimage icon;
        icon.width = iconWidth;
        icon.height = iconHeight;
        icon.pixels = iconPixels;
        glfwSetWindowIcon(m_window, 1, &icon);
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    m_windowW = width;
    m_windowH = height;

    glfwSetWindowUserPointer(m_window, this);
    setupCallbacks();

    return true;
}

bool Application::initGL() {
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        LOG_FATAL("GLAD loader failed.");
        return false;
    }

    const char* vendor   = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    LOG_INFO("GL Vendor:   %s", vendor   ? vendor   : "(null)");
    LOG_INFO("GL Renderer: %s", renderer ? renderer : "(null)");
    LOG_INFO("GL Version:  %s", version  ? version  : "(null)");

    GLint ctxFlags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &ctxFlags);
    if (ctxFlags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
                              0, nullptr, GL_FALSE);
        LOG_INFO("OpenGL debug output enabled.");
    } else {
        LOG_WARN("OpenGL debug context not available; debug output disabled.");
    }

    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);

    return true;
}

void Application::setupCallbacks() {
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->m_windowW = width;
        app->m_windowH = height;
    }
}

void Application::keyCallback(GLFWwindow* window, int key, int, int action, int) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    if (action != GLFW_PRESS) return;

    switch (key) {
        case GLFW_KEY_SPACE:
            app->m_paused = !app->m_paused;
            app->m_ui.triggerPauseOverlay(app->m_paused);
            break;
        case GLFW_KEY_R:
            app->m_engine.reset(app->m_params);
            break;
        case GLFW_KEY_C:
            app->m_engine.clear();
            break;
        case GLFW_KEY_TAB:
            app->m_showUI = !app->m_showUI;
            break;
        case GLFW_KEY_F11:
            app->toggleFullscreen();
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_HOME:
            app->m_params.zoom = 1.0f;
            app->m_params.panX = 0.0f;
            app->m_params.panY = 0.0f;
            break;
        case GLFW_KEY_EQUAL:
        case GLFW_KEY_KP_ADD:
            app->m_params.zoom *= 1.25f;
            break;
        case GLFW_KEY_MINUS:
        case GLFW_KEY_KP_SUBTRACT:
            app->m_params.zoom = std::max(0.1f, app->m_params.zoom / 1.25f);
            break;
        case GLFW_KEY_1: app->m_stepsPerFrame = 1; break;
        case GLFW_KEY_2: app->m_stepsPerFrame = 2; break;
        case GLFW_KEY_3: app->m_stepsPerFrame = 5; break;
        case GLFW_KEY_4: app->m_stepsPerFrame = 10; break;
        case GLFW_KEY_5: app->m_stepsPerFrame = 20; break;
        case GLFW_KEY_S:
            if (app->m_paused) {
                app->m_singleStepRequested = true;
            }
            break;
    }
}

void Application::scrollCallback(GLFWwindow* window, double, double yoffset) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    float uvX = static_cast<float>(mx) / static_cast<float>(app->m_windowW) - 0.5f;
    float uvY = 0.5f - static_cast<float>(my) / static_cast<float>(app->m_windowH);

    float gridAspect = (app->m_params.gridH > 0) ?
        static_cast<float>(app->m_params.gridW) / static_cast<float>(app->m_params.gridH) : 1.0f;
    float viewAspect = (app->m_windowH > 0) ?
        static_cast<float>(app->m_windowW) / static_cast<float>(app->m_windowH) : 1.0f;
    float relAspect = viewAspect / gridAspect;
    if (relAspect > 1.0f) uvX *= relAspect;
    else                  uvY /= relAspect;

    float oldZoom = app->m_params.zoom;
    float factor = yoffset > 0 ? 1.1f : 1.0f / 1.1f;
    float newZoom = std::clamp(oldZoom * factor, 0.1f, 20.0f);

    float worldX = uvX / oldZoom - app->m_params.panX + 0.5f;
    float worldY = uvY / oldZoom - app->m_params.panY + 0.5f;

    app->m_params.panX = 0.5f + uvX / newZoom - worldX;
    app->m_params.panY = 0.5f + uvY / newZoom - worldY;
    app->m_params.zoom = newZoom;
}

}
