// ============================================================================
// File: tests/test_core_list.cpp
// Purpose: Unit tests for fx::listDirectory and fx::getInfo.
// OS Concept: Verifying directory stream traversal and attribute retrieval.
// ============================================================================

#include "test_runner.hpp"
#include "../src/core/fs_ops.hpp"
#include <fstream>

int main() {
    using namespace fx;
    std::cout << "[Running Core List Tests]" << std::endl;

    test::Sandbox sandbox("fx_test_list_");
    const auto& root = sandbox.path();

    // 1. Empty directory listing
    auto emptyRes = listDirectory(root);
    TEST_ASSERT(emptyRes.ok, "listDirectory on empty directory succeeds");
    TEST_ASSERT(emptyRes.value.empty(), "Empty directory has 0 entries");

    // 2. Nonexistent path
    auto nonExist = listDirectory(root / "nonexistent_subfolder");
    TEST_ASSERT(!nonExist.ok, "listDirectory on nonexistent path fails");
    TEST_ASSERT(nonExist.error.find("Path not found") != std::string::npos, "Nonexistent error message is clear");

    // 3. Create items: subdirs, files, hidden file, unicode file
    createDirectory(root / "dir_b");
    createDirectory(root / "dir_a");
    createFile(root / "file_2.txt");
    createFile(root / "file_1.txt");
    createFile(root / ".hidden_file");
    createFile(root / "unicode_файл_тест.txt");

    // Write some bytes to file_2 to test size sorting
    {
        std::ofstream ofs(root / "file_2.txt");
        ofs << "Hello World! Operating Systems Mini Project";
    }

    // 4. Listing without hidden files
    ListOptions optDefault;
    optDefault.showHidden = false;
    auto defaultList = listDirectory(root, optDefault);
    TEST_ASSERT(defaultList.ok, "Default listDirectory succeeds");

    bool hasHidden = false;
    for (const auto& item : defaultList.value) {
        if (item.name == ".hidden_file") {
            hasHidden = true;
        }
    }
    TEST_ASSERT(!hasHidden, "Hidden file omitted when showHidden=false");

    // Check directory grouping: all directories must appear before files
    bool encounteredFile = false;
    bool groupingCorrect = true;
    for (const auto& item : defaultList.value) {
        if (item.type == EntryType::File) {
            encounteredFile = true;
        } else if (item.type == EntryType::Directory && encounteredFile) {
            groupingCorrect = false;
        }
    }
    TEST_ASSERT(groupingCorrect, "Directories are grouped before regular files");

    // 5. Listing with hidden files
    ListOptions optHidden;
    optHidden.showHidden = true;
    auto hiddenList = listDirectory(root, optHidden);
    TEST_ASSERT(hiddenList.ok, "listDirectory with showHidden=true succeeds");
    bool foundHidden = false;
    for (const auto& item : hiddenList.value) {
        if (item.name == ".hidden_file") {
            foundHidden = true;
            TEST_ASSERT(item.isHidden, "item.isHidden is true for dot-file");
        }
    }
    TEST_ASSERT(foundHidden, "Hidden file included when showHidden=true");

    // 6. Test Sorting by Name (Ascending vs Descending)
    ListOptions optAsc;
    optAsc.sortBy = SortBy::Name;
    optAsc.ascending = true;
    auto ascList = listDirectory(root, optAsc);
    TEST_ASSERT(ascList.ok && ascList.value.size() >= 2, "Ascending name listing ok");
    TEST_ASSERT(ascList.value[0].name == "dir_a" && ascList.value[1].name == "dir_b",
                "Directories sorted alphabetically ascending (dir_a before dir_b)");

    ListOptions optDesc;
    optDesc.sortBy = SortBy::Name;
    optDesc.ascending = false;
    auto descList = listDirectory(root, optDesc);
    TEST_ASSERT(descList.ok && descList.value.size() >= 2, "Descending name listing ok");
    TEST_ASSERT(descList.value[0].name == "dir_b" && descList.value[1].name == "dir_a",
                "Directories sorted alphabetically descending (dir_b before dir_a)");

    // 7. Test getInfo
    auto infoRes = getInfo(root / "file_2.txt");
    TEST_ASSERT(infoRes.ok, "getInfo on existing file succeeds");
    TEST_ASSERT(infoRes.value.type == EntryType::File, "getInfo returns File type");
    TEST_ASSERT(infoRes.value.size > 0, "getInfo returns accurate non-zero file size");

    // 8. Test listDirectory on a file (should fail with 'Not a directory')
    auto fileList = listDirectory(root / "file_2.txt");
    TEST_ASSERT(!fileList.ok, "listDirectory on a file fails");
    TEST_ASSERT(fileList.error.find("Not a directory") != std::string::npos, "Error reports 'Not a directory'");

    return test::reportSummary("Core List Tests");
}

