/**
 * @file Application.hpp
 * @brief Main application class that manages window, input, and coordinates subsystems.
 */

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "LeniaEngine.hpp"
#include "UIOverlay.hpp"
#include <string>

namespace lenia {

/**
 * @brief Main application controller for Lenia Explorer.
 * 
 * Manages the application lifecycle including:
 * - Window creation and OpenGL context setup
 * - Input handling (keyboard, mouse, zoom, pan)
 * - Coordination between simulation engine and UI
 * - Main render loop timing
 */
class Application {
public:
    Application() = default;
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool init(int width, int height, const std::string& title);
    void run();

private:
    GLFWwindow*  m_window{nullptr};    // Main application window
    LeniaEngine  m_engine;             // Core simulation engine
    UIOverlay    m_ui;                 // ImGui-based user interface
    LeniaParams  m_params;             // Current simulation parameters
    bool         m_paused{true};       // Simulation pause state
    bool         m_showUI{true};       // UI visibility toggle
    bool         m_fullscreen{false};  // Fullscreen mode flag
    int          m_stepsPerFrame{1};   // Simulation steps per render frame
    int          m_windowW{960};       // Current window width
    int          m_windowH{640};       // Current window height
    int          m_savedWinX{0};       // Saved window position for fullscreen toggle
    int          m_savedWinY{0};
    int          m_savedWinW{960};     // Saved window size for fullscreen toggle
    int          m_savedWinH{640};
    float        m_simTimeMs{0.0f};    // Last simulation step duration
    bool         m_singleStepRequested{false};  // Manual step request while paused
    double       m_lastStepTime{0.0};  // Time of last manual step (for repeat stepping)
    bool         m_sKeyWasDown{false}; // Previous frame S key state
    
    // Brush drawing state
    int          m_lastBrushX{-1};     // Last brush position for continuous drawing
    int          m_lastBrushY{-1};
    float        m_lastBrushRawX{0.0f};
    float        m_lastBrushRawY{0.0f};
    
    // Line/shape drawing state
    int          m_lineStartX{-1};     // Starting point for line drawing
    int          m_lineStartY{-1};
    int          m_ctrl1X{-1};         // Bezier control points
    int          m_ctrl1Y{-1};
    int          m_ctrl2X{-1};
    int          m_ctrl2Y{-1};
    bool         m_lineDrawing{false};  // Currently drawing a line/shape
    bool         m_curveCtrl1Set{false};
    bool         m_curveCtrl2Set{false};
    UICallbacks  m_callbacks;           // UI action callbacks

    // Pan/drag state
    bool         m_isPanning{false};    // Currently panning the view
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
