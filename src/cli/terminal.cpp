// ============================================================================
// File: src/cli/terminal.cpp
// Purpose: Implements terminal setup and color formatting.
// OS Concept: Standard I/O handles, console mode manipulation.
// ============================================================================

#include "terminal.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace fx::cli {

void setupTerminal() {
#if defined(_WIN32)
    // Set console output and input to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Enable Virtual Terminal Processing for ANSI color escape sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

std::string boldBlue(const std::string& text) {
    return std::string(Color::Bold) + Color::Blue + text + Color::Reset;
}

std::string red(const std::string& text) {
    return std::string(Color::Red) + text + Color::Reset;
}

std::string green(const std::string& text) {
    return std::string(Color::Green) + text + Color::Reset;
}

std::string yellow(const std::string& text) {
    return std::string(Color::Yellow) + text + Color::Reset;
}

std::string cyan(const std::string& text) {
    return std::string(Color::Cyan) + text + Color::Reset;
}

} // namespace fx::cli
