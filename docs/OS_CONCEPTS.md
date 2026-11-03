# Operating Systems Concepts Mapping

This document connects every feature implemented in the **OS File Explorer** directly to the theoretical concepts taught in **Unit IV: Storage Management** of standard Operating System syllabi (Silberschatz, Galvin & Gagne, *Operating System Concepts*).

---

## 1. Directory Structure

- **Tree-Structured Directory:**
  In modern operating systems, files are organized hierarchically starting from a root (`/` on Unix, drive letters like `C:\` on Windows). Every directory entry maps a human-readable name to file metadata.
- **Acyclic-Graph Directory & Symlinks:**
  Symbolic links allow files or directories to have multiple paths within the tree hierarchy. However, cycles in directory links can cause infinite traversal loops.
  - *Project Implementation:* In `src/core/search.cpp`, `std::filesystem::recursive_directory_iterator` is constructed without `follow_directory_symlink` to prevent cyclic recursion loops.

---

## 2. File Attributes & Metadata (The Inode Concept)

The file system separates file **content** (stored on disk blocks) from file **attributes** (stored in an Inode on Unix or the Master File Table / MFT record on NTFS).
Key attributes retrieved by `fx::getInfo` and `fx::listDirectory`:
- **Identifier & Name:** Stored in directory entries.
- **Type:** Regular file, directory, symbolic link, or special block/character device.
- **Size:** Actual byte count of the data stream.
- **Timestamps:** Last modification time (`mtime`), creation time (`ctime`), access time (`atime`).
- **Protection / Permissions:** Bitmask defining Read, Write, and Execute permissions.

---

## 3. Path Resolution & Process Context

Every running process maintains a **Current Working Directory (CWD)** attribute in its process control block (PCB).
- **Absolute Paths:** Fully qualified paths starting from the filesystem root (e.g. `C:\Windows` or `/usr/bin`).
- **Relative Paths:** Resolved relative to the process CWD (e.g. `docs/PROGRESS.md`).
- **Special Directory Entries:**
  - `.` refers to the current directory (loop-back pointer in directory block).
  - `..` refers to the parent directory block.
- *Project Implementation:* `fx::Navigator` resolves relative paths, expands the user home tilde (`~`), and canonicalizes using `std::filesystem::weakly_canonical` to eliminate `.` and `..` segments safely.

---

## 4. File System Operations & System Call Costs

| Operation | Typical POSIX Calls | Typical Win32 Calls | OS System Call Cost |
|---|---|---|---|
| **Directory Listing** | `opendir()`, `readdir()`, `closedir()` | `FindFirstFileW()`, `FindNextFileW()`, `FindClose()` | $O(N)$ directory entry traversal |
| **File Creation** | `open(O_CREAT)`, `close()` | `CreateFileW(CREATE_NEW)` | Updates directory block and allocates new inode / MFT record |
| **File Rename** | `rename()` | `MoveFileExW()` | Single metadata directory entry update (pointer swap, zero data copied) |
| **Cross-Volume Move** | `read()`, `write()`, `unlink()` | `CopyFileW()`, `DeleteFileW()` | Copies all data blocks across physical partitions, then frees old blocks |
| **File Deletion** | `unlink()` | `DeleteFileW()` | Decrements hard link count; marks disk blocks as free in allocation bitmap |
| **Free Space Query** | `statvfs()` | `GetDiskFreeSpaceExW()` | Reads filesystem superblock / volume metadata header |

---

## 5. Access Control & Protection

Operating systems protect multi-user systems through access control lists (ACLs) or permission bitmasks:
- **POSIX Mode Bits:** 9 permission bits grouped by Owner (`rwx`), Group (`rwx`), and Others (`rwx`).
- **Windows File Security:** While Windows supports complex Security Descriptors (DACLs), standard attributes map primarily to Read-Only, Hidden, System, and Archive flags.
- *Project Implementation:* `fx::formatPermissions` decodes the 9-bit mask into standard `rwxr-xr-x` format.

---

## 6. Real System Calls vs C++17 Standard Library

The C++17 `<filesystem>` library does not perform raw disk access; it is an abstract portability wrapper over native OS system calls.
- In `src/core/os_layer_demo.cpp`, direct invocations of Win32 `FindFirstFileW`/`FindNextFileW` and POSIX `opendir`/`readdir` demonstrate to students and examiners the concrete C system calls executing under the hood.

