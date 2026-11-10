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

## Phase 8: Qt GUI Architecture & Deployment
- **UI Framework:** Qt 6.8.0 Widgets (`QMainWindow`, `QTableView`, `QTreeWidget`, `QSplitter`, `QDockWidget`). Compatible with Qt 5.15 fallback via CMake `find_package(Qt6 ... QUIET)` and `find_package(Qt5 ...)`.
- **Presentation Abstraction:** Implemented custom `FileTableModel` inheriting from `QAbstractTableModel`. Explicitly avoided `QFileSystemModel` and `QDir` to adhere to PRD specification and ensure `fxcore` remains the single source of truth for all filesystem operations.
- **Portability & Deployment:** Used `windeployqt` to bundle required Qt6 platform plugins (`platforms/qwindows.dll`), styles, and MinGW runtime libraries (`libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll`) into the output directory, enabling standalone execution.

