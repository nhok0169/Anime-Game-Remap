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
// Standalone test for ModBranches::states -- which states of a merged master a fix has to handle.
//
// The first forward (split) fix of a merged master took the mod's states from the BLEND's branches.
// That is right for a master whose every CommandList is the same `$swapvar` chain, and wrong for an
// animated one: a real HuoHuo-over-Bennett mod binds ONE blend for its whole frame range
// (`if $swapvar >= $frameStart && $swapvar <= $frameEnd`) and a different index buffer per frame,
// so the blend alone says there is one state where there are eleven. The frames of that mod happen
// to keep the same vertices, so its output could not tell the two apart -- which is why this is a
// test and not a note.
//
// The sections are built as a mod writes them and read through ModBranches::valsThroughRun, so the
// conditions are the ones a fix actually sees (through `run =`).
//
// On Linux the repo's own runner globs core/tests/*_test.cpp. Exits non-zero on any failure.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixData/ModBranches.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"

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
    using KVPs = tsl::ordered_map<std::string, std::vector<std::pair<long long, std::string>>>;
    using RawPart = std::pair<int, std::variant<std::string, KVPs>>;
    using States = std::vector<std::optional<AGRC::Z3Predicate>>;


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
            result[entry.first].emplace_back(index++, entry.second);
        }

        return result;
    }


    // A chain `if <var> == 0 ... else if <var> == n-1 ... endif`, each branch binding <key> = <prefix><i>.
    std::vector<RawPart> chain(const std::string& var, int branches, const std::string& key, const std::string& prefix) {
        std::vector<RawPart> raw;
        int id = 0;
        for (int i = 0; i < branches; ++i) {
            raw.emplace_back(id++, std::string((i == 0 ? "if " : "else if ") + var + " == " + std::to_string(i)));
            raw.emplace_back(id++, kvps({{key, prefix + std::to_string(i)}}));
        }

        raw.emplace_back(id++, std::string("endif"));
        return raw;
    }


    struct Mod {
        AGRC::ModBranches::Templates templates;

        void add(const std::string& name, std::vector<RawPart> parts) {
            templates[name] = Section::build(parts, runConfig(), name, nullptr, nullptr);
        }

        // A TextureOverride that binds nothing and only runs <list>, as a merged master's does.
        void addOverride(const std::string& name, const std::string& list, std::vector<RawPart> listParts) {
            add(list, std::move(listParts));
            add(name, {{0, kvps({{AGRC::IniKeywords::Hash, "00000000"}, {AGRC::IniKeywords::Run, list}})}});
        }
    };


    // Every state can hold, no two can hold together.
    void checkDisjoint(AGRC::ModBranches& branches, const States& states, const std::string& label) {
        int unsat = 0;
        int overlapping = 0;
        for (std::size_t i = 0; i < states.size(); ++i) {
            if (states[i].has_value() && !states[i]->isSatisfiable()) {
                ++unsat;
            }

            for (std::size_t j = i + 1; j < states.size(); ++j) {
                if (states[i].has_value() && states[j].has_value() && branches.compatible(*states[i], *states[j])) {
                    ++overlapping;
                }
            }
        }

        check(unsat == 0, label + ": every state can hold (" + std::to_string(unsat) + " contradictory)");
        check(overlapping == 0, label + ": no two states hold at once (" + std::to_string(overlapping) + " overlapping pairs)");
    }


    // ---- 1. the animated master: one blend over the frame range, an ib per frame ----
    void testSharedBlendPerFrameIb() {
        std::printf("\ntestSharedBlendPerFrameIb\n");
        const int frames = 11;

        Mod mod;
        mod.addOverride("TextureOverrideBlend", "CommandListBlend",
                        {{0, std::string("if $swapvar >= 0 && $swapvar <= 10")},
                         {1, kvps({{AGRC::IniKeywords::Vb1, "ResourceBlend0"}})},
                         {2, std::string("endif")}});
        mod.addOverride("TextureOverrideBody", "CommandListBody", chain("$swapvar", frames, AGRC::IniKeywords::Ib, "ResourceIb"));

        AGRC::ModBranches branches;
        std::vector<AGRC::BranchVal> blends = branches.valsThroughRun(mod.templates, "TextureOverrideBlend", AGRC::IniKeywords::Vb1);
        std::vector<AGRC::BranchVal> ibs = branches.valsThroughRun(mod.templates, "TextureOverrideBody", AGRC::IniKeywords::Ib);
        check(blends.size() == 1 && ibs.size() == static_cast<std::size_t>(frames), "read one blend and one ib per frame");

        // What enumerating the BLEND's branches gave: one state, and every frame split with frame 0's ib.
        const States blendOnly = branches.states({&blends});
        check(blendOnly.size() == 1, "the blend alone describes a single state (the old enumeration)");

        const States states = branches.states({&blends, &ibs});
        check(states.size() == static_cast<std::size_t>(frames),
              "blend + ibs describe one state per frame (" + std::to_string(states.size()) + " of " + std::to_string(frames) + ")");
        checkDisjoint(branches, states, "frames");

        bool eachOwnIb = states.size() == static_cast<std::size_t>(frames);
        for (std::size_t i = 0; eachOwnIb && i < states.size(); ++i) {
            eachOwnIb = branches.pick(ibs, "", states[i]) == "ResourceIb" + std::to_string(i)
                        && branches.pick(blends, "", states[i]) == "ResourceBlend0";
        }
        check(eachOwnIb, "each state picks its own frame's ib and the shared blend");
    }


    // ---- 2. the ordinary merged master: every list is the same chain ----
    void testSameChainEverywhere() {
        std::printf("\ntestSameChainEverywhere\n");
        Mod mod;
        mod.addOverride("TextureOverrideBlend", "CommandListBlend", chain("$swapvar", 4, AGRC::IniKeywords::Vb1, "ResourceBlend"));
        mod.addOverride("TextureOverrideHead", "CommandListHead", chain("$swapvar", 4, AGRC::IniKeywords::Ib, "ResourceHeadIb"));
        mod.addOverride("TextureOverrideBody", "CommandListBody", chain("$swapvar", 4, AGRC::IniKeywords::Ib, "ResourceBodyIb"));

        AGRC::ModBranches branches;
        auto blends = branches.valsThroughRun(mod.templates, "TextureOverrideBlend", AGRC::IniKeywords::Vb1);
        auto heads = branches.valsThroughRun(mod.templates, "TextureOverrideHead", AGRC::IniKeywords::Ib);
        auto bodies = branches.valsThroughRun(mod.templates, "TextureOverrideBody", AGRC::IniKeywords::Ib);

        const States states = branches.states({&blends, &heads, &bodies});
        check(states.size() == 4, "three copies of one chain are still four states (" + std::to_string(states.size()) + ")");
        checkDisjoint(branches, states, "chain");

        bool paired = states.size() == 4;
        for (std::size_t i = 0; paired && i < states.size(); ++i) {
            const std::string n = std::to_string(i);
            paired = branches.pick(blends, "", states[i]) == "ResourceBlend" + n
                     && branches.pick(heads, "", states[i]) == "ResourceHeadIb" + n
                     && branches.pick(bodies, "", states[i]) == "ResourceBodyIb" + n;
        }
        check(paired, "each state pairs the same variant's blend, head and body");
    }


    // ---- 3. independent toggles: the states are the combinations ----
    void testIndependentToggles() {
        std::printf("\ntestIndependentToggles\n");
        Mod mod;
        mod.addOverride("TextureOverrideHead", "CommandListHead", chain("$hat", 2, AGRC::IniKeywords::Ib, "ResourceHeadIb"));
        mod.addOverride("TextureOverrideBody", "CommandListBody", chain("$coat", 3, AGRC::IniKeywords::Ib, "ResourceBodyIb"));

        AGRC::ModBranches branches;
        auto heads = branches.valsThroughRun(mod.templates, "TextureOverrideHead", AGRC::IniKeywords::Ib);
        auto bodies = branches.valsThroughRun(mod.templates, "TextureOverrideBody", AGRC::IniKeywords::Ib);

        const States states = branches.states({&heads, &bodies});
        check(states.size() == 6, "2 hats x 3 coats is six states (" + std::to_string(states.size()) + ")");
        checkDisjoint(branches, states, "toggles");
    }


    // ---- 4. nothing branches: one unconditional state, the ordinary mod ----
    void testNothingBranches() {
        std::printf("\ntestNothingBranches\n");
        Mod mod;
        mod.add("TextureOverrideBody", {{0, kvps({{AGRC::IniKeywords::Hash, "00000000"}, {AGRC::IniKeywords::Ib, "ResourceBodyIb"}})}});

        AGRC::ModBranches branches;
        auto bodies = branches.valsThroughRun(mod.templates, "TextureOverrideBody", AGRC::IniKeywords::Ib);
        const States states = branches.states({&bodies});
        check(states.size() == 1 && !states.front().has_value(), "an unbranched mod is one state with no condition");
        check(branches.pick(bodies, "", states.front()) == "ResourceBodyIb", "and it picks the mod's only ib");
    }
}


int main() {
    testSharedBlendPerFrameIb();
    testSameChainEverywhere();
    testIndependentToggles();
    testNothingBranches();

    std::printf("\n%s (%d failure%s)\n", failures == 0 ? "ALL PASSED" : "FAILURES", failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
