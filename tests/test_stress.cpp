// ============================================================================
// File: tests/test_stress.cpp
// Purpose: Stress tests for large directory listing (10,000 files) and performance benchmark.
// OS Concept: Performance of directory block traversal and caching under heavy I/O.
// ============================================================================

#include "test_runner.hpp"
#include "../src/core/fs_ops.hpp"
#include <chrono>

int main() {
    using namespace fx;
    std::cout << "[Running Stress & Performance Benchmark]" << std::endl;

    test::Sandbox sandbox("fx_test_stress_");
    const auto& root = sandbox.path();

    auto largeDir = root / "large_folder";
    createDirectory(largeDir);

    std::cout << "Creating 10,000 test files..." << std::endl;
    for (int i = 0; i < 10000; ++i) {
        std::string fname = (largeDir / ("file_" + std::to_string(i) + ".dat")).string();
        FILE* f = fopen(fname.c_str(), "wb");
        if (f) fclose(f);
    }

    std::cout << "Measuring listDirectory on 10,000 files..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    ListOptions opt;
    auto listRes = listDirectory(largeDir, opt);

    auto end = std::chrono::high_resolution_clock::now();
    double elapsedSeconds = std::chrono::duration<double>(end - start).count();

    std::cout << "Time elapsed: " << elapsedSeconds << " seconds." << std::endl;

    TEST_ASSERT(listRes.ok, "listDirectory on 10,000 files succeeded");
    TEST_ASSERT(listRes.value.size() == 10000, "Exactly 10,000 entries retrieved");
#if defined(_WIN32)
    const double maxAllowedSeconds = 6.0; // Windows NTFS file handle querying with Windows Defender
#else
    const double maxAllowedSeconds = 2.0; // Linux / macOS in-memory inode traversal
#endif
    TEST_ASSERT(elapsedSeconds < maxAllowedSeconds, "Listing 10,000 files finished within threshold");

    std::cout << "Cleaning up stress files..." << std::endl;
    return test::reportSummary("Stress Benchmark");
}
