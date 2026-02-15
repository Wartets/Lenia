#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "LeniaEngine.hpp"
#include "UIOverlay.hpp"
#include <string>

namespace lenia {

class Application {
public:
    Application() = default;
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool init(int width, int height, const std::string& title);
    void run();

private:
    GLFWwindow*  m_window{nullptr};
    LeniaEngine  m_engine;
    UIOverlay    m_ui;
    LeniaParams  m_params;
    bool         m_paused{true};
    bool         m_showUI{true};
    bool         m_fullscreen{false};
    int          m_stepsPerFrame{1};
    int          m_windowW{960};
    int          m_windowH{640};
    int          m_savedWinX{0};
    int          m_savedWinY{0};
    int          m_savedWinW{960};
    int          m_savedWinH{640};
    float        m_simTimeMs{0.0f};
    bool         m_singleStepRequested{false};
    double       m_lastStepTime{0.0};
    bool         m_sKeyWasDown{false};
    int          m_lastBrushX{-1};
    int          m_lastBrushY{-1};
    float        m_lastBrushRawX{0.0f};
    float        m_lastBrushRawY{0.0f};
    int          m_lineStartX{-1};
    int          m_lineStartY{-1};
    int          m_ctrl1X{-1};
    int          m_ctrl1Y{-1};
    int          m_ctrl2X{-1};
    int          m_ctrl2Y{-1};
    bool         m_lineDrawing{false};
    bool         m_curveCtrl1Set{false};
    bool         m_curveCtrl2Set{false};
    UICallbacks  m_callbacks;

    bool         m_isPanning{false};
    double       m_panStartMouseX{0.0};
    double       m_panStartMouseY{0.0};
    float        m_panStartX{0.0f};
    float        m_panStartY{0.0f};

    bool initWindow(int width, int height, const std::string& title);
    bool initGL();
    void setupCallbacks();
    void processInput();
    void toggleFullscreen();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
};

}
