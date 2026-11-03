# PRD: OS File Explorer (C++17 Core, Terminal CLI + Qt GUI)

Operating Systems mini project (NSUT). Paste this entire document into the coding agent and build phase by phase.

## 0. Instructions for the AI agent (read first)

- Build strictly in phase order (Phase 0 to Phase 9). Do not start a phase until the acceptance criteria of the previous one pass.
- After every phase: compile with warnings on, run all tests, fix every error and warning, then write a short entry in `docs/PROGRESS.md`.
- Never invent an API, header, flag or function signature. Every external call must be checked against Section 12 and against the real header or official documentation.
- Do not ask the user questions. When something is ambiguous, pick the simplest option that satisfies this PRD and record it in `docs/DECISIONS.md`.
- Never disable warnings, delete failing tests or hide errors to make a build pass. Fix the root cause.
- Never run destructive operations (delete, rename, overwrite) outside the sandbox directory used by tests (Section 14).
- The terminal version (Phases 0 to 7) is the guaranteed deliverable. The GUI (Phase 8) is built only after Phase 7 passes completely.
- The student must be able to explain every file, so keep the code simple, commented and readable. Prefer clarity over cleverness.

## 1. Overview

- **Product:** a file explorer that lets a user browse directories, view file details, and create, rename, copy, move, delete and search files and folders.
- **Course context:** Operating Systems, Unit IV (Storage management: file system, files and directories, directory structure, file attributes and permissions, free space).
- **Architecture idea:** one C++ core library holds all file system logic with no printing and no user input. Two thin front ends sit on top: a terminal CLI and a Qt GUI.
- **Primary use:** a viva and demo in front of a professor. The project must run reliably, look clean, and map every feature to an OS concept.

## 2. Goals and non-goals

**Goals**

- Correct, crash-free file system operations with clear error messages.
- Clean separation: core library, CLI front end, GUI front end.
- Every feature explainable in terms of OS concepts (directory entries, metadata, permissions, path resolution).
- Cross-platform behaviour (Windows first, Linux and macOS also supported).
- A full documentation pack for the viva (Phase 9).

**Non-goals**

- No network features, cloud sync, or web server. There are no HTTP endpoints in this project.
- No custom file system implementation or raw disk access.
- No file content editing (only a read-only text preview).
- No database.

## 3. Tech stack

- **Language:** C++17 (strictly; do not use C++20 features such as `std::u8string` paths or `std::chrono::clock_cast`).
- **File system API:** `<filesystem>` (std::filesystem) for all operations.
- **Optional OS-level layer (explainability module):** POSIX `opendir`, `readdir`, `closedir`, `stat` guarded by `#ifdef __unix__` / `__APPLE__`, and Win32 `FindFirstFileW`, `FindNextFileW` guarded by `#ifdef _WIN32`. Used only in a small demo module `os_layer_demo.cpp` that prints what the OS returns, so the student can show the real system calls behind std::filesystem.
- **Build system:** CMake 3.16 or newer.
- **Compiler:** g++ 9 or newer (MinGW-w64 or MSYS2 on Windows), clang 10 or newer, or MSVC 2019 or newer.
- **GUI:** Qt 6 Widgets (Phase 8 only). Fall back to Qt 5.15 only if Qt 6 is unavailable; write code compatible with both where possible.
- **Testing:** plain assert-based test runner written in the project (no external test framework, so there is nothing to download). Run through CTest.
- **Quality tools (use if installed, skip if not):** `-Wall -Wextra -Wpedantic`, AddressSanitizer and UBSan (`-fsanitize=address,undefined`), `clang-tidy`, `cppcheck`.
- **Version control:** Git, with one commit per completed phase.

## 4. Architecture

- **Layer 1: core library `fxcore` (static library).** All logic. Pure functions and one `Navigator` class. No `std::cout`, no `std::cin`, no Qt. Returns results, never throws to callers.
- **Layer 2: `fx_cli` (executable).** Command parser, REPL loop, table formatting, colours. Calls only the core API.
- **Layer 3: `fx_gui` (executable, Phase 8).** Qt window. Calls only the core API. Must not use `QFileSystemModel` or `QDir` for file operations, so that the core stays the single source of truth.
- **Rule:** if a feature exists in the CLI, it must be implemented in the core first.

## 5. Project structure

```
file-explorer/
  CMakeLists.txt
  README.md
  src/
    core/
      types.hpp          (EntryInfo, EntryType, Result)
      fs_ops.hpp/.cpp    (list, info, create, rename, remove, copy, move)
      navigator.hpp/.cpp (current path, back/forward history)
      search.hpp/.cpp    (find by name, directory size)
      preview.hpp/.cpp   (text preview, binary detection)
      format.hpp/.cpp    (size, time, permissions to strings)
      safety.hpp/.cpp    (protected paths, validation)
      os_layer_demo.cpp  (POSIX/Win32 calls for explanation)
    cli/
      main.cpp
      commands.hpp/.cpp
      terminal.hpp/.cpp  (colours, Windows console setup)
    gui/                 (Phase 8)
      main.cpp
      mainwindow.hpp/.cpp
      models.hpp/.cpp
  tests/
    test_runner.hpp
    test_core_list.cpp
    test_core_ops.cpp
    test_navigator.cpp
    test_search.cpp
    test_format.cpp
    test_api_contract.cpp  (Section 12 verification)
    cli_scenarios/         (scripted CLI input and expected output)
  docs/
    PROGRESS.md  DECISIONS.md  BUGLOG.md  API_CONTRACT.md
    ARCHITECTURE.md  OS_CONCEPTS.md  VIVA_QA.md  DEMO_SCRIPT.md
```

## 6. Core API specification (`namespace fx`)

All functions use the `std::error_code` overloads of std::filesystem internally, catch nothing silently, and return a `Result<T>`.

```cpp
enum class EntryType { File, Directory, Symlink, Other };

struct EntryInfo {
    std::string name;
    std::filesystem::path path;
    EntryType type;
    std::uintmax_t size;                         // 0 for directories
    std::filesystem::file_time_type modified;
    std::filesystem::perms permissions;
    bool isHidden;
};

template <class T>
struct Result {
    bool ok = false;
    T value{};
    std::string error;      // human-readable, empty when ok
};
```

- `Result<std::vector<EntryInfo>> listDirectory(const path&, ListOptions)` where ListOptions has `showHidden`, `sortBy` (Name, Size, Date, Type) and `ascending`. Directories are listed before files.
- `Result<EntryInfo> getInfo(const path&)`.
- `class Navigator`: `cd(path)`, `up()`, `back()`, `forward()`, `current()`, `canGoBack()`, `canGoForward()`. Navigating to a new path clears the forward history.
- `Result<bool> createFile(const path&)`, `createDirectory(const path&)` (non-recursive; fails if it exists).
- `Result<bool> renamePath(const path& from, const std::string& newName)` (new name only, no separators allowed).
- `Result<bool> removePath(const path&, bool recursive)` (non-empty directory requires `recursive = true`).
- `Result<bool> copyPath(const path& from, const path& toDir)` (files and directory trees; refuses to copy a directory into itself).
- `Result<bool> movePath(const path& from, const path& toDir)` (rename first; if it fails across drives, copy then remove).
- `Result<std::vector<EntryInfo>> searchByName(const path& root, const std::string& pattern, SearchOptions)` with `maxDepth`, `caseInsensitive`, and `maxResults` (default 500).
- `Result<std::uintmax_t> directorySize(const path&)` (recursive, skips unreadable entries and counts how many were skipped).
- `Result<std::string> readTextPreview(const path&, std::size_t maxBytes = 4096)` (returns an error for binary files).
- `Result<SpaceInfo> getSpace(const path&)` (capacity, free, available, from `std::filesystem::space`).
- Formatters: `formatSize(uintmax_t)` (B, KB, MB, GB), `formatTime(file_time_type)` (`YYYY-MM-DD HH:MM`), `formatPermissions(perms)` (`rwxr-xr-x` style).

## 7. CLI specification

The CLI is a REPL with the prompt showing the current path, for example ` fx:C:/Users/Mohit>  `.

- `help`: list all commands.
- `pwd`: print the current directory.
- `ls [path] [-l] [-a] [--sort name|size|date|type] [--desc]`: list entries. Default view is a clean aligned table with Type, Name, Size, Modified. Folders are blue and bold, files are white, errors are red (ANSI).
- `cd <path>`, `up`, `back`, `forward`: navigation. Support `..`, `.`, `~`, absolute and relative paths, and paths with spaces in quotes.
- `tree [depth]`: ASCII tree, default depth 2, maximum depth 5.
- `info <path>`: full properties (type, size, created or modified time, permissions, hidden flag, absolute path).
- `view <file>`: text preview.
- `mkdir <name>`, `touch <name>`.
- `rename <path> <newname>`.
- `cp <src> <destdir>`, `mv <src> <destdir>`.
- `rm <path> [-r]`: asks `Delete 'x'? (y/N)` before deleting.
- `find <pattern> [--depth n]`.
- `du <path>`: directory size.
- `space`: free and total space of the current drive.
- `clear`, `exit`, `quit`.
- **Rules:** unknown commands print `Unknown command. Type help.`, never crash. Empty input is ignored. Every error shows a short human-readable message, never a raw exception text or stack trace.
- **Windows console setup:** call `SetConsoleOutputCP(CP_UTF8)` and enable `ENABLE_VIRTUAL_TERMINAL_PROCESSING` so colours and Unicode names work. Guard with `#ifdef _WIN32`.

## 8. GUI specification (Phase 8, Qt Widgets)

- **Window:** main window with a toolbar (Back, Forward, Up, Refresh), an address bar, a folder tree on the left, a file table on the right, a status bar, and a properties panel (dock widget).
- **Folder tree:** `QTreeWidget`, loads children lazily when a node is expanded, using `fx::listDirectory`.
- **File table:** `QTableView` with a custom model (`QAbstractTableModel`) fed from the core. Columns: Name, Type, Size, Modified. Click a header to sort. Double-click a folder to open it.
- **Context menu:** New Folder, New File, Rename, Copy, Cut, Paste, Delete (with confirmation dialog), Properties.
- **Search box:** filters through `fx::searchByName`, run so that the UI does not freeze (use `QtConcurrent` or a worker `QThread` for searches and directory size).
- **Errors:** shown in `QMessageBox::warning` with the core's error text. The GUI must never crash on a failed operation.
- **Keyboard:** Backspace (up), Alt+Left (back), Alt+Right (forward), Delete, F2 (rename), F5 (refresh), Ctrl+C / X / V.
- **Theme:** clean light theme with folder and file icons from `QStyle::standardIcon`. No external image assets required.
- **String conversion:** convert paths with `QString::fromStdString(path.u8string())` in C++17 and back with `QString::toStdString()` and `std::filesystem::u8path`. Do not use `QString::fromStdWString` on non-Windows.

## 9. Phase-wise plan

### Phase 0: Environment and toolchain

- Detect the operating system, the compiler and its version, and the CMake version.
- Confirm C++17 and `<filesystem>` work with a 10-line test that lists the current directory.
- If g++ 8 is found, link with `-lstdc++fs`. Prefer g++ 9 or newer.
- Do not install Qt yet. Check for it only in Phase 8.
- **Acceptance:** the hello-filesystem program compiles and runs. Versions are logged in `docs/DECISIONS.md`.

### Phase 1: Project scaffolding

- Create the folder structure from Section 5, the root `CMakeLists.txt`, the `fxcore` static library target, the `fx_cli` target, and a `tests` target registered with CTest.
- Compiler flags: `-Wall -Wextra -Wpedantic` (or `/W4` on MSVC). Add an option `FX_SANITIZE` for ASan and UBSan.
- `git init`, add `.gitignore` (build directory, IDE files), first commit.
- **Acceptance:** `cmake -S . -B build && cmake --build build` succeeds with zero warnings and `ctest` runs (even with a placeholder test).

### Phase 2: Core read-only operations

- Implement `types.hpp`, `listDirectory`, `getInfo`, sorting, and the formatters.
- Use `directory_iterator` with `directory_options::skip_permission_denied` and the `error_code` constructor. A single unreadable entry must not abort the whole listing.
- Use `symlink_status` (not `status`) to detect symlinks, and handle broken symlinks.
- Call `file_size` only for regular files. Directories report size 0 here.
- **Acceptance:** `test_core_list.cpp` and `test_format.cpp` pass, including empty directory, hidden files, unicode names, and nonexistent path.

### Phase 3: Navigation

- Implement `Navigator` with current path, back stack and forward stack.
- Normalise every path with `weakly_canonical(p, ec)` after `absolute`. Reject paths that are not directories.
- `up()` at a filesystem root stays at the root and returns an informational result, not an error.
- **Acceptance:** `test_navigator.cpp` passes (cd, up, back, forward, forward cleared after a new cd, invalid target keeps the current path unchanged).

### Phase 4: Mutating operations

- Implement `createFile`, `createDirectory`, `renamePath`, `removePath`, `copyPath`, `movePath`.
- Safety: block deletion or modification of protected paths (drive roots, `C:/Windows`, `/`, `/etc`, `/usr`, the user's home directory itself) in `safety.cpp`.
- Validate names: reject empty names, names with path separators, and on Windows the characters `< > : \" | ? *` and reserved names (CON, PRN, NUL).
- Use `copy_options::recursive | copy_options::copy_symlinks` for directories and `overwrite_existing` only when the caller requests it.
- Copy-into-itself check: compare `weakly_canonical` paths before copying a directory.
- **Acceptance:** `test_core_ops.cpp` passes inside the sandbox, including rename to an existing name, delete of a non-empty directory without recursive, copy into itself, move across directories, and read-only files.

### Phase 5: Search, preview, size, space

- Implement `searchByName`, `directorySize`, `readTextPreview`, `getSpace`.
- Use `recursive_directory_iterator` without following directory symlinks (no `follow_directory_symlink`) to avoid infinite loops. Use `disable_recursion_pending()` to enforce `maxDepth`.
- Binary detection: if the first 512 bytes contain a NUL byte, treat the file as binary.
- **Acceptance:** `test_search.cpp` passes, including case-insensitive search, depth limit, result cap, and unreadable subfolders.

### Phase 6: CLI front end

- Implement the REPL, the tokenizer (quotes for spaces), the command table, aligned table output, colours, and the Windows console setup.
- Wire every command from Section 7 to the core only.
- **Acceptance:** all scripted scenarios in `tests/cli_scenarios` pass. Piping input (`fx_cli < script.txt`) works and the process exits with code 0.

### Phase 7: Hardening, API verification and bug-fix loop

- Run the full API contract verification (Section 12) and the self-debug loop (Section 13).
- Run the whole test suite with ASan and UBSan where available.
- Run `clang-tidy` and `cppcheck` if installed and fix real findings.
- Manual stress test: a folder with 10,000 files, a very long path, special characters, and a folder without permissions.
- **Acceptance:** two consecutive fully clean runs (Section 13 exit criteria). The terminal version is now a complete, safe submission.

### Phase 8: Qt GUI (only after Phase 7 passes)

- Check for Qt with `find_package(Qt6 COMPONENTS Widgets REQUIRED)` and fall back to Qt5. Set `CMAKE_AUTOMOC ON`. If Qt is missing, stop, write the install instructions in `docs/DECISIONS.md`, and skip to Phase 9.
- Build the window in this order: main window layout, file table with model, folder tree, navigation toolbar and address bar, context menu operations, search with a background thread, properties panel, keyboard shortcuts.
- The GUI must call only `fx::` functions. Perform no `QDir` or `QFile` operations for the actual file work.
- Windows: provide a `windeployqt` step in the README so the executable runs on another machine.
- **Acceptance:** the GUI launches, browses, and performs every operation from Section 8 with no crash. Run the GUI checklist in Section 14.

### Phase 9: Documentation and viva pack

- `README.md`: what it is, features, build steps for Windows, Linux and macOS, usage examples, screenshots placeholder.
- `docs/ARCHITECTURE.md`: the three-layer diagram and a description of each file.
- `docs/OS_CONCEPTS.md`: Section 15 expanded in plain language.
- `docs/VIVA_QA.md`: at least 40 likely viva questions with short answers.
- `docs/DEMO_SCRIPT.md`: a 5-minute demo flow (GUI first impression, then the same operations in the terminal, then the OS layer demo).
- **Acceptance:** all docs exist, the build steps in the README work from a clean clone, and the demo script runs without errors.

## 10. Error handling matrix

- **Path does not exist:** `Path not found: <path>`.
- **Permission denied:** `Permission denied: <path>`. The operation stops but the program continues. In directory listings, skip the entry and show a note.
- **Not a directory (cd to a file):** `Not a directory: <path>`.
- **Already exists (create, rename, copy):** `Already exists: <path>`.
- **Directory not empty (delete without -r):** `Directory not empty. Use rm -r.`
- **Invalid name:** `Invalid name. It cannot be empty or contain / \\ : * ? \" < > |`.
- **Copy into itself:** `Cannot copy a folder into itself.`
- **Protected path:** `This path is protected and cannot be changed.`
- **Binary file preview:** `Binary file, preview not available.`
- **Disk full or I/O error:** show the `error_code.message()` text.
- **Rule:** no function may throw an uncaught exception to the front end. Wrap any std::filesystem call that has no `error_code` overload in `try/catch (const std::filesystem::filesystem_error&)` and convert it to a `Result` error.

## 11. Non-functional requirements

- Listing a folder of 10,000 entries finishes in under 2 seconds.
- No memory leaks and no undefined behaviour (clean under ASan and UBSan).
- Zero compiler warnings at `-Wall -Wextra -Wpedantic`.
- Code is commented in simple English, with one header comment per file that explains its purpose and the related OS concept.
- Naming: `camelCase` functions, `PascalCase` types, `UPPER_CASE` constants.

## 12. API call and interface verification checklist

There are no network endpoints in this project. The equivalent risk is calling a library function wrongly (wrong signature, wrong header, undefined behaviour). The agent must verify every external call and every core function interface before declaring a phase done.

**Method**

- For every library call, confirm the header and the exact overload being used. Compile with `-Wall -Wextra -Wpedantic -Werror` in the verification build.
- Create `docs/API_CONTRACT.md`: a table listing each call, its header, the overload used, what it returns on failure, and the project function that uses it.
- Create `tests/test_api_contract.cpp`: for each call, one test for the success case and one for the failure case (nonexistent path, permission denied, wrong type).
- Create a contract test for each public `fx::` function: valid input, invalid input, boundary input. Each must return a `Result` and never throw or crash.

**std::filesystem calls to verify (use `std::error_code` overloads)**

- `directory_iterator(path, options, ec)` and `recursive_directory_iterator(path, options, ec)`; increment with `it.increment(ec)`, not `++it`, when errors must not throw.
- `status(p, ec)`, `symlink_status(p, ec)`, `exists`, `is_directory`, `is_regular_file`, `is_symlink`.
- `file_size(p, ec)`: valid only for regular files; calling it on a directory returns an error.
- `last_write_time(p, ec)`: returns `file_time_type`, which is not `system_clock`. In C++17 convert it with the duration-offset method (`time_point_cast` plus the difference between `file_time_type::clock::now()` and `system_clock::now()`). Do not use `clock_cast` (C++20).
- `permissions(p, perms, ec)` and `status(p).permissions()`: on Windows only the read-only flag is meaningful, so the formatter must handle that.
- `create_directory(p, ec)`, `create_directories`, `rename(from, to, ec)` (fails across drives on Windows), `remove(p, ec)` (fails on non-empty directories), `remove_all(p, ec)`.
- `copy(from, to, options, ec)` and `copy_file(from, to, options, ec)` with the correct `copy_options` flags.
- `weakly_canonical(p, ec)`, `canonical(p, ec)` (requires the path to exist), `absolute(p, ec)`, `relative`, `equivalent(p1, p2, ec)`, `current_path(ec)`, `space(p, ec)`, `temp_directory_path(ec)`.

**Known pitfalls the agent must handle**

- `path::string()` on Windows may lose non-ASCII characters; use `u8string()` for display and conversion (returns `std::string` in C++17).
- `directory_iterator` throws if the directory is unreadable and no `error_code` is passed.
- Symlink loops: do not follow directory symlinks while searching or computing size.
- Copying a directory into itself recurses forever; check before copying.
- Reserved Windows names and trailing dots or spaces in names.
- Calling `std::filesystem::path::filename()` on a path ending with a separator returns an empty path; normalise first.
- Case sensitivity differs by platform, so tests must not assume it.
- Windows console: needs UTF-8 and virtual terminal setup for colours; guard all Win32 calls with `#ifdef _WIN32` and include `<windows.h>` only there.
- POSIX calls (`opendir`, `readdir`, `stat`) only inside `#if defined(__unix__) || defined(__APPLE__)`.

**Qt calls to verify (Phase 8)**

- Confirm each signal and slot signature, and use the function-pointer `connect` syntax so mismatches fail at compile time.
- `find_package(Qt6 COMPONENTS Widgets)`, `CMAKE_AUTOMOC ON`, link `Qt6::Widgets`.
- Qt 6 removed `QRegExp`; use `QRegularExpression`. Confirm each class used exists in the installed Qt version.
- Any model class must implement `rowCount`, `columnCount`, `data`, and `headerData` with correct `const` signatures and call `beginResetModel` / `endResetModel` when data changes.
- Do not touch widgets from worker threads; send results back through signals.

## 13. Agent self-debug and error-fixing loop

The agent must actively look for errors, not wait for them to appear.

1. Build from scratch (clean build directory) with warnings enabled.
2. Run the unit tests (`ctest --output-on-failure`).
3. Run the CLI scenario scripts and diff the output against the expected output.
4. Run the API contract tests and, where available, the sanitizer build and `clang-tidy` / `cppcheck`.
5. Collect every compiler error, warning, test failure, crash, sanitizer report and unexpected output.
6. For each problem: identify the root cause, fix it in the right layer (core first), and add or update a test that would have caught it.
7. Log it in `docs/BUGLOG.md` (symptom, root cause, fix, test added).
8. Repeat from step 1.

**Exit criteria (all must be true, on two consecutive runs)**

- Zero compiler errors and zero warnings.
- All tests pass.
- No sanitizer reports.
- All CLI scenarios match.
- No uncaught exceptions on any invalid input.

**Rules**

- Maximum 5 fix attempts per issue. If it is still failing, document it in `docs/BUGLOG.md` with the best workaround and move on.
- Never remove a test, silence a warning, or catch-and-ignore an error to pass.
- Re-read the exact error message before changing code. Do not guess.
- After a fix, rerun the whole suite, not only the failing test, to catch regressions.
- Also review the code manually for bugs the tools cannot see: unchecked return values, off-by-one errors in depth limits, unclosed streams, missing `error_code` checks, wrong path joins.

## 14. Testing plan

**Sandbox rule:** every test creates a unique temporary folder under `std::filesystem::temp_directory_path()`, works only inside it, and removes it at the end, even on failure.

**Unit and edge cases**

- Empty directory, directory with 10,000 files, nonexistent path, file used where a directory is expected.
- Names with spaces, unicode, very long names, trailing slash in the path, relative paths with `..`.
- Hidden files, read-only files, zero-byte files, binary files, a broken symlink, a symlink loop (skip symlink tests where the OS needs elevated rights, and report them as skipped, not passed).
- Rename to an existing name, rename with an invalid name, copy into itself, move across directories, delete non-empty directory with and without recursive, delete a protected path (must be refused).
- Search: case-insensitive, depth limit, result cap, unreadable subfolder.

**CLI scenarios:** a scripted session for each command, a session full of invalid commands (must never crash), and a session with quoted paths.

**GUI manual checklist (Phase 8):** launch, expand tree, open a folder, back and forward, create, rename, copy, paste, delete with confirm, search, properties, resize window, trigger an error (open a protected folder) and confirm a message box appears and the app continues.

## 15. OS concepts mapping (for the viva)

- **Directory structure:** the explorer shows a tree-structured directory; symlinks give acyclic-graph behaviour. Discuss single-level, two-level, tree and acyclic-graph directories.
- **File attributes:** name, type, size, time, protection, and the identifying information stored in metadata (the inode on Unix). `getInfo` and `info` show these.
- **File operations:** create, delete, rename, read (preview), copy and move, and what each costs in system calls.
- **Directory operations:** list (readdir), search, create, delete, rename, traverse the file system.
- **Path resolution:** absolute vs relative paths, `.` and `..`, canonical paths.
- **Access control:** read, write and execute permissions for owner, group and others; why Windows only shows read-only.
- **Links:** symbolic links versus hard links and how the explorer treats them.
- **Free space and file system info:** `space()` gives capacity, free and available space.
- **System calls behind the code:** `std::filesystem` wraps `opendir`/`readdir`/`stat` (POSIX) or `FindFirstFile`/`FindNextFile` (Windows). The `os_layer_demo.cpp` module shows this directly.
- **Design argument:** why the logic is separated from the interface (modularity, testing, reuse).
- **Reference:** the file system chapters of Silberschatz, Galvin and Gagne, *Operating System Concepts*.

## 16. Definition of done

- The project builds from a clean clone with the README steps, with zero warnings.
- All tests and CLI scenarios pass in two consecutive clean runs.
- Every command in Section 7 works and every error in Section 10 is handled with its message.
- `docs/API_CONTRACT.md` is complete and every row is covered by a test.
- `docs/BUGLOG.md` lists every fixed bug and no unresolved item without a documented workaround.
- The GUI (if built) performs all Section 8 operations without a crash.
- The documentation pack from Phase 9 is complete.
- A final `docs/PROGRESS.md` summary lists each phase, its status, and what was verified.
