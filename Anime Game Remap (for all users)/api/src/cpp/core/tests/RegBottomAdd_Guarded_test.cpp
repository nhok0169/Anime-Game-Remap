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
// Standalone test for where a part added at the BOTTOM of a section lands when the section is a
// namespace-merged mod's guarded one -- RegFillMissing::guardEndIndex, RegBottomAdd, and
// RegFillMissing's BottomCover mode.
//
// namespace_merge.py writes every key of an outfit's section under one `if $\<Char>\Master\swapvar == n`.
// The section's own depth is then OUTSIDE the guard, and a draw added there runs for every outfit at once:
// Lumine12's 18 outfits all drew their merged members on Lumine together (2026-10-20). Pinned here: the
// added part goes before the guard's `endif` (both edits, with and without RegBottomAdd's condition), and a
// collect's `if 1 ... endif` -- the same shape, guarding nothing -- is NOT taken for a guard.
//
// NOTE: nothing builds core/tests/*.cpp. Compile as RegDelimitedAdd_PerPath_test.cpp's header says,
// with this file's name.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/RegFillMissingMode.h"
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBottomAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegFillMissing.h"

namespace AGRC = AGRemapCore;

namespace {
    int failures = 0;

    void check(bool condition, const std::string& description) {
        std::printf("[%s] %s\n", condition ? "PASS" : "FAIL", description.c_str());
        if (!condition) {
            ++failures;
        }
    }

    using Section = AGRC::IfTemplate<std::string, std::string>;
    using Graph = AGRC::IniSectionGraph<std::string, std::string>;
    using KVPs = tsl::ordered_map<std::string, std::vector<std::pair<long long, std::string>>>;
    using RawPart = std::pair<int, std::variant<std::string, KVPs>>;
    using ContentPart = AGRC::IfContentPart<std::string, std::string>;

    AGRC::IfTemplateRunConfig<std::string, std::string> runConfig() {
        return AGRC::IfTemplateRunConfig<std::string, std::string>{
            AGRC::IniKeywords::Run, [](const std::string& val) { return val; }, [](const std::string& name) { return name; }};
    }

    KVPs kvps(const std::vector<std::pair<std::string, std::string>>& entries) {
        KVPs result;
        long long index = 0;
        for (const auto& entry : entries) {
            result[entry.first].emplace_back(index++, entry.second);
        }
        return result;
    }

    // A namespace-merged outfit's body section, or the same shape under a collect's `if 1`.
    std::unique_ptr<Section> guarded(const std::string& condition) {
        std::vector<RawPart> raw{
            {0, kvps({{"hash", "e7612ed8"}, {"match_priority", "5"}})},
            {1, condition},
            {2, kvps({{"match_first_index", "6915"}, {"ps-t0", "BodyDiffuse"}, {AGRC::IniKeywords::DrawIndexed, "65013, 0, 0"}})},
            {3, std::string("endif")}};
        return Section::build(raw, runConfig(), "Body");
    }

    // Where the content part holding 'key' sits: its index in the parts, and how many `if`s are open there.
    std::pair<long long, int> placeOf(const Section& section, const std::string& key, const std::string& val) {
        int depth = 0;
        const auto& parts = section.parts();
        for (std::size_t i = 0; i < parts.size(); ++i) {
            if (const auto* pred = dynamic_cast<const AGRC::IfPredPart*>(parts[i].get())) {
                if (pred->type == AGRC::IfPredPartType::If) {
                    ++depth;
                } else if (pred->type == AGRC::IfPredPartType::EndIf) {
                    --depth;
                }
                continue;
            }
            const auto* content = dynamic_cast<const ContentPart*>(parts[i].get());
            if (content == nullptr) {
                continue;
            }
            for (const std::string& v : content->getVals(key)) {
                if (v == val) {
                    return {static_cast<long long>(i), depth};
                }
            }
        }
        return {-1, -1};
    }

    void bottomAdd(Section& section, const std::string& condition) {
        std::unordered_map<std::string, Section*> sections{{section.name, &section}};
        Graph graph(sections, {section.name}, runConfig());
        AGRC::RegBottomAdd<> edit({{"ps-t0", "HeadDiffuse"}, {AGRC::IniKeywords::DrawIndexed, "1620, 65013, 0"}}, condition);
        edit.edit(graph, nullptr, "", {}, false, std::nullopt);
    }

    void bottomCover(Section& section) {
        std::unordered_map<std::string, Section*> sections{{section.name, &section}};
        Graph graph(sections, {section.name}, runConfig());
        AGRC::RegFillMissing<> fill("vb0", AGRC::RegFillMissing<>::makeFillMissing("vb0", "ResourceFill"),
                                    AGRC::RegFillMissingMode::BottomCover);
        fill.edit(graph, nullptr, "", {}, false, std::nullopt);
    }

    void testGuardEndIndex() {
        std::printf("\ntestGuardEndIndex\n");
        auto merged = guarded("if $\\Lumine\\Master\\swapvar==5");
        auto collect = guarded("if 1");
        check(AGRC::RegFillMissing<>::guardEndIndex(*merged) == std::optional<std::size_t>(3),
              "a namespace-merged section's guard ends at its closing endif (part 3)");
        check(!AGRC::RegFillMissing<>::guardEndIndex(*collect).has_value(), "a collect's `if 1` guards nothing");
    }

    void testRegBottomAdd() {
        std::printf("\ntestRegBottomAdd\n");
        auto merged = guarded("if $\\Lumine\\Master\\swapvar==5");
        bottomAdd(*merged, "");
        const auto place = placeOf(*merged, AGRC::IniKeywords::DrawIndexed, "1620, 65013, 0");
        check(place.second == 1, "the member's draw lands INSIDE the outfit's guard");
        check(place.first == static_cast<long long>(merged->parts().size()) - 2, "...as the guard's last part, before its endif");

        auto conditioned = guarded("if $\\Lumine\\Master\\swapvar==5");
        bottomAdd(*conditioned, "vs != 037730.0");
        check(placeOf(*conditioned, AGRC::IniKeywords::DrawIndexed, "1620, 65013, 0").second == 2,
              "a conditioned block nests inside the guard too");

        auto collect = guarded("if 1");
        bottomAdd(*collect, "");
        check(placeOf(*collect, AGRC::IniKeywords::DrawIndexed, "1620, 65013, 0").second == 0,
              "after a collect's `if 1` the block stays at the section's own depth");
    }

    void testBottomCover() {
        std::printf("\ntestBottomCover\n");
        auto merged = guarded("if $\\Lumine\\Master\\swapvar==5");
        bottomCover(*merged);
        check(placeOf(*merged, "vb0", "ResourceFill").second == 1, "BottomCover's fill lands inside the guard");

        auto collect = guarded("if 1");
        bottomCover(*collect);
        check(placeOf(*collect, "vb0", "ResourceFill").second == 0, "and at the section's depth after an `if 1`");
    }
}

int main() {
    std::printf("===== a part added at the bottom of a guarded section =====\n");
    testGuardEndIndex();
    testRegBottomAdd();
    testBottomCover();
    std::printf("\n");
    if (failures == 0) {
        std::printf("ALL PASSED\n");
        return 0;
    }
    std::printf("%d FAILURE(S)\n", failures);
    return 1;
}
