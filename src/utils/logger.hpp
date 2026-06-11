#pragma once

#include <string>

namespace PhotoStudio::Utils {

// Lightweight, thread-safe logger writing to stderr (and optionally a file).
class Logger {
public:
    enum class Level { Debug, Info, Warning, Error };

    static void setLogFile(const std::string& path);
    static void setMinLevel(Level level);

    static void debug(const std::string& message) { log(Level::Debug, message); }
    static void info(const std::string& message) { log(Level::Info, message); }
    static void warning(const std::string& message) { log(Level::Warning, message); }
    static void error(const std::string& message) { log(Level::Error, message); }

private:
    static void log(Level level, const std::string& message);
};

} // namespace PhotoStudio::Utils
