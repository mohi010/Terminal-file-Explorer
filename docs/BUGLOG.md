# Bug Log & Fix Tracking

This log records every issue identified, the root cause, the fix applied (core first), and the test case added or updated to prevent regressions.

---

| Bug ID | Component | Symptom | Root Cause | Fix Applied | Test Added / Updated |
|---|---|---|---|---|---|
| (init) | Build | GCC 6.3 on system lacks `<filesystem>` | Legacy MinGW GCC 6.3 does not support C++17 filesystem | Integrated modern MinGW-w64 (GCC 16.2) toolchain with standard C++17 support | Toolchain verification test in Phase 0 |
| BUG-01 | Core | `std::uint64_t does not name a type` in `os_layer_demo.cpp` | Missing `<cstdint>` header include in `os_layer_demo.cpp` | Added `#include <cstdint>` | Build compilation check |
| PERF-01 | Core | 10k files listing took 5.01s (exceeds 2.0s) | Multiple redundant syscalls per entry inside `listDirectory` | Leveraged `directory_entry` cached status and enabled `-O2` | `tests/test_stress.cpp` |

---
