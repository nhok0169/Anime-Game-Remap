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
// Standalone test for AGRemapCore::RegDelimitedAddMode::PerPath -- one addition per execution
// path, at the last position preceding every delimiter on it.
//
// WHY THIS MODE EXISTS (2026-09-14). GIMI's NNFix/ORFix command lists READ the bound ps-t registers
// and write them back re-slotted, so calling one twice over the same bindings undoes it:
// CommandListReferenceNoNormal reads the diffuse out of ps-t0 and the light map out of ps-t1, and
// CommandListLDX writes the light map back to ps-t0 and the diffuse to ps-t1. The PerSegment rule
// puts one call before EVERY draw, which is right only while no path draws more than once -- and a
// section whose draws sit in independent `if` blocks issues several in a single pass. Measured over
// one real mod library, 66 sections across 14 mod folders do, and every second draw of one came out
// rendering its light map as the albedo: a model flat green in game.
//
// The four cases below are the four placement rules, and the third is the one that is not obvious:
// a section that draws ONLY inside independent `if` blocks has no position inside any block that
// serves the paths through the others, so the addition has to move up to the header content -- and
// then must NOT also land inside the blocks.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only when
// somebody compiles it. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      RegDelimitedAdd_PerPath_test.cpp ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/RegDelimitedAddMode.h"
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegDelimitedAdd.h"

namespace AGRC = AGRemapCore;

namespace {
    int failures = 0;

    void check(bool condition, const std::string& description) {
        if (condition) {
            std::printf("[PASS] %s\n", description.c_str());
        } else {
            std::printf("[FAIL] %s\n", description.c_str());
            ++failures;
        }
    }


    using Section = AGRC::IfTemplate<std::string, std::string>;
    using Graph = AGRC::IniSectionGraph<std::string, std::string>;
    using KVPs = tsl::ordered_map<std::string, std::vector<std::pair<long long, std::string>>>;
    using RawPart = std::pair<int, std::variant<std::string, KVPs>>;

    const std::string Fix = "CommandList\\global\\ORFix\\NNFix";


    AGRC::IfTemplateRunConfig<std::string, std::string> runConfig() {
        return AGRC::IfTemplateRunConfig<std::string, std::string>{
            AGRC::IniKeywords::Run,
            [](const std::string& val) { return val; },
            [](const std::string& name) { return name; }};
    }


    KVPs kvps(const std::vector<std::pair<std::string, std::string>>& entries) {
        KVPs result;
        long long index = 0;
        for (const auto& entry : entries) {
            result[entry.first].emplace_back(index, entry.second);
            ++index;
        }
        return result;
    }


    // Renders a built section back to text, so an assertion reads like the .ini file a modder would
    // look at rather than like a part index.
    std::string render(const Section& section) {
        std::string result;
        for (const auto& part : section.parts()) {
            auto* content = dynamic_cast<const AGRC::IfContentPart<std::string, std::string>*>(part.get());
            if (content == nullptr) {
                continue;
            }

            for (const auto& entry : content->items()) {
                result += entry.key + " = " + entry.value + "\n";
            }
        }
        return result;
    }


    int countFixCalls(const Section& section) {
        int count = 0;
        for (const auto& part : section.parts()) {
            auto* content = dynamic_cast<const AGRC::IfContentPart<std::string, std::string>*>(part.get());
            if (content == nullptr) {
                continue;
            }

            for (const std::string& val : content->getVals(AGRC::IniKeywords::Run)) {
                if (val == Fix) {
                    ++count;
                }
            }
        }
        return count;
    }


    // The order index of the first fix call inside one part, or -1.
    long long fixCallIndex(const AGRC::IfContentPart<std::string, std::string>& part) {
        for (const auto& indVal : part.getValsWithInds(AGRC::IniKeywords::Run)) {
            if (indVal.second == Fix) {
                return indVal.first;
            }
        }
        return -1;
    }


    std::vector<AGRC::IfContentPart<std::string, std::string>*> contentParts(Section& section) {
        std::vector<AGRC::IfContentPart<std::string, std::string>*> result;
        for (auto& part : section.parts()) {
            auto* content = dynamic_cast<AGRC::IfContentPart<std::string, std::string>*>(part.get());
            if (content != nullptr) {
                result.push_back(content);
            }
        }
        return result;
    }


    void runEdit(Section& section, AGRC::RegDelimitedAddMode mode) {
        std::unordered_map<std::string, Section*> sections{{section.name, &section}};
        Graph graph(sections, {section.name}, runConfig());

        AGRC::RegDelimitedAdd<> edit(AGRC::RegDelimitedAdd<>::Additions{{AGRC::IniKeywords::Run, Fix}},
                                      AGRC::RegDelimitedAdd<>::RegMap{{AGRC::IniKeywords::DrawIndexed, {}}},
                                      /*pathEndOnlyWhenUndelimited*/ true, mode);
        edit.edit(graph, nullptr, "", {}, false, std::nullopt);
    }


    // ---- 1. one draw: both modes agree, and the call goes immediately before it ----
    void testSingleDraw() {
        std::printf("\ntestSingleDraw\n");

        std::vector<RawPart> raw{{0, kvps({{"ps-t0", "Diffuse"}, {"ps-t1", "LightMap"},
                                            {AGRC::IniKeywords::DrawIndexed, "auto"}})}};
        std::unique_ptr<Section> section = Section::build(raw, runConfig(), "Body");
        runEdit(*section, AGRC::RegDelimitedAddMode::PerPath);

        check(countFixCalls(*section) == 1, "one draw gets exactly one fix call");
        check(render(*section) == "ps-t0 = Diffuse\nps-t1 = LightMap\nrun = " + Fix + "\ndrawindexed = auto\n",
              "and it sits immediately before the draw, after the registers it re-slots");
    }


    // ---- 2. the shape that was rendering green: a top-level draw, then independent toggles ----
    void testTopLevelDrawThenIndependentIfs() {
        std::printf("\ntestTopLevelDrawThenIndependentIfs\n");

        // What a real mod looks like (HuTao Flame Fusion, YelanTranquil NSFW): the body drawn
        // unconditionally, then one `if` per accessory. Several are on at once, so several draws
        // run in ONE pass.
        std::vector<RawPart> raw{
            {0, kvps({{"ps-t0", "Diffuse"}, {"ps-t1", "LightMap"}, {AGRC::IniKeywords::DrawIndexed, "154392, 0, 0"}})},
            {1, std::string("if $Jacket == 1")},
            {2, kvps({{AGRC::IniKeywords::DrawIndexed, "15882, 154392, 0"}})},
            {3, std::string("endif")},
            {4, std::string("if $Glasses == 1")},
            {5, kvps({{AGRC::IniKeywords::DrawIndexed, "2628, 212103, 0"}})},
            {6, std::string("endif")}};

        std::unique_ptr<Section> perPath = Section::build(raw, runConfig(), "Body");
        runEdit(*perPath, AGRC::RegDelimitedAddMode::PerPath);
        check(countFixCalls(*perPath) == 1, "three draws in one pass still get exactly ONE fix call");

        std::vector<AGRC::IfContentPart<std::string, std::string>*> parts = contentParts(*perPath);
        check(!parts.empty() && fixCallIndex(*parts[0]) == 2,
              "placed before the first draw of the header part, after both texture registers");
        check(parts.size() < 2 || fixCallIndex(*parts[1]) == -1,
              "and nothing inside the first toggle");

        // The old rule, for contrast -- this is the bug, pinned so a regression is visible.
        std::unique_ptr<Section> perSegment = Section::build(raw, runConfig(), "Body");
        runEdit(*perSegment, AGRC::RegDelimitedAddMode::PerSegment);
        check(countFixCalls(*perSegment) == 3,
              "PerSegment gives the same section three, which is what left every second draw unfixed");
    }


    // ---- 3. draws ONLY inside independent ifs: the call has to move up to the header ----
    void testDrawsOnlyInsideIndependentIfs() {
        std::printf("\ntestDrawsOnlyInsideIndependentIfs\n");

        // Mommy Arlecchino's head. No draw at the top level at all, so there is no position inside
        // either block that serves the paths through the other one.
        std::vector<RawPart> raw{
            {0, kvps({{"ps-t0", "Diffuse"}, {"ps-t1", "LightMap"}})},
            {1, std::string("if $LongHair == 1")},
            {2, kvps({{AGRC::IniKeywords::DrawIndexed, "17631, 0, 0"}})},
            {3, std::string("endif")},
            {4, std::string("if $ShortHair == 1")},
            {5, kvps({{AGRC::IniKeywords::DrawIndexed, "106092, 17631, 0"}})},
            {6, std::string("endif")}};

        std::unique_ptr<Section> section = Section::build(raw, runConfig(), "Head");
        runEdit(*section, AGRC::RegDelimitedAddMode::PerPath);

        check(countFixCalls(*section) == 1, "exactly one fix call, however many blocks draw");

        std::vector<AGRC::IfContentPart<std::string, std::string>*> parts = contentParts(*section);
        check(!parts.empty() && fixCallIndex(*parts[0]) == 2,
              "at the END of the header content -- where a mod author writes it by hand");
        for (std::size_t i = 1; i < parts.size(); ++i) {
            check(fixCallIndex(*parts[i]) == -1,
                  "and nothing inside block " + std::to_string(i));
        }
    }


    // ---- 4. a path that never draws still gets its call, once, at the end ----
    void testUndelimitedPath() {
        std::printf("\ntestUndelimitedPath\n");

        std::vector<RawPart> raw{{0, kvps({{"ps-t0", "Diffuse"}, {"ps-t1", "LightMap"}})}};
        std::unique_ptr<Section> section = Section::build(raw, runConfig(), "Body");
        runEdit(*section, AGRC::RegDelimitedAddMode::PerPath);

        check(countFixCalls(*section) == 1, "a section that never draws still gets one fix call");
        check(render(*section) == "ps-t0 = Diffuse\nps-t1 = LightMap\nrun = " + Fix + "\n",
              "at the end, after the registers -- the shape a fix that moves the draw call leaves");
    }


    // ---- 5. a `run =` whose target is NOT in the graph is a continuation, not a way out ----
    void testExternalCall() {
        std::printf("\ntestExternalCall\n");

        // Every GIMI mod calls command lists it does not define -- TexFx here, on the one path that
        // configures transparency. Reading an unresolvable callee as "leaving the region nothing
        // has delimited" put a SECOND fix call at the end of that block, so the transparency path
        // drew with its registers swapped back. Found on YelanTranquil NSFW's dress (2026-09-14).
        std::vector<RawPart> raw{
            {0, kvps({{"ps-t0", "Diffuse"}, {"ps-t1", "LightMap"}})},
            {1, std::string("if $transparency == 1")},
            {2, kvps({{"ps-t69", "ResourceTransparency"}, {AGRC::IniKeywords::Run, "CommandList\\TexFx\\Transparency.0"}})},
            {3, std::string("endif")},
            {4, kvps({{AGRC::IniKeywords::DrawIndexed, "auto"}})}};

        std::unique_ptr<Section> section = Section::build(raw, runConfig(), "Dress");
        runEdit(*section, AGRC::RegDelimitedAddMode::PerPath);

        if (countFixCalls(*section) != 1) {
            std::printf("       what it produced:\n%s\n", render(*section).c_str());
        }

        check(countFixCalls(*section) == 1,
              "a path that calls an EXTERNAL command list still gets exactly one fix call");

        std::vector<AGRC::IfContentPart<std::string, std::string>*> parts = contentParts(*section);
        check(parts.size() < 2 || fixCallIndex(*parts[1]) == -1,
              "and none at the end of the block that made the call");
    }


    // ---- 6. the draw in a CALLEE: the call must follow the path, not the section ----
    void testDrawInCalledSection() {
        std::printf("\ntestDrawInCalledSection\n");

        // GanyuTwilight's shape: the object section binds and then runs a shared list that draws.
        std::vector<RawPart> rawRoot{
            {0, kvps({{"ps-t0", "Diffuse"}, {"ps-t1", "LightMap"}, {AGRC::IniKeywords::Run, "CommandListIB"}})}};
        std::vector<RawPart> rawCallee{{0, kvps({{AGRC::IniKeywords::DrawIndexed, "auto"}})}};

        std::unique_ptr<Section> root = Section::build(rawRoot, runConfig(), "Body");
        std::unique_ptr<Section> callee = Section::build(rawCallee, runConfig(), "CommandListIB");

        std::unordered_map<std::string, Section*> sections{{root->name, root.get()}, {callee->name, callee.get()}};
        Graph graph(sections, {root->name}, runConfig());

        AGRC::RegDelimitedAdd<> edit(AGRC::RegDelimitedAdd<>::Additions{{AGRC::IniKeywords::Run, Fix}},
                                      AGRC::RegDelimitedAdd<>::RegMap{{AGRC::IniKeywords::DrawIndexed, {}}},
                                      true, AGRC::RegDelimitedAddMode::PerPath);
        edit.edit(graph, nullptr, "", {}, false, std::nullopt);

        check(countFixCalls(*root) + countFixCalls(*callee) == 1,
              "a draw behind a `run =` call still gets exactly one fix call on the path");
        check(countFixCalls(*callee) == 1,
              "and it lands in the CALLEE, immediately before the draw, not in the caller");
    }
}


int main() {
    std::printf("===== RegDelimitedAddMode::PerPath =====\n");

    testSingleDraw();
    testTopLevelDrawThenIndependentIfs();
    testDrawsOnlyInsideIndependentIfs();
    testUndelimitedPath();
    testExternalCall();
    testDrawInCalledSection();

    std::printf("\n");
    if (failures == 0) {
        std::printf("ALL PASSED\n");
        return 0;
    }

    std::printf("%d FAILURE(S)\n", failures);
    return 1;
}
