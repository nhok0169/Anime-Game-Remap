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
// Standalone test for AGRemapCore::GraphInherit's 'adder' -- the plain-C++ half of it.
//
// WHY THIS FILE EXISTS (2026-09-18). An adder decides how GraphInherit's `<reg> = <root of dst>`
// KVPs reach the graph at 'src': it hands back a register edit or a graph edit built from them,
// which GraphInherit runs over 'src' through GraphGroupEdit::editSectionGraph, or nothing, having
// inserted them itself. The Python binding never reaches that dispatch -- it runs a Python adder's
// edit through a real Python GraphGroupEdit, so a pure-Python subclass's own 'edit' runs -- which
// leaves test_GraphInherit.py structurally unable to see the core half. A fixer written in core
// (the WuWa one this was built for) uses exactly that half, so it is pinned here.
//
// The motivating shape: a WuWa TextureOverride binds no textures of its own, and its textures are
// separate resource sections. Composing the resource graphs into it gives it GI-style
// 'ps-t0'/'ps-t1' bindings.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only when
// somebody compiles it. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      GraphInherit_Adder_test.cpp ^
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
#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegSurroundedAdd.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphInherit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IIniGraphGroups.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAdd.h"
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
    using Group = AGRC::IniGraphGroup<std::string, std::string>;
    using Groups = AGRC::IniGraphGroupsVec<std::string, std::string>;
    using Inherit = AGRC::GraphInherit<std::string, std::string>;
    using ContentPart = AGRC::IfContentPart<std::string, std::string>;
    using KVPs = tsl::ordered_map<std::string, std::vector<std::pair<long long, std::string>>>;
    using RawPart = std::pair<int, std::variant<std::string, KVPs>>;


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


    // Renders a section's content back to text, one line per KVP, with each `if`/`endif` line and
    // an indent per depth -- so an assertion can tell a KVP at the top of the section from one
    // inside a branch.
    std::string render(const Section& section) {
        std::string result;
        for (const auto& part : section.parts()) {
            auto* content = dynamic_cast<const ContentPart*>(part.get());
            if (content == nullptr) {
                result += "<pred>\n";
                continue;
            }

            for (const auto& entry : content->items()) {
                result += std::string(static_cast<std::size_t>(content->depth()) * 4, ' ') + entry.key + " = " + entry.value + "\n";
            }
        }
        return result;
    }


    // The WuWa component and its two texture resources, each its own graph in one .ini file's group
    struct WuWaFixture {
        AGRC::Z3Context z3Ctx;  // declared first: destroyed after every predicate built against it
        std::unique_ptr<Section> component;
        std::unique_ptr<Section> diffuse;
        std::unique_ptr<Section> lightMap;
        std::vector<Group> groups;
        Groups view;

        WuWaFixture(): view(groups, runConfig()) {
            std::vector<RawPart> componentRaw{
                {0, kvps({{"hash", "33e4890f"}, {"match_first_index", "0"}, {"match_index_count", "8199"}, {"$object_detected", "1"}})},
                {0, std::string("if $mod_enabled")},
                {1, kvps({{"handling", "skip"}, {AGRC::IniKeywords::DrawIndexed, "8199, 0, 0"},
                          {AGRC::IniKeywords::Run, "CommandListCleanupSharedResources"}})},
                {0, std::string("endif")}};
            component = Section::build(componentRaw, runConfig(), "TextureOverrideComponent0", nullptr, &z3Ctx);
            diffuse = Section::build({{0, kvps({{"filename", "HeadDiffuse.dds"}})}}, runConfig(), "ResourceHeadDiffuse", nullptr, &z3Ctx);
            lightMap = Section::build({{0, kvps({{"filename", "HeadLightMap.dds"}})}}, runConfig(), "ResourceHeadLightMap", nullptr, &z3Ctx);

            groups.emplace_back();
            groups[0].addGraph({"", "Component0"}, makeGraph(*component));
            groups[0].addGraph({"", "HeadDiffuse"}, makeGraph(*diffuse));
            groups[0].addGraph({"", "HeadLightMap"}, makeGraph(*lightMap));
        }

        std::unique_ptr<Graph> makeGraph(Section& section) {
            std::unordered_map<std::string, Section*> sections{{section.name, &section}};
            return std::make_unique<Graph>(sections, std::vector<std::string>{section.name}, runConfig(), true, false, &z3Ctx);
        }

        Graph* componentGraph() const {
            return groups[0].getGraph({"", "Component0"});
        }
    };


    // Every KVP of the part holding the hash, after its last hash/match_*/ps-t KVP
    Inherit::OrderRanges afterHeader(const Inherit::IterData& iterData, const AGRC::ModType*, AGRC::IniFile*) {
        const ContentPart& part = *iterData.part;
        if (!part.containsKey("hash")) {
            return Inherit::OrderRanges::createEmpty();
        }

        long long last = -1;
        long long i = 0;
        for (const auto& entry : part.items()) {
            if (entry.key == "hash" || entry.key == "match_first_index" || entry.key == "match_index_count" || entry.key.rfind("ps-t", 0) == 0) {
                last = i;
            }
            ++i;
        }

        return Inherit::OrderRanges({{last + 1, std::nullopt}});
    }


    // ---- 1. a register edit: the WuWa component gains its texture bindings after its match keys ----
    void testRegEditAdder() {
        std::printf("\ntestRegEditAdder\n");
        WuWaFixture fixture;

        Inherit::Adder addAtFront = [](Graph&, const Inherit::KVPs& kvps, AGRC::IniFile*, const AGRC::ModType*, const std::string&) -> Inherit::AddEdit {
            return std::make_shared<AGRC::RegAdd<std::string, std::string>>(kvps, false);
        };

        Inherit diffuse({0, "", "Component0"}, {0, "", "HeadDiffuse"}, "ps-t0", true, afterHeader, addAtFront);
        Inherit lightMap({0, "", "Component0"}, {0, "", "HeadLightMap"}, "ps-t1", true, afterHeader, addAtFront);
        diffuse.edit(fixture.view, nullptr);
        lightMap.edit(fixture.view, nullptr);

        std::string expected = "hash = 33e4890f\nmatch_first_index = 0\nmatch_index_count = 8199\n"
                               "ps-t0 = ResourceHeadDiffuse\nps-t1 = ResourceHeadLightMap\n$object_detected = 1\n"
                               "<pred>\n"
                               "    handling = skip\n    drawindexed = 8199, 0, 0\n    run = CommandListCleanupSharedResources\n"
                               "<pred>\n";
        std::string result = render(*fixture.component);
        check(result == expected, "both textures bound after the match keys, in order, outside the if");
        if (result != expected) {
            std::printf("%s", result.c_str());
        }

        check(render(*fixture.diffuse) == "filename = HeadDiffuse.dds\n", "the texture's own graph is untouched");
    }


    // ---- 2. a graph edit: bound as late as possible before the draw that reads it ----
    void testGraphEditAdder() {
        std::printf("\ntestGraphEditAdder\n");
        WuWaFixture fixture;

        Inherit::Adder beforeDraw = [](Graph&, const Inherit::KVPs& kvps, AGRC::IniFile*, const AGRC::ModType*, const std::string&) -> Inherit::AddEdit {
            using Surrounded = AGRC::RegSurroundedAdd<std::string, std::string>;
            return std::make_shared<Surrounded>(kvps, Surrounded::RegMap{}, Surrounded::RegMap{{AGRC::IniKeywords::DrawIndexed, {}}}, true);
        };

        Inherit diffuse({0, "", "Component0"}, {0, "", "HeadDiffuse"}, "ps-t0", true, {}, beforeDraw);
        diffuse.edit(fixture.view, nullptr);

        std::string expected = "hash = 33e4890f\nmatch_first_index = 0\nmatch_index_count = 8199\n$object_detected = 1\n"
                               "<pred>\n"
                               "    handling = skip\n    ps-t0 = ResourceHeadDiffuse\n    drawindexed = 8199, 0, 0\n"
                               "    run = CommandListCleanupSharedResources\n"
                               "<pred>\n";
        std::string result = render(*fixture.component);
        check(result == expected, "the texture is bound immediately before the draw");
        if (result != expected) {
            std::printf("%s", result.c_str());
        }
    }


    // ---- 3. the adder inserts the KVPs itself: it is handed the right things, and nothing else runs ----
    void testAdderDoesItItself() {
        std::printf("\ntestAdderDoesItItself\n");
        WuWaFixture fixture;

        int calls = 0;
        Graph* seenGraph = nullptr;
        Inherit::KVPs seenKVPs;
        AGRC::IniFile* seenIni = nullptr;
        std::string seenModName;

        Inherit::Adder adder = [&](Graph& srcGraph, const Inherit::KVPs& kvps, AGRC::IniFile* ini, const AGRC::ModType*,
                                   const std::string& modName) -> Inherit::AddEdit {
            ++calls;
            seenGraph = &srcGraph;
            seenKVPs = kvps;
            seenIni = ini;
            seenModName = modName;
            srcGraph.getRootSections()[0]->addKVPsToBack(kvps);
            return std::monostate{};
        };

        // never dereferenced: nothing between editFromIni and the adder reads the .ini, and the
        // adder only compares it
        int notAnIni = 0;
        auto* ini = reinterpret_cast<AGRC::IniFile*>(&notAnIni);

        Inherit diffuse({0, "", "Component0"}, {0, "", "HeadDiffuse"}, "ps-t0", true, {}, adder);
        diffuse.editFromIni(fixture.view, ini, nullptr, "Target");

        check(calls == 1, "the adder is called once");
        check(seenGraph == fixture.componentGraph(), "with the graph at 'src'");
        check(seenKVPs == Inherit::KVPs{{"ps-t0", "ResourceHeadDiffuse"}}, "with the reg = root KVP");
        check(seenIni == ini, "with the .ini editFromIni was given");
        check(seenModName == "Target", "with the mod name");
        check(render(*fixture.component).find("<pred>\nps-t0 = ResourceHeadDiffuse\n") != std::string::npos,
              "and what it inserted is the only insertion");

        calls = 0;
        Inherit none({0, "", "Component0"}, {0, "", "Missing"}, "ps-t0", true, {}, adder);
        none.edit(fixture.view, nullptr);
        check(calls == 0, "a missing 'dst' graph never reaches the adder");
    }


    // ---- 4. a graph edit that hands back a DIFFERENT graph: that graph replaces 'src' in its group ----
    class CopyingEdit: public AGRC::BaseIniGraphEdit<std::string, std::string> {
        public:
            CopyingEdit(Groups& view, Inherit::KVPs kvps): view_(view), kvps_(std::move(kvps)) {}

            Graph& edit(Graph& graph, const AGRC::ModType*, const std::string& = "", const PartFilter& = {}, bool = false,
                        const std::optional<KeySet>& = std::nullopt) override {
                Graph* copy = view_.deepcopyGraph(graph);
                copy->getRootSections()[0]->addKVPsToFront(kvps_);
                return *copy;
            }

        private:
            Groups& view_;
            Inherit::KVPs kvps_;
    };

    void testGraphEditReturnsNewGraph() {
        std::printf("\ntestGraphEditReturnsNewGraph\n");
        WuWaFixture fixture;
        Graph* original = fixture.componentGraph();

        Groups* view = &fixture.view;
        Inherit::Adder copying = [view](Graph&, const Inherit::KVPs& kvps, AGRC::IniFile*, const AGRC::ModType*, const std::string&) -> Inherit::AddEdit {
            return std::make_shared<CopyingEdit>(*view, kvps);
        };

        Inherit diffuse({0, "", "Component0"}, {0, "", "HeadDiffuse"}, "ps-t0", true, {}, copying);
        diffuse.edit(fixture.view, nullptr);

        Graph* replaced = fixture.componentGraph();
        check(replaced != nullptr && replaced != original, "the returned graph takes the place of 'src' in its group");
        check(replaced != nullptr && render(*replaced->getRootSections()[0]).rfind("ps-t0 = ResourceHeadDiffuse\nhash = 33e4890f\n", 0) == 0,
              "and it is the edited copy");
    }
}


int main() {
    testRegEditAdder();
    testGraphEditAdder();
    testAdderDoesItItself();
    testGraphEditReturnsNewGraph();

    std::printf("\n%s: %d failure(s)\n", failures == 0 ? "ALL PASSED" : "FAILED", failures);
    return failures == 0 ? 0 : 1;
}
