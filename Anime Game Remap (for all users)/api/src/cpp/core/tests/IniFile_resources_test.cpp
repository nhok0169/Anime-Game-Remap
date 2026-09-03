// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniFile's resource-model accessors
// (model/files/IniFile.h) -- specifically the two members the pure-Python
// IniFile exposed that the C++ one did not, added so RemapService can read a
// mod's referenced folders and drop its models without going through the
// deprecated Python class:
//   * clearModels(): empties getResources() and getFileDownloads(), and leaves
//     the text read in from disk alone (unlike clear()/clearRead())
//   * getReferencedFolders(): the parent folder of every resource's srcPath,
//     across BOTH getResources() and getFileDownloads(), deduplicated and in
//     first-seen order -- mirroring the pure-Python original's OrderedSet
//   * getReferencedFolders() only ever looks at a resource's *source* side: an
//     IniFixResource's fixedPath folder is deliberately NOT reported, matching
//     what the pure-Python original walked ('origFullPath'/'fullPath' only)
//   * clear() still empties the models, now by routing through clearModels()
//
// This file DOES need the full static lib -- IniFile's destructor alone reaches
// Z3Context. Build AGRemapCore first ("cd cbuild && ninja AGRemapCore"), then:
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      IniFile_resources_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib ^
//      /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// (copy libz3.dll next to test.exe before running -- see the note in
// IniFile_classify_test.cpp for why the header recipes elsewhere don't link).
// -----------------------------------------------------------------------------

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/tools/files/FileService.h"

namespace AGRC = AGRemapCore;

static int failures = 0;


static void check(bool condition, const std::string& what) {
    if (condition) {
        return;
    }

    std::printf("  FAILED: %s\n", what.c_str());
    ++failures;
}


static void checkEqual(const std::string& got, const std::string& expected, const std::string& what) {
    if (got == expected) {
        return;
    }

    std::printf("  FAILED: %s\n    expected: '%s'\n    got:      '%s'\n",
                what.c_str(), expected.c_str(), got.c_str());
    ++failures;
}


static void checkEqual(std::size_t got, std::size_t expected, const std::string& what) {
    if (got == expected) {
        return;
    }

    std::printf("  FAILED: %s\n    expected: %zu\n    got:      %zu\n", what.c_str(), expected, got);
    ++failures;
}


// The folder every resource in these tests is resolved against. IniResource's constructor makes
// srcPath absolute against it, so the expectations below are built the same way rather than
// hardcoded -- keeping this test independent of which drive the repo sits on.
static std::string iniFolder() {
    return std::filesystem::absolute("resourceTestFolder").string();
}


static std::string folderOf(const std::string& relPath) {
    return std::filesystem::path(AGRC::FileService::absPathOfRelPath(relPath, iniFolder())).parent_path().string();
}


static void seedResources(AGRC::IniFile& ini) {
    const std::string folder = iniFolder();

    // Two resources under the same folder -- the second must NOT produce a second entry.
    ini.getResources().push_back(std::make_unique<AGRC::IniResource>("blend", folder, "blends/one.buf"));
    ini.getResources().push_back(std::make_unique<AGRC::IniResource>("blend", folder, "blends/two.buf"));

    // A fix resource whose fixed side lives somewhere the source side does not: only the source
    // folder may be reported.
    ini.getResources().push_back(std::make_unique<AGRC::IniFixResource>("position", folder, "positions/orig.buf",
                                                                       "fixedOnly/remap.buf"));

    // Downloads are part of the same walk in the pure-Python original.
    ini.getFileDownloads().push_back(std::make_unique<AGRC::IniResource>("download", folder, "downloads/tex.dds"));
}


static void testGetReferencedFoldersDedupesInOrder() {
    std::printf("testGetReferencedFoldersDedupesInOrder\n");

    AGRC::IniFile ini(std::nullopt, "[TextureOverrideBody]\n");
    seedResources(ini);

    std::vector<std::string> folders = ini.getReferencedFolders();

    checkEqual(folders.size(), static_cast<std::size_t>(3), "three distinct folders, not four resources");
    checkEqual(folders[0], folderOf("blends/one.buf"), "first folder is the blends one");
    checkEqual(folders[1], folderOf("positions/orig.buf"), "second folder is the positions one");
    checkEqual(folders[2], folderOf("downloads/tex.dds"), "downloads are walked too, and come last");
}


static void testGetReferencedFoldersIgnoresFixedPath() {
    std::printf("testGetReferencedFoldersIgnoresFixedPath\n");

    AGRC::IniFile ini(std::nullopt, "[TextureOverrideBody]\n");
    seedResources(ini);

    const std::string fixedFolder = folderOf("fixedOnly/remap.buf");
    for (const std::string& folder : ini.getReferencedFolders()) {
        check(folder != fixedFolder, "an IniFixResource's fixedPath folder is not reported");
    }
}


static void testGetReferencedFoldersEmptyWithNoResources() {
    std::printf("testGetReferencedFoldersEmptyWithNoResources\n");

    AGRC::IniFile ini(std::nullopt, "[TextureOverrideBody]\n");
    check(ini.getReferencedFolders().empty(), "no resources -> no folders");
}


static void testClearModelsKeepsReadText() {
    std::printf("testClearModelsKeepsReadText\n");

    const std::string txt = "[TextureOverrideBody]\nhash = abc123\n";
    AGRC::IniFile ini(std::nullopt, txt);
    seedResources(ini);

    check(!ini.getResources().empty(), "resources are seeded before the call");
    check(!ini.getFileDownloads().empty(), "downloads are seeded before the call");

    ini.clearModels();

    check(ini.getResources().empty(), "clearModels empties getResources");
    check(ini.getFileDownloads().empty(), "clearModels empties getFileDownloads");
    check(ini.getReferencedFolders().empty(), "and so getReferencedFolders reports nothing");

    // The whole point of clearModels over clear/clearRead: the text survives.
    checkEqual(ini.getFileTxt(), txt, "clearModels leaves the read text alone");
    check(ini.fileLinesRead(), "clearModels leaves fileLinesRead alone");
    checkEqual(ini.getFileLines().size(), static_cast<std::size_t>(2), "clearModels leaves the read lines alone");
}


static void testClearAlsoEmptiesModels() {
    std::printf("testClearAlsoEmptiesModels\n");

    AGRC::IniFile ini(std::nullopt, "[TextureOverrideBody]\nhash = abc123\n");
    seedResources(ini);

    ini.clear();

    check(ini.getResources().empty(), "clear still empties getResources");
    check(ini.getFileDownloads().empty(), "clear still empties getFileDownloads");
}


int main() {
    testGetReferencedFoldersDedupesInOrder();
    testGetReferencedFoldersIgnoresFixedPath();
    testGetReferencedFoldersEmptyWithNoResources();
    testClearModelsKeepsReadText();
    testClearAlsoEmptiesModels();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
