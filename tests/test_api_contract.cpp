// ============================================================================
// File: tests/test_api_contract.cpp
// Purpose: Verification of API contract for all core fx:: functions (PRD Section 12).
// OS Concept: API verification, boundary conditions, and exception-safety guarantees.
// ============================================================================

#include "test_runner.hpp"
#include "../src/core/fs_ops.hpp"
#include "../src/core/navigator.hpp"
#include "../src/core/search.hpp"
#include "../src/core/preview.hpp"
#include "../src/core/format.hpp"
#include "../src/core/safety.hpp"

int main() {
    using namespace fx;
    std::cout << "[Running API Contract Verification Tests]" << std::endl;

    test::Sandbox sandbox("fx_test_contract_");
    const auto& root = sandbox.path();

    // 1. listDirectory contract
    // Valid input
    auto listOk = listDirectory(root);
    TEST_ASSERT(listOk.ok, "listDirectory valid path returns ok=true");
    // Invalid input (empty path / nonexistent)
    auto listBad = listDirectory(root / "missing_dir_xyz");
    TEST_ASSERT(!listBad.ok && !listBad.error.empty(), "listDirectory nonexistent returns ok=false with error");

    // 2. getInfo contract
    auto fPath = root / "contract_file.txt";
    createFile(fPath);
    auto infoOk = getInfo(fPath);
    TEST_ASSERT(infoOk.ok && infoOk.value.name == "contract_file.txt", "getInfo valid file returns ok=true");
    auto infoBad = getInfo(root / "nonexistent_file_xyz.txt");
    TEST_ASSERT(!infoBad.ok && !infoBad.error.empty(), "getInfo nonexistent file returns ok=false with error");

    // 3. createFile contract
    auto f2 = root / "create_test.txt";
    auto crOk = createFile(f2);
    TEST_ASSERT(crOk.ok, "createFile valid returns ok=true");
    auto crDup = createFile(f2);
    TEST_ASSERT(!crDup.ok, "createFile duplicate returns ok=false");
    auto crBadName = createFile(root / "bad:name?.txt");
    TEST_ASSERT(!crBadName.ok, "createFile illegal characters returns ok=false");

    // 4. createDirectory contract
    auto d1 = root / "contract_dir";
    auto cdOk = createDirectory(d1);
    TEST_ASSERT(cdOk.ok, "createDirectory valid returns ok=true");
    auto cdDup = createDirectory(d1);
    TEST_ASSERT(!cdDup.ok, "createDirectory duplicate returns ok=false");

    // 5. renamePath contract
    auto renOk = renamePath(f2, "create_test_renamed.txt");
    TEST_ASSERT(renOk.ok, "renamePath valid returns ok=true");
    auto renBadChar = renamePath(root / "create_test_renamed.txt", "sub/folder.txt");
    TEST_ASSERT(!renBadChar.ok, "renamePath with separator returns ok=false");

    // 6. copyPath contract
    auto targetDir = root / "copy_dest_dir";
    createDirectory(targetDir);
    auto cpOk = copyPath(root / "create_test_renamed.txt", targetDir);
    TEST_ASSERT(cpOk.ok, "copyPath valid returns ok=true");
    auto cpSelf = copyPath(targetDir, targetDir);
    TEST_ASSERT(!cpSelf.ok, "copyPath into itself returns ok=false");

    // 7. movePath contract
    auto mvOk = movePath(root / "create_test_renamed.txt", targetDir);
    TEST_ASSERT(!mvOk.ok, "movePath to already existing destination returns ok=false (duplicate)");

    // 8. removePath contract
    auto rmOk = removePath(targetDir / "create_test_renamed.txt");
    TEST_ASSERT(rmOk.ok, "removePath valid file returns ok=true");
    auto rmMissing = removePath(root / "missing_file_to_remove.txt");
    TEST_ASSERT(!rmMissing.ok, "removePath nonexistent returns ok=false");

    // 9. searchByName contract
    auto srchOk = searchByName(root, "contract");
    TEST_ASSERT(srchOk.ok, "searchByName valid returns ok=true");
    auto srchBadRoot = searchByName(root / "no_such_dir", "anything");
    TEST_ASSERT(!srchBadRoot.ok, "searchByName bad root returns ok=false");

    // 10. directorySize contract
    auto szOk = directorySize(root);
    TEST_ASSERT(szOk.ok, "directorySize valid returns ok=true");
    auto szBad = directorySize(root / "no_such_dir");
    TEST_ASSERT(!szBad.ok, "directorySize nonexistent returns ok=false");

    // 11. readTextPreview contract
    auto prevOk = readTextPreview(fPath);
    TEST_ASSERT(prevOk.ok, "readTextPreview valid text file returns ok=true");
    auto prevDir = readTextPreview(root);
    TEST_ASSERT(!prevDir.ok, "readTextPreview on directory returns ok=false");

    // 12. getSpace contract
    auto spc = getSpace(root);
    TEST_ASSERT(spc.ok && spc.value.capacity > 0, "getSpace returns valid drive capacity");

    // 13. Navigator contract
    Navigator nav(root);
    TEST_ASSERT(nav.current() == root, "Navigator initialized to root");
    auto navCdBad = nav.cd("completely_invalid_subfolder_name");
    TEST_ASSERT(!navCdBad.ok, "Navigator cd invalid returns ok=false without exception");
    TEST_ASSERT(nav.current() == root, "Navigator path intact on failed cd");

    return test::reportSummary("API Contract Verification Tests");
}

