// ============================================================================
// File: src/core/format.cpp
// Purpose: Implementations for formatting sizes, timestamps, and permissions.
// OS Concept: Decodes file system metadata into human-readable representations.
// ============================================================================

#include "format.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace fx {

std::string formatSize(std::uintmax_t bytes) {
    if (bytes == 0) {
        return "0 B";
    }

    const double kByte = 1024.0;
    const double mByte = 1024.0 * 1024.0;
    const double gByte = 1024.0 * 1024.0 * 1024.0;

    std::ostringstream out;
    out << std::fixed << std::setprecision(1);

    if (static_cast<double>(bytes) < kByte) {
        return std::to_string(bytes) + " B";
    }
    if (static_cast<double>(bytes) < mByte) {
        out << (static_cast<double>(bytes) / kByte) << " KB";
        return out.str();
    }
    if (static_cast<double>(bytes) < gByte) {
        out << (static_cast<double>(bytes) / mByte) << " MB";
        return out.str();
    }
    out << (static_cast<double>(bytes) / gByte) << " GB";
    return out.str();
}

std::string formatTime(std::filesystem::file_time_type fileTime) {
    // In C++17, std::filesystem::file_time_type clock may not be system_clock.
    // We convert using the duration offset between now on both clocks.
    auto sysNow = std::chrono::system_clock::now();
    auto fileNow = std::filesystem::file_time_type::clock::now();
    auto sysTime = sysNow + std::chrono::duration_cast<std::chrono::system_clock::duration>(fileTime - fileNow);

    std::time_t timeT = std::chrono::system_clock::to_time_t(sysTime);
    std::tm timeBuf{};

#if defined(_WIN32)
    localtime_s(&timeBuf, &timeT);
#else
    localtime_r(&timeT, &timeBuf);
#endif

    std::ostringstream out;
    out << std::put_time(&timeBuf, "%Y-%m-%d %H:%M");
    return out.str();
}

std::string formatPermissions(std::filesystem::perms permissions) {
    using std::filesystem::perms;

    auto check = [permissions](perms bit, char symbol) {
        return (permissions & bit) != perms::none ? symbol : '-';
    };

    std::string result;
    result.reserve(9);

    // User / Owner
    result += check(perms::owner_read, 'r');
    result += check(perms::owner_write, 'w');
    result += check(perms::owner_exec, 'x');

    // Group
    result += check(perms::group_read, 'r');
    result += check(perms::group_write, 'w');
    result += check(perms::group_exec, 'x');

    // Others
    result += check(perms::others_read, 'r');
    result += check(perms::others_write, 'w');
    result += check(perms::others_exec, 'x');

    return result;
}

} // namespace fx

