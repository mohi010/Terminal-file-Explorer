# Technical Decisions & Environment Log

This document records architectural, toolchain, and design choices made during development.

## Phase 0: Environment and Toolchain
- **Operating System:** Windows 11 (64-bit)
- **CMake Version:** CMake 3.31.5 (Kitware portable binary distribution)
- **C++ Compiler:** GCC 13.2.0 (w64devkit MinGW-w64 x86_64 distribution with POSIX threads and full C++17 `<filesystem>` support)
- **Build Generator:** MinGW Makefiles / CMake
- **C++ Standard:** C++17 (`-std=c++17` with `-Wall -Wextra -Wpedantic`)
- **Filesystem Implementation:** Standard C++17 `<filesystem>` (`std::filesystem`), avoiding non-standard extensions or C++20-only features.
- **Path normalisation decision:** Use `std::filesystem::weakly_canonical` following `std::filesystem::absolute` to ensure symlinks and relative path components (`.` and `..`) are cleanly resolved without crashing if leaf names do not yet exist.
- **Safety boundaries:** Block dangerous operations on drive roots (e.g., `C:\`), system directories (`C:\Windows`, `/etc`, etc.), and user profile root.
