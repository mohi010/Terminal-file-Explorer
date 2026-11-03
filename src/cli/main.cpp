// ============================================================================
// File: src/cli/main.cpp
// Purpose: Entry point for the Terminal CLI front end.
// OS Concept: Process entry point, REPL execution cycle, and standard streams.
// ============================================================================

#include "terminal.hpp"
#include "commands.hpp"
#include "../core/navigator.hpp"
#include <iostream>
#include <string>

int main() {
    using namespace fx::cli;

    // Set console to UTF-8 and enable ANSI escape sequences
    setupTerminal();

    std::cout << cyan("====================================================================\n")
              << boldBlue("       OS File Explorer (C++17 Core Library + Terminal CLI)         \n")
              << cyan("====================================================================\n")
              << "Course: Operating Systems, Unit IV (Storage Management)\n"
              << "Type " << yellow("help") << " to see all available commands, or "
              << yellow("exit") << " to quit.\n\n";

    fx::Navigator navigator;
    CommandContext ctx(navigator);

    std::string line;
    while (!ctx.shouldQuit()) {
        std::string currentPath = navigator.current().u8string();
        std::cout << "fx:" << boldBlue(currentPath) << "> ";

        if (!std::getline(std::cin, line)) {
            break; // EOF encountered (e.g. redirected script input finished)
        }

        auto tokens = tokenize(line);
        executeCommand(tokens, ctx);
    }

    std::cout << "\nExiting File Explorer. Goodbye!" << std::endl;
    return 0;
}

