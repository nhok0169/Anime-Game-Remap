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
// Standalone test for AGRemapCore::RegSurroundedAdd's window when the register that OPENS the
// window and the one that CLOSES it are ADJACENT KVPs.
//
// WHY THIS FILE EXISTS (2026-09-25). A `beforeRegs` entry with a predicate means "the most recent
// definition of this register before here was accepted". The window was computed by asking the
// colouring for the satisfied range with `includeKeyDefs = false`, which drops EVERY index at which
// the register is (re)defined -- correct for the accepted definition's own index, since the addition
// has to land after it, and wrong for the index of the definition that CLOSES the window, where
// inserting means "before the rebind", which is exactly where the addition belongs.
//
// While something sits between the two, the off-by-one is invisible: the window is [d + 1, j) and
// still has room. With them adjacent it is [d + 1, d + 1) -- empty -- and the edit silently places
// nothing, which is indistinguishable in the output from an edit that was never asked to run.
//
// Found in a real mod: Chisa1's component 5, whose author commented their own `drawindexed` out, so
// `run = CommandListOverrideSharedResources` (accepted) and `run = CommandListCleanupSharedResources`
// (rejected) became neighbours -- a comment is not a KVP. Two sections in 61 .ini files.
//
// THIS FILE FAILS AGAINST THE BUILD THAT HAD THE BUG (verified before it was trusted against the
// fixed one): testAdjacentDefs reports 0 additions, and testSpacedDefs passes either way, which is
// what makes the pair worth having -- the second says the harness is capable of seeing an addition
// at all, so a red first line is about the window and not about the setup.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only when
// somebody compiles it. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      RegSurroundedAdd_AdjacentDefs_test.cpp ^
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
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.h"
#include "AGRemapCore/tools/z3/Z3Context.h"

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
    using ContentPart = AGRC::IfContentPart<std::string, std::string>;
    using KVPs = tsl::ordered_map<std::string, std::vector<std::pair<long long, std::string>>>;
    using RawPart = std::pair<int, std::variant<std::string, KVPs>>;
    using Surrounded = AGRC::RegSurroundedAdd<std::string, std::string>;

    const std::string Override = "CommandListOverrideSharedResources";
    const std::string Cleanup = "CommandListCleanupSharedResources";
    const std::string Textures = "CommandListComponent5Textures";


    AGRC::IfTemplateRunConfig<std::string, std::string> runConfig() {
        // No name is resolvable to a section, so a `run =` is a plain KVP here rather than a call to
        // follow -- which is the point: this is about one part's KVP order, not about the call graph
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


    std::string render(const Section& section) {
        std::string result;
        for (const auto& part : section.parts()) {
            auto* content = dynamic_cast<const ContentPart*>(part.get());
            if (content == nullptr) {
                result += "<pred>\n";
                continue;
            }

            for (const auto& entry : content->items()) {
                result += std::string(static_cast<std::size_t>(content->depth()) * 4, ' ')
                          + entry.key + " = " + entry.value + "\n";
            }
        }
        return result;
    }


    // A component section whose body is a single part: the shared-resource override, then whatever
    // 'between' holds, then the cleanup call. 'between' is what decides whether the two `run =`
    // definitions are neighbours.
    struct Fixture {
        AGRC::Z3Context z3Ctx;  // declared first: destroyed after every predicate built against it
        std::unique_ptr<Section> component;
        std::unique_ptr<Graph> graph;

        explicit Fixture(const std::vector<std::pair<std::string, std::string>>& between) {
            std::vector<std::pair<std::string, std::string>> body{
                {"handling", "skip"},
                {AGRC::IniKeywords::Run, Override}};
            for (const auto& entry : between) {
                body.push_back(entry);
            }
            body.emplace_back(AGRC::IniKeywords::Run, Cleanup);

            std::vector<RawPart> raw{
                {0, kvps({{"hash", "e611d493"}, {"match_first_index", "250398"}})},
                {0, std::string("if $mod_enabled")},
                {1, kvps(body)},
                {0, std::string("endif")}};
            component = Section::build(raw, runConfig(), "TextureOverrideComponent5", nullptr, &z3Ctx);

            std::unordered_map<std::string, Section*> sections{{component->name, component.get()}};
            graph = std::make_unique<Graph>(sections, std::vector<std::string>{component->name},
                                            runConfig(), true, false, &z3Ctx);
        }
    };


    // The WuWa fixer's own shape: put the texture command lists after the shared-resource override
    // and before the cleanup, at the EARLIEST valid index
    std::shared_ptr<Surrounded> texturesAfterOverride() {
        Surrounded::RegMap before{
            {AGRC::IniKeywords::Run, [](const std::string& val) { return val == Override; }}};
        Surrounded::RegMap after{
            {AGRC::IniKeywords::Run, [](const std::string& val) { return val == Cleanup; }}};
        return std::make_shared<Surrounded>(Surrounded::Additions{{AGRC::IniKeywords::Run, Textures}},
                                            before, after, false);
    }


    // ---- 1. the two definitions ADJACENT: the shape that placed nothing ----
    void testAdjacentDefs() {
        std::printf("\ntestAdjacentDefs\n");
        Fixture fixture({});
        texturesAfterOverride()->edit(*fixture.graph, nullptr);

        std::string expected = "hash = e611d493\nmatch_first_index = 250398\n"
                               "<pred>\n"
                               "    handling = skip\n"
                               "    run = " + Override + "\n"
                               "    run = " + Textures + "\n"
                               "    run = " + Cleanup + "\n"
                               "<pred>\n";
        std::string result = render(*fixture.component);
        check(result == expected, "the addition lands between two ADJACENT accepted/rejected definitions");
        if (result != expected) {
            std::printf("---- got ----\n%s", result.c_str());
        }
    }


    // ---- 2. something between them: this always worked, and says the harness can see an addition ----
    void testSpacedDefs() {
        std::printf("\ntestSpacedDefs\n");
        Fixture fixture({{AGRC::IniKeywords::DrawIndexed, "7602, 250398, 0"}});
        texturesAfterOverride()->edit(*fixture.graph, nullptr);

        std::string expected = "hash = e611d493\nmatch_first_index = 250398\n"
                               "<pred>\n"
                               "    handling = skip\n"
                               "    run = " + Override + "\n"
                               "    run = " + Textures + "\n"
                               "    drawindexed = 7602, 250398, 0\n"
                               "    run = " + Cleanup + "\n"
                               "<pred>\n";
        std::string result = render(*fixture.component);
        check(result == expected, "and still lands at the earliest valid index when they are spaced apart");
        if (result != expected) {
            std::printf("---- got ----\n%s", result.c_str());
        }
    }


    // ---- 3. the window is still CLOSED: an addition may not cross the rejecting definition ----
    void testWindowStillCloses() {
        std::printf("\ntestWindowStillCloses\n");

        // the accepted definition comes only AFTER the rejected one, so the sole valid index is the
        // very end of the part -- and with `latest = false` that is where the earliest valid index is
        Fixture fixture({});
        Surrounded::RegMap before{
            {AGRC::IniKeywords::Run, [](const std::string& val) { return val == Cleanup; }}};
        auto edit = std::make_shared<Surrounded>(Surrounded::Additions{{AGRC::IniKeywords::Run, Textures}},
                                                 before, Surrounded::RegMap{}, false);
        edit->edit(*fixture.graph, nullptr);

        std::string expected = "hash = e611d493\nmatch_first_index = 250398\n"
                               "<pred>\n"
                               "    handling = skip\n"
                               "    run = " + Override + "\n"
                               "    run = " + Cleanup + "\n"
                               "    run = " + Textures + "\n"
                               "<pred>\n";
        std::string result = render(*fixture.component);
        check(result == expected, "an addition still may not precede the definition that satisfies it");
        if (result != expected) {
            std::printf("---- got ----\n%s", result.c_str());
        }
    }
}


int main() {
    testAdjacentDefs();
    testSpacedDefs();
    testWindowStillCloses();

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "ALL PASSED" : "FAILED", failures);
    return failures == 0 ? 0 : 1;
}
