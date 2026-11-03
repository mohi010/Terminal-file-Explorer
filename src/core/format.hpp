// ============================================================================
// File: src/core/format.hpp
// Purpose: Formatting utilities for file sizes, timestamps, and permissions.
// OS Concept: Translates raw binary metadata (byte counts, epochs, permission bitmasks)
//             into human-readable directory attributes.
// ============================================================================

#pragma once

#include <string>
#include <filesystem>
#include <cstdint>

namespace fx {

// Formats byte count into B, KB, MB, or GB (e.g., "1.2 MB" or "0 B")
std::string formatSize(std::uintmax_t bytes);

// Formats file modification time into "YYYY-MM-DD HH:MM" in local time
std::string formatTime(std::filesystem::file_time_type fileTime);

// Formats std::filesystem::perms into standard POSIX-style "rwxr-xr-x" string
std::string formatPermissions(std::filesystem::perms permissions);

} // namespace fx

