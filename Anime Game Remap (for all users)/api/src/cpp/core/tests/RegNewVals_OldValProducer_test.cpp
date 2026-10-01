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


// RegNewVals::OldValProducer -- a new register value computed from the one already there.
//
// The rest of this family moves a KVP around; none of them could write a value derived from the old
// one, which is why the WuWa fixer was rendering sections to text and rewriting the lines. The
// alternative is additive, so the first thing this pins is that every OTHER spec form still goes
// through IfContentPart::replaceVals unchanged -- and then that the three forms mean the SAME thing
// with an OldValProducer in them as they do without, which is the whole reason it went here rather
// than into a class of its own.
//
//   cl /std:c++17 /EHsc /I <core>/include /I <core>/src RegNewVals_OldValProducer_test.cpp

#include <cstdio>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"

using namespace AGRemapCore;


namespace {

    int failures = 0;

    void check(bool condition, const std::string& what) {
        if (!condition) {
            ++failures;
            std::printf("  FAIL %s\n", what.c_str());
        }
    }

    using Part = IfContentPart<std::string, std::string>;
    using Edit = RegNewVals<std::string, std::string>;

    Part make(const std::vector<std::pair<std::string, std::string>>& kvps) {
        return Part(kvps);
    }

    std::string dump(const Part& part) {
        std::string out;
        for (const auto& item : part.items()) {
            out += item.key + "=" + item.value + ";";
        }

        return out;
    }

    // What the fixer actually wants: the edited copy of whatever this slot was already bound to.
    Edit::OldValProducer suffix(const std::string& tail) {
        return [tail](const std::string& oldValue, const ModType*) { return oldValue + tail; };
    }
}


int main() {
    std::printf("=== RegNewVals::OldValProducer ===\n");

    // ---- 1. a bare spec rewrites EVERY occurrence, from its own old value ----------------------
    {
        Part part = make({{"this", "A"}, {"ps-t0", "B"}, {"this", "C"}});
        Edit({{"this", Edit::NewVal(suffix("Edited"))}}).edit(part, "section");
        check(dump(part) == "this=AEdited;ps-t0=B;this=CEdited;",
              "a bare OldValProducer did not rewrite every occurrence from its own value: " + dump(part));
    }

    // ---- 2. a list is POSITIONAL, and an entry past the end is unused --------------------------
    // replaceVals' own rule. Mixed on purpose: a list may hold a plain value, a ValProducer and an
    // OldValProducer at once, and only the key's whole spec decides which path it takes.
    {
        Part part = make({{"this", "A"}, {"this", "B"}, {"this", "C"}});
        Edit({{"this", std::vector<Edit::NewVal>{
            Edit::NewVal(suffix("1")),
            Edit::NewVal(std::string("flat")),
            Edit::NewVal(Edit::ValProducer([](const ModType*) { return std::string("made"); })),
            Edit::NewVal(suffix("unused"))
        }}}).edit(part, "section");

        check(dump(part) == "this=A1;this=flat;this=made;",
              "the positional list did not apply entry by entry, or used the entry past the end: " + dump(part));
    }

    // ---- 2b. and an OCCURRENCE past the end of the list is left alone ---------------------------
    // The other half of the positional rule, and the one that can actually go wrong: an
    // implementation that clamps to the last entry gets 2. above entirely right and writes that
    // entry over every occurrence after it.
    {
        Part part = make({{"this", "A"}, {"this", "B"}, {"this", "C"}});
        Edit({{"this", std::vector<Edit::NewVal>{
            Edit::NewVal(suffix("1")),
            Edit::NewVal(suffix("2"))
        }}}).edit(part, "section");

        check(dump(part) == "this=A1;this=B2;this=C;",
              "an occurrence past the end of the list was written: " + dump(part));
    }

    // ---- 3. a conditional writes only what its predicate accepts -------------------------------
    {
        Part part = make({{"this", "keep"}, {"this", "take"}, {"ps-t0", "take"}});
        Edit({{"this", std::pair<Edit::NewVal, Edit::ModTypePredicate>(
            Edit::NewVal(suffix("!")),
            [](const std::string& oldValue, const ModType*) { return oldValue == "take"; })}})
            .edit(part, "section");

        check(dump(part) == "this=keep;this=take!;ps-t0=take;",
              "the conditional OldValProducer did not respect its predicate, or reached another key: " + dump(part));
    }

    // ---- 4. an absent key stays absent, addNewKVPs or not ---------------------------------------
    // A producer of the old value has nothing to read when there is no old value, so there is no
    // value to add -- and inventing one would be the silent wrong-output failure this repo keeps
    // paying for.
    {
        Part part = make({{"ps-t0", "B"}});
        Edit({{"this", Edit::NewVal(suffix("X"))}}, true).edit(part, "section");
        check(dump(part) == "ps-t0=B;", "an absent key was added from an OldValProducer: " + dump(part));
    }

    // ---- 5. every spec form WITHOUT one still goes through replaceVals unchanged -----------------
    // The point of the additive alternative: this is the behaviour every existing caller has, and
    // it is the same code path it was before.
    {
        Part part = make({{"this", "A"}, {"this", "B"}, {"ps-t0", "C"}, {"gone", "D"}});
        Edit({
            {"this", std::vector<Edit::NewVal>{Edit::NewVal(std::string("one"))}},
            {"ps-t0", Edit::NewVal(Edit::ValProducer([](const ModType*) { return std::string("two"); }))},
            {"gone", std::pair<Edit::NewVal, Edit::ModTypePredicate>(
                Edit::NewVal(std::string("three")),
                [](const std::string&, const ModType*) { return false; })},
            {"added", Edit::NewVal(std::string("four"))}
        }, true).edit(part, "section");

        check(dump(part) == "this=one;this=B;ps-t0=two;gone=D;added=four;",
              "a spec with no OldValProducer changed behaviour: " + dump(part));
    }

    // ---- 6. a rewrite to the value it already holds is not a change ------------------------------
    {
        Part part = make({{"this", "A"}});
        Edit({{"this", Edit::NewVal(Edit::OldValProducer(
            [](const std::string& oldValue, const ModType*) { return oldValue; }))}})
            .edit(part, "section");

        check(dump(part) == "this=A;", "an identity rewrite moved the part: " + dump(part));
    }

    if (failures == 0) {
        std::printf("\nALL PASSED\n");
        return 0;
    }

    std::printf("\n%d FAILURE(S)\n", failures);
    return 1;
}
