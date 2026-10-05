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
// Standalone regression test for IniSectionGraph::iterByQuery across a `run =` call.
//
// Bug summary
// -----------
// iterByQuery reports the conditional predicate each IfContentPart sits under. It tracks, per
// nesting depth, how many entries the chain open at that depth has put on the query path, so that
// cleaning the chain knows how many to take back off. That counter was a std::vector INDEXED BY
// `frame.depth` but GROWN by one push_back per explored NODE -- which agree only while depth rises
// exactly once per node, and it does not: a content part's children are pushed at depth + 1, and a
// section reached through `run =` is pushed at depth + 1 again. Following one call therefore ran the
// depth past the end of the vector, where operator[] is undefined behaviour rather than an error.
//
// What came out was an if/else-if chain whose every branch after the first carried the FIRST
// branch's predicate as well as its own -- `$swapvar == 0 AND $swapvar != 0 AND $swapvar == 1`,
// which is unsatisfiable. Nothing reported it, because an .ini renders from its parts and only a
// caller asking the graph for a part's CONDITION ever sees this. The first such caller was the
// merge's per-branch draws (GIMIMergeFixer, RegBranchAdd), and every branch of a twelve-variant
// merged mod was attributed to no branch at all.
//
// The shape below is that mod's, reduced: a TextureOverride that binds nothing and only
// `run =`s a CommandList, whose branches pick a different index buffer each. Rooting the graph AT
// the CommandList was always correct, which is exactly why every hand-written reproduction of the
// problem passed -- the call is the trigger, so the two are tested side by side here.
//
// Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <core>/src /I <extern>/ordered-map/include /I <extern>/utf8proc ^
//      /I <cext>/z3/include IniSectionGraph_RunQuery_test.cpp <core>/../AGRemapCore.lib ...
//
// On Linux the repo's own runner globs core/tests/*_test.cpp, so it is picked up with no list to
// maintain. Exits non-zero on any failure.
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
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"

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


    // The CommandList a merged mod's master hides its buffers behind: one branch per variant, each
    // naming that variant's index buffer.
    //
    // 'nested' wraps each branch's ib in an `if` of its own, which is what ResGroupCollect leaves
    // behind when it splices a per-group call in -- and it is load-bearing for this test rather
    // than decoration. The counter that broke is indexed by DEPTH, so the fault needs the depth to
    // get far enough past the vector's end to matter: a flat chain behind one `run =` survives, and
    // the first version of this test was written flat and passed against the broken build (which is
    // how it nearly shipped meaning nothing).
    std::vector<RawPart> chainParts(int branches, bool nested) {
        std::vector<RawPart> raw;
        int id = 0;

        raw.emplace_back(id++, kvps({{"ps-t0", "Diffuse"}, {"ps-t1", "LightMap"}}));
        for (int branch = 0; branch < branches; ++branch) {
            const std::string number = std::to_string(branch);
            const std::string keyword = (branch == 0) ? "if" : "else if";
            raw.emplace_back(id++, std::string(keyword + " $swapvar == " + number));

            if (nested) {
                raw.emplace_back(id++, std::string("if $swapvar == " + number));
            }

            raw.emplace_back(id++, kvps({{AGRC::IniKeywords::Ib, "ResourceIb" + number}}));

            if (nested) {
                raw.emplace_back(id++, std::string("endif"));
            }
        }

        raw.emplace_back(id++, std::string("endif"));
        return raw;
    }


    // Every branch's predicate, in the order the graph hands them out, for the parts that name an ib.
    std::vector<AGRC::Z3Predicate> branchQueries(Graph& graph) {
        std::vector<AGRC::Z3Predicate> result;

        auto parts = graph.iterByQuery();
        while (parts.next()) {
            auto& data = parts.value();
            if (data.part == nullptr || data.part->getVals(AGRC::IniKeywords::Ib).empty()) {
                continue;
            }

            result.push_back(data.query);
        }

        return result;
    }


    // Each branch can happen, and no two can happen together -- which is what an if/else-if chain
    // means, and all that anything reading these predicates needs of them.
    void checkChain(std::vector<AGRC::Z3Predicate> queries, int branches, const std::string& label) {
        check(static_cast<int>(queries.size()) == branches,
              label + ": one predicate per branch (" + std::to_string(queries.size()) + " of "
                  + std::to_string(branches) + ")");

        if (static_cast<int>(queries.size()) != branches) {
            return;
        }

        int unsatisfiable = 0;
        for (const AGRC::Z3Predicate& query : queries) {
            if (!query.isSatisfiable()) {
                ++unsatisfiable;
            }
        }

        check(unsatisfiable == 0,
              label + ": every branch's predicate can actually hold (" + std::to_string(unsatisfiable)
                  + " contradictory)");

        int overlapping = 0;
        for (std::size_t i = 0; i < queries.size(); ++i) {
            for (std::size_t j = i + 1; j < queries.size(); ++j) {
                if ((queries[i] & queries[j]).isSatisfiable()) {
                    ++overlapping;
                }
            }
        }

        check(overlapping == 0,
              label + ": no two branches can hold at once (" + std::to_string(overlapping) + " overlapping pairs)");
    }


    // ---- 1. the chain on its own: the control, and it was always right ----
    void testChainAtTheRoot() {
        std::printf("\ntestChainAtTheRoot\n");

        AGRC::Z3Context z3Ctx;
        const int branches = 12;

        std::unique_ptr<Section> chain = Section::build(chainParts(branches, /*nested*/ true), runConfig(),
                                                        "CommandListBody", nullptr, &z3Ctx);
        std::unordered_map<std::string, Section*> sections{{"CommandListBody", chain.get()}};
        Graph graph(sections, {"CommandListBody"}, runConfig(), true, false, &z3Ctx);

        checkChain(branchQueries(graph), branches, "rooted at the chain");
    }


    // ---- 2. the same chain, one `run =` away: the regression ----
    void testChainThroughRun() {
        std::printf("\ntestChainThroughRun\n");

        AGRC::Z3Context z3Ctx;
        const int branches = 12;

        std::unique_ptr<Section> chain = Section::build(chainParts(branches, /*nested*/ true), runConfig(),
                                                        "CommandListBody", nullptr, &z3Ctx);
        std::vector<RawPart> overrideParts{
            {0, kvps({{AGRC::IniKeywords::Hash, "cdc66323"},
                       {AGRC::IniKeywords::MatchFirstIndex, "9879"},
                       {AGRC::IniKeywords::Run, "CommandListBody"}})}};
        std::unique_ptr<Section> root = Section::build(overrideParts, runConfig(), "TextureOverrideBody",
                                                       nullptr, &z3Ctx);

        std::unordered_map<std::string, Section*> sections{{"TextureOverrideBody", root.get()},
                                                            {"CommandListBody", chain.get()}};
        Graph graph(sections, {"TextureOverrideBody"}, runConfig(), true, false, &z3Ctx);

        // Before the fix every branch but the first came back unsatisfiable here, and only here.
        checkChain(branchQueries(graph), branches, "reached through `run =`");
    }


    // ---- 3. two calls deep, because the bug was about depth rather than about one call ----
    void testChainThroughTwoRuns() {
        std::printf("\ntestChainThroughTwoRuns\n");

        AGRC::Z3Context z3Ctx;
        const int branches = 4;

        std::unique_ptr<Section> chain = Section::build(chainParts(branches, /*nested*/ true), runConfig(),
                                                        "CommandListInner", nullptr, &z3Ctx);
        std::vector<RawPart> middleParts{{0, kvps({{AGRC::IniKeywords::Run, "CommandListInner"}})}};
        std::unique_ptr<Section> middle = Section::build(middleParts, runConfig(), "CommandListOuter",
                                                         nullptr, &z3Ctx);
        std::vector<RawPart> rootParts{{0, kvps({{AGRC::IniKeywords::Hash, "cdc66323"},
                                                   {AGRC::IniKeywords::Run, "CommandListOuter"}})}};
        std::unique_ptr<Section> root = Section::build(rootParts, runConfig(), "TextureOverrideBody",
                                                        nullptr, &z3Ctx);

        std::unordered_map<std::string, Section*> sections{{"TextureOverrideBody", root.get()},
                                                            {"CommandListOuter", middle.get()},
                                                            {"CommandListInner", chain.get()}};
        Graph graph(sections, {"TextureOverrideBody"}, runConfig(), true, false, &z3Ctx);

        checkChain(branchQueries(graph), branches, "two calls deep");
    }
}


int main() {
    testChainAtTheRoot();
    testChainThroughRun();
    testChainThroughTwoRuns();

    std::printf("\n%s\n", failures == 0 ? "ALL PASSED (0 failure(s))"
                                         : (std::to_string(failures) + " test(s) FAILED").c_str());
    return failures == 0 ? 0 : 1;
}
