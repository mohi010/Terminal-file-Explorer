// ============================================================================
// File: src/core/search.cpp
// Purpose: Implementations for recursive searching and size computation.
// OS Concept: Tree traversal algorithms, symlink handling, and error recovery.
// ============================================================================

#include "search.hpp"
#include "fs_ops.hpp"
#include <algorithm>
#include <cctype>

namespace fx {

static std::string toLowerString(std::string str) {
    for (char& c : str) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return str;
}

// Simple wildcard match (* and ?) or substring match
static bool matchesPattern(const std::string& text, const std::string& pattern, bool caseInsensitive) {
    std::string t = caseInsensitive ? toLowerString(text) : text;
    std::string p = caseInsensitive ? toLowerString(pattern) : pattern;

    // Check substring match
    if (t.find(p) != std::string::npos) {
        return true;
    }

    // Glob match check
    auto globMatch = [](const char* pat, const char* str, auto& self) -> bool {
        if (*pat == '\0') return *str == '\0';
        if (*pat == '*') {
            while (*(pat + 1) == '*') ++pat;
            if (*(pat + 1) == '\0') return true;
            while (*str != '\0') {
                if (self(pat + 1, str, self)) return true;
                ++str;
            }
            return self(pat + 1, str, self);
        }
        if (*pat == '?' || *pat == *str) {
            return (*str != '\0') && self(pat + 1, str + 1, self);
        }
        return false;
    };

    return globMatch(p.c_str(), t.c_str(), globMatch);
}

Result<std::vector<EntryInfo>> searchByName(
    const std::filesystem::path& root,
    const std::string& pattern,
    SearchOptions options)
{
    std::error_code ec;

    if (!std::filesystem::exists(root, ec)) {
        return Result<std::vector<EntryInfo>>::failure("Path not found: " + root.u8string());
    }

    if (!std::filesystem::is_directory(root, ec)) {
        return Result<std::vector<EntryInfo>>::failure("Not a directory: " + root.u8string());
    }

    std::vector<EntryInfo> results;

    // Do NOT pass follow_directory_symlink to avoid symlink loop recursion
    std::filesystem::recursive_directory_iterator it(
        root,
        std::filesystem::directory_options::skip_permission_denied,
        ec);
    std::filesystem::recursive_directory_iterator endIt;

    if (ec) {
        return Result<std::vector<EntryInfo>>::failure("Permission denied: " + root.u8string());
    }

    while (it != endIt) {
        if (results.size() >= options.maxResults) {
            break;
        }

        // Enforce maximum recursion depth
        if (it.depth() >= options.maxDepth) {
            it.disable_recursion_pending();
        }

        const auto& entryPath = it->path();
        std::string filename = entryPath.filename().u8string();

        if (matchesPattern(filename, pattern, options.caseInsensitive)) {
            auto infoRes = getInfo(entryPath);
            if (infoRes.ok) {
                results.push_back(infoRes.value);
            }
        }

        it.increment(ec);
        if (ec) {
            ec.clear(); // Skip unreadable subentries gracefully
        }
    }

    return Result<std::vector<EntryInfo>>::success(results);
}

Result<std::uintmax_t> directorySize(const std::filesystem::path& path) {
    std::error_code ec;

    if (!std::filesystem::exists(path, ec)) {
        return Result<std::uintmax_t>::failure("Path not found: " + path.u8string());
    }

    if (std::filesystem::is_regular_file(path, ec)) {
        auto sz = std::filesystem::file_size(path, ec);
        if (ec) {
            return Result<std::uintmax_t>::failure(ec.message());
        }
        return Result<std::uintmax_t>::success(sz);
    }

    if (!std::filesystem::is_directory(path, ec)) {
        return Result<std::uintmax_t>::success(0);
    }

    std::uintmax_t totalBytes = 0;

    std::filesystem::recursive_directory_iterator it(
        path,
        std::filesystem::directory_options::skip_permission_denied,
        ec);
    std::filesystem::recursive_directory_iterator endIt;

    if (ec) {
        return Result<std::uintmax_t>::failure("Permission denied: " + path.u8string());
    }

    while (it != endIt) {
        auto status = it->symlink_status(ec);
        if (!ec && !std::filesystem::is_symlink(status) && std::filesystem::is_regular_file(status)) {
            auto sz = it->file_size(ec);
            if (!ec) {
                totalBytes += sz;
            }
            ec.clear();
        }

        it.increment(ec);
        if (ec) {
            ec.clear();
        }
    }

    return Result<std::uintmax_t>::success(totalBytes);
}

} // namespace fx

