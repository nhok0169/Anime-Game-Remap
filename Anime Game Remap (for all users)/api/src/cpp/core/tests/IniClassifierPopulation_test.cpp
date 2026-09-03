// -----------------------------------------------------------------------------
// Standalone regression test for the population of the global .ini classifier --
// what makes IniFile::classify() able to name a mod type at all. Covers:
//   * ModTypeIdTools::getSectionKeywords, row by row, against the keyword sets
//     GENERATED from the pure-Python IniClassifierBuilderOld's own addGIModType
//     calls (the KEYS of each call's keyword dict). Not retyped from
//     ModTypeId.cpp's switch -- restating the implementation would pin nothing.
//   * The three mod types carrying a second spelling (CherryHuTao, Raiden,
//     XianglingCheer) and the two target-only ids carrying none.
//   * GlobalIniClassifiers::classifier() arriving already populated, and
//     actually classifying real section names.
//   * The overlap cases the pure-Python builder handles with negative-lookahead
//     regexes ("amber" must not swallow "ambercn"). IniClassifier matches
//     MAXIMALLY, so the longer keyword wins without any regex -- this is the
//     part most at risk if the keyword table is ever edited by hand.
//   * The classifier singleton staying a singleton.
//
// Needs the full static lib. Build AGRemapCore first ("cd cbuild && ninja
// AGRemapCore"), then compile as described in IniFile_resources_test.cpp.
// -----------------------------------------------------------------------------

#include <algorithm>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalIniClassifiers.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifier.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"

namespace AGRC = AGRemapCore;
using AGRC::ModTypeId;
using AGRC::ModTypeIdTools;

static int failures = 0;


static void check(bool condition, const std::string& what) {
    if (condition) {
        return;
    }

    std::printf("  FAILED: %s\n", what.c_str());
    ++failures;
}


static std::string join(const std::vector<std::string>& values) {
    std::string result;
    for (const std::string& value : values) {
        if (!result.empty()) {
            result += ",";
        }
        result += value;
    }
    return result;
}


static void checkKeywords(std::vector<std::string> got, std::vector<std::string> expected, const std::string& what) {
    std::sort(got.begin(), got.end());
    std::sort(expected.begin(), expected.end());

    if (got == expected) {
        return;
    }

    std::printf("  FAILED: %s\n    expected: [%s]\n    got:      [%s]\n",
                what.c_str(), join(expected).c_str(), join(got).c_str());
    ++failures;
}


struct KeywordRow {
    const char* name;
    std::vector<std::string> keywords;
};


static const std::vector<KeywordRow>& expectedRows() {
    static const std::vector<KeywordRow> rows = {
        {"Amber", {"amber"}},
        {"AmberCN", {"ambercn"}},
        {"Arlecchino", {"arlecchino"}},
        {"Ayaka", {"ayaka"}},
        {"AyakaSpringBloom", {"ayakaspringbloom"}},
        {"Barbara", {"barbara"}},
        {"BarbaraSummertime", {"barbarasummertime"}},
        {"CherryHuTao", {"cherryhutao", "hutaocherry"}},
        {"Diluc", {"diluc"}},
        {"DilucFlamme", {"dilucflamme"}},
        {"Fischl", {"fischl"}},
        {"FischlHighness", {"fischlhighness"}},
        {"Ganyu", {"ganyu"}},
        {"GanyuTwilight", {"ganyutwilight"}},
        {"HuTao", {"hutao"}},
        {"Jean", {"jean"}},
        {"JeanCN", {"jeancn"}},
        {"JeanSea", {"jeansea"}},
        {"Kaeya", {"kaeya"}},
        {"KaeyaSailwind", {"kaeyasailwind"}},
        {"Keqing", {"keqing"}},
        {"KeqingOpulent", {"keqingopulent"}},
        {"Kirara", {"kirara"}},
        {"KiraraBoots", {"kiraraboots"}},
        {"Klee", {"klee"}},
        {"KleeBlossomingStarlight", {"kleeblossomingstarlight"}},
        {"Lisa", {"lisa"}},
        {"LisaStudent", {"lisastudent"}},
        {"Mona", {"mona"}},
        {"MonaCN", {"monacn"}},
        {"Nilou", {"nilou"}},
        {"NilouBreeze", {"niloubreeze"}},
        {"Ningguang", {"ningguang"}},
        {"NingguangOrchid", {"ningguangorchid"}},
        {"Raiden", {"raiden", "shogun"}},
        {"Rosaria", {"rosaria"}},
        {"RosariaCN", {"rosariacn"}},
        {"Shenhe", {"shenhe"}},
        {"ShenheFrostFlower", {"shenhefrostflower"}},
        {"Xiangling", {"xiangling"}},
        {"XianglingCheer", {"xianglingcheer", "xianglingnewyear"}},
        {"Xingqiu", {"xingqiu"}},
        {"XingqiuBamboo", {"xingqiubamboo"}},
    };
    return rows;
}


static ModTypeId idOf(const std::string& name) {
    for (int raw = 0; raw < 200; ++raw) {
        std::optional<ModTypeId> id = ModTypeIdTools::getEnum(raw);
        if (id.has_value() && ModTypeIdTools::getName(*id) == name) {
            return *id;
        }
    }

    std::printf("  FAILED: no ModTypeId goes by the name '%s'\n", name.c_str());
    ++failures;
    return ModTypeId::Amber;
}


static void testEveryKeywordRowMatchesPython() {
    std::printf("testEveryKeywordRowMatchesPython\n");

    check(expectedRows().size() == 43, "the oracle itself still has all 43 rows");

    for (const KeywordRow& row : expectedRows()) {
        checkKeywords(ModTypeIdTools::getSectionKeywords(idOf(row.name)), row.keywords,
                      std::string(row.name) + ": section keywords");
    }
}


static void testTheThreeTwoKeywordModTypes() {
    std::printf("testTheThreeTwoKeywordModTypes\n");

    checkKeywords(ModTypeIdTools::getSectionKeywords(ModTypeId::Raiden), {"raiden", "shogun"},
                  "Raiden carries its 'shogun' spelling too");
    checkKeywords(ModTypeIdTools::getSectionKeywords(ModTypeId::CherryHuTao), {"cherryhutao", "hutaocherry"},
                  "CherryHuTao carries both orderings");
    checkKeywords(ModTypeIdTools::getSectionKeywords(ModTypeId::XianglingCheer),
                  {"xianglingcheer", "xianglingnewyear"}, "XianglingCheer carries its new-year spelling");
}


static void testTargetOnlyIdsHaveNoKeywords() {
    std::printf("testTargetOnlyIdsHaveNoKeywords\n");

    for (ModTypeId boss : {ModTypeId::RaidenBoss, ModTypeId::ArlecchinoBoss}) {
        check(ModTypeIdTools::getSectionKeywords(boss).empty(),
              ModTypeIdTools::getName(boss) + " has no keywords -- nothing classifies a .ini file AS it");
    }
}


static void testGlobalClassifierArrivesPopulated() {
    std::printf("testGlobalClassifierArrivesPopulated\n");

    AGRC::IniClassifier& classifier = AGRC::GlobalIniClassifiers::classifier();

    AGRC::IniClassifyStats stats = classifier.classify("[TextureOverrideRaidenBody]\nhash = abc123\n");

    check(stats.isMod, "a TextureOverride section marks the .ini file as a mod");
    check(stats.modType.find(static_cast<int>(ModTypeId::Raiden)) != stats.modType.end(),
          "the global classifier identifies Raiden by section name -- it used to identify nothing at all");
}


static void testMaximalMatchDisambiguatesOverlappingNames() {
    std::printf("testMaximalMatchDisambiguatesOverlappingNames\n");

    AGRC::IniClassifier& classifier = AGRC::GlobalIniClassifiers::classifier();

    // The whole reason the pure-Python builder needs a negative lookahead per keyword. Here the
    // longest keyword simply wins, so these have to come out as the SPECIFIC mod type, never the
    // shorter one it contains.
    struct Overlap {
        const char* sectionName;
        ModTypeId expected;
        ModTypeId notExpected;
    };

    const std::vector<Overlap> overlaps = {
        {"[TextureOverrideAmberCNBody]", ModTypeId::AmberCN, ModTypeId::Amber},
        {"[TextureOverrideJeanSeaBody]", ModTypeId::JeanSea, ModTypeId::Jean},
        {"[TextureOverrideJeanCNBody]", ModTypeId::JeanCN, ModTypeId::Jean},
        {"[TextureOverrideKleeBlossomingStarlightBody]", ModTypeId::KleeBlossomingStarlight, ModTypeId::Klee},
        {"[TextureOverrideXingqiuBambooBody]", ModTypeId::XingqiuBamboo, ModTypeId::Xingqiu},
        {"[TextureOverrideNingguangOrchidBody]", ModTypeId::NingguangOrchid, ModTypeId::Ningguang},
    };

    for (const Overlap& overlap : overlaps) {
        AGRC::IniClassifyStats stats = classifier.classify(std::string(overlap.sectionName) + "\n");

        check(stats.modType.find(static_cast<int>(overlap.expected)) != stats.modType.end(),
              std::string(overlap.sectionName) + " classifies as " + ModTypeIdTools::getName(overlap.expected));
        check(stats.modType.find(static_cast<int>(overlap.notExpected)) == stats.modType.end(),
              std::string(overlap.sectionName) + " does NOT also classify as "
                  + ModTypeIdTools::getName(overlap.notExpected));
    }

    // ...and the plain name still resolves to the plain mod type.
    AGRC::IniClassifyStats plain = classifier.classify("[TextureOverrideAmberBody]\n");
    check(plain.modType.find(static_cast<int>(ModTypeId::Amber)) != plain.modType.end(),
          "a plain Amber section still classifies as Amber");
}


static void testEveryModTypeIsReachable() {
    std::printf("testEveryModTypeIsReachable\n");

    // Every registered mod type has to be findable from a section name built out of its own first
    // keyword -- a keyword registered but unreachable would be a silent hole in the table.
    AGRC::IniClassifier& classifier = AGRC::GlobalIniClassifiers::classifier();

    int unreachable = 0;
    for (const KeywordRow& row : expectedRows()) {
        ModTypeId id = idOf(row.name);
        std::string section = "[TextureOverride" + row.keywords[0] + "Body]\n";

        AGRC::IniClassifyStats stats = classifier.classify(section);
        if (stats.modType.find(static_cast<int>(id)) == stats.modType.end()) {
            std::printf("  FAILED: '%s' does not classify as %s\n", section.c_str(), row.name);
            ++failures;
            ++unreachable;
        }
    }

    std::printf("  (%zu mod types checked, %d unreachable)\n", expectedRows().size(), unreachable);
}


static void testHiddenSectionsStillClassify() {
    std::printf("testHiddenSectionsStillClassify\n");

    AGRC::IniClassifier& classifier = AGRC::GlobalIniClassifiers::classifier();

    // What a .ini file looks like after a fix ran with 'hideOrig': every touched section line is
    // prefixed with IniKeywords::HideOriginalComment. Re-classifying one has to see past that --
    // otherwise the section reads as an ordinary comment and the whole file classifies as no mod
    // type at all, which is exactly what happens on a second run over an already-fixed mod.
    const std::string marker = AGRC::IniKeywords::HideOriginalComment;

    AGRC::IniClassifyStats hidden =
        classifier.classify(marker + "[TextureOverrideRaidenBody]\n" + marker + "hash = abc123\n");

    check(hidden.modType.find(static_cast<int>(ModTypeId::Raiden)) != hidden.modType.end(),
          "a section hidden by a previous fix still classifies as Raiden");
    check(hidden.isMod, "...and the file is still recognised as a mod");

    // The marker also turns up mid-line, and with whitespace around it.
    AGRC::IniClassifyStats spaced =
        classifier.classify("   " + marker + "   [TextureOverrideAmberCNBody]\n");
    check(spaced.modType.find(static_cast<int>(ModTypeId::AmberCN)) != spaced.modType.end(),
          "the marker is stripped before the whitespace strip, so a padded hidden section still classifies");

    // A genuine comment that is NOT the hide marker stays a comment.
    AGRC::IniClassifyStats realComment = classifier.classify(";[TextureOverrideRaidenBody]\n");
    check(realComment.modType.find(static_cast<int>(ModTypeId::Raiden)) == realComment.modType.end(),
          "an ordinary commented-out section is still ignored -- only the hide marker is stripped");
}


static void testClassifierIsStillASingleton() {
    std::printf("testClassifierIsStillASingleton\n");

    check(&AGRC::GlobalIniClassifiers::classifier() == &AGRC::GlobalIniClassifiers::classifier(),
          "classifier() returns the same instance on every call");
}


int main() {
    testEveryKeywordRowMatchesPython();
    testTheThreeTwoKeywordModTypes();
    testTargetOnlyIdsHaveNoKeywords();
    testGlobalClassifierArrivesPopulated();
    testMaximalMatchDisambiguatesOverlappingNames();
    testEveryModTypeIsReachable();
    testHiddenSectionsStillClassify();
    testClassifierIsStillASingleton();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
