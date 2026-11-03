// ============================================================================
// File: src/core/navigator.hpp
// Purpose: Directory navigation manager with history stacks (back/forward).
// OS Concept: Current Working Directory (CWD) state, path resolution, and traversal.
// ============================================================================

#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace fx {

class Navigator {
public:
    // Initializes navigator at the given starting path (or current working directory if empty)
    explicit Navigator(const std::filesystem::path& startPath = {});

    // Navigates to a new path. Clears the forward history on success.
    // Supports ~, relative paths, .., and absolute paths.
    Result<bool> cd(const std::string& target);

    // Navigates up to the parent directory.
    // At root (e.g. C:\ or /), stays at root and returns success with informative status.
    Result<bool> up();

    // Navigates backward in history
    Result<bool> back();

    // Navigates forward in history
    Result<bool> forward();

    // Gets the current normalized directory path
    std::filesystem::path current() const;

    // Checks whether back navigation is possible
    bool canGoBack() const;

    // Checks whether forward navigation is possible
    bool canGoForward() const;

    // Resolves a user-provided path string relative to current directory and expands ~
    static std::filesystem::path resolvePath(const std::string& input, const std::filesystem::path& base);

private:
    std::filesystem::path m_current;
    std::vector<std::filesystem::path> m_backStack;
    std::vector<std::filesystem::path> m_forwardStack;
};

} // namespace fx

