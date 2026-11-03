// ============================================================================
// File: tests/test_navigator.cpp
// Purpose: Unit tests for fx::Navigator (cd, up, back, forward, path normalisation).
// OS Concept: Process working directory state and path resolution.
// ============================================================================

#include "test_runner.hpp"
#include "../src/core/navigator.hpp"
#include "../src/core/fs_ops.hpp"

int main() {
    using namespace fx;
    std::cout << "[Running Navigator Tests]" << std::endl;

    test::Sandbox sandbox("fx_test_nav_");
    const auto& root = sandbox.path();

    auto dir1 = root / "folder_alpha";
    auto dir2 = dir1 / "sub_beta";
    createDirectory(dir1);
    createDirectory(dir2);

    Navigator nav(root);
    TEST_ASSERT(nav.current() == root, "Navigator starts at specified path");
    TEST_ASSERT(!nav.canGoBack(), "Initial navigator cannot go back");
    TEST_ASSERT(!nav.canGoForward(), "Initial navigator cannot go forward");

    // cd to subfolder
    auto cdRes1 = nav.cd("folder_alpha");
    TEST_ASSERT(cdRes1.ok, "cd to relative child directory succeeds");
    TEST_ASSERT(nav.current() == dir1, "nav.current() reflects folder_alpha");
    TEST_ASSERT(nav.canGoBack(), "Can go back after cd");
    TEST_ASSERT(!nav.canGoForward(), "Cannot go forward immediately after cd");

    // cd into second subfolder
    auto cdRes2 = nav.cd("sub_beta");
    TEST_ASSERT(cdRes2.ok, "cd to sub_beta succeeds");
    TEST_ASSERT(nav.current() == dir2, "nav.current() reflects sub_beta");

    // Test back()
    auto backRes1 = nav.back();
    TEST_ASSERT(backRes1.ok, "back() succeeds");
    TEST_ASSERT(nav.current() == dir1, "nav.current() returns to folder_alpha");
    TEST_ASSERT(nav.canGoForward(), "Can go forward after back()");

    // Test forward()
    auto fwdRes = nav.forward();
    TEST_ASSERT(fwdRes.ok, "forward() succeeds");
    TEST_ASSERT(nav.current() == dir2, "forward() returns to sub_beta");

    // Test up()
    auto upRes = nav.up();
    TEST_ASSERT(upRes.ok, "up() succeeds");
    TEST_ASSERT(nav.current() == dir1, "up() navigates to parent directory");

    // Test invalid target does NOT change current directory
    auto oldPath = nav.current();
    auto badCd = nav.cd("non_existent_folder_xyz");
    TEST_ASSERT(!badCd.ok, "cd to nonexistent path fails");
    TEST_ASSERT(nav.current() == oldPath, "Failed cd keeps current directory unchanged");

    // Test cd to a file (fails with Not a directory)
    createFile(dir1 / "somefile.txt");
    auto fileCd = nav.cd("somefile.txt");
    TEST_ASSERT(!fileCd.ok, "cd to file fails");
    TEST_ASSERT(nav.current() == oldPath, "Failed cd to file leaves current path intact");

    return test::reportSummary("Navigator Tests");
}
