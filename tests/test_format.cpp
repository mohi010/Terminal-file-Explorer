// ============================================================================
// File: tests/test_format.cpp
// Purpose: Tests for formatters (size, time, permissions).
// OS Concept: Verifying metadata string conversion.
// ============================================================================

#include "test_runner.hpp"
#include "../src/core/format.hpp"

int main() {
    using namespace fx;
    std::cout << "[Running Format Tests]" << std::endl;

    // Test formatSize
    TEST_ASSERT(formatSize(0) == "0 B", "formatSize 0 bytes");
    TEST_ASSERT(formatSize(512) == "512 B", "formatSize 512 bytes");
    TEST_ASSERT(formatSize(1024) == "1.0 KB", "formatSize 1024 bytes (1.0 KB)");
    TEST_ASSERT(formatSize(1536) == "1.5 KB", "formatSize 1536 bytes (1.5 KB)");
    TEST_ASSERT(formatSize(1024 * 1024) == "1.0 MB", "formatSize 1 MB");
    TEST_ASSERT(formatSize(1024ULL * 1024 * 1024 * 2) == "2.0 GB", "formatSize 2 GB");

    // Test formatPermissions
    using std::filesystem::perms;
    TEST_ASSERT(formatPermissions(perms::none) == "---------", "formatPermissions none");
    TEST_ASSERT(formatPermissions(perms::all) == "rwxrwxrwx", "formatPermissions all");
    TEST_ASSERT(formatPermissions(perms::owner_all | perms::group_read | perms::group_exec | perms::others_read | perms::others_exec) == "rwxr-xr-x",
                "formatPermissions 755 (rwxr-xr-x)");

    // Test formatTime
    auto now = std::filesystem::file_time_type::clock::now();
    std::string timeStr = formatTime(now);
    TEST_ASSERT(timeStr.length() == 16, "formatTime produces YYYY-MM-DD HH:MM (16 chars)");
    TEST_ASSERT(timeStr[4] == '-' && timeStr[7] == '-' && timeStr[10] == ' ' && timeStr[13] == ':',
                "formatTime matches structure YYYY-MM-DD HH:MM");

    return fx::test::reportSummary("Format Tests");
}

