// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::MultiModFixer
// (model/strategies/iniFixers/MultiModFixer.h). Covers:
//   * Delegation: every child runs, and their results merge the way IniFile::fix
//     merges its own per-mod-type fixers (later write to a path wins).
//   * Selection by IniFile::filteredToModTypeIds -- nullopt runs everything, a
//     set runs just those ids, and an EMPTY set runs nothing (the distinction
//     that filter's doc comment calls out).
//   * Insertion ordering: Children is a tsl::ordered_map, so the CALLER
//     decides which child is "first" and which is "last".
//   * IniFixingContext narrowing: exactly one child is told isFirstModType and
//     exactly one isLastModType...
//   * ...and that it NARROWS rather than replaces, so a nested MultiModFixer's
//     children never claim the file's first/last word when the outer one was
//     told it does not hold it. This is the property that makes nesting safe.
//   * A nullptr child is skipped rather than crashing, and does not consume a
//     first/last slot.
//
// Needs the full static lib. Build AGRemapCore first ("cd cbuild && ninja
// AGRemapCore"), then compile as described in IniFile_resources_test.cpp.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniFixers/MultiModFixer.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"

namespace AGRC = AGRemapCore;

static int failures = 0;


static void check(bool condition, const std::string& what) {
    if (condition) {
        return;
    }

    std::printf("  FAILED: %s\n", what.c_str());
    ++failures;
}


static void checkEqual(const std::string& got, const std::string& expected, const std::string& what) {
    if (got == expected) {
        return;
    }

    std::printf("  FAILED: %s\n    expected: '%s'\n    got:      '%s'\n",
                what.c_str(), expected.c_str(), got.c_str());
    ++failures;
}


// Records what it was handed and writes one identifiable entry, so the parent's merging and
// context-narrowing are both observable from the outside.
class RecordingFixer: public AGRC::BaseIniFixer<> {
    public:
        explicit RecordingFixer(std::string name, std::string path):
            name(std::move(name)), path(std::move(path)) {}

        std::string name;
        std::string path;

        int runs = 0;
        bool sawFirst = false;
        bool sawLast = false;

    protected:
        FixResult fixImpl(ParseData& parseData, bool keepBackup, bool fixOnly, bool hideOrig,
                           bool withBoilerPlate, bool withSrc, AGRC::IniFixingContext fixingCtx) override {
            (void)parseData;
            (void)keepBackup;
            (void)fixOnly;
            (void)hideOrig;
            (void)withBoilerPlate;
            (void)withSrc;

            ++runs;
            sawFirst = fixingCtx.isFirstModType;
            sawLast = fixingCtx.isLastModType;

            FixResult result;
            result[path] = name;
            return result;
        }
};


// Reaches the protected fixImpl, so a test can hand in a context the way a nesting parent would.
class TestableMultiModFixer: public AGRC::MultiModFixer<> {
    public:
        using AGRC::MultiModFixer<>::MultiModFixer;

        FixResult runWith(ParseData& parseData, AGRC::IniFixingContext fixingCtx) {
            return fixImpl(parseData, true, false, false, true, true, fixingCtx);
        }
};


static std::shared_ptr<RecordingFixer> makeChild(const std::string& name, const std::string& path) {
    return std::make_shared<RecordingFixer>(name, path);
}


static void testDelegatesToEveryChildAndMerges() {
    std::printf("testDelegatesToEveryChildAndMerges\n");

    auto first = makeChild("fromFirst", "a.ini");
    auto second = makeChild("fromSecond", "b.ini");

    AGRC::MultiModFixer<> fixer({{10, first}, {20, second}});
    AGRC::MultiModFixer<>::ParseData parseData;

    AGRC::MultiModFixer<>::FixResult result = fixer.fix(parseData);

    check(first->runs == 1 && second->runs == 1, "with no .ini file to filter by, every child runs");
    check(result.size() == 2, "both children's entries are merged");
    checkEqual(result["a.ini"], "fromFirst", "the first child's entry survives");
    checkEqual(result["b.ini"], "fromSecond", "the second child's entry survives");
}


static void testLaterChildOverwritesTheSamePath() {
    std::printf("testLaterChildOverwritesTheSamePath\n");

    // Both write "same.ini". Insertion order decides, NOT the ids: 10 is inserted second, so it
    // runs last and wins even though it is the lower id.
    AGRC::MultiModFixer<> fixer({{20, makeChild("insertedFirst", "same.ini")},
                                 {10, makeChild("insertedSecond", "same.ini")}});
    AGRC::MultiModFixer<>::ParseData parseData;

    checkEqual(fixer.fix(parseData)["same.ini"], "insertedSecond",
               "the last-INSERTED child runs last and overwrites, regardless of its id");
}


static void testExactlyOneChildIsFirstAndOneIsLast() {
    std::printf("testExactlyOneChildIsFirstAndOneIsLast\n");

    auto first = makeChild("first", "first.ini");
    auto middle = makeChild("middle", "middle.ini");
    auto last = makeChild("last", "last.ini");

    // Ids deliberately NOT ascending: 30 is inserted first and 20 last, so if this ever went back
    // to sorting by id every assertion below would flip.
    AGRC::MultiModFixer<> fixer({{30, first}, {10, middle}, {20, last}});
    AGRC::MultiModFixer<>::ParseData parseData;
    fixer.fix(parseData);

    check(first->sawFirst && !first->sawLast, "the first-INSERTED child holds the file's first word");
    check(!middle->sawFirst && !middle->sawLast, "a middle child holds neither");
    check(!last->sawFirst && last->sawLast, "the last-INSERTED child holds the file's last word");
}


static void testContextIsNarrowedNotReplaced() {
    std::printf("testContextIsNarrowedNotReplaced\n");

    auto low = makeChild("low", "low.ini");
    auto high = makeChild("high", "high.ini");

    TestableMultiModFixer fixer({{10, low}, {20, high}});
    AGRC::MultiModFixer<>::ParseData parseData;

    // What an outer MultiModFixer hands a middle child: neither the file's first nor its last.
    fixer.runWith(parseData, AGRC::IniFixingContext(false, false));

    check(!low->sawFirst, "told it is not the file's first, no child may claim the backup");
    check(!high->sawLast, "told it is not the file's last, no child may hide the original");

    // And the halves narrow independently.
    auto onlyFirstLow = makeChild("low", "low.ini");
    auto onlyFirstHigh = makeChild("high", "high.ini");
    TestableMultiModFixer firstOnly({{10, onlyFirstLow}, {20, onlyFirstHigh}});
    firstOnly.runWith(parseData, AGRC::IniFixingContext(true, false));

    check(onlyFirstLow->sawFirst, "holding the file's first word, the first-inserted child claims it");
    check(!onlyFirstHigh->sawLast, "...while the last word is still withheld");
}


static void testNestedMultiModFixersCompose() {
    std::printf("testNestedMultiModFixersCompose\n");

    auto innerLow = makeChild("innerLow", "innerLow.ini");
    auto innerHigh = makeChild("innerHigh", "innerHigh.ini");
    auto outerHigh = makeChild("outerHigh", "outerHigh.ini");

    // A MultiModFixer IS a BaseIniFixer, so it can be a child of another.
    auto inner = std::make_shared<AGRC::MultiModFixer<>>(
        AGRC::MultiModFixer<>::Children{{10, innerLow}, {20, innerHigh}});

    AGRC::MultiModFixer<> outer({{1, inner}, {2, outerHigh}});
    AGRC::MultiModFixer<>::ParseData parseData;

    AGRC::MultiModFixer<>::FixResult result = outer.fix(parseData);

    check(result.size() == 3, "every leaf across the nest contributes");
    check(innerLow->runs == 1 && innerHigh->runs == 1 && outerHigh->runs == 1, "every leaf ran once");

    // 'inner' is the outer's first child, so its own first child holds the file's first word.
    check(innerLow->sawFirst, "the first leaf of the first branch holds the file's first word");
    check(!innerHigh->sawFirst, "...and no other leaf does");

    // 'inner' is NOT the outer's last, so neither of its leaves may hold the last word.
    check(!innerLow->sawLast && !innerHigh->sawLast,
          "a leaf inside a non-last branch never holds the file's last word");
    check(outerHigh->sawLast, "the last leaf overall does");
}


static void testFilteredToModTypeIdsSelectsChildren() {
    std::printf("testFilteredToModTypeIdsSelectsChildren\n");

    AGRC::IniFile ini(std::nullopt, "[TextureOverrideBody]\n");

    auto low = makeChild("low", "low.ini");
    auto high = makeChild("high", "high.ini");

    // The fixer reaches the .ini file through its parser, the way a real one is wired.
    AGRC::BaseIniParser<> parser(&ini);
    AGRC::MultiModFixer<> fixer({{10, low}, {20, high}}, &parser);
    AGRC::MultiModFixer<>::ParseData parseData;

    // nullopt -> no filter.
    ini.filteredToModTypeIds = std::nullopt;
    fixer.fix(parseData);
    check(low->runs == 1 && high->runs == 1, "std::nullopt runs every child");

    // A set -> just those ids.
    ini.filteredToModTypeIds = std::unordered_set<int>{20};
    fixer.fix(parseData);
    check(low->runs == 1, "an id outside the filter does not run again");
    check(high->runs == 2, "an id inside the filter runs");

    // ...and the single selected child is both first and last.
    check(high->sawFirst && high->sawLast, "the only selected child holds both the first and last word");

    // An EMPTY set is NOT the same as nullopt -- it selects nothing.
    ini.filteredToModTypeIds = std::unordered_set<int>{};
    AGRC::MultiModFixer<>::FixResult empty = fixer.fix(parseData);
    check(low->runs == 1 && high->runs == 2, "an empty filter runs nothing at all");
    check(empty.empty(), "...and so produces no fix");
}


static void testNullChildIsSkipped() {
    std::printf("testNullChildIsSkipped\n");

    auto real = makeChild("real", "real.ini");

    AGRC::MultiModFixer<> fixer({{10, nullptr}, {20, real}});
    AGRC::MultiModFixer<>::ParseData parseData;

    AGRC::MultiModFixer<>::FixResult result = fixer.fix(parseData);

    check(result.size() == 1, "a nullptr child contributes nothing");
    // The null child must not consume the first slot -- otherwise nothing would take the backup.
    check(real->sawFirst && real->sawLast, "the one real child holds both words, not the nullptr one");
}


static void testNoChildrenIsANoOp() {
    std::printf("testNoChildrenIsANoOp\n");

    AGRC::MultiModFixer<> fixer;
    AGRC::MultiModFixer<>::ParseData parseData;

    check(fixer.fix(parseData).empty(), "a fixer with no children produces nothing");
}


static void testClearReachesChildren() {
    std::printf("testClearReachesChildren\n");

    auto leaf = makeChild("leaf", "leaf.ini");
    auto inner = std::make_shared<AGRC::MultiModFixer<>>(AGRC::MultiModFixer<>::Children{{10, leaf}});
    AGRC::MultiModFixer<> outer({{1, inner}});

    // Nothing observable to assert beyond it not crashing through the nest -- BaseIniFixer::clear
    // is a no-op -- but a missing override here would silently stop reaching children the moment
    // one of them holds state.
    outer.clear();
    check(true, "clear() walks the whole nest without crashing");
}


int main() {
    testDelegatesToEveryChildAndMerges();
    testLaterChildOverwritesTheSamePath();
    testExactlyOneChildIsFirstAndOneIsLast();
    testContextIsNarrowedNotReplaced();
    testNestedMultiModFixersCompose();
    testFilteredToModTypeIdsSelectsChildren();
    testNullChildIsSkipped();
    testNoChildrenIsANoOp();
    testClearReachesChildren();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
