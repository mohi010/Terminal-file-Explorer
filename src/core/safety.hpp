// ============================================================================
// File: src/core/safety.hpp
// Purpose: Path safety validation and filename rules.
// OS Concept: Operating system protection boundaries (system directories, root file systems,
//             and filesystem naming rules).
// ============================================================================

#pragma once

#include "types.hpp"
#include <string>
#include <filesystem>

namespace fx {

// Checks whether a path represents a protected system path or drive root
// that must never be deleted, overwritten, or moved.
bool isProtectedPath(const std::filesystem::path& path);

// Validates a single filename (not a full path).
// Rejects empty names, path separators, forbidden Windows characters, and reserved device names.
Result<bool> validateFileName(const std::string& name);

// Checks if destination is located inside source directory (copy into itself prevention)
bool isDestinationInsideSource(const std::filesystem::path& src, const std::filesystem::path& dest);

} // namespace fx

