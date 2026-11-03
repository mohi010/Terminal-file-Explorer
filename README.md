# OS File Explorer (C++17 Core, Terminal CLI + Qt GUI)

> **Course:** Operating Systems, Unit IV (Storage Management: File System, Directory Structures, Metadata, Access Control & Free Space)  
> **Institution:** NSUT / University Mini Project  
> **Language & Standard:** C++17 (Strictly without non-standard extensions)  
> **Build System:** CMake 3.16+ (Ninja / MinGW Makefiles)

---

## 1. Overview & Architecture

The **OS File Explorer** is an educational, production-quality file management system designed to demonstrate operating system storage management principles. It features a three-tier modular architecture:

1. **Layer 1: Core Static Library (`fxcore`)**  
   Pure C++17 business logic with non-throwing `Result<T>` containers. Direct integration with standard filesystem system call wrappers (`std::filesystem`).
2. **Layer 2: Terminal Front End (`fx_cli`)**  
   Interactive REPL shell supporting path tokenization (quoted spaces), ANSI color formatting, table output, and navigation history.
3. **Layer 3: Desktop GUI Front End (`fx_gui`)**  
   Native Qt Widgets desktop application utilizing a custom `QAbstractTableModel` that queries `fxcore` directly without third-party abstraction layers.

---

## 2. Key Features

- **Directory Operations:** List, sort (name, size, date, type), filter hidden files, hierarchical ASCII `tree` visualization.
- **File Mutation:** Create (`touch`, `mkdir`), rename, copy (single file and directory tree), move (cross-drive fallback), and delete (`rm`, `rm -r`).
- **Safety Invariants:** Protected system roots (`C:\`, `/`, `/etc`, Windows directory), self-copy loop prevention, and reserved filename enforcement (`CON`, `PRN`, `AUX`, `NUL`).
- **Search & Inspection:** Depth-limited case-insensitive search (`find`), directory size accounting (`du`), disk free space query (`space`), and text preview with binary detection (`view`).
- **OS Explainability (`osdemo`):** Direct POSIX (`opendir`/`readdir`) and Win32 (`FindFirstFileW`/`FindNextFileW`) demonstration module.

---

## 3. Quick Start & Build Instructions

### Prerequisites
- Modern C++17 compiler (GCC 9+, Clang 10+, or MSVC 2019+)
- CMake 3.16 or newer

### Windows Build (Native)
Open PowerShell in the project directory:

```powershell
# 1. Configure build directory
cmake -S . -B build -G "MinGW Makefiles"

# 2. Compile all targets with warnings enabled
cmake --build build

# 3. Run all unit and stress tests
cd build
ctest --output-on-failure
cd ..

# 4. Launch the Interactive Terminal CLI
.\build\fx_cli.exe
```

### Linux & macOS Build
```bash
cmake -S . -B build
cmake --build build
cd build && ctest --output-on-failure && cd ..
./build/fx_cli
```

---

## 4. Terminal Command Reference

| Command | Usage | Description |
|---|---|---|
| `help` | `help` | Lists all available commands |
| `pwd` | `pwd` | Prints current working directory |
| `ls` | `ls [path] [-a] [--sort name\|size\|date\|type] [--desc]` | Lists directory entries in an aligned table |
| `cd` | `cd <path>` | Changes directory (supports `~`, `..`, quoted paths) |
| `up` | `up` | Moves to parent directory |
| `back` / `forward` | `back` / `forward` | History stack navigation |
| `tree` | `tree [depth]` | Displays ASCII directory tree (default depth 2) |
| `info` | `info <path>` | Displays file attributes, timestamps, permissions |
| `view` | `view <file>` | Displays read-only text preview (safely rejects binary files) |
| `mkdir` / `touch` | `mkdir <name>`, `touch <name>` | Creates directory or empty file |
| `rename` | `rename <path> <newname>` | Renames file or directory |
| `cp` / `mv` | `cp <src> <destdir>`, `mv <src> <destdir>` | Copies or moves files and directory trees |
| `rm` | `rm <path> [-r]` | Deletes file or directory with confirmation prompt |
| `find` | `find <pattern> [--depth n]` | Depth-limited recursive search |
| `du` | `du <path>` | Calculates recursive directory size |
| `space` | `space` | Displays drive capacity, free space, and available space |
| `osdemo` | `osdemo` | Prints entries using native Win32/POSIX system calls |
| `exit` | `exit` | Exits the shell |

---

## 5. Documentation Pack

Full viva preparation materials and technical specifications are available in the [`docs/`](docs/) directory:
- [`docs/PROGRESS.md`](docs/PROGRESS.md): Phase-by-phase implementation log and acceptance verification.
- [`docs/DECISIONS.md`](docs/DECISIONS.md): Architectural decisions, toolchain choices, and environment setup.
- [`docs/BUGLOG.md`](docs/BUGLOG.md): Bug tracking, root cause analyses, and regression tests.
- [`docs/API_CONTRACT.md`](docs/API_CONTRACT.md): Comprehensive system call and standard library mapping table.
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md): Multi-layer architecture diagram and file responsibilities.
- [`docs/OS_CONCEPTS.md`](docs/OS_CONCEPTS.md): Operating systems theoretical mapping (Silberschatz Unit IV).
- [`docs/VIVA_QA.md`](docs/VIVA_QA.md): 42 viva examination questions and answers.
- [`docs/DEMO_SCRIPT.md`](docs/DEMO_SCRIPT.md): 5-minute practical viva demo walkthrough.
