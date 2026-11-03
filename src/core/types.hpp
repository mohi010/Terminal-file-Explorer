// ============================================================================
// File: src/core/types.hpp
// Purpose: Core data structures and Result template used across the file explorer.
// OS Concept: Represents directory entries, file attributes (metadata/inode concepts),
//             and safe error handling without throwing exceptions.
// ============================================================================

#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

namespace fx {

// Types of filesystem entries recognized by the system
enum class EntryType {
    File,
    Directory,
    Symlink,
    Other
};

// Metadata for a single filesystem entry (mirrors OS directory entry attributes)
struct EntryInfo {
    std::string name;
    std::filesystem::path path;
    EntryType type = EntryType::Other;
    std::uintmax_t size = 0;                         // 0 for directories
    std::filesystem::file_time_type modified{};
    std::filesystem::perms permissions = std::filesystem::perms::none;
    bool isHidden = false;
};

// Result container: avoids throwing uncaught exceptions to callers
template <class T>
struct Result {
    bool ok = false;
    T value{};
    std::string error; // Human-readable error message; empty when ok is true

    static Result<T> success(T val) {
        Result<T> res;
        res.ok = true;
        res.value = std::move(val);
        return res;
    }

    static Result<T> failure(std::string errMsg) {
        Result<T> res;
        res.ok = false;
        res.error = std::move(errMsg);
        return res;
    }
};

// Storage space attributes for a disk drive or mount point
struct SpaceInfo {
    std::uintmax_t capacity = 0;
    std::uintmax_t free = 0;
    std::uintmax_t available = 0;
};

// Sorting criteria for directory listings
enum class SortBy {
    Name,
    Size,
    Date,
    Type
};

// Options for listing a directory
struct ListOptions {
    bool showHidden = false;
    SortBy sortBy = SortBy::Name;
    bool ascending = true;
};

// Options for searching files recursively
struct SearchOptions {
    int maxDepth = 10;
    bool caseInsensitive = true;
    std::size_t maxResults = 500;
};

} // namespace fx
