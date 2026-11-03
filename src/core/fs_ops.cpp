// ============================================================================
// File: src/core/fs_ops.cpp
// Purpose: Implementations for core file system operations.
// OS Concept: Encapsulates directory enumeration, file creation, metadata lookup,
//             and safe error translation into Result<T>.
// ============================================================================

#include "fs_ops.hpp"
#include "safety.hpp"
#include <algorithm>
#include <fstream>
#include <cctype>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace fx {

static bool isHiddenEntry(const std::filesystem::path& path) {
    std::string filename = path.filename().u8string();
    if (!filename.empty() && filename.front() == '.') {
        return true;
    }

#if defined(_WIN32)
    DWORD attrs = GetFileAttributesW(path.wstring().c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_HIDDEN)) {
        return true;
    }
#endif

    return false;
}


Result<EntryInfo> getInfo(const std::filesystem::path& path) {
    std::error_code ec;

    // Use symlink_status to avoid resolving broken symlinks
    auto symStatus = std::filesystem::symlink_status(path, ec);
    if (ec || symStatus.type() == std::filesystem::file_type::not_found) {
        return Result<EntryInfo>::failure("Path not found: " + path.u8string());
    }

    EntryInfo info;
    info.path = path;
    info.name = path.filename().u8string();
    if (info.name.empty()) {
        info.name = path.u8string();
    }

    // Determine type
    if (std::filesystem::is_symlink(symStatus)) {
        info.type = EntryType::Symlink;
    } else if (std::filesystem::is_directory(symStatus)) {
        info.type = EntryType::Directory;
    } else if (std::filesystem::is_regular_file(symStatus)) {
        info.type = EntryType::File;
    } else {
        info.type = EntryType::Other;
    }

    // Size: only meaningful for regular files at getInfo level
    if (info.type == EntryType::File) {
        auto sz = std::filesystem::file_size(path, ec);
        info.size = ec ? 0 : sz;
    } else {
        info.size = 0;
    }

    // Modified time
    auto mTime = std::filesystem::last_write_time(path, ec);
    if (!ec) {
        info.modified = mTime;
    }

    // Permissions
    info.permissions = symStatus.permissions();

    // Hidden check
    info.isHidden = isHiddenEntry(path);

    return Result<EntryInfo>::success(info);
}

Result<std::vector<EntryInfo>> listDirectory(const std::filesystem::path& path, ListOptions options) {
    std::error_code ec;

    if (!std::filesystem::exists(path, ec)) {
        return Result<std::vector<EntryInfo>>::failure("Path not found: " + path.u8string());
    }

    if (!std::filesystem::is_directory(path, ec)) {
        return Result<std::vector<EntryInfo>>::failure("Not a directory: " + path.u8string());
    }

    std::vector<EntryInfo> dirs;
    std::vector<EntryInfo> files;
    dirs.reserve(64);
    files.reserve(1024);

    // Iterate directory, skip permission denied errors gracefully
    std::filesystem::directory_iterator it(path, std::filesystem::directory_options::skip_permission_denied, ec);
    std::filesystem::directory_iterator endIt;

    if (ec) {
        return Result<std::vector<EntryInfo>>::failure("Permission denied: " + path.u8string());
    }

    while (it != endIt) {
        const auto& entry = *it;
        const auto& entryPath = entry.path();
        std::string filename = entryPath.filename().string();

        bool hidden = (!filename.empty() && filename.front() == '.');

        if (!options.showHidden && hidden) {
            it.increment(ec);
            if (ec) ec.clear();
            continue;
        }

        EntryInfo info;
        info.path = entryPath;
        info.name = std::move(filename);
        info.isHidden = hidden;

        auto symSt = entry.symlink_status(ec);
        if (ec) {
            ec.clear();
            symSt = std::filesystem::file_status{};
        }

        if (std::filesystem::is_symlink(symSt)) {
            info.type = EntryType::Symlink;
        } else if (std::filesystem::is_directory(symSt)) {
            info.type = EntryType::Directory;
        } else if (std::filesystem::is_regular_file(symSt)) {
            info.type = EntryType::File;
            auto sz = entry.file_size(ec);
            info.size = ec ? 0 : sz;
            if (ec) ec.clear();
        } else {
            info.type = EntryType::Other;
        }

        info.permissions = symSt.permissions();
        if (options.sortBy == SortBy::Date) {
            auto mTime = entry.last_write_time(ec);
            if (!ec) {
                info.modified = mTime;
            } else {
                ec.clear();
            }
        }

        if (info.type == EntryType::Directory) {
            dirs.push_back(std::move(info));
        } else {
            files.push_back(std::move(info));
        }

        it.increment(ec);
        if (ec) {
            ec.clear(); // Continue through next entry on single error
        }
    }

    auto caseInsensitiveLess = [](const std::string& a, const std::string& b) {
        auto itA = a.begin(), itB = b.begin();
        while (itA != a.end() && itB != b.end()) {
            char ca = static_cast<char>(std::tolower(static_cast<unsigned char>(*itA)));
            char cb = static_cast<char>(std::tolower(static_cast<unsigned char>(*itB)));
            if (ca != cb) return ca < cb;
            ++itA;
            ++itB;
        }
        return a.length() < b.length();
    };

    // Comparator for sorting
    auto comparator = [&options, &caseInsensitiveLess](const EntryInfo& a, const EntryInfo& b) {
        bool less = false;
        switch (options.sortBy) {
            case SortBy::Name:
                less = caseInsensitiveLess(a.name, b.name);
                break;
            case SortBy::Size:
                less = a.size < b.size;
                break;
            case SortBy::Date:
                less = a.modified < b.modified;
                break;
            case SortBy::Type: {
                std::string extA = a.path.extension().string();
                std::string extB = b.path.extension().string();
                less = (extA == extB) ? caseInsensitiveLess(a.name, b.name) : caseInsensitiveLess(extA, extB);
                break;
            }
        }
        return options.ascending ? less : !less;
    };

    std::sort(dirs.begin(), dirs.end(), comparator);
    std::sort(files.begin(), files.end(), comparator);

    // Combine: directories listed first, then files (using move iterators to avoid copies)
    std::vector<EntryInfo> result;
    result.reserve(dirs.size() + files.size());
    result.insert(result.end(), std::make_move_iterator(dirs.begin()), std::make_move_iterator(dirs.end()));
    result.insert(result.end(), std::make_move_iterator(files.begin()), std::make_move_iterator(files.end()));

    return Result<std::vector<EntryInfo>>::success(result);
}

Result<bool> createFile(const std::filesystem::path& path) {
    std::error_code ec;

    if (std::filesystem::exists(path, ec)) {
        return Result<bool>::failure("Already exists: " + path.u8string());
    }

    auto nameValidation = validateFileName(path.filename().u8string());
    if (!nameValidation.ok) {
        return nameValidation;
    }

    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file) {
        return Result<bool>::failure("Permission denied: " + path.u8string());
    }
    file.close();

    return Result<bool>::success(true);
}

Result<bool> createDirectory(const std::filesystem::path& path) {
    std::error_code ec;

    if (std::filesystem::exists(path, ec)) {
        return Result<bool>::failure("Already exists: " + path.u8string());
    }

    auto nameValidation = validateFileName(path.filename().u8string());
    if (!nameValidation.ok) {
        return nameValidation;
    }

    if (!std::filesystem::create_directory(path, ec)) {
        if (ec) {
            return Result<bool>::failure(ec.message() + ": " + path.u8string());
        }
        return Result<bool>::failure("Failed to create directory: " + path.u8string());
    }

    return Result<bool>::success(true);
}

Result<bool> renamePath(const std::filesystem::path& from, const std::string& newName) {
    std::error_code ec;

    if (!std::filesystem::exists(from, ec)) {
        return Result<bool>::failure("Path not found: " + from.u8string());
    }

    if (isProtectedPath(from)) {
        return Result<bool>::failure("This path is protected and cannot be changed.");
    }

    auto nameValidation = validateFileName(newName);
    if (!nameValidation.ok) {
        return nameValidation;
    }

    std::filesystem::path target = from.parent_path() / newName;
    if (std::filesystem::exists(target, ec)) {
        return Result<bool>::failure("Already exists: " + target.u8string());
    }

    std::filesystem::rename(from, target, ec);
    if (ec) {
        return Result<bool>::failure(ec.message());
    }

    return Result<bool>::success(true);
}

Result<bool> removePath(const std::filesystem::path& path, bool recursive) {
    std::error_code ec;

    if (!std::filesystem::exists(path, ec)) {
        return Result<bool>::failure("Path not found: " + path.u8string());
    }

    if (isProtectedPath(path)) {
        return Result<bool>::failure("This path is protected and cannot be changed.");
    }

    bool isDir = std::filesystem::is_directory(path, ec);
    if (isDir) {
        if (!recursive) {
            // Check if directory is empty
            std::filesystem::directory_iterator it(path, ec);
            if (!ec && it != std::filesystem::directory_iterator()) {
                return Result<bool>::failure("Directory not empty. Use rm -r.");
            }
            std::filesystem::remove(path, ec);
        } else {
            std::filesystem::remove_all(path, ec);
        }
    } else {
        std::filesystem::remove(path, ec);
    }

    if (ec) {
        return Result<bool>::failure(ec.message());
    }

    return Result<bool>::success(true);
}

Result<bool> copyPath(const std::filesystem::path& from, const std::filesystem::path& toDir) {
    std::error_code ec;

    if (!std::filesystem::exists(from, ec)) {
        return Result<bool>::failure("Path not found: " + from.u8string());
    }

    if (!std::filesystem::exists(toDir, ec)) {
        return Result<bool>::failure("Path not found: " + toDir.u8string());
    }

    if (!std::filesystem::is_directory(toDir, ec)) {
        return Result<bool>::failure("Not a directory: " + toDir.u8string());
    }

    std::filesystem::path target = toDir / from.filename();
    if (std::filesystem::exists(target, ec)) {
        return Result<bool>::failure("Already exists: " + target.u8string());
    }

    if (std::filesystem::is_directory(from, ec)) {
        if (isDestinationInsideSource(from, target)) {
            return Result<bool>::failure("Cannot copy a folder into itself.");
        }
        std::filesystem::copy(from, target,
                              std::filesystem::copy_options::recursive | std::filesystem::copy_options::copy_symlinks,
                              ec);
    } else {
        std::filesystem::copy_file(from, target, std::filesystem::copy_options::none, ec);
    }

    if (ec) {
        return Result<bool>::failure(ec.message());
    }

    return Result<bool>::success(true);
}

Result<bool> movePath(const std::filesystem::path& from, const std::filesystem::path& toDir) {
    std::error_code ec;

    if (!std::filesystem::exists(from, ec)) {
        return Result<bool>::failure("Path not found: " + from.u8string());
    }

    if (isProtectedPath(from)) {
        return Result<bool>::failure("This path is protected and cannot be changed.");
    }

    if (!std::filesystem::exists(toDir, ec)) {
        return Result<bool>::failure("Path not found: " + toDir.u8string());
    }

    if (!std::filesystem::is_directory(toDir, ec)) {
        return Result<bool>::failure("Not a directory: " + toDir.u8string());
    }

    std::filesystem::path target = toDir / from.filename();
    if (std::filesystem::exists(target, ec)) {
        return Result<bool>::failure("Already exists: " + target.u8string());
    }

    if (std::filesystem::is_directory(from, ec)) {
        if (isDestinationInsideSource(from, target)) {
            return Result<bool>::failure("Cannot move a folder into itself.");
        }
    }

    // Try fast rename
    std::filesystem::rename(from, target, ec);
    if (!ec) {
        return Result<bool>::success(true);
    }

    // If rename fails (e.g. across filesystem/volume boundaries), fall back to copy + delete
    ec.clear();
    auto copyRes = copyPath(from, toDir);
    if (!copyRes.ok) {
        return copyRes;
    }

    auto removeRes = removePath(from, true);
    if (!removeRes.ok) {
        return removeRes;
    }

    return Result<bool>::success(true);
}

Result<SpaceInfo> getSpace(const std::filesystem::path& path) {
    std::error_code ec;
    auto spaceData = std::filesystem::space(path, ec);
    if (ec) {
        return Result<SpaceInfo>::failure(ec.message());
    }

    SpaceInfo info;
    info.capacity = spaceData.capacity;
    info.free = spaceData.free;
    info.available = spaceData.available;
    return Result<SpaceInfo>::success(info);
}

} // namespace fx
