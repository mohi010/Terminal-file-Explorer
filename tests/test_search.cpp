// ============================================================================
// File: tests/test_search.cpp
// Purpose: Unit tests for searchByName, directorySize, and readTextPreview.
// OS Concept: Tree search, depth limits, file size aggregation, and magic byte checking.
// ============================================================================

#include "test_runner.hpp"
#include "../src/core/search.hpp"
#include "../src/core/preview.hpp"
#include "../src/core/fs_ops.hpp"
#include <fstream>

int main() {
    using namespace fx;
    std::cout << "[Running Search & Preview Tests]" << std::endl;

    test::Sandbox sandbox("fx_test_search_");
    const auto& root = sandbox.path();

    // Setup folder tree
    // root/
    //   alpha.txt (10 bytes)
    //   dir1/
    //     BETA.TXT (20 bytes)
    //     dir2/
    //       gamma.log (30 bytes)
    //       dir3/
    //         deep_target.txt (40 bytes)
    //   binary.bin (contains NUL byte)

    createFile(root / "alpha.txt");
    {
        std::ofstream ofs(root / "alpha.txt");
        ofs << "0123456789"; // 10 bytes
    }

    createDirectory(root / "dir1");
    createFile(root / "dir1" / "BETA.TXT");
    {
        std::ofstream ofs(root / "dir1" / "BETA.TXT");
        ofs << "12345678901234567890"; // 20 bytes
    }

    createDirectory(root / "dir1" / "dir2");
    createFile(root / "dir1" / "dir2" / "gamma.log");
    {
        std::ofstream ofs(root / "dir1" / "dir2" / "gamma.log");
        ofs << "123456789012345678901234567890"; // 30 bytes
    }

    createDirectory(root / "dir1" / "dir2" / "dir3");
    createFile(root / "dir1" / "dir2" / "dir3" / "deep_target.txt");
    {
        std::ofstream ofs(root / "dir1" / "dir2" / "dir3" / "deep_target.txt");
        ofs << "1234567890123456789012345678901234567890"; // 40 bytes
    }

    // Binary file creation with NUL bytes
    createFile(root / "binary.bin");
    {
        std::ofstream ofs(root / "binary.bin", std::ios::binary);
        const char binData[] = {'E', 'L', 'F', '\0', 0x01, 0x02, '\0', 0x7F};
        ofs.write(binData, sizeof(binData));
    }

    // 1. Search case-insensitive substring
    SearchOptions sOpt1;
    sOpt1.caseInsensitive = true;
    auto resBeta = searchByName(root, "beta", sOpt1);
    TEST_ASSERT(resBeta.ok, "searchByName succeeded");
    TEST_ASSERT(resBeta.value.size() == 1, "Found BETA.TXT with lowercase pattern");
    TEST_ASSERT(resBeta.value[0].name == "BETA.TXT", "Matches exact file name");

    // 2. Search depth limit
    SearchOptions sOptDepth;
    sOptDepth.maxDepth = 2; // Should NOT reach deep_target.txt at depth 3
    auto resDepth = searchByName(root, "deep_target", sOptDepth);
    TEST_ASSERT(resDepth.ok, "Depth-limited search succeeded");
    TEST_ASSERT(resDepth.value.empty(), "deep_target.txt not found when depth is limited to 2");

    // Allow full depth
    SearchOptions sOptDeep;
    sOptDeep.maxDepth = 10;
    auto resDeep = searchByName(root, "deep_target", sOptDeep);
    TEST_ASSERT(resDeep.ok && resDeep.value.size() == 1, "deep_target.txt found with depth 10");

    // 3. Directory size calculation
    auto sizeRes = directorySize(root / "dir1");
    TEST_ASSERT(sizeRes.ok, "directorySize succeeded");
    TEST_ASSERT(sizeRes.value == 90, "Total size of dir1 tree is 20 + 30 + 40 = 90 bytes");

    // 4. Text preview on text file
    auto prevText = readTextPreview(root / "alpha.txt");
    TEST_ASSERT(prevText.ok, "readTextPreview succeeds on text file");
    TEST_ASSERT(prevText.value == "0123456789", "readTextPreview matches file content");

    // 5. Text preview on binary file
    auto prevBin = readTextPreview(root / "binary.bin");
    TEST_ASSERT(!prevBin.ok, "readTextPreview on binary file fails");
    TEST_ASSERT(prevBin.error.find("Binary file, preview not available") != std::string::npos,
                "Reports 'Binary file, preview not available.'");

    return test::reportSummary("Search & Preview Tests");
}

