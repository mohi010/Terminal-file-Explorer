// ============================================================================
// File: src/core/os_layer_demo.cpp
// Purpose: Demonstrates native OS system calls (Win32 FindFirstFileW/FindNextFileW
//          and POSIX opendir/readdir/stat) beneath std::filesystem.
// OS Concept: System call layer vs Standard Library abstraction layer.
// ============================================================================

#include <iostream>
#include <string>
#include <cstdint>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

void printNativeDirectoryEntries(const std::wstring& directoryPath) {
    std::wcout << L"[OS Layer Demo: Win32 FindFirstFileW / FindNextFileW]" << std::endl;
    std::wstring searchPattern = directoryPath + L"\\*";

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "FindFirstFileW failed with error code: " << GetLastError() << std::endl;
        return;
    }

    do {
        bool isDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        std::wcout << (isDir ? L"[DIR]  " : L"[FILE] ")
                   << findData.cFileName << L" ("
                   << ((static_cast<std::uint64_t>(findData.nFileSizeHigh) << 32) | findData.nFileSizeLow)
                   << L" bytes)" << std::endl;
    } while (FindNextFileW(hFind, &findData) != 0);

    FindClose(hFind);
}

#elif defined(__unix__) || defined(__APPLE__)
#include <dirent.h>
#include <sys/stat.h>

void printNativeDirectoryEntries(const std::string& directoryPath) {
    std::cout << "[OS Layer Demo: POSIX opendir / readdir / stat]" << std::endl;
    DIR* dir = opendir(directoryPath.c_str());
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string fullPath = directoryPath + "/" + entry->d_name;
        struct stat st{};
        stat(fullPath.c_str(), &st);

        bool isDir = S_ISDIR(st.st_mode);
        std::cout << (isDir ? "[DIR]  " : "[FILE] ")
                  << entry->d_name << " (" << st.st_size << " bytes)" << std::endl;
    }

    closedir(dir);
}
#endif
