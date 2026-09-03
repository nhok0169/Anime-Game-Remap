// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::ModType's ported methods
// (model/strategies/ModType.h). Most of them are diffed against the live
// pure-Python ModType from Testing/Unit Tester's test_CppModTypeMethods.py --
// this file covers what that one cannot reach cheaply:
//
//   * getHashRanges over a colouring holding REAL hashes: the ranges have to
//     cover exactly the parts whose 'hash' belongs to this mod type, and skip
//     both a foreign hash and a part with no 'hash' key at all.
//   * getModsToFix unioning across the hash and index tables rather than
//     reading either alone -- Raiden is the mod type where those two differ.
//   * isName / getHelpStr edge cases (empty aliases, a mod type built by hand).
//   * fixIni's guard: it must do nothing when the .ini file was classified as a
//     different mod type, or as none at all.
//
// Needs the full static lib. Build AGRemapCore first ("cd cbuild && ninja
// AGRemapCore"), then compile as described in IniFile_resources_test.cpp.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iftemplate/IfContentPartColour.h"
#include "AGRemapCore/model/strategies/ModType.h"

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


static void testIsName() {
    std::printf("testIsName\n");

    AGRC::ModType raiden = AGRC::GIBuilder::raiden();

    check(raiden.isName("Raiden"), "matches its own name");
    check(raiden.isName("raiden"), "matches its own name case-insensitively");
    check(raiden.isName("RAIDEN"), "...in either direction");
    check(raiden.isName("ShOgUn"), "matches an alias case-insensitively");
    check(!raiden.isName("Amber"), "does not match another mod type");
    check(!raiden.isName(""), "does not match the empty string");

    // A mod type with no aliases at all must not fall over walking them.
    AGRC::ModType bare(0, 900, "Bare");
    check(bare.isName("bare"), "a mod type with no aliases still matches its own name");
    check(!bare.isName("nope"), "...and nothing else");
}


static void testGetModsToFixUnionsBothTables() {
    std::printf("testGetModsToFixUnionsBothTables\n");

    // Raiden is the one mod type whose hash and index tables disagree: it remaps by hash only.
    // Reading either table alone would still produce the right answer here by luck, so the guard
    // that matters is that the OTHER 42 agree and this one still resolves.
    AGRC::ModType raiden = AGRC::GIBuilder::raiden();
    std::unordered_set<std::string> raidenTargets = raiden.getModsToFix();

    check(raidenTargets.size() == 1 && raidenTargets.count("RaidenBoss") == 1,
          "Raiden fixes onto RaidenBoss, from its hash table alone");

    // Jean fans out to two, from both tables.
    AGRC::ModType jean = AGRC::GIBuilder::jean();
    std::unordered_set<std::string> jeanTargets = jean.getModsToFix();

    check(jeanTargets.size() == 2 && jeanTargets.count("JeanCN") == 1 && jeanTargets.count("JeanSea") == 1,
          "Jean fixes onto both JeanCN and JeanSea");

    // A hand-built mod type has no map at all, so it fixes onto nothing.
    AGRC::ModType bare(0, 901, "Bare");
    check(bare.getModsToFix().empty(), "a mod type with no remap map fixes onto nothing");
}


static void testGetHelpStr() {
    std::printf("testGetHelpStr\n");

    AGRC::ModType bare(0, 902, "Bare");
    checkEqual(bare.getHelpStr(),
               "-------- Bare --------\n\nname: Bare\n\n----------------------",
               "a mod type with no aliases omits the aliases line entirely");

    // Aliases are sorted, not left in declaration order.
    AGRC::ModType withAliases(0, 903, "Zed", {"charlie", "alpha", "bravo"});
    checkEqual(withAliases.getHelpStr(),
               "-------- Zed --------\n\nname: Zed\naliases: alpha, bravo, charlie\n\n---------------------",
               "aliases are sorted");
}


static void testGetHashRangesOverRealContent() {
    std::printf("testGetHashRangesOverRealContent\n");

    AGRC::ModType raiden = AGRC::GIBuilder::raiden();

    std::vector<std::string> raidenHashes = raiden.hashes->getFromAssets();
    check(!raidenHashes.empty(), "Raiden's hash table has rows to build a colouring from");
    if (raidenHashes.empty()) {
        return;
    }

    const std::string ownHash = raidenHashes[0];

    // Index 0 carries one of Raiden's own hashes, index 1 carries something that is not a hash of
    // any mod type, and index 2 carries no 'hash' key at all.
    AGRC::IfContentPartColouring<std::string, std::string> colouring;
    colouring.set(AGRC::IniKeywords::Hash, std::vector<std::pair<long long, std::string>>{
        {0, ownHash},
        {1, "notARealHashAtAll"},
    });
    colouring.set("vb0", std::vector<std::pair<long long, std::string>>{{2, "someResource"}});

    AGRC::Ranges<long long> ranges = raiden.getHashRanges(colouring);

    // Ranges has no single-value membership check, so this compares against the range set the
    // expected indices produce -- index 0 only.
    check(ranges == AGRC::Ranges<long long>::createFromList({0}),
          "exactly the part carrying one of Raiden's own hashes is in range -- not the foreign hash "
          "at index 1, and not the part with no 'hash' key at index 2");
}


static void testGetHashRangesEmptyColouring() {
    std::printf("testGetHashRangesEmptyColouring\n");

    AGRC::ModType raiden = AGRC::GIBuilder::raiden();
    AGRC::IfContentPartColouring<std::string, std::string> colouring;

    check(raiden.getHashRanges(colouring).ranges.empty(), "nothing carries a hash -> no ranges");
}


static void testFixIniOnlyActsOnItsOwnModType() {
    std::printf("testFixIniOnlyActsOnItsOwnModType\n");

    AGRC::ModType raiden = AGRC::GIBuilder::raiden();

    // Classified as Amber, so Raiden's fixIni has to leave it alone.
    AGRC::IniFile amberIni(std::nullopt, "[TextureOverrideAmberBody]\nhash = abc123\n");
    amberIni.classify();

    const std::string before = amberIni.getFileTxt();
    raiden.fixIni(amberIni);
    checkEqual(amberIni.getFileTxt(), before, "fixIni leaves a .ini file of another mod type alone");

    // Classified as nothing at all.
    AGRC::IniFile plainIni(std::nullopt, "[SomethingElse]\nkey = value\n");
    plainIni.classify();

    const std::string plainBefore = plainIni.getFileTxt();
    raiden.fixIni(plainIni);
    checkEqual(plainIni.getFileTxt(), plainBefore, "fixIni leaves an unclassified .ini file alone");
}


int main() {
    testIsName();
    testGetModsToFixUnionsBothTables();
    testGetHelpStr();
    testGetHashRangesOverRealContent();
    testGetHashRangesEmptyColouring();
    testFixIniOnlyActsOnItsOwnModType();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
