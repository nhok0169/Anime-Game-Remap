// -----------------------------------------------------------------------------
// BaseAhoCorasickDFA::addMany, and ModTypeIdTools::registerModTypes on top of it (2026-09-20)
//
// add() has no incremental form -- a keyword's failure links depend on every other keyword, so it
// copies out everything already held and rebuilds the whole automaton. Adding n keywords one at a
// time therefore rebuilds it n times, which is how filing the library's own 49 mod types came to
// cost 0.40s of a 1.3s run. addMany builds once instead.
//
// The ONLY thing that makes that a safe trade is that the result is identical, so that is what
// this file tests: every check below builds the same keywords BOTH ways and requires the two
// automatons to answer the same. A bulk build that quietly dropped a keyword, or merged a
// duplicate differently, or left the failure links half-built, fails here -- and would otherwise
// show up as a --types argument the CLI no longer recognises, which nothing else would catch until
// somebody typed that character's name.
//
// NOTE: nothing builds core/tests/*.cpp -- not CMake, not CI, not main.py. This file runs only
// when somebody compiles it. Compile directly, e.g. (after `vcvarsall.bat x64`):
//
//   cl /std:c++latest /EHsc /nologo /MD /bigobj ^
//      /I <core>/include /I <core>/src /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      BaseAhoCorasickDFA_AddMany_test.cpp ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"

using namespace AGRemapCore;

namespace {
    int failures = 0;


    void check(bool ok, const std::string& what) {
        std::printf("  %s %s\n", ok ? "ok  " : "FAIL", what.c_str());
        if (!ok) {
            failures++;
        }
    }

    using Dfa = BaseAhoCorasickDFA<std::unordered_set<int>>;
    using Entries = std::unordered_map<std::string, std::unordered_set<int>>;


    /** The union-merging handler ModTypeIdTools installs on its own name automaton */
    Dfa::DupHandler unionHandler() {
        return [](std::string_view, const std::unordered_set<int>& existing,
                  const std::unordered_set<int>& added) -> std::unordered_set<int> {
            std::unordered_set<int> combined = existing;
            for (int value : added) {
                combined.insert(value);
            }
            return combined;
        };
    }

    /**
     * What the automaton answers for a body of text, flattened into something comparable.
     *
     * Deliberately read through a SEARCH rather than by inspecting the automaton's internals: the
     * failure links are exactly the part a bulk build could get wrong, and they are invisible to
     * anything that only asks which keywords are held.
     */
    std::set<std::string> found(Dfa& dfa, const std::string& text) {
        std::set<std::string> result;

        for (const auto& [keyword, value] : dfa.getAll(text)) {
            (void) value;
            result.insert(keyword);
        }

        return result;
    }

    std::string describe(const std::set<std::string>& values) {
        std::string result = "{";
        for (const std::string& value : values) {
            if (result.size() > 1) {
                result += ", ";
            }
            result += value;
        }
        return result + "}";
    }


    // ----- 1. the same keywords, added both ways, must answer the same -----
    void testMatchesRepeatedAdd() {
        std::printf("addMany agrees with add() called in a loop\n");

        // Overlapping prefixes and suffixes on purpose -- "an" inside "banana", "ana" spanning it
        // twice -- because those are the cases that exist only in the failure links.
        const std::vector<std::string> keywords = {
            "a", "an", "ana", "anana", "banana", "band", "and", "nana", "b"
        };

        Dfa oneAtATime;
        oneAtATime.setHandleDuplicate(unionHandler());

        Entries batch;
        int id = 0;

        for (const std::string& keyword : keywords) {
            oneAtATime.add(keyword, std::unordered_set<int>{id});
            batch[keyword] = std::unordered_set<int>{id};
            id++;
        }

        Dfa allAtOnce;
        allAtOnce.setHandleDuplicate(unionHandler());
        const std::size_t inserted = allAtOnce.addMany(batch);

        check(inserted == keywords.size(), "addMany reports every keyword as newly inserted");

        for (const std::string& text : {"banana", "bandana", "a", "xxx", "abandanana", ""}) {
            const std::set<std::string> expected = found(oneAtATime, text);
            const std::set<std::string> actual = found(allAtOnce, text);

            check(expected == actual,
                  "\"" + text + "\" -> " + describe(actual) + " (add() gave " + describe(expected) + ")");
        }
    }

    // ----- 2. a batch on top of keywords already held -----
    void testAddsOntoExisting() {
        std::printf("a batch added to an automaton that already holds keywords\n");

        Dfa oneAtATime;
        oneAtATime.setHandleDuplicate(unionHandler());
        Dfa allAtOnce;
        allAtOnce.setHandleDuplicate(unionHandler());

        for (Dfa* dfa : {&oneAtATime, &allAtOnce}) {
            dfa->add(std::string("keqing"), std::unordered_set<int>{1});
            dfa->add(std::string("keqingopulent"), std::unordered_set<int>{2});
        }

        oneAtATime.add(std::string("shenhe"), std::unordered_set<int>{3});
        oneAtATime.add(std::string("qing"), std::unordered_set<int>{4});

        allAtOnce.addMany(Entries{{"shenhe", {3}}, {"qing", {4}}});

        for (const std::string& text : {"keqingopulent", "shenhefrostflower", "qing", "keqing"}) {
            check(found(oneAtATime, text) == found(allAtOnce, text),
                  "\"" + text + "\" -> " + describe(found(allAtOnce, text)));
        }
    }

    // ----- 3. a keyword already held is MERGED, not replaced and not duplicated -----
    void testMergesDuplicates() {
        std::printf("a keyword already held is merged through handleDuplicate\n");

        Dfa dfa;
        dfa.setHandleDuplicate(unionHandler());
        dfa.add(std::string("jean"), std::unordered_set<int>{1});

        const std::size_t inserted = dfa.addMany(Entries{{"jean", {2}}, {"jeansea", {3}}});

        check(inserted == 1, "only the keyword that was actually new counts as inserted");

        bool merged = false;
        for (const auto& [keyword, value] : dfa.getAll(std::string("jean"))) {
            if (keyword == "jean") {
                merged = (value->count(1) == 1 && value->count(2) == 1);
            }
        }

        check(merged, "both ids survive the merge -- a replace would drop the first");
    }

    // ----- 4. an empty batch changes nothing -----
    void testEmptyBatchIsANoOp() {
        std::printf("an empty batch does nothing\n");

        Dfa dfa;
        dfa.setHandleDuplicate(unionHandler());
        dfa.add(std::string("yelan"), std::unordered_set<int>{1});

        const std::set<std::string> before = found(dfa, "yelantranquil");
        const std::size_t inserted = dfa.addMany(Entries{});

        check(inserted == 0, "nothing was inserted");
        check(found(dfa, "yelantranquil") == before, "and what it answers is unchanged");
    }

    // ----- 5. the real registry still resolves every shipped name and alias -----
    void testEveryShippedNameStillResolves() {
        std::printf("every shipped mod type still resolves by name and by alias\n");

        // registerMissing is what the CLI calls, and it now hands the whole batch over at once.
        GlobalModTypes::registerMissing();

        int checked = 0;
        bool allFound = true;
        std::string firstMissing;

        for (const ModType& modType : GlobalModTypes::all()) {
            std::vector<std::string> names = {modType.name};
            names.insert(names.end(), modType.aliases.begin(), modType.aliases.end());

            for (const std::string& name : names) {
                checked++;
                const std::optional<ModTypeId> resolved = ModTypeIdTools::findByName(name);

                if (!resolved.has_value() || static_cast<int>(*resolved) != modType.modTypeId) {
                    allFound = false;
                    if (firstMissing.empty()) {
                        firstMissing = name;
                    }
                }
            }
        }

        check(checked > 40, "there are mod types to check at all (" + std::to_string(checked) + " names)");
        check(allFound, allFound ? "all of them resolve to their own id"
                                 : "FIRST UNRESOLVED NAME: " + firstMissing);

        // Called twice in an ordinary run -- once by the CLI, once when the classifiers are
        // populated -- and the second must not disturb what the first filed.
        GlobalModTypes::registerMissing();
        const std::optional<ModTypeId> again = ModTypeIdTools::findByName("keqing");
        check(again.has_value(), "a second registerMissing leaves the names resolving");
    }

    // ----- 6. and it must NOTICE when the registry has been emptied underneath it -----
    void testRegisterMissingRefilesAfterClear() {
        std::printf("registerMissing files the shipped set again after a clear()\n");

        GlobalModTypes::registerMissing();
        check(ModTypeIdTools::findByName("keqing").has_value(), "filed to begin with");

        // registerMissing skips its (expensive) work when nothing can have gone missing since it
        // last ran -- it builds all 49 mod types just to ask which ids are absent, and an ordinary
        // run calls it twice. clear() is the ONE thing that can make a filed id absent again, and
        // it is what the guard keys on. A guard that were a plain "already done" flag would leave
        // the registry empty for the rest of the process, and every .ini file would come back
        // isMod == true with no mod types at all -- which is the exact bug the same pattern in
        // GlobalIniClassifiers was written to fix.
        ModTypeIdTools::clear();
        check(!ModTypeIdTools::findByName("keqing").has_value(), "clear() really emptied it");

        GlobalModTypes::registerMissing();
        check(ModTypeIdTools::findByName("keqing").has_value(),
              "and registerMissing filled it back in");

        const std::optional<ModTypeId> sanhua = ModTypeIdTools::findByName("sanhua");
        check(sanhua.has_value(), "the WuWa half came back too, not just the GI half");
    }
}


int main() {
    std::printf("===== BaseAhoCorasickDFA::addMany =====\n");

    testMatchesRepeatedAdd();
    testAddsOntoExisting();
    testMergesDuplicates();
    testEmptyBatchIsANoOp();
    testEveryShippedNameStillResolves();
    testRegisterMissingRefilesAfterClear();

    std::printf("\n%s (%d failure%s)\n", failures == 0 ? "PASSED" : "FAILED", failures,
                failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
