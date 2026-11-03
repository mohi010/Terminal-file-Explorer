# API Contract & System Call Mapping

This document specifies every external standard library and operating system call used by `fxcore`, the exact signature and overload used, failure behaviour, and the corresponding project function.

---

## 1. std::filesystem Call Verification Table

| Function | Header | Exact Overload Used | Failure Behaviour / Error Handling | Used in Core Function |
|---|---|---|---|---|
| `directory_iterator` | `<filesystem>` | `(const path&, directory_options, std::error_code&)` | Sets `ec`, constructs empty iterator on error | `fx::listDirectory` |
| `it.increment` | `<filesystem>` | `(std::error_code&)` | Sets `ec`, avoids throwing exceptions on unreadable entries | `fx::listDirectory` |
| `recursive_directory_iterator` | `<filesystem>` | `(const path&, directory_options, std::error_code&)` | Sets `ec`, allows controlled tree traversal | `fx::searchByName`, `fx::directorySize` |
| `it.disable_recursion_pending` | `<filesystem>` | `()` | Enforces `maxDepth` by stopping deeper descent | `fx::searchByName` |
| `symlink_status` | `<filesystem>` | `(const path&, std::error_code&)` | Sets `ec`, does NOT follow symlinks (detects broken symlinks) | `fx::getInfo`, `fx::listDirectory` |
| `status` | `<filesystem>` | `(const path&, std::error_code&)` | Sets `ec`, checks target existence and file type | `fx::getInfo`, `fx::listDirectory` |
| `is_regular_file` | `<filesystem>` | `(file_status)` | Returns `bool`, no-throw | `fx::getInfo`, `fx::readTextPreview` |
| `is_directory` | `<filesystem>` | `(file_status)` | Returns `bool`, no-throw | `fx::getInfo`, `fx::listDirectory`, `Navigator::cd` |
| `is_symlink` | `<filesystem>` | `(file_status)` | Returns `bool`, no-throw | `fx::getInfo` |
| `file_size` | `<filesystem>` | `(const path&, std::error_code&)` | Fails on directories; only called for regular files | `fx::getInfo`, `fx::directorySize` |
| `last_write_time` | `<filesystem>` | `(const path&, std::error_code&)` | Returns `file_time_type`, converted via duration offset | `fx::getInfo` |
| `create_directory` | `<filesystem>` | `(const path&, std::error_code&)` | Fails if path exists; sets `ec` on failure | `fx::createDirectory` |
| `rename` | `<filesystem>` | `(const path&, const path&, std::error_code&)` | Fails across different drives on Windows | `fx::renamePath`, `fx::movePath` |
| `remove` | `<filesystem>` | `(const path&, std::error_code&)` | Fails if directory is non-empty; sets `ec` | `fx::removePath` |
| `remove_all` | `<filesystem>` | `(const path&, std::error_code&)` | Recursively removes directory tree; sets `ec` | `fx::removePath` (recursive) |
| `copy` | `<filesystem>` | `(const path&, const path&, copy_options, std::error_code&)` | Sets `ec` on failure; checks self-copy first | `fx::copyPath` |
| `copy_file` | `<filesystem>` | `(const path&, const path&, copy_options, std::error_code&)` | Copies single file; sets `ec` on failure | `fx::copyPath`, `fx::movePath` |
| `weakly_canonical` | `<filesystem>` | `(const path&, std::error_code&)` | Resolves `.` and `..` without requiring leaf existence | `fx::safety`, `Navigator::cd` |
| `canonical` | `<filesystem>` | `(const path&, std::error_code&)` | Resolves full path; requires path to exist | `fx::safety` |
| `space` | `<filesystem>` | `(const path&, std::error_code&)` | Returns `space_info` (capacity, free, available) | `fx::getSpace` |
| `temp_directory_path` | `<filesystem>` | `(std::error_code&)` | Returns OS temporary directory for tests | Test suites |

---
