// ============================================================================
// File: src/core/navigator.cpp
// Purpose: Implementation of directory navigation and history.
// OS Concept: Path resolution, canonicalization, process CWD navigation.
// ============================================================================

#include "navigator.hpp"
#include <cstdlib>

namespace fx {

static std::filesystem::path getHomeDirectory() {
    const char* userProfile = std::getenv("USERPROFILE");
    if (!userProfile) {
        userProfile = std::getenv("HOME");
    }
    if (userProfile) {
        return std::filesystem::u8path(userProfile);
    }
    return std::filesystem::current_path();
}

Navigator::Navigator(const std::filesystem::path& startPath) {
    std::error_code ec;
    std::filesystem::path target = startPath;
    if (target.empty()) {
        target = std::filesystem::current_path(ec);
        if (ec) {
            target = getHomeDirectory();
        }
    }

    m_current = std::filesystem::weakly_canonical(std::filesystem::absolute(target, ec), ec);
    if (ec) {
        m_current = target;
    }
}

std::filesystem::path Navigator::resolvePath(const std::string& input, const std::filesystem::path& base) {
    std::error_code ec;
    std::filesystem::path resolved;

    if (input.empty()) {
        return base;
    }

    if (input == "~") {
        resolved = getHomeDirectory();
    } else if (input.rfind("~/", 0) == 0 || input.rfind("~\\", 0) == 0) {
        resolved = getHomeDirectory() / input.substr(2);
    } else {
        std::filesystem::path inputPath = std::filesystem::u8path(input);
        if (inputPath.is_absolute()) {
            resolved = inputPath;
        } else {
            resolved = base / inputPath;
        }
    }

    auto absPath = std::filesystem::absolute(resolved, ec);
    if (!ec) {
        auto canon = std::filesystem::weakly_canonical(absPath, ec);
        if (!ec) {
            return canon;
        }
        return absPath;
    }
    return resolved;
}

Result<bool> Navigator::cd(const std::string& target) {
    std::error_code ec;
    std::filesystem::path resolved = resolvePath(target, m_current);

    if (!std::filesystem::exists(resolved, ec)) {
        return Result<bool>::failure("Path not found: " + target);
    }

    if (!std::filesystem::is_directory(resolved, ec)) {
        return Result<bool>::failure("Not a directory: " + target);
    }

    // Keep current in back stack and update current
    m_backStack.push_back(m_current);
    m_current = resolved;
    m_forwardStack.clear();

    return Result<bool>::success(true);
}

Result<bool> Navigator::up() {
    std::filesystem::path parent = m_current.parent_path();

    // If at filesystem root (e.g. C:\ or /), stay at root and do not report an error
    if (parent.empty() || parent == m_current) {
        return Result<bool>::success(true);
    }

    m_backStack.push_back(m_current);
    m_current = parent;
    m_forwardStack.clear();

    return Result<bool>::success(true);
}

Result<bool> Navigator::back() {
    if (!canGoBack()) {
        return Result<bool>::failure("No previous directory in history.");
    }

    m_forwardStack.push_back(m_current);
    m_current = m_backStack.back();
    m_backStack.pop_back();

    return Result<bool>::success(true);
}

Result<bool> Navigator::forward() {
    if (!canGoForward()) {
        return Result<bool>::failure("No forward directory in history.");
    }

    m_backStack.push_back(m_current);
    m_current = m_forwardStack.back();
    m_forwardStack.pop_back();

    return Result<bool>::success(true);
}

std::filesystem::path Navigator::current() const {
    return m_current;
}

bool Navigator::canGoBack() const {
    return !m_backStack.empty();
}

bool Navigator::canGoForward() const {
    return !m_forwardStack.empty();
}

} // namespace fx
