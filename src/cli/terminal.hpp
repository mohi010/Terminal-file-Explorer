// ============================================================================
// File: src/cli/terminal.hpp
// Purpose: Terminal console setup (UTF-8, ANSI virtual terminal sequences) and color utilities.
// OS Concept: Console subsystem, code pages, and terminal escape sequences.
// ============================================================================

#pragma once

#include <string>

namespace fx::cli {

// Initializes Windows console for UTF-8 code page and ANSI escape sequence processing
void setupTerminal();

// ANSI color escapes
namespace Color {
    inline const char* Reset   = "\033[0m";
    inline const char* Bold    = "\033[1m";
    inline const char* Red     = "\033[31m";
    inline const char* Green   = "\033[32m";
    inline const char* Yellow  = "\033[33m";
    inline const char* Blue    = "\033[34m";
    inline const char* Magenta = "\033[35m";
    inline const char* Cyan    = "\033[36m";
    inline const char* White   = "\033[37m";
}

std::string boldBlue(const std::string& text);
std::string red(const std::string& text);
std::string green(const std::string& text);
std::string yellow(const std::string& text);
std::string cyan(const std::string& text);

} // namespace fx::cli
