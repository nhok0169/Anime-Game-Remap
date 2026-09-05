// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniFile::classify() (and the new
// AGRemapCore::GlobalIniClassifiers module it falls back to) --
// model/files/IniFile.h, constants/GlobalIniClassifiers.h.
//
// Uses a hand-written FakeIniClassifier (subclassing BaseIniClassifier) that
// returns CANNED results instead of doing real DFA-based classification. This
// deliberately isolates IniFile::classify()'s own logic -- branching on
// forcedModTypeIds, filtering by filteredModTypeIds, resolving an int
// ModTypeId to an actual ModType via overrideModTypes-then-global-registry,
// and converting the constructor's plain-int gameTypeId to a real GameTypeId
// -- from IniClassifier's separate DFA-construction correctness, which is its
// own concern and isn't exercised here.
//
// Covers, against the documented contract in IniFile.h (itself derived from
// the maintainer's own step-by-step spec + follow-up clarifications, not the
// deprecated pure-Python IniFile, which only ever supported a single ModType):
//   * Normal (non-forced) branch: isMod/isFixed come from the classifier's
//     IniClassifyStats; modTypes is built from stats.modType, filtered by
//     filteredModTypeIds (nullopt == accept everything the classifier found),
//     each id resolved via overrideModTypes first, falling back to the global
//     ModTypeIdTools registry, and silently dropped if resolvable by neither
//   * Forced branch (forcedModTypeIds has a value): the classifier's own
//     classify() is NEVER called; only checkIsFixedMod() runs, and its two
//     independent bool out-params set isMod/isFixed directly (proven
//     independent of each other, and independent of whatever a wrongly-called
//     classify() would have produced); modTypes is built directly from
//     forcedModTypeIds, each id resolved the same override-then-global way
//   * gameTypeId: a valid int converts to the matching GameTypeId enum before
//     being passed to the classifier; an unrecognized custom int falls back
//     to std::nullopt (this applies to both the normal and forced branches)
//   * GlobalIniClassifiers::classifier() is a true lazy singleton (same
//     instance every call); an IniFile constructed with no explicit
//     iniClassifier argument falls back to it without crashing
//   * classify() reads the .ini file from disk first (via readFileLines) if
//     it hasn't been read yet -- same "read on first use" contract as
//     readFileLines() itself already has
//   * defaultModTypeIds: the fallback set is folded straight into modTypes,
//     but ONLY when the classifier itself recognized nothing. Three negatives
//     matter as much as the positive -- it does not apply on the forced
//     branch, it does not apply when the classifier DID recognize something
//     that filteredFromModTypeIds then rejected, and an id naming nothing
//     registered is skipped rather than faked
//
// This file has NO dependency on the project's build system (CMake/pybind11)
// or z3, but DOES need utf8proc (transitively, via StringTools) and the
// ordered-map header-only library (via IniClassifyStats). Compile directly,
// e.g.:
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <utf8proc-src> /I <ordered-map>/include ^
//      IniFile_classify_test.cpp <utf8proc-src>/utf8proc.c ^
//      <core>/src/tools/StringTools.cpp <core>/src/tools/StringHash.cpp ^
//      <core>/src/tools/grapheme/GraphemeIterator.cpp <core>/src/tools/grapheme/GraphemeRange.cpp ^
//      <core>/src/tools/idGenerator/UuidIdGenerator.cpp ^
//      <core>/src/constants/GameTypeId.cpp <core>/src/constants/ModTypeId.cpp ^
//      <core>/src/constants/GlobalIniClassifiers.cpp ^
//      <core>/src/model/strategies/ModType.cpp <core>/src/model/strategies/ModTypeIdData.cpp ^
//      <core>/src/model/strategies/iniClassifiers/BaseIniClassifier.cpp ^
//      <core>/src/model/strategies/iniClassifiers/IniClassifier.cpp ^
//      <core>/src/model/strategies/iniClassifiers/IniClassifyStats.cpp ^
//      <core>/src/model/files/IniFile.cpp ^
//      /Fe:test.exe
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/files/IniFile.h"

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalIniClassifiers.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/ModTypeIdData.h"
#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifier.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"

#include <cstdio>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace AGRemapCore;

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (condition) {
        std::printf("[PASS] %s\n", description);
    } else {
        std::printf("[FAIL] %s\n", description);
        failures++;
    }
}

// Exposes IniFile's protected isMod/isFixed/modTypes for the test to read, without weakening the
// real class's encapsulation.
class TestableIniFile: public IniFile {
    public:
        using IniFile::IniFile;

        bool testIsMod() const { return isMod; }
        bool testIsFixed() const { return isFixed; }
        // modTypes is a tsl::ordered_map now -- insertion order decides which mod type takes
        // the .ini file's backup and which hides the original (see IniFile::fix).
        const tsl::ordered_map<int, ModType>& testModTypes() const { return modTypes; }
};

class FakeIniClassifier: public BaseIniClassifier {
    public:
        IniClassifyStats cannedStats;
        bool cannedIsFixed = false;
        bool cannedIsMod = false;

        int classifyCallCount = 0;
        int checkIsFixedModCallCount = 0;
        std::optional<GameTypeId> lastClassifyGameTypeId;
        std::optional<GameTypeId> lastCheckIsFixedModGameTypeId;

        IniClassifyStats classify(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId) override {
            classifyCallCount++;
            lastClassifyGameTypeId = gameTypeId;
            return cannedStats;
        }

        void checkIsFixedMod(const std::vector<std::string>& iniTxt, bool* isFixedOut, bool* isModOut, std::optional<GameTypeId> gameTypeId) override {
            checkIsFixedModCallCount++;
            lastCheckIsFixedModGameTypeId = gameTypeId;
            *isFixedOut = cannedIsFixed;
            *isModOut = cannedIsMod;
        }
};

void testNormalClassifyBranch() {
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, 20, "GlobalTwenty"));
    ModTypeIdTools::registerModType(ModType(0, 30, "GlobalThirty"));

    tsl::ordered_map<int, ModTypeIdData> found;
    found.emplace(10, ModTypeIdData(0, 10));
    found.emplace(20, ModTypeIdData(0, 20));
    found.emplace(30, ModTypeIdData(0, 30));
    found.emplace(40, ModTypeIdData(0, 40));

    FakeIniClassifier fake;
    fake.cannedStats = IniClassifyStats(found, true, false);

    std::unordered_map<int, ModType> overrides;
    overrides.emplace(10, ModType(0, 10, "OverrideTen"));

    std::unordered_set<int> filtered = {10, 20, 40};  // deliberately excludes 30

    TestableIniFile ini(std::nullopt, "some content\n", std::nullopt, filtered, std::nullopt, overrides, &fake);
    ini.classify();

    check(fake.classifyCallCount == 1, "normal branch: classify() called exactly once on the classifier");
    check(fake.checkIsFixedModCallCount == 0, "normal branch: checkIsFixedMod() never called");
    check(ini.testIsMod() == true, "normal branch: isMod comes from classifier stats");
    check(ini.testIsFixed() == false, "normal branch: isFixed comes from classifier stats");

    const auto& modTypes = ini.testModTypes();
    check(modTypes.size() == 2, "normal branch: only filtered+resolvable ids end up in modTypes");
    check(modTypes.count(10) == 1 && modTypes.at(10).name == "OverrideTen", "normal branch: overrideModTypes takes precedence over the global registry");
    check(modTypes.count(20) == 1 && modTypes.at(20).name == "GlobalTwenty", "normal branch: falls back to the global registry when not overridden");
    check(modTypes.count(30) == 0, "normal branch: id excluded by filteredModTypeIds is dropped even though resolvable");
    check(modTypes.count(40) == 0, "normal branch: id passing the filter but unresolvable anywhere is silently dropped");
}

void testFilteredModTypeIdsNulloptAcceptsAll() {
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, 5, "Five"));

    tsl::ordered_map<int, ModTypeIdData> found;
    found.emplace(5, ModTypeIdData(0, 5));
    found.emplace(6, ModTypeIdData(0, 6));  // unresolvable anywhere

    FakeIniClassifier fake;
    fake.cannedStats = IniClassifyStats(found, true, true);

    TestableIniFile ini(std::nullopt, "x\n", std::nullopt, std::nullopt, std::nullopt, std::nullopt, &fake);
    ini.classify();

    check(ini.testModTypes().size() == 1 && ini.testModTypes().count(5) == 1,
          "filteredModTypeIds == nullopt: accepts every id the classifier found (that's still resolvable)");
}

void testForcedBranch() {
    // NOTE: ModTypeIdTools::getModType now takes a raw int directly, so it can resolve a
    // genuinely custom id (one outside ModTypeId's declared enum range) via the global registry
    // too, same as an in-range one -- as long as it was actually registered there. Id 2 below is
    // a real in-range ordinal, registered globally; id 88 is deliberately NOT registered globally
    // at all (only via overrideModTypes below), to prove overrideModTypes still takes precedence
    // and that IniFile::getModType doesn't require an id to be globally registered to resolve it.
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, 2, "GlobalTwo"));

    FakeIniClassifier fake;
    fake.cannedIsFixed = true;
    fake.cannedIsMod = false;  // deliberately different from isFixed, to prove independence

    // If classify() were wrongly called in the forced branch, this id (999) would leak into
    // modTypes -- included so that bug would be caught, not silently passed.
    tsl::ordered_map<int, ModTypeIdData> wouldBeWrong;
    wouldBeWrong.emplace(999, ModTypeIdData(0, 999));
    fake.cannedStats = IniClassifyStats(wouldBeWrong, true, true);

    std::unordered_set<int> forced = {2, 88};
    std::unordered_map<int, ModType> overrides;
    overrides.emplace(88, ModType(0, 88, "Override88"));

    TestableIniFile ini(std::nullopt, "x\n", std::nullopt, std::nullopt, forced, overrides, &fake);
    ini.classify();

    check(fake.checkIsFixedModCallCount == 1, "forced branch: checkIsFixedMod() called exactly once");
    check(fake.classifyCallCount == 0, "forced branch: classify() is never called on the classifier");
    check(ini.testIsFixed() == true, "forced branch: isFixed comes from checkIsFixedMod's out-param");
    check(ini.testIsMod() == false, "forced branch: isMod comes from checkIsFixedMod's out-param, independent of isFixed");

    const auto& modTypes = ini.testModTypes();
    check(modTypes.size() == 2, "forced branch: modTypes built from forcedModTypeIds, not the classifier's own stats");
    check(modTypes.count(2) == 1 && modTypes.at(2).name == "GlobalTwo", "forced branch: resolves via the global registry");
    check(modTypes.count(88) == 1 && modTypes.at(88).name == "Override88", "forced branch: resolves via overrideModTypes");
    check(modTypes.count(999) == 0, "forced branch: the canned classify() stats never leak in");
}

void testGameTypeIdConversion() {
    FakeIniClassifier fake1;
    fake1.cannedStats = IniClassifyStats({}, false, false);
    TestableIniFile giIni(std::nullopt, "x\n", static_cast<int>(GameTypeId::GI), std::nullopt, std::nullopt, std::nullopt, &fake1);
    giIni.classify();
    check(fake1.lastClassifyGameTypeId.has_value() && *fake1.lastClassifyGameTypeId == GameTypeId::GI,
          "gameTypeId: a valid int converts to the matching GameTypeId enum for the classifier call");

    FakeIniClassifier fake2;
    fake2.cannedStats = IniClassifyStats({}, false, false);
    TestableIniFile unknownIni(std::nullopt, "x\n", 999999, std::nullopt, std::nullopt, std::nullopt, &fake2);
    unknownIni.classify();
    check(!fake2.lastClassifyGameTypeId.has_value(),
          "gameTypeId: an unrecognized custom int falls back to std::nullopt for the classifier call");

    FakeIniClassifier fake3;
    fake3.cannedIsFixed = true;
    fake3.cannedIsMod = true;
    TestableIniFile forcedIni(std::nullopt, "x\n", static_cast<int>(GameTypeId::WuWa), std::nullopt, std::unordered_set<int>{}, std::nullopt, &fake3);
    forcedIni.classify();
    check(fake3.lastCheckIsFixedModGameTypeId.has_value() && *fake3.lastCheckIsFixedModGameTypeId == GameTypeId::WuWa,
          "gameTypeId: also converted correctly for the forced (checkIsFixedMod) branch");
}

void testGlobalIniClassifiersSingleton() {
    IniClassifier& a = GlobalIniClassifiers::classifier();
    IniClassifier& b = GlobalIniClassifiers::classifier();
    check(&a == &b, "GlobalIniClassifiers::classifier() returns the same instance on every call");

    // An IniFile constructed with no explicit iniClassifier argument falls back to the global
    // default -- just confirm this doesn't crash and produces a sane (empty) result, since the
    // default classifier currently has no mod types registered on it.
    TestableIniFile ini(std::nullopt, "hello\n");
    ini.classify();
    check(ini.testModTypes().empty(), "default classifier fallback (no iniClassifier arg): bare global classifier finds nothing registered");
}

void testClassifyReadsFileFirst(const std::string& scratchDir) {
    std::string path = scratchDir + "/IniFile_classify_test.ini";
    {
        std::ofstream out(path, std::ios::binary);
        out << "[TextureOverrideSomething]\nhash=deadbeef\n";
    }

    FakeIniClassifier fake;
    fake.cannedStats = IniClassifyStats({}, true, false);

    TestableIniFile ini(path, "", std::nullopt, std::nullopt, std::nullopt, std::nullopt, &fake);
    check(!ini.fileLinesRead(), "classify-reads-file: fileLinesRead() is false before classify()");

    ini.classify();
    check(ini.fileLinesRead(), "classify-reads-file: fileLinesRead() becomes true after classify()");
    check(ini.getFileLines().size() == 2, "classify-reads-file: file was actually read from disk before classifying");

    std::remove(path.c_str());
}

}  // namespace

void testDefaultModTypeIdsAppliedWhenClassifierFindsNothing() {
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, 40, "GlobalForty"));
    ModTypeIdTools::registerModType(ModType(0, 50, "GlobalFifty"));

    FakeIniClassifier classifier;
    classifier.cannedStats = IniClassifyStats();
    classifier.cannedStats.isMod = true;

    TestableIniFile ini(std::nullopt, "[TextureOverrideBody]\n", std::nullopt, std::nullopt,
                        std::nullopt, std::nullopt, &classifier);
    ini.defaultModTypeIds = tsl::ordered_set<int>{40, 50};
    ini.classify();

    check(ini.testModTypes().size() == 2, "the fallback ids land in modTypes when nothing classified");

    // Insertion order preserved -- that is why this is a tsl::ordered_set and not an
    // std::unordered_set.
    auto it = ini.testModTypes().begin();
    check(it->first == 40, "the first fallback id comes first");
    ++it;
    check(it->first == 50, "and the second comes second");
}

void testDefaultModTypeIdsSkipsUnregisteredIds() {
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, 40, "GlobalForty"));

    FakeIniClassifier classifier;
    classifier.cannedStats = IniClassifyStats();

    TestableIniFile ini(std::nullopt, "[TextureOverrideBody]\n", std::nullopt, std::nullopt,
                        std::nullopt, std::nullopt, &classifier);
    // 999 is registered nowhere.
    ini.defaultModTypeIds = tsl::ordered_set<int>{40, 999};
    ini.classify();

    check(ini.testModTypes().size() == 1, "a fallback id naming nothing registered is skipped");
    check(ini.testModTypes().begin()->first == 40, "and the registered one still lands");
}

void testDefaultModTypeIdsNotAppliedWhenClassifierFoundSomething() {
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, 20, "GlobalTwenty"));
    ModTypeIdTools::registerModType(ModType(0, 40, "GlobalForty"));

    tsl::ordered_map<int, ModTypeIdData> found;
    found.emplace(20, ModTypeIdData(0, 20));

    FakeIniClassifier classifier;
    classifier.cannedStats = IniClassifyStats();
    classifier.cannedStats.modType = found;

    TestableIniFile ini(std::nullopt, "[TextureOverrideBody]\n", std::nullopt, std::nullopt,
                        std::nullopt, std::nullopt, &classifier);
    ini.defaultModTypeIds = tsl::ordered_set<int>{40};
    ini.classify();

    check(ini.testModTypes().size() == 1, "the classifier's own answer is not joined by the fallback");
    check(ini.testModTypes().begin()->first == 20, "and it is the classified id, not the fallback one");
}

void testDefaultModTypeIdsNotAppliedWhenFilterRejectedTheClassifiersAnswer() {
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, 20, "GlobalTwenty"));
    ModTypeIdTools::registerModType(ModType(0, 40, "GlobalForty"));

    tsl::ordered_map<int, ModTypeIdData> found;
    found.emplace(20, ModTypeIdData(0, 20));

    FakeIniClassifier classifier;
    classifier.cannedStats = IniClassifyStats();
    classifier.cannedStats.modType = found;

    // The classifier recognizes 20; the caller only accepts 30. The .ini file WAS classified --
    // the caller simply filtered its answer away, so handing it the fallback would quietly undo
    // the filter that was asked for.
    TestableIniFile ini(std::nullopt, "[TextureOverrideBody]\n", std::nullopt,
                        std::unordered_set<int>{30}, std::nullopt, std::nullopt, &classifier);
    ini.defaultModTypeIds = tsl::ordered_set<int>{40};
    ini.classify();

    check(ini.testModTypes().empty(), "a filtered-out classification does NOT fall back");
}

void testDefaultModTypeIdsNotAppliedOnForcedBranch() {
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, 20, "GlobalTwenty"));
    ModTypeIdTools::registerModType(ModType(0, 40, "GlobalForty"));

    FakeIniClassifier classifier;
    classifier.cannedStats = IniClassifyStats();

    TestableIniFile ini(std::nullopt, "[TextureOverrideBody]\n", std::nullopt, std::nullopt,
                        std::unordered_set<int>{20}, std::nullopt, &classifier);
    ini.defaultModTypeIds = tsl::ordered_set<int>{40};
    ini.classify();

    check(ini.testModTypes().size() == 1, "the forced branch takes only what was forced");
    check(ini.testModTypes().begin()->first == 20, "and never the fallback");
    check(classifier.classifyCallCount == 0, "and still never calls classify()");
}

void testDefaultModTypeIdsKeepsIsModWhenFilterRejectsEverything() {
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, 20, "GlobalTwenty"));

    tsl::ordered_map<int, ModTypeIdData> found;
    found.emplace(20, ModTypeIdData(0, 20));

    FakeIniClassifier classifier;
    classifier.cannedStats = IniClassifyStats();
    classifier.cannedStats.isMod = true;
    classifier.cannedStats.modType = found;

    // Filter rejects the one thing found, so modTypes ends up empty. Without a fallback set at all
    // that forces isMod false; having one keeps the classifier's own answer.
    TestableIniFile noFallback(std::nullopt, "[TextureOverrideBody]\n", std::nullopt,
                               std::unordered_set<int>{30}, std::nullopt, std::nullopt, &classifier);
    noFallback.classify();
    check(!noFallback.testIsMod(), "no fallback + nothing survives the filter -> not a mod");

    TestableIniFile withFallback(std::nullopt, "[TextureOverrideBody]\n", std::nullopt,
                                 std::unordered_set<int>{30}, std::nullopt, std::nullopt, &classifier);
    withFallback.defaultModTypeIds = tsl::ordered_set<int>{999};
    withFallback.classify();
    check(withFallback.testIsMod(), "having a fallback at all keeps the classifier's own isMod");
}


int main(int argc, char** argv) {
    std::string scratchDir = ".";
    if (argc > 1) {
        scratchDir = argv[1];
    }

    testNormalClassifyBranch();
    testFilteredModTypeIdsNulloptAcceptsAll();
    testForcedBranch();
    testGameTypeIdConversion();
    testGlobalIniClassifiersSingleton();
    testClassifyReadsFileFirst(scratchDir);
    testDefaultModTypeIdsAppliedWhenClassifierFindsNothing();
    testDefaultModTypeIdsSkipsUnregisteredIds();
    testDefaultModTypeIdsNotAppliedWhenClassifierFoundSomething();
    testDefaultModTypeIdsNotAppliedWhenFilterRejectedTheClassifiersAnswer();
    testDefaultModTypeIdsNotAppliedOnForcedBranch();
    testDefaultModTypeIdsKeepsIsModWhenFilterRejectsEverything();

    ModTypeIdTools::clear();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
