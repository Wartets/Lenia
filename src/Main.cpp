#include "Application.hpp"
#include "Utils/Logger.hpp"
#include <iostream>
#include <exception>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

static bool readShowConsoleConfig() {
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

int main() {
#ifdef _WIN32
    bool showConsole = readShowConsoleConfig();
    if (!showConsole) {
        FreeConsole();
    }
#endif

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
