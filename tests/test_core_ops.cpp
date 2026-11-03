// ============================================================================
// File: tests/test_core_ops.cpp
// Purpose: Unit tests for mutating operations (create, rename, remove, copy, move).
// OS Concept: File system mutation, integrity invariants, and directory safety.
// ============================================================================

#include "test_runner.hpp"
#include "../src/core/fs_ops.hpp"
#include "../src/core/safety.hpp"
#include <fstream>

int main() {
    using namespace fx;
    std::cout << "[Running Core Mutating Ops Tests]" << std::endl;

    test::Sandbox sandbox("fx_test_ops_");
    const auto& root = sandbox.path();

    // 1. Create file and duplicate creation
    auto fPath = root / "test_file.txt";
    auto cRes = createFile(fPath);
    TEST_ASSERT(cRes.ok, "createFile succeeds");
    TEST_ASSERT(std::filesystem::exists(fPath), "Created file actually exists");

    auto cDup = createFile(fPath);
    TEST_ASSERT(!cDup.ok, "Duplicate createFile fails");
    TEST_ASSERT(cDup.error.find("Already exists") != std::string::npos, "Duplicate returns 'Already exists'");

    // 2. Create directory and duplicate creation
    auto dPath = root / "test_dir";
    auto dRes = createDirectory(dPath);
    TEST_ASSERT(dRes.ok, "createDirectory succeeds");
    TEST_ASSERT(std::filesystem::is_directory(dPath), "Created directory actually exists");

    auto dDup = createDirectory(dPath);
    TEST_ASSERT(!dDup.ok, "Duplicate createDirectory fails");

    // 3. Rename file
    auto renRes = renamePath(fPath, "renamed_file.txt");
    TEST_ASSERT(renRes.ok, "renamePath succeeds");
    TEST_ASSERT(!std::filesystem::exists(fPath), "Old filename no longer exists");
    TEST_ASSERT(std::filesystem::exists(root / "renamed_file.txt"), "New filename exists");

    // Rename with invalid name (e.g. containing slashes or illegal chars)
    auto badRen = renamePath(root / "renamed_file.txt", "sub/folder.txt");
    TEST_ASSERT(!badRen.ok, "renamePath with path separator is rejected");

    // 4. Copy file
    auto destFolder = root / "copy_dest";
    createDirectory(destFolder);
    auto cpFileRes = copyPath(root / "renamed_file.txt", destFolder);
    TEST_ASSERT(cpFileRes.ok, "copyPath for file succeeds");
    TEST_ASSERT(std::filesystem::exists(destFolder / "renamed_file.txt"), "Copied file exists in destination");
    TEST_ASSERT(std::filesystem::exists(root / "renamed_file.txt"), "Source file still exists after copy");

    // 5. Copy directory tree and prevent copy-into-itself
    auto treeRoot = root / "source_tree";
    createDirectory(treeRoot);
    createDirectory(treeRoot / "sub");
    createFile(treeRoot / "sub" / "leaf.txt");

    // Attempt to copy source_tree into its own subdirectory (must be refused!)
    auto selfCopy = copyPath(treeRoot, treeRoot / "sub");
    TEST_ASSERT(!selfCopy.ok, "Copy directory into itself is refused");
    TEST_ASSERT(selfCopy.error.find("Cannot copy a folder into itself") != std::string::npos,
                "Reports correct copy-into-itself error");

    // Valid copy directory tree
    auto validTreeDest = root / "tree_destination";
    createDirectory(validTreeDest);
    auto cpTreeRes = copyPath(treeRoot, validTreeDest);
    TEST_ASSERT(cpTreeRes.ok, "Copy directory tree succeeds");
    TEST_ASSERT(std::filesystem::exists(validTreeDest / "source_tree" / "sub" / "leaf.txt"),
                "Recursive tree exists in destination");

    // 6. Move operation
    auto moveDest = root / "move_destination";
    createDirectory(moveDest);
    auto mvRes = movePath(treeRoot, moveDest);
    TEST_ASSERT(mvRes.ok, "movePath succeeds");
    TEST_ASSERT(!std::filesystem::exists(treeRoot), "Source directory removed after move");
    TEST_ASSERT(std::filesystem::exists(moveDest / "source_tree" / "sub" / "leaf.txt"),
                "Moved tree exists in destination");

    // 7. Remove non-empty directory without recursive flag (must fail!)
    auto rmFail = removePath(moveDest / "source_tree", false);
    TEST_ASSERT(!rmFail.ok, "Removing non-empty directory without recursive flag fails");
    TEST_ASSERT(rmFail.error.find("Directory not empty") != std::string::npos,
                "Error indicates 'Directory not empty. Use rm -r.'");

    // Remove non-empty directory with recursive flag (must succeed!)
    auto rmRec = removePath(moveDest / "source_tree", true);
    TEST_ASSERT(rmRec.ok, "Removing non-empty directory with recursive flag succeeds");
    TEST_ASSERT(!std::filesystem::exists(moveDest / "source_tree"), "Directory removed after recursive delete");

    // 8. Protected path refusal test
    TEST_ASSERT(isProtectedPath(std::filesystem::path("C:/")), "Drive root is protected");
    TEST_ASSERT(isProtectedPath(std::filesystem::path("/")), "Root slash is protected");
    auto badDelRoot = removePath("C:/", true);
    TEST_ASSERT(!badDelRoot.ok, "removePath on protected root is blocked");

    return test::reportSummary("Core Mutating Ops Tests");
}
