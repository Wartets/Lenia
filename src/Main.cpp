/**
 * @file Main.cpp
 * @brief Entry point for the Lenia Explorer application.
 * 
 * Lenia is a continuous cellular automaton system that generalizes
 * Conway's Game of Life. This application provides a GPU-accelerated
 * simulation and visualization environment.
 * 
 * @see https://arxiv.org/abs/1812.05433 - Original Lenia paper by Bert Chan
 */

#include "Application.hpp"
#include "Utils/Logger.hpp"
#include <iostream>
#include <exception>
#include <fstream>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/**
 * @brief Reads the console visibility setting from config file.
 * @return true if console should be shown, false otherwise.
 */
static bool readShowConsoleConfig() {
    std::ifstream cfg("lenia_config.txt");
    if (!cfg.is_open()) return false;
    std::string line;
    while (std::getline(cfg, line)) {
        if (line.find("showConsole=") == 0) {
            return line.substr(12) == "1";
        }
    }
    return false;
}

/**
 * @brief Initializes and runs the Lenia application.
 * 
 * Sets up logging, creates the application instance, and enters
 * the main loop. Handles all top-level exceptions.
 * 
 * @return EXIT_SUCCESS on clean exit, EXIT_FAILURE on error.
 */
static int runApplication() {
    lenia::Logger::init();
    LOG_INFO("===== Lenia starting =====");

    int exitCode = EXIT_SUCCESS;
    try {
        lenia::Application app;

        if (!app.init(960, 640, "Lenia Explorer")) {
            LOG_FATAL("Application initialisation failed. See messages above.");
            exitCode = EXIT_FAILURE;
        } else {
            app.run();  // Enter main simulation loop
        }
    } catch (const std::exception& e) {
        LOG_FATAL("Unhandled exception: %s", e.what());
        exitCode = EXIT_FAILURE;
    } catch (...) {
        LOG_FATAL("Unhandled unknown exception.");
        exitCode = EXIT_FAILURE;
    }

    LOG_INFO("===== Lenia exiting (code %d) =====", exitCode);
    lenia::Logger::shutdown();
    return exitCode;
}

#ifdef _WIN32
// Windows GUI subsystem entry point (no console window by default)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    bool showConsole = readShowConsoleConfig();
    if (showConsole) {
        AllocConsole();  // Create console window for debug output
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);
    }
    return runApplication();
}
#endif

// Standard console entry point
int main() {
#ifdef _WIN32
    bool showConsole = readShowConsoleConfig();
    if (showConsole) {
        AllocConsole();
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);
    }
#endif
    return runApplication();
}
