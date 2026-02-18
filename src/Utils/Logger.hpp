/**
 * @file Logger.hpp
 * @brief Thread-safe logging system with file output.
 * 
 * Usage: LOG_INFO("Message: %s", value);
 * Log levels: DEBUG, INFO, WARN, ERROR, FATAL
 * Logs are written to both stderr and timestamped files in log/
 */

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <filesystem>

namespace lenia {

/// Log severity levels
enum class LogLevel { Debug, Info, Warn, Error, Fatal };

class Logger {
public:
    static void init() {
        std::lock_guard<std::mutex> lk(mutex());
        if (instance().m_open) return;

        namespace fs = std::filesystem;
        fs::create_directories("log");

        std::time_t now = std::time(nullptr);
        std::tm     tm{};
#ifdef _WIN32
        localtime_s(&tm, &now);
#else
        localtime_r(&now, &tm);
#endif
        char fname[128];
        std::snprintf(fname, sizeof(fname),
                      "log/lenia_%04d-%02d-%02d_%02d-%02d-%02d.log",
                      tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                      tm.tm_hour, tm.tm_min, tm.tm_sec);

        instance().m_file.open(fname, std::ios::out | std::ios::trunc);
        instance().m_open = instance().m_file.is_open();

        if (instance().m_open) {
            const char* msg = "[INFO]  Logger started";
            std::fprintf(stderr, "%s  ->  %s\n", msg, fname);
            instance().m_file << msg << "  ->  " << fname << "\n";
            instance().m_file.flush();
        }
    }

    static void shutdown() {
        std::lock_guard<std::mutex> lk(mutex());
        if (instance().m_open) {
            instance().m_file.flush();
            instance().m_file.close();
            instance().m_open = false;
        }
    }

    template <typename... Args>
    static void log(LogLevel level, const char* fmt, Args&&... args) {
        std::lock_guard<std::mutex> lk(mutex());
        char buf[2048];
        std::snprintf(buf, sizeof(buf), fmt, std::forward<Args>(args)...);

        const char* tag = "";
        switch (level) {
            case LogLevel::Debug: tag = "[DEBUG] "; break;
            case LogLevel::Info:  tag = "[INFO]  "; break;
            case LogLevel::Warn:  tag = "[WARN]  "; break;
            case LogLevel::Error: tag = "[ERROR] "; break;
            case LogLevel::Fatal: tag = "[FATAL] "; break;
        }

        std::fprintf(stderr, "%s%s\n", tag, buf);

        if (instance().m_open) {
            instance().m_file << tag << buf << "\n";
            instance().m_file.flush();
        }
    }

private:
    std::ofstream m_file;
    bool          m_open{false};

    Logger() = default;
    static Logger& instance() { static Logger s; return s; }
    static std::mutex& mutex()  { static std::mutex m; return m; }
};

#define LOG_DEBUG(fmt, ...) ::lenia::Logger::log(::lenia::LogLevel::Debug, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  ::lenia::Logger::log(::lenia::LogLevel::Info,  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  ::lenia::Logger::log(::lenia::LogLevel::Warn,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) ::lenia::Logger::log(::lenia::LogLevel::Error, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) ::lenia::Logger::log(::lenia::LogLevel::Fatal, fmt, ##__VA_ARGS__)

}
