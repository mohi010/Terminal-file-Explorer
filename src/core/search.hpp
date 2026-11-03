// ============================================================================
// File: src/core/search.hpp
// Purpose: Recursive file searching and directory size calculation.
// OS Concept: File tree traversal, depth-limited search, avoiding symlink loops,
//             and space accounting.
// ============================================================================

#pragma once

#include "types.hpp"
#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>

namespace fx {

// Searches for files and directories matching a pattern starting from root.
// Respects maxDepth, case sensitivity, and maxResults cap. Does not follow symlinks.
Result<std::vector<EntryInfo>> searchByName(
    const std::filesystem::path& root,
    const std::string& pattern,
    SearchOptions options = {});

// Recursively calculates the total disk space consumed by files in a directory tree.
// Skips symlinks and unreadable directories.
Result<std::uintmax_t> directorySize(const std::filesystem::path& path);

} // namespace fx
