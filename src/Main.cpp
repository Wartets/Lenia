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
            app.run();
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
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    bool showConsole = readShowConsoleConfig();
    if (showConsole) {
        AllocConsole();
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);
    }
    return runApplication();
}
#endif

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
