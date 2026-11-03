// ============================================================================
// File: src/core/preview.hpp
// Purpose: Safe file previewer with binary content detection.
// OS Concept: File types, magic bytes, character encodings, and read buffers.
// ============================================================================

#pragma once

#include "types.hpp"
#include <string>
#include <filesystem>
#include <cstddef>

namespace fx {

// Reads up to maxBytes of a text file for safe previewing.
// Fails with an error message if the file is binary or not a regular file.
Result<std::string> readTextPreview(const std::filesystem::path& path, std::size_t maxBytes = 4096);

// Inspects the initial bytes of a buffer to detect binary signatures (such as NUL bytes)
bool isBinaryContent(const char* buffer, std::size_t length);

} // namespace fx
