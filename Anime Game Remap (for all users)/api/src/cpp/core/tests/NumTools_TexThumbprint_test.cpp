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
// Standalone test for AGRemapCore::NumTools and the file-free half of
// AGRemapCore::TexThumbprint -- two modules lifted out of WWMIFixer, neither bound to Python.
//
// WHY THIS FILE EXISTS (2026-09-25). NumTools::formatDouble writes a number INTO a .ini file, so
// its trimming is observable in shipped output. TexThumbprint::identify is the rule that decides
// whether a mod's texture IS one of the game's, and the half of it worth pinning is the runner-up
// gate: "the best of these" is a far weaker claim than "this one and nothing else", and the two
// look identical on any example where the answer is easy.
//
// TexThumbprint::of is NOT covered here -- it needs a real .dds, and no image fixture is committed
// (TextureFile_Bc7Decode_test takes its file as an argument for the same reason). What of() computes
// is instead kept in step with Tools/Misc/Diagnostics/wwmiTextureThumbs.py, which generates the
// tables the fixers carry; if that ever drifts, every thumbprint table drifts with it and the
// in-game result says so loudly.
//
// NOTE: nothing builds core/tests/*.cpp. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      NumTools_TexThumbprint_test.cpp ^
//      /link <repo>/cbuild/src/cpp/core/AGRemapCore.lib <repo>/cbuild/utf8proc/utf8proc.lib
// -----------------------------------------------------------------------------

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "AGRemapCore/model/textures/TexThumbprint.h"
#include "AGRemapCore/tools/NumTools.h"

namespace AGRC = AGRemapCore;

namespace {
    int failures = 0;

    void check(bool condition, const char* description) {
        if (condition) {
            std::printf("[PASS] %s\n", description);
        } else {
            std::printf("[FAIL] %s\n", description);
            ++failures;
        }
    }

    void checkStr(const std::string& actual, const std::string& expected, const char* description) {
        if (actual == expected) {
            std::printf("[PASS] %s\n", description);
        } else {
            std::printf("[FAIL] %s\n       got '%s', wanted '%s'\n", description, actual.c_str(), expected.c_str());
            ++failures;
        }
    }


    // ---- 1. formatDouble: the shortest text that still says the number ----
    void testFormatDouble() {
        std::printf("\ntestFormatDouble\n");

        checkStr(AGRC::NumTools::formatDouble(1.5), "1.5", "a value that needs one place keeps one");
        checkStr(AGRC::NumTools::formatDouble(2.0), "2", "a whole number loses its point entirely");
        checkStr(AGRC::NumTools::formatDouble(0.1 + 0.2), "0.3", "and a floating-point accident is rounded away");
        checkStr(AGRC::NumTools::formatDouble(0.0), "0", "zero is '0', not '0.0000'");
        checkStr(AGRC::NumTools::formatDouble(-1.25), "-1.25", "a negative keeps its sign");

        // The trailing-zero strip must stop at the point, not eat into the integer part
        checkStr(AGRC::NumTools::formatDouble(100.0), "100", "100 does not become '1'");
        checkStr(AGRC::NumTools::formatDouble(1000.5), "1000.5", "nor does 1000.5 lose its zeros");

        checkStr(AGRC::NumTools::formatDouble(1.23456789), "1.2346", "four places by default, rounded");
        checkStr(AGRC::NumTools::formatDouble(1.23456789, 2), "1.23", "and 'decimals' says how many");
        checkStr(AGRC::NumTools::formatDouble(1.9, 0), "2", "zero places rounds to a whole number");
        checkStr(AGRC::NumTools::formatDouble(2000.0, 0), "2000", "and still does not eat the zeros");
    }


    // ---- 2. correlation: same picture, any exposure ----
    void testCorrelation() {
        std::printf("\ntestCorrelation\n");

        const std::vector<std::uint8_t> stored = {0, 64, 128, 255};

        std::vector<double> same = {0.0, 64.0, 128.0, 255.0};
        check(std::fabs(AGRC::TexThumbprint::correlation(same, stored) - 1.0) < 1e-9,
              "an identical thumbprint correlates 1");

        // Mean-centred, so a brightness or contrast change is still the same picture
        std::vector<double> brighter = {10.0, 74.0, 138.0, 265.0};
        check(std::fabs(AGRC::TexThumbprint::correlation(brighter, stored) - 1.0) < 1e-9,
              "so does the same picture shifted brighter");
        std::vector<double> stretched = {0.0, 128.0, 256.0, 510.0};
        check(std::fabs(AGRC::TexThumbprint::correlation(stretched, stored) - 1.0) < 1e-9,
              "and the same picture with its contrast stretched");

        std::vector<double> inverted = {255.0, 128.0, 64.0, 0.0};
        check(AGRC::TexThumbprint::correlation(inverted, stored) < -0.9, "an inverted one correlates negatively");

        std::vector<double> flat = {50.0, 50.0, 50.0, 50.0};
        check(AGRC::TexThumbprint::correlation(flat, stored) == 0.0,
              "a flat image correlates with nothing -- no divide by zero");
        check(AGRC::TexThumbprint::correlation({1.0, 2.0}, stored) == 0.0, "different lengths correlate 0");
        check(AGRC::TexThumbprint::correlation({}, {}) == 0.0, "and so does nothing at all");
    }


    // ---- 3. identify: the runner-up gate is the whole point ----
    void testIdentify() {
        std::printf("\ntestIdentify\n");

        AGRC::TexThumbprint::Table table;
        table["diffuse"] = {0, 64, 128, 255};
        table["lightmap"] = {255, 0, 255, 0};

        std::vector<double> diffuse = {0.0, 64.0, 128.0, 255.0};
        check(AGRC::TexThumbprint::identify(diffuse, table, 0.97, 0.90).value_or("") == "diffuse",
              "a clean match is named");

        // Two near-copies: the best still wins on score, and must NOT be returned
        AGRC::TexThumbprint::Table twins;
        twins["a"] = {0, 64, 128, 255};
        twins["b"] = {0, 65, 127, 255};
        check(!AGRC::TexThumbprint::identify(diffuse, twins, 0.97, 0.90).has_value(),
              "two near-copies answer NOTHING, though one of them scores best");

        // and the same pair with the gate opened does answer
        check(AGRC::TexThumbprint::identify(diffuse, twins, 0.97, 1.01).value_or("") == "a",
              "which is the gate doing it, not the score");

        std::vector<double> unrelated = {7.0, 3.0, 9.0, 1.0};
        check(!AGRC::TexThumbprint::identify(unrelated, table, 0.97, 0.90).has_value(),
              "something in neither table entry is not identified");
        check(!AGRC::TexThumbprint::identify(diffuse, {}, 0.97, 0.90).has_value(),
              "an empty table identifies nothing");
        check(!AGRC::TexThumbprint::identifyFile("no/such/file.dds", table, 16, 0.97, 0.90).has_value(),
              "and a file that will not read is not identified either");
    }
}


int main() {
    testFormatDouble();
    testCorrelation();
    testIdentify();

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "ALL PASSED" : "FAILED", failures);
    return failures == 0 ? 0 : 1;
}
