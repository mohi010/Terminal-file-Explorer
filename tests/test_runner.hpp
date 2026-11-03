// ============================================================================
// File: tests/test_runner.hpp
// Purpose: Minimal standalone test runner and RAII temporary test sandbox.
// OS Concept: Isolated process workspace and temporary filesystem sandboxing.
// ============================================================================

#pragma once

#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>
#include <random>

namespace fx::test {

inline int g_testsRun = 0;
inline int g_testsFailed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        ++fx::test::g_testsRun; \
        if (!(cond)) { \
            ++fx::test::g_testsFailed; \
            std::cerr << "[-] ASSERTION FAILED: " << (msg) << "\n" \
                      << "    Condition: " << #cond << "\n" \
                      << "    File: " << __FILE__ << ":" << __LINE__ << std::endl; \
        } else { \
            std::cout << "  [PASS] " << (msg) << std::endl; \
        } \
    } while (0)

// RAII Sandbox: guarantees unique temp folder creation and cleanup
class Sandbox {
public:
    Sandbox(const std::string& prefix = "fx_test_") {
        std::error_code ec;
        auto tempBase = std::filesystem::temp_directory_path(ec);
        if (ec) {
            tempBase = std::filesystem::current_path();
        }

        auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        m_path = tempBase / (prefix + std::to_string(timestamp) + "_" + std::to_string(generateRandomId()));
        std::filesystem::create_directories(m_path, ec);
    }

    ~Sandbox() {
        std::error_code ec;
        std::filesystem::remove_all(m_path, ec);
    }

    const std::filesystem::path& path() const {
        return m_path;
    }

private:
    static int generateRandomId() {
        static std::mt19937 rng(1337);
        return static_cast<int>(rng() % 100000);
    }

    std::filesystem::path m_path;
};

inline int reportSummary(const std::string& suiteName) {
    std::cout << "\n========================================\n"
              << " Test Suite: " << suiteName << "\n"
              << " Total: " << g_testsRun << ", Failed: " << g_testsFailed << "\n"
              << "========================================\n" << std::endl;
    return g_testsFailed == 0 ? 0 : 1;
}

} // namespace fx::test
