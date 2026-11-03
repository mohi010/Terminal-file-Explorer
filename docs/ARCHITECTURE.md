# Architecture & System Design

## 1. Architectural Overview

The **OS File Explorer** is built with a strict three-layer decoupled architecture adhering to modern operating system principles: separation of mechanism from policy, and separation of presentation from business logic.

```
+-------------------------------------------------------------------------+
|                              User Layer                                 |
+------------------------------------+------------------------------------+
|        Terminal CLI Front End      |       Qt6 Widgets GUI Front End    |
|              (fx_cli)              |               (fx_gui)             |
|   - ANSI Color Terminal Setup      |   - QTableView & QAbstractModel    |
|   - Lexer / String Tokenizer       |   - QTreeWidget Lazy Hierarchy     |
|   - REPL Command Loop & Dispatch   |   - Context Menus & File Dialogs   |
+------------------------------------+------------------------------------+
                                     |
                                     v (Strict Core C++ API)
+-------------------------------------------------------------------------+
|                  Layer 1: Core Static Library (fxcore)                  |
|                           [namespace fx]                                |
+------------------------------------+------------------------------------+
|  types.hpp                         |  fs_ops.hpp / fs_ops.cpp           |
|  - Result<T> container             |  - listDirectory, getInfo          |
|  - EntryInfo & EntryType           |  - createFile, createDirectory     |
|  - ListOptions, SearchOptions      |  - renamePath, removePath, copy/mv |
|  --------------------------------- |  --------------------------------- |
|  navigator.hpp / navigator.cpp     |  search.hpp / search.cpp           |
|  - Working directory state (CWD)   |  - searchByName (depth-limited)    |
|  - Back & Forward history stacks   |  - directorySize (recursive)       |
|  - Path resolution (~, ., ..)      |  --------------------------------- |
|  --------------------------------- |  preview.hpp / preview.cpp         |
|  safety.hpp / safety.cpp           |  - readTextPreview (max 4096B)     |
|  - Root & System path protection   |  - Binary magic byte inspection    |
|  - Reserved name validation (CON..) |  --------------------------------- |
|  - Copy-into-itself prevention     |  format.hpp / format.cpp           |
|  --------------------------------- |  - formatSize, formatTime          |
|  os_layer_demo.cpp                 |  - formatPermissions (rwxr-xr-x)   |
|  - Native POSIX / Win32 syscalls   |                                    |
+------------------------------------+------------------------------------+
                                     |
                                     v (Standard OS System Calls)
+-------------------------------------------------------------------------+
|                        Underlying Operating System                      |
|      POSIX (Linux/macOS)                    Windows NT (Win32)          |
|   opendir, readdir, stat, remove        FindFirstFileW, FindNextFileW   |
+-------------------------------------------------------------------------+
```

---

## 2. File Responsibilities

| File Path | Component | Responsibility | OS Concept |
|---|---|---|---|
| `src/core/types.hpp` | Core | Data structures: `EntryInfo`, `EntryType`, `Result<T>`, `SpaceInfo` | Inode/metadata abstraction and safe error codes |
| `src/core/format.hpp/.cpp` | Core | Converts bytes to B/KB/MB/GB, timestamps to YYYY-MM-DD HH:MM, permissions to rwxr-xr-x | Translating disk metadata bitmasks to human text |
| `src/core/safety.hpp/.cpp` | Core | Validates filenames against reserved names (`CON`, `PRN`), forbids self-copy, protects system roots | Filesystem security boundaries & namespace rules |
| `src/core/fs_ops.hpp/.cpp` | Core | Non-throwing operations: list, getInfo, create, rename, remove, copy, move | Core filesystem system call wrappers |
| `src/core/navigator.hpp/.cpp` | Core | Encapsulates process Current Working Directory (CWD), back/forward stacks | Process execution context and path resolution |
| `src/core/search.hpp/.cpp` | Core | Depth-limited tree traversal, pattern matching, directory size aggregation | Acyclic directory search & symlink loop prevention |
| `src/core/preview.hpp/.cpp` | Core | Reads up to 4KB of text, detects NUL bytes to avoid binary dump | Stream I/O, file format inspection |
| `src/core/os_layer_demo.cpp` | Core | Direct calls to Win32 `FindFirstFileW` or POSIX `opendir`/`readdir` | Showing syscalls directly under `std::filesystem` |
| `src/cli/terminal.hpp/.cpp` | CLI | Configures UTF-8 console and virtual terminal ANSI colors | Console subsystem, TTY/Virtual Terminal |
| `src/cli/commands.hpp/.cpp` | CLI | Command-line tokenizer and command execution routines | Shell command dispatch and formatted presentation |
| `src/cli/main.cpp` | CLI | REPL entry point | Process lifecycle & interactive execution loop |
| `src/gui/models.hpp/.cpp` | GUI | Custom `QAbstractTableModel` for files and folders | Model-View-Controller presentation architecture |
| `src/gui/mainwindow.hpp/.cpp`| GUI | Main window layout, lazy-loaded folder tree, shortcuts, context menu | Desktop shell user interface |
| `src/gui/main.cpp` | GUI | Qt application entry point and event loop | Windowing system event loop |

