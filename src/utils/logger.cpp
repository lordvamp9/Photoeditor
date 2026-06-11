#include "utils/logger.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>

namespace PhotoStudio::Utils {

namespace {

std::mutex g_mutex;
std::ofstream g_file;
Logger::Level g_minLevel = Logger::Level::Info;

const char* levelTag(Logger::Level level)
{
    switch (level) {
    case Logger::Level::Debug:
        return "DEBUG";
    case Logger::Level::Info:
        return "INFO ";
    case Logger::Level::Warning:
        return "WARN ";
    case Logger::Level::Error:
        return "ERROR";
    }
    return "?    ";
}

std::string timestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[32];
#if defined(_WIN32)
    std::tm tm{};
    localtime_s(&tm, &t);
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
#else
    std::tm tm{};
    localtime_r(&t, &tm);
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
#endif
    return buf;
}

} // namespace

void Logger::setLogFile(const std::string& path)
{
    std::lock_guard lock(g_mutex);
    g_file.open(path, std::ios::app);
}

void Logger::setMinLevel(Level level)
{
    g_minLevel = level;
}

void Logger::log(Level level, const std::string& message)
{
    if (level < g_minLevel)
        return;
    std::lock_guard lock(g_mutex);
    const std::string line = "[" + timestamp() + "] [" + levelTag(level) + "] " + message;
    std::cerr << line << '\n';
    if (g_file.is_open())
        g_file << line << '\n';
}

} // namespace PhotoStudio::Utils
