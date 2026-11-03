// ============================================================================
// File: src/core/fs_ops.hpp
// Purpose: Core file system operations (list, info, create, rename, remove, copy, move).
// OS Concept: System call abstractions for storage management, directory entry traversal,
//             and file manipulation without exceptions.
// ============================================================================

#pragma once

#include "types.hpp"
#include <vector>
#include <filesystem>

namespace fx {

// Lists entries in a directory with sorting and hidden filtering.
// Directories are always grouped before regular files.
Result<std::vector<EntryInfo>> listDirectory(const std::filesystem::path& path, ListOptions options = {});

// Retrieves detailed attributes for a single filesystem entry.
Result<EntryInfo> getInfo(const std::filesystem::path& path);

// Creates a new empty regular file. Fails if the file already exists.
Result<bool> createFile(const std::filesystem::path& path);

// Creates a new single directory (non-recursive). Fails if it already exists.
Result<bool> createDirectory(const std::filesystem::path& path);

// Renames an existing file or directory to a new filename in the same parent directory.
Result<bool> renamePath(const std::filesystem::path& from, const std::string& newName);

// Removes a file or directory. Non-empty directories require recursive = true.
Result<bool> removePath(const std::filesystem::path& path, bool recursive = false);

// Copies a file or directory tree into a target destination directory.
Result<bool> copyPath(const std::filesystem::path& from, const std::filesystem::path& toDir);

// Moves a file or directory into a target directory (rename first, fallback to copy+delete across drives).
Result<bool> movePath(const std::filesystem::path& from, const std::filesystem::path& toDir);

// Queries total capacity, free space, and available space on the filesystem of the given path.
Result<SpaceInfo> getSpace(const std::filesystem::path& path);

} // namespace fx

