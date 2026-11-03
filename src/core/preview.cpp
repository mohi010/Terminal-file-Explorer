// ============================================================================
// File: src/core/preview.cpp
// Purpose: Implementations for text file preview and binary detection.
// OS Concept: I/O buffers, EOF handling, content inspection.
// ============================================================================

#include "preview.hpp"
#include <fstream>
#include <vector>

namespace fx {

bool isBinaryContent(const char* buffer, std::size_t length) {
    // Check first 512 bytes for NUL byte '\0' or non-printable control characters
    std::size_t checkLen = std::min(length, static_cast<std::size_t>(512));
    for (std::size_t i = 0; i < checkLen; ++i) {
        if (buffer[i] == '\0') {
            return true;
        }
    }
    return false;
}

Result<std::string> readTextPreview(const std::filesystem::path& path, std::size_t maxBytes) {
    std::error_code ec;

    if (!std::filesystem::exists(path, ec)) {
        return Result<std::string>::failure("Path not found: " + path.u8string());
    }

    if (!std::filesystem::is_regular_file(path, ec)) {
        return Result<std::string>::failure("Not a regular file: " + path.u8string());
    }

    std::ifstream stream(path, std::ios::in | std::ios::binary);
    if (!stream) {
        return Result<std::string>::failure("Permission denied: " + path.u8string());
    }

    std::vector<char> buffer(maxBytes);
    stream.read(buffer.data(), static_cast<std::streamsize>(maxBytes));
    std::streamsize bytesRead = stream.gcount();

    if (bytesRead > 0 && isBinaryContent(buffer.data(), static_cast<std::size_t>(bytesRead))) {
        return Result<std::string>::failure("Binary file, preview not available.");
    }

    return Result<std::string>::success(std::string(buffer.data(), static_cast<std::size_t>(bytesRead)));
}

} // namespace fx

