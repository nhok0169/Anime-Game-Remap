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
// Standalone test for what IniFile and IniScan count as a SECTION HEADER.
//
// WHY THIS FILE EXISTS (2026-10-20). 3DMigoto opens a section on any line that starts with '[',
// closed or not, and names it up to the first ']' (or the rest of the line). IniFile asked for a
// ']' too, so a mod whose author pasted a line into the middle of a header --
//
//     [Textrun = CommandList\global\ORFix\ORFix
//     ureOverrideLumineSkinBangsIB]
//
// (Lumine11's Nod Krai outfit) -- had both halves read as KEYS of the section before it, which then
// carried the next section's hash, skip and draw. The remap of that outfit drew its body through the
// wrong section with the normal map where the diffuse goes. Every case below fails against the old
// rule.
//
// NOTE: nothing builds core/tests/*.cpp. Compile as IniScan_test.cpp's header says, plus Z3's
// include directory and import library (IniFile parses conditions with it).
// -----------------------------------------------------------------------------

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/files/IniScan.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"

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

    // How many times 'key' is set anywhere in the section's parts
    size_t countKey(const AGRC::IfTemplate<std::string, std::string>& section, const std::string& key) {
        size_t count = 0;
        for (const auto& part : section.parts()) {
            auto* content = dynamic_cast<const AGRC::IfContentPart<std::string, std::string>*>(part.get());
            if (content != nullptr) {
                count += content->getVals(key).size();
            }
        }
        return count;
    }

    const std::string MalformedIni =
        "[TextureOverrideBody]\r\n"
        "hash = 9634fc3d\r\n"
        "match_first_index = 57141\r\n"
        "run = CommandList\\global\\ORFix\\ORFix\r\n"
        "\r\n"
        "[Textrun = CommandList\\global\\ORFix\\ORFix\r\n"
        "ureOverrideBangsIB]\r\n"
        "hash = e4e2950f\r\n"
        "handling = skip\r\n"
        "drawindexed = auto\r\n"
        "\r\n"
        "[TextureOverrideBangsA]  ; trailing text after the bracket\r\n"
        "hash = e4e2950f\r\n";


    // ---- 1. the two static rules ----
    void testStaticRules() {
        std::printf("\ntestStaticRules\n");

        check(AGRC::IniFile::isSectionHeaderLine("[TextureOverrideX]"), "a closed header is a header");
        check(AGRC::IniFile::isSectionHeaderLine("   [TextureOverrideX]"), "leading whitespace is skipped");
        check(AGRC::IniFile::isSectionHeaderLine("[Textrun = CommandList\\global\\ORFix\\ORFix"),
              "a line starting with '[' and no ']' is a header, as in 3DMigoto");
        check(!AGRC::IniFile::isSectionHeaderLine("ureOverrideBangsIB]"), "a line not starting with '[' is not");
        check(!AGRC::IniFile::isSectionHeaderLine("hash = [abc]"), "nor is a key whose value has brackets");

        check(AGRC::IniFile::getSectionNameFromLine("[TextureOverrideX]") == "TextureOverrideX", "the plain name");
        check(AGRC::IniFile::getSectionNameFromLine("[Textrun = CommandList\\global\\ORFix\\ORFix") ==
                  "Textrun = CommandList\\global\\ORFix\\ORFix",
              "no ']': the rest of the line");
        check(AGRC::IniFile::getSectionNameFromLine("[A] ; then [B]") == "A", "up to the FIRST ']'");
    }


    // ---- 2. what IniFile files under each section ----
    void testIniFileSplitsOnMalformedHeader() {
        std::printf("\ntestIniFileSplitsOnMalformedHeader\n");

        AGRC::IniFile ini(std::nullopt, MalformedIni);
        const auto& sections = ini.getIfTemplates();

        check(sections.size() == 3, "three sections");
        auto body = sections.find("TextureOverrideBody");
        check(body != sections.end(), "the Body section is there");
        if (body != sections.end()) {
            check(countKey(*body->second, "hash") == 1, "Body keeps ONE hash, not the next section's too");
            check(countKey(*body->second, "handling") == 0, "nor its skip");
            check(countKey(*body->second, "drawindexed") == 0, "nor its draw");
        }

        check(sections.find("Textrun = CommandList\\global\\ORFix\\ORFix") != sections.end(),
              "the malformed header is its own section, under 3DMigoto's name for it");
        check(sections.find("TextureOverrideBangsA") != sections.end(), "trailing text after ']' is dropped");
    }


    // ---- 3. IniScan agrees ----
    void testIniScanAgrees() {
        std::printf("\ntestIniScanAgrees\n");

        const std::string path = (std::filesystem::temp_directory_path() / "AGRemapSectionHeaderTest.ini").string();
        {
            std::ofstream out(path, std::ios::binary);
            out << MalformedIni;
        }

        std::vector<AGRC::IniScanSection> sections = AGRC::IniScan::scan(path);
        std::error_code ignored;
        std::filesystem::remove(path, ignored);

        check(sections.size() == 3, "three sections");
        if (sections.size() == 3) {
            check(sections[0].name == "TextureOverrideBody", "Body first");
            check(AGRC::IniScan::firstVal(sections[0], "handling") == std::nullopt, "Body has no skip");
            check(sections[1].name == "Textrun = CommandList\\global\\ORFix\\ORFix", "the malformed header");
            check(sections[2].name == "TextureOverrideBangsA", "and the next, trailing text dropped");
        }
    }
}


int main() {
    testStaticRules();
    testIniFileSplitsOnMalformedHeader();
    testIniScanAgrees();

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "ALL PASSED" : "FAILED", failures);
    return failures == 0 ? 0 : 1;
}
