// ============================================================================
// File: src/core/safety.cpp
// Purpose: Implementations for filesystem safety checks.
// OS Concept: Access control and namespace protection.
// ============================================================================

#include "safety.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace fx {

static std::string toUpper(std::string str) {
    for (char& c : str) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return str;
}

bool isProtectedPath(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::path absPath = std::filesystem::weakly_canonical(std::filesystem::absolute(path, ec), ec);
    if (ec) {
        absPath = path;
    }

    // 1. Filesystem root check (e.g. "C:\" or "/")
    if (absPath == absPath.root_path() || absPath.empty()) {
        return true;
    }

    // 2. Check against user home directory
    const char* userProfile = std::getenv("USERPROFILE");
    if (!userProfile) {
        userProfile = std::getenv("HOME");
    }
    if (userProfile) {
        std::filesystem::path homePath = std::filesystem::weakly_canonical(std::filesystem::u8path(userProfile), ec);
        if (!ec && absPath == homePath) {
            return true;
        }
    }

#if defined(_WIN32)
    // Protected Windows directories
    std::string pathStr = toUpper(absPath.string());
    // Normalise slashes for comparison
    std::replace(pathStr.begin(), pathStr.end(), '/', '\\');

    const char* windir = std::getenv("WINDIR");
    if (windir) {
        std::string winStr = toUpper(windir);
        std::replace(winStr.begin(), winStr.end(), '/', '\\');
        if (pathStr == winStr) {
            return true;
        }
    } else {
        if (pathStr == "C:\\WINDOWS" || pathStr == "C:\\WINDOWS\\SYSTEM32") {
            return true;
        }
    }
#else
    std::string pathStr = absPath.string();
    if (pathStr == "/" || pathStr == "/etc" || pathStr == "/usr" ||
        pathStr == "/bin" || pathStr == "/sbin" || pathStr == "/boot" ||
        pathStr == "/sys" || pathStr == "/proc" || pathStr == "/dev") {
        return true;
    }
#endif

    return false;
}

Result<bool> validateFileName(const std::string& name) {
    if (name.empty()) {
        return Result<bool>::failure("Invalid name. It cannot be empty or contain / \\ : * ? \" < > |.");
    }

    // Prohibit forbidden characters across platforms and specifically Windows
    const std::string forbidden = "/\\:*?\"<>|";
    for (char ch : name) {
        if (forbidden.find(ch) != std::string::npos || static_cast<unsigned char>(ch) < 32) {
            return Result<bool>::failure("Invalid name. It cannot be empty or contain / \\ : * ? \" < > |.");
        }
    }

    // Trailing dot or space is invalid on Windows filesystem
    if (name.back() == '.' || name.back() == ' ') {
        return Result<bool>::failure("Invalid name. It cannot end with a dot or space.");
    }

    // Check Windows reserved device names: CON, PRN, AUX, NUL, COM1-9, LPT1-9
    std::string baseName = name;
    size_t dotPos = baseName.find('.');
    if (dotPos != std::string::npos) {
        baseName.resize(dotPos);
    }
    std::string upperBase = toUpper(baseName);

    const std::string reserved[] = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    for (const auto& r : reserved) {
        if (upperBase == r) {
            return Result<bool>::failure("Invalid name. '" + name + "' is a reserved device name.");
        }
    }

    return Result<bool>::success(true);
}

bool isDestinationInsideSource(const std::filesystem::path& src, const std::filesystem::path& dest) {
    std::error_code ec;
    std::filesystem::path canonSrc = std::filesystem::weakly_canonical(src, ec);
    if (ec) canonSrc = src;
    std::filesystem::path canonDest = std::filesystem::weakly_canonical(dest, ec);
    if (ec) canonDest = dest;

    auto itSrc = canonSrc.begin();
    auto itDest = canonDest.begin();

    while (itSrc != canonSrc.end() && itDest != canonDest.end()) {
        if (*itSrc != *itDest) {
            return false;
        }
        ++itSrc;
        ++itDest;
    }

    // If we walked past all elements of src, dest is inside or identical to src
    return (itSrc == canonSrc.end());
}

} // namespace fx
