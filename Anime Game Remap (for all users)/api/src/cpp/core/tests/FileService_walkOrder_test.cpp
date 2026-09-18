// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

// -----------------------------------------------------------------------------
// Standalone regression test for the ORDER FileService::getFilesAndDirs hands its
// results back in -- tools/files/FileService.h.
//
// RemapService walks mod folders with getFilesAndDirs, and the order it visits folders
// and .ini files in reaches the output: which .ini of a folder is fixed first decides
// the names its fix generates, and which mod is visited last decides what the summary
// log opens with. std::filesystem promises no order at all. On NTFS the OS happens to
// return names sorted, so every Windows run -- and every golden produced from a
// checkout on /mnt/e -- saw one order, while an ext4 runner (GitHub Actions) sees the
// directory's hash order: the Integration Tester failed 8 tests there on 2026-09-18.
//
// The rule is Windows' order, so that fixing it changed nothing a Windows user sees:
//   * names are compared with ASCII letters folded to upper case, byte by byte --
//     which is how NTFS collates them. So 'a' < 'B', and 'ab' < 'a_b', because
//     'B' (0x42) sorts before '_' (0x5F)
//   * a recursive walk is PRE-ORDER, siblings in that order: a folder, then
//     everything under it, then its next sibling. Sorting the whole path as one
//     string is NOT the same thing: 'A-B' would come before 'A/X', because '-' (0x2D)
//     sorts before '/' (0x2F)
//
// The expected lists below are written out by hand, not computed with the rule under
// test. NOTE: on NTFS this test cannot fail against an unsorted build, because NTFS
// sorts for it -- run it on Linux (ext4) to see it catch anything. Twelve files in one
// folder arrive in hash order on ext4; the chance they arrive sorted is 1 in 12!.
//
// Needs only libAGRemapCore (+ utf8proc). On Linux, via Tools/Misc/Linux/buildTests.sh:
//   bash buildTests.sh FileService_walkOrder_test
// -----------------------------------------------------------------------------

#include "AGRemapCore/tools/files/FileService.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using AGRemapCore::FileService;
namespace fs = std::filesystem;

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "  PASS  " : "  FAIL  ") << what << "\n";
    if (!ok) {
        ++failures;
    }
}

// the part of each path below 'root', with '/' as the separator on every OS
static std::vector<std::string> relative(const std::vector<std::string>& paths, const fs::path& root) {
    std::vector<std::string> result;
    for (const std::string& path : paths) {
        result.push_back(FileService::strToPath(path).lexically_relative(root).generic_string());
    }
    return result;
}

static std::string join(const std::vector<std::string>& items) {
    std::string result;
    for (const std::string& item : items) {
        result += (result.empty() ? "" : ", ") + item;
    }
    return result;
}

static void touch(const fs::path& path) {
    std::ofstream(path) << "x";
}

int main() {
    const fs::path root = fs::temp_directory_path() / ("agremap_walkorder_" + std::to_string(std::random_device{}()));
    fs::remove_all(root);
    fs::create_directories(root);

    // ===== one folder's files =====
    const fs::path flat = root / "flat";
    fs::create_directory(flat);

    const std::vector<std::string> expectedFiles = {
        "0.ini", "9.ini", "a.ini", "ab.ini", "a_b.ini", "B.ini", "c.ini", "D.ini",
        "griffith.ini", "Jean.ini", "merged.ini", "Z.ini"
    };

    // created in a shuffled order, so nothing depends on a filesystem echoing creation order
    std::vector<std::string> created = expectedFiles;
    std::shuffle(created.begin(), created.end(), std::mt19937(169));
    for (const std::string& name : created) {
        touch(flat / name);
    }

    std::cout << "files of one folder:\n";
    const std::vector<std::string> files = relative(FileService::getFilesAndDirs(FileService::pathToStr(flat)).first, flat);
    check(files == expectedFiles, "in Windows order\n        got      " + join(files) + "\n        expected " + join(expectedFiles));

    // the same call twice must agree -- trivially true of a sort, and the thing a caller relies on
    const std::vector<std::string> again = relative(FileService::getFilesAndDirs(FileService::pathToStr(flat)).first, flat);
    check(files == again, "the same on a second call");

    // ===== a recursive walk's folders =====
    const fs::path tree = root / "tree";
    for (const char* dir : {"B", "a_b", "A-B", "ab", "A/X", "A/X/deep", "A/W", "C"}) {
        fs::create_directories(tree / dir);
    }

    // pre-order: 'A' and everything under it before 'A-B', even though "A-B" < "A/X" as strings
    const std::vector<std::string> expectedDirs = {
        "A", "A/W", "A/X", "A/X/deep", "A-B", "ab", "a_b", "B", "C"
    };

    std::cout << "folders of a recursive walk:\n";
    const std::vector<std::string> dirs = relative(FileService::getFilesAndDirs(FileService::pathToStr(tree), true).second, tree);
    check(dirs == expectedDirs, "pre-order, siblings in Windows order\n        got      " + join(dirs) + "\n        expected " + join(expectedDirs));

    // ===== a recursive walk's files come out in the same shape =====
    touch(tree / "A" / "X" / "x.ini");
    touch(tree / "A" / "a.ini");
    touch(tree / "A-B" / "b.ini");
    touch(tree / "root.ini");

    const std::vector<std::string> expectedTreeFiles = {"A/a.ini", "A/X/x.ini", "A-B/b.ini", "root.ini"};
    const std::vector<std::string> treeFiles = relative(FileService::getFilesAndDirs(FileService::pathToStr(tree), true).first, tree);
    check(treeFiles == expectedTreeFiles, "files of a recursive walk, pre-order\n        got      " + join(treeFiles) + "\n        expected " + join(expectedTreeFiles));

    fs::remove_all(root);

    std::cout << (failures == 0 ? "ALL PASSED" : std::to_string(failures) + " FAILED") << "\n";
    return failures == 0 ? 0 : 1;
}
