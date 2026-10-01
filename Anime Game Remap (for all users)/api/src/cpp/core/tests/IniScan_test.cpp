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
// Standalone test for AGRemapCore::IniScan -- the flat .ini line reader.
//
// WHY THIS FILE EXISTS (2026-09-25). IniScan was file-local to WWMIFixer, where it read the
// `filename =` / `hash =` / `this =` lines out of .ini files the API never parses (a mod's
// namespaced companion file, a LOD folder's own file). It has no binding, so the Python suite
// cannot reach it.
//
// What is pinned here is as much what it does NOT do as what it does: it is deliberately not a
// parser, and the temptation to grow it into one is the reason its limits are assertions rather
// than prose. A caller that needs `if` nesting, conditions, `run =` following or a section declared
// twice wants IniFile, and this test failing because somebody taught IniScan one of those is the
// intended outcome.
//
// NOTE: nothing builds core/tests/*.cpp. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      IniScan_test.cpp ^
//      /link <repo>/cbuild/src/cpp/core/AGRemapCore.lib <repo>/cbuild/utf8proc/utf8proc.lib
// -----------------------------------------------------------------------------

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "AGRemapCore/model/files/IniScan.h"

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


    // A real temporary file: IniScan reads from disk, and stubbing that would test nothing
    class TempIni {
        public:
            explicit TempIni(const std::string& body) {
                path_ = (std::filesystem::temp_directory_path() / "AGRemapIniScanTest.ini").string();
                std::ofstream out(path_, std::ios::binary);
                out << body;
            }

            ~TempIni() {
                std::error_code ignored;
                std::filesystem::remove(path_, ignored);
            }

            const std::string& path() const {
                return path_;
            }

        private:
            std::string path_;
    };


    // ---- 1. what it reads ----
    void testReadsSectionsAndKvps() {
        std::printf("\ntestReadsSectionsAndKvps\n");

        TempIni ini(
            "; a comment before anything\r\n"
            "stray = ignored, there is no section yet\r\n"
            "\r\n"
            "[ResourceChisaBodyDiffuse]\r\n"
            "  filename = .\\Textures\\Body_D.dds   \r\n"
            "; a comment inside\r\n"
            "\r\n"
            "[TextureOverrideComponent0]\r\n"
            "hash = e611d493\r\n"
            "this = ResourceChisaBodyDiffuse\r\n");

        std::vector<AGRC::IniScanSection> sections = AGRC::IniScan::scan(ini.path());
        check(sections.size() == 2, "two sections, and the line before the first is dropped");
        if (sections.size() != 2) {
            return;
        }

        check(sections[0].name == "ResourceChisaBodyDiffuse", "a header comes back without its brackets");
        check(sections[0].kvps.size() == 1, "its one KVP, the comment dropped");
        check(sections[0].kvps[0].first == "filename", "the key is stripped");
        check(sections[0].kvps[0].second == ".\\Textures\\Body_D.dds",
              "and so is the value, its backslashes left alone");

        check(sections[1].name == "TextureOverrideComponent0", "sections come back in file order");
        check(AGRC::IniScan::firstVal(sections[1], "hash").value_or("") == "e611d493", "firstVal finds a hash");
        check(AGRC::IniScan::firstVal(sections[1], "this").value_or("") == "ResourceChisaBodyDiffuse",
              "and a 'this'");
        check(!AGRC::IniScan::firstVal(sections[1], "filename").has_value(),
              "and answers nothing for a key the section lacks");
    }


    // ---- 2. a missing file is empty, not an exception ----
    void testMissingFile() {
        std::printf("\ntestMissingFile\n");
        check(AGRC::IniScan::scan("no/such/file/at/all.ini").empty(), "a file that will not open scans as empty");
    }


    // ---- 3. the limits, asserted so that growing this into a parser breaks here first ----
    void testDeliberateLimits() {
        std::printf("\ntestDeliberateLimits\n");

        TempIni ini(
            "[Constants]\r\n"
            "global $mesh_vertex_count = 1\r\n"
            "\r\n"
            "[TextureOverrideComponent0]\r\n"
            "if $mod_enabled\r\n"
            "    local $state\r\n"
            "    ps-t0 = ResourceA\r\n"
            "endif\r\n"
            "\r\n"
            "[Constants]\r\n"
            "global $second = 2\r\n");

        std::vector<AGRC::IniScanSection> sections = AGRC::IniScan::scan(ini.path());
        check(sections.size() == 3, "a section declared twice comes back TWICE -- nothing is concatenated");

        const AGRC::IniScanSection* body = nullptr;
        for (const auto& section : sections) {
            if (section.name == "TextureOverrideComponent0") {
                body = &section;
            }
        }

        check(body != nullptr, "the branching section is there");
        if (body == nullptr) {
            return;
        }

        // `if`, `endif` and `local $state` all have no '=', so all three are skipped
        check(body->kvps.size() == 1, "only the one KVP: if/endif/local are not KVPs and are dropped");
        check(body->kvps[0].first == "ps-t0", "and nesting is ignored rather than tracked");

        // What a caller must NOT conclude from the above
        check(AGRC::IniScan::firstVal(sections[0], "global $mesh_vertex_count").value_or("") == "1",
              "a 'global $x = 1' line reads as the key 'global $x'");
    }
}


int main() {
    testReadsSectionsAndKvps();
    testMissingFile();
    testDeliberateLimits();

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "ALL PASSED" : "FAILED", failures);
    return failures == 0 ? 0 : 1;
}
