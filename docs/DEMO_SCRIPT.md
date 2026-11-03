# 5-Minute Practical Demonstration Script

Use this structured walkthrough for presenting the project to the professor or viva examiner.

---

## Step 1: Architecture & Course Alignment (1 Minute)
- **Statement:**
  > *"Good morning sir/ma'am. This project is a multi-tier File Explorer built strictly in C++17 for Operating Systems (Unit IV: Storage Management). The design separates mechanism from policy: a pure C++ static library `fxcore` implements all OS concepts and system calls, while two front ends—a Terminal CLI (`fx_cli`) and a Qt GUI (`fx_gui`)—provide user interfaces."*
- **Highlight:**
  > *"The core never throws unhandled exceptions; it uses Result containers, guarantees safe path resolution, blocks dangerous operations on protected roots, and avoids symlink loops."*

---

## Step 2: Terminal Interactive Demo (`fx_cli`) (2.5 Minutes)

1. **Launch the CLI:**
   ```powershell
   .\build\fx_cli.exe
   ```
2. **Display Disk Space & Environment:**
   ```text
   space
   ```
   *Explain:* Direct wrapper around filesystem superblock capacity and free block count.

3. **Inspect Directory & Attributes:**
   ```text
   ls
   info CMakeLists.txt
   ```
   *Explain:* Shows inode attributes: file size, modification timestamps, permissions bitmask (`rwxr-xr-x`), and hidden flags.

4. **Directory Navigation & History:**
   ```text
   mkdir demo_test
   cd demo_test
   touch file_a.txt
   touch file_b.txt
   ls
   up
   back
   forward
   ```
   *Explain:* Demonstrates process Current Working Directory (CWD) manipulation and browser-style history stacks.

5. **Recursive Operations & Safety Invariants:**
   ```text
   mkdir nested
   cp demo_test nested
   tree 2
   du demo_test
   rm demo_test -r
   y
   ```
   *Explain:* Shows directory tree traversal, recursive disk accounting (`du`), and confirmation before destructive removal.

6. **Demonstrate Safety Boundary (Intentional Failure):**
   ```text
   rm C:/ -r
   ```
   *Output:* `This path is protected and cannot be changed.`
   *Explain:* Shows security boundary protection against accidental system destruction.

---

## Step 3: Direct OS Layer Explainability (`osdemo`) (1.5 Minutes)

1. **Run the OS Layer Demo inside CLI:**
   ```text
   osdemo
   ```
2. **Explain the Code:**
   > *"Here we demonstrate the actual system call boundary underneath `std::filesystem`. On Windows, it invokes `FindFirstFileW` and `FindNextFileW` from `kernel32.dll`; on Linux/macOS, it uses `opendir()`, `readdir()`, and `stat()` from `dirent.h`. This proves to the examiner that standard library abstractions map 1:1 to underlying kernel primitives."*

3. **Run CTest Verification:**
   ```powershell
   cd build
   ctest --output-on-failure
   ```
   *Show that 100% of unit tests, API contracts, and stress benchmarks pass with zero warnings.*

