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
// Standalone test for FileService's path READS -- baseName, parentOf, stem and pathKey.
//
// WHY THIS FILE EXISTS (2026-09-25). The first three are the pathToStr(strToPath(x).filename())
// idiom, which was written out by hand in over forty places across core and file-locally in
// WWMIFixer. They have no pybind11 binding, so the Python suite cannot reach them.
//
// The case that matters is a WINDOWS path read on either OS: a path inside a .ini is separated with
// a backslash whatever produced the file (see FileService::pathToIniStr), so these have to split on
// one even on POSIX -- which std::filesystem::path does NOT do there. That asymmetry is recorded
// rather than asserted away: the POSIX expectations below are what std::filesystem actually gives,
// so this file passes on both operating systems and says plainly which is which.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only when
// somebody compiles it. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      FileService_pathParts_test.cpp ^
//      /link <repo>/cbuild/src/cpp/core/AGRemapCore.lib <repo>/cbuild/utf8proc/utf8proc.lib
// -----------------------------------------------------------------------------

#include <cstdio>
#include <string>

#include "AGRemapCore/tools/files/FileService.h"

namespace AGRC = AGRemapCore;

namespace {
    int failures = 0;

    void check(const std::string& actual, const std::string& expected, const char* description) {
        if (actual == expected) {
            std::printf("[PASS] %s\n", description);
        } else {
            std::printf("[FAIL] %s\n       got '%s', wanted '%s'\n", description, actual.c_str(), expected.c_str());
            ++failures;
        }
    }


#ifdef _WIN32
    const bool WindowsSeparators = true;
#else
    const bool WindowsSeparators = false;
#endif


    // ---- 1. the three splits, on the separator this OS understands ----
    void testSplits() {
        std::printf("\ntestSplits\n");

        // a forward slash is a separator everywhere
        check(AGRC::FileService::baseName("Mods/Chisa1/mod.ini"), "mod.ini", "baseName takes the last component");
        check(AGRC::FileService::parentOf("Mods/Chisa1/mod.ini"), "Mods/Chisa1", "parentOf takes everything before it");
        check(AGRC::FileService::stem("Mods/Chisa1/mod.ini"), "mod", "stem drops the extension");

        check(AGRC::FileService::baseName("mod.ini"), "mod.ini", "a bare name is its own baseName");
        check(AGRC::FileService::parentOf("mod.ini"), "", "and has no parent");
        check(AGRC::FileService::stem("Blend.buf"), "Blend", "stem of a buffer");

        // a name with several dots keeps all but the last
        check(AGRC::FileService::stem("Meshes/Chisa.Component1.buf"), "Chisa.Component1",
              "stem drops only the LAST extension");
    }


    // ---- 2. a backslash: a separator on Windows, an ordinary character on POSIX ----
    void testWindowsSeparator() {
        std::printf("\ntestWindowsSeparator\n");

        // This is the shape that comes out of a .ini file, on both operating systems
        const std::string path = ".\\Textures\\Chisa.dds";
        check(AGRC::FileService::baseName(path), WindowsSeparators ? "Chisa.dds" : path,
              WindowsSeparators ? "baseName splits a .ini-style path on Windows"
                                : "baseName leaves a backslash alone on POSIX (std::filesystem does)");
        check(AGRC::FileService::parentOf(path), WindowsSeparators ? ".\\Textures" : "",
              WindowsSeparators ? "parentOf splits it too" : "and finds no parent on POSIX");
    }


    // ---- 3. pathKey: the comparison key, which normalizes what the splits do not ----
    void testPathKey() {
        std::printf("\ntestPathKey\n");

        check(AGRC::FileService::pathKey(".\\Textures\\Chisa.dds"), "./textures/chisa.dds",
              "pathKey lowercases and forward-slashes");
        check(AGRC::FileService::pathKey("./Textures/CHISA.DDS"), "./textures/chisa.dds",
              "so the two spellings of one file agree");

        const bool agree = AGRC::FileService::pathKey("Mods\\Chisa1\\mod.ini")
                           == AGRC::FileService::pathKey("mods/chisa1/MOD.INI");
        std::printf("%s a key is separator- and case-insensitive on EVERY OS\n", agree ? "[PASS]" : "[FAIL]");
        if (!agree) {
            ++failures;
        }

        // What it deliberately does NOT do -- see the header's warning
        const bool unresolved = AGRC::FileService::pathKey("a/../b.dds") != AGRC::FileService::pathKey("b.dds");
        std::printf("%s and does not resolve '..', so two keys differing proves nothing\n",
                    unresolved ? "[PASS]" : "[FAIL]");
        if (!unresolved) {
            ++failures;
        }
    }


    // ---- 4. non-ASCII, which is the whole reason these go through pathToStr ----
    void testNonAscii() {
        std::printf("\ntestNonAscii\n");

        // The Mona CN mod's file that took down a whole run through path.string()
        check(AGRC::FileService::baseName("Mods/MonaCN/\xe5\x91\xbd\xe4\xbb\xa4.txt"), "\xe5\x91\xbd\xe4\xbb\xa4.txt",
              "baseName round-trips a Chinese file name");
        check(AGRC::FileService::stem("Mods/MonaCN/\xe5\x91\xbd\xe4\xbb\xa4.txt"), "\xe5\x91\xbd\xe4\xbb\xa4",
              "and so does stem");

        // toLower is grapheme-aware and leaves an uncased script alone
        check(AGRC::FileService::pathKey("Mods/\xe5\x91\xbd\xe4\xbb\xa4.TXT"), "mods/\xe5\x91\xbd\xe4\xbb\xa4.txt",
              "pathKey lowercases the cased part and passes the rest through");
    }
}


int main() {
    testSplits();
    testWindowsSeparator();
    testPathKey();
    testNonAscii();

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "ALL PASSED" : "FAILED", failures);
    return failures == 0 ? 0 : 1;
}
