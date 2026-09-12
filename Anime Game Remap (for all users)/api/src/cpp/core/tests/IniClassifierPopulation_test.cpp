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
//   * THE HASHES, which the GI population passed as {} until 2026-09-12 --
//     that a mod type is reachable by its ib hash alone, and that the
//     hashes outvote a section name that says something else (the
//     LisaStudent2 case: every section named after 'lisa', every hash
//     LisaStudent's).
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
#include "AGRemapCore/data/HashData.h"
#include "AGRemapCore/model/strategies/ModType.h"
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


static void testAModTypeIsReachableByItsIbHashAlone() {
    std::printf("testAModTypeIsReachableByItsIbHashAlone\n");

    AGRC::IniClassifier& classifier = AGRC::GlobalIniClassifiers::classifier();

    // Every GI mod type the classifier registers AND that HashData files an 'ib' under
    // must be nameable from that hash with no section name in the file at all. Driven off
    // the data rather than a retyped list, so a renamed character or a new one is covered
    // the day it lands -- and so that a name the two tables spell differently, which would
    // silently leave that character with no hashes, fails here instead of in a user's run.
    int checked = 0;

    for (const AGRC::ModType& modType : AGRC::GlobalModTypes::all()) {
        std::optional<ModTypeId> modTypeId = ModTypeIdTools::getEnum(modType.modTypeId);
        if (!modTypeId.has_value() || ModTypeIdTools::getSectionKeywords(*modTypeId).empty()) {
            continue;
        }

        std::string ibHash;
        for (const std::pair<std::vector<std::string>, std::string>& row : AGRC::Data::getHashDataRows()) {
            if (row.first.size() >= 3 && row.first[1] == modType.name && row.first[2] == "ib") {
                ibHash = row.second;
                break;
            }
        }

        if (ibHash.empty()) {
            continue;
        }

        checked++;
        AGRC::IniClassifyStats stats = classifier.classify("hash = " + ibHash + "\n");
        check(stats.modType.find(modType.modTypeId) != stats.modType.end(),
              modType.name + " is reachable by its ib hash " + ibHash + " alone");
    }

    // The count itself is load-bearing: a lookup that quietly matched nothing would leave
    // every assertion above unexecuted and this function passing.
    check(checked >= 40, "at least 40 mod types were actually checked, not zero");
}


static void testHashesOutvoteASectionNameThatDisagrees() {
    std::printf("testHashesOutvoteASectionNameThatDisagrees\n");

    AGRC::IniClassifier& classifier = AGRC::GlobalIniClassifiers::classifier();

    // A real mod, reduced: LisaStudent2 in the maintainer's folder is built on LisaStudent's
    // model and has every section named after 'lisa', because that is what its author called
    // the files. On section names alone it classifies as Lisa, the fix then runs Lisa ->
    // LisaStudent, and every reverse hash lookup fails and writes 'HashNotFound'.
    //
    // f30eece6 is LisaStudent's 4.3 ib, bfca9d94 her 4.1 draw_vb. Five name votes (1 each)
    // against four hash votes (2 each), so the hashes win 8 to 5.
    const std::string ini =
        "[TextureOverridelisaIB]\nhash = f30eece6\n"
        "[TextureOverridelisaHead]\nhash = f30eece6\n"
        "[TextureOverridelisaBody]\nhash = f30eece6\n"
        "[TextureOverridelisaVertexLimitRaise]\nhash = bfca9d94\n"
        "[TextureOverridelisaTexcoord]\n";

    AGRC::IniClassifyStats stats = classifier.classify(ini);

    check(stats.modType.find(static_cast<int>(ModTypeId::LisaStudent)) != stats.modType.end(),
          "a mod whose sections say 'lisa' but whose hashes say LisaStudent classifies as LisaStudent");
    check(stats.modType.find(static_cast<int>(ModTypeId::Lisa)) == stats.modType.end(),
          "...and NOT also as Lisa -- the hash weight is meant to break the tie, not join it");

    // The other direction has to keep working, or this would have traded one
    // misclassification for another: a real Lisa mod agrees with itself on both signals.
    // 518a6840 is Lisa's 4.3 ib, e6af2c6d her 4.1 draw_vb.
    AGRC::IniClassifyStats lisa = classifier.classify(
        "[TextureOverrideLisaIB]\nhash = 518a6840\n"
        "[TextureOverrideLisaBody]\nhash = 518a6840\n"
        "[TextureOverrideLisaVertexLimitRaise]\nhash = e6af2c6d\n");

    check(lisa.modType.find(static_cast<int>(ModTypeId::Lisa)) != lisa.modType.end(),
          "a genuine Lisa mod still classifies as Lisa");
    check(lisa.modType.find(static_cast<int>(ModTypeId::LisaStudent)) == lisa.modType.end(),
          "...and not as LisaStudent");

    // AND THE ALREADY-FIXED CASE, which is the one that could have gone badly. A fixed Lisa
    // mod carries LisaStudent's hashes inside its RemapFix sections; if those voted, every
    // fixed mod would reclassify as its own target the second time it was run over. readLine
    // skips a 'hash =' inside a Remap-named section, which is what stops that.
    AGRC::IniClassifyStats fixed = classifier.classify(
        "[TextureOverrideLisaIB]\nhash = 518a6840\n"
        "[TextureOverrideLisaBody]\nhash = 518a6840\n"
        "[TextureOverrideLisaBodyLisaStudentRemapFix]\nhash = f30eece6\n"
        "[TextureOverrideLisaLisaStudentRemapIB]\nhash = f30eece6\n");

    check(fixed.modType.find(static_cast<int>(ModTypeId::Lisa)) != fixed.modType.end(),
          "an already-fixed Lisa mod still classifies as Lisa");
    check(fixed.modType.find(static_cast<int>(ModTypeId::LisaStudent)) == fixed.modType.end(),
          "...and the target hashes its own fix wrote in do not vote");
    check(fixed.isFixed, "...and it is still seen as already fixed");
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
    testAModTypeIsReachableByItsIbHashAlone();
    testHashesOutvoteASectionNameThatDisagrees();
    testClassifierIsStillASingleton();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
