// -----------------------------------------------------------------------------
// Standalone regression test for the GI remap graph -- which mod types each mod
// type can be fixed onto. Covers:
//   * ModTypeIdTools::getHashRemapTargets / getIndexRemapTargets, row by row,
//     against all 43 entries dumped out of the LIVE pure-Python ModTypes
//     (ModTypes.getAll(), reading each mod type's hashes.map / indices.map).
//     The expectations below are GENERATED from that dump, not retyped from
//     ModTypeId.cpp's own switch -- restating the implementation would pin
//     nothing.
//   * The one asymmetry in the whole table: Raiden remaps by hash only. Its
//     pure-Python factory passes a bare Indices() with no map, where all 42
//     others pass the same map to both.
//   * The two ModTypeIds that are only ever remap TARGETS (RaidenBoss,
//     ArlecchinoBoss) -- neither is a source, and neither has a GIBuilder
//     factory.
//   * GIBuilder::all() building 43 mod types, each carrying that map on its own
//     Hashes/Indices. This is what "nullptr hashes/indices" used to lose:
//     ModMappedAssets::resolveToAssetNames returns nullopt for a from-name that
//     is not a key, so an empty map means NO targets, not all of them.
//   * GlobalModTypes::registerAll() filing all 43 into ModTypeIdTools, so
//     getModType resolves by id and findByName by name AND by alias.
//
// Needs the full static lib. Build AGRemapCore first ("cd cbuild && ninja
// AGRemapCore"), then compile as described in IniFile_resources_test.cpp.
// -----------------------------------------------------------------------------

#include <algorithm>
#include <cstdio>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/ModType.h"

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


static void checkNames(std::vector<std::string> got, std::vector<std::string> expected, const std::string& what) {
    // Order is not part of the contract -- ModMappedAssets treats these as a set of candidates.
    std::sort(got.begin(), got.end());
    std::sort(expected.begin(), expected.end());

    if (got == expected) {
        return;
    }

    std::printf("  FAILED: %s\n    expected: [%s]\n    got:      [%s]\n",
                what.c_str(), join(expected).c_str(), join(got).c_str());
    ++failures;
}


// The oracle: one row per mod type, generated from the live pure-Python ModTypes.
struct RemapRow {
    const char* name;
    std::vector<std::string> hashTargets;
    std::vector<std::string> indexTargets;
};


static const std::vector<RemapRow>& expectedRows() {
    static const std::vector<RemapRow> rows = {
        {"Amber", {"AmberCN"}, {"AmberCN"}},
        {"AmberCN", {"Amber"}, {"Amber"}},
        {"Arlecchino", {"ArlecchinoBoss"}, {"ArlecchinoBoss"}},
        {"Ayaka", {"AyakaSpringBloom"}, {"AyakaSpringBloom"}},
        {"AyakaSpringBloom", {"Ayaka"}, {"Ayaka"}},
        {"Barbara", {"BarbaraSummertime"}, {"BarbaraSummertime"}},
        {"BarbaraSummertime", {"Barbara"}, {"Barbara"}},
        {"CherryHuTao", {"HuTao"}, {"HuTao"}},
        {"Diluc", {"DilucFlamme"}, {"DilucFlamme"}},
        {"DilucFlamme", {"Diluc"}, {"Diluc"}},
        {"Fischl", {"FischlHighness"}, {"FischlHighness"}},
        {"FischlHighness", {"Fischl"}, {"Fischl"}},
        {"Ganyu", {"GanyuTwilight"}, {"GanyuTwilight"}},
        {"GanyuTwilight", {"Ganyu"}, {"Ganyu"}},
        {"HuTao", {"CherryHuTao"}, {"CherryHuTao"}},
        {"Jean", {"JeanCN", "JeanSea"}, {"JeanCN", "JeanSea"}},
        {"JeanCN", {"Jean", "JeanSea"}, {"Jean", "JeanSea"}},
        {"JeanSea", {"Jean", "JeanCN"}, {"Jean", "JeanCN"}},
        {"Kaeya", {"KaeyaSailwind"}, {"KaeyaSailwind"}},
        {"KaeyaSailwind", {"Kaeya"}, {"Kaeya"}},
        {"Keqing", {"KeqingOpulent"}, {"KeqingOpulent"}},
        {"KeqingOpulent", {"Keqing"}, {"Keqing"}},
        {"Kirara", {"KiraraBoots"}, {"KiraraBoots"}},
        {"KiraraBoots", {"Kirara"}, {"Kirara"}},
        {"Klee", {"KleeBlossomingStarlight"}, {"KleeBlossomingStarlight"}},
        {"KleeBlossomingStarlight", {"Klee"}, {"Klee"}},
        {"Lisa", {"LisaStudent"}, {"LisaStudent"}},
        {"LisaStudent", {"Lisa"}, {"Lisa"}},
        {"Mona", {"MonaCN"}, {"MonaCN"}},
        {"MonaCN", {"Mona"}, {"Mona"}},
        {"Nilou", {"NilouBreeze"}, {"NilouBreeze"}},
        {"NilouBreeze", {"Nilou"}, {"Nilou"}},
        {"Ningguang", {"NingguangOrchid"}, {"NingguangOrchid"}},
        {"NingguangOrchid", {"Ningguang"}, {"Ningguang"}},
        {"Raiden", {"RaidenBoss"}, {}},
        {"Rosaria", {"RosariaCN"}, {"RosariaCN"}},
        {"RosariaCN", {"Rosaria"}, {"Rosaria"}},
        {"Shenhe", {"ShenheFrostFlower"}, {"ShenheFrostFlower"}},
        {"ShenheFrostFlower", {"Shenhe"}, {"Shenhe"}},
        {"Xiangling", {"XianglingCheer"}, {"XianglingCheer"}},
        {"XianglingCheer", {"Xiangling"}, {"Xiangling"}},
        {"Xingqiu", {"XingqiuBamboo"}, {"XingqiuBamboo"}},
        {"XingqiuBamboo", {"Xingqiu"}, {"Xingqiu"}},
    };
    return rows;
}


static std::vector<std::string> namesOf(const std::vector<ModTypeId>& ids) {
    std::vector<std::string> result;
    result.reserve(ids.size());
    for (ModTypeId id : ids) {
        result.push_back(ModTypeIdTools::getName(id));
    }
    return result;
}


// Resolved by name rather than hardcoding enumerators, because an enumerator's spelling and the
// name it carries deliberately differ in one case (AyakaSpringbloom -> "AyakaSpringBloom").
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


static void testEveryRowMatchesPython() {
    std::printf("testEveryRowMatchesPython\n");

    check(expectedRows().size() == 43, "the oracle itself still has all 43 rows");

    for (const RemapRow& row : expectedRows()) {
        ModTypeId id = idOf(row.name);

        checkNames(namesOf(ModTypeIdTools::getHashRemapTargets(id)), row.hashTargets,
                   std::string(row.name) + ": hash remap targets");
        checkNames(namesOf(ModTypeIdTools::getIndexRemapTargets(id)), row.indexTargets,
                   std::string(row.name) + ": index remap targets");
    }
}


static void testRaidenRemapsByHashOnly() {
    std::printf("testRaidenRemapsByHashOnly\n");

    checkNames(namesOf(ModTypeIdTools::getHashRemapTargets(ModTypeId::Raiden)), {"RaidenBoss"},
               "Raiden remaps onto RaidenBoss by hash");
    check(ModTypeIdTools::getIndexRemapTargets(ModTypeId::Raiden).empty(),
          "Raiden has NO index remap targets -- the one asymmetry in the table");
}


static void testBossIdsAreTargetsOnly() {
    std::printf("testBossIdsAreTargetsOnly\n");

    for (ModTypeId boss : {ModTypeId::RaidenBoss, ModTypeId::ArlecchinoBoss}) {
        check(ModTypeIdTools::getHashRemapTargets(boss).empty(),
              ModTypeIdTools::getName(boss) + " is a remap target only, never a source (hashes)");
        check(ModTypeIdTools::getIndexRemapTargets(boss).empty(),
              ModTypeIdTools::getName(boss) + " is a remap target only, never a source (indices)");
    }

    // ...and so neither is built by GIBuilder.
    for (const AGRC::ModType& modType : AGRC::GIBuilder::all()) {
        check(modType.modTypeId != static_cast<int>(ModTypeId::RaidenBoss), "GIBuilder never builds RaidenBoss");
        check(modType.modTypeId != static_cast<int>(ModTypeId::ArlecchinoBoss), "GIBuilder never builds ArlecchinoBoss");
    }
}


static void testBuiltModTypesCarryTheirMap() {
    std::printf("testBuiltModTypesCarryTheirMap\n");

    std::vector<AGRC::ModType> built = AGRC::GIBuilder::all();
    check(built.size() == 43, "GIBuilder::all() builds all 43 mod types");

    for (const AGRC::ModType& modType : built) {
        const RemapRow* expected = nullptr;
        for (const RemapRow& row : expectedRows()) {
            if (modType.name == row.name) {
                expected = &row;
                break;
            }
        }

        if (expected == nullptr) {
            std::printf("  FAILED: GIBuilder built '%s', which the Python oracle doesn't have\n",
                        modType.name.c_str());
            ++failures;
            continue;
        }

        // The map is keyed by the mod type's OWN name.
        const auto& hashMap = modType.hashes->getMap();
        auto hashIt = hashMap.find(modType.name);
        checkNames(hashIt == hashMap.end() ? std::vector<std::string>{} : hashIt->second,
                   expected->hashTargets, modType.name + ": built Hashes map");

        const auto& indexMap = modType.indices->getMap();
        auto indexIt = indexMap.find(modType.name);
        checkNames(indexIt == indexMap.end() ? std::vector<std::string>{} : indexIt->second,
                   expected->indexTargets, modType.name + ": built Indices map");
    }
}


static void testEachBuiltModTypeOwnsItsTables() {
    std::printf("testEachBuiltModTypeOwnsItsTables\n");

    // Each call builds fresh tables -- a ModType's assets are mutable, so two mod types must never
    // share one. Same reasoning Hashes' own constructor records for copying its prototype repo.
    std::vector<AGRC::ModType> first = AGRC::GIBuilder::all();
    std::vector<AGRC::ModType> second = AGRC::GIBuilder::all();

    check(first[0].hashes.get() != second[0].hashes.get(), "two builds do not share one Hashes table");
    check(first[0].hashes.get() != first[1].hashes.get(), "two mod types do not share one Hashes table");
}


// The payoff: with a map in place, a mod type's hashes actually resolve to a target's. This is what
// "nullptr hashes/indices" cost -- ModMappedAssets::resolveToAssetNames returns nullopt for a
// from-name that isn't a key, so every replace/replaceAll came back empty no matter the input.
static void testHashesActuallyRemapNow() {
    std::printf("testHashesActuallyRemapNow\n");

    // The payoff, and the exact counts the pure-Python side produces for the same call -- measured
    // by running ModTypes.getAll()'s own Hashes.replaceAll over every one of its fromAssets, not
    // guessed. These count *replacements*, not hashes: a mod type with two targets contributes two
    // per hash it resolves. 'fromAssets' is the shared 532-row repo, so only the handful of rows belonging to
    // that character AND having a counterpart resolve; the rest legitimately map to nothing.
    struct Expectation {
        AGRC::ModType (*build)();
        const char* name;
        int remapped;
        std::size_t targetsPerHit;
    };

    const std::vector<Expectation> expectations = {
        {&AGRC::GIBuilder::raiden, "Raiden", 1, 1},
        {&AGRC::GIBuilder::amber, "Amber", 16, 1},
        // Jean fans out to BOTH JeanCN and JeanSea, so its 24 replacements come from 12 hashes.
        {&AGRC::GIBuilder::jean, "Jean", 24, 2},
    };

    for (const Expectation& expectation : expectations) {
        AGRC::ModType modType = expectation.build();
        std::vector<std::string> fromAssets = modType.hashes->getFromAssets();

        int remapped = 0;
        std::size_t widest = 0;
        for (const std::string& hash : fromAssets) {
            std::unordered_map<std::string, std::string> result =
                modType.hashes->replaceAll(hash, std::nullopt, {}, std::nullopt, {}, false);

            remapped += static_cast<int>(result.size());
            widest = std::max(widest, result.size());
        }

        check(remapped == expectation.remapped,
              std::string(expectation.name) + ": resolves the same number of hashes as pure Python (expected "
                  + std::to_string(expectation.remapped) + ", got " + std::to_string(remapped) + ")");
        check(widest == expectation.targetsPerHit,
              std::string(expectation.name) + ": each resolved hash fans out to "
                  + std::to_string(expectation.targetsPerHit) + " target(s)");
    }

    // A mod type with no map remaps onto nothing -- the same code path, so the difference really is
    // the map and not something incidental. This is exactly what every GI mod type used to do.
    AGRC::ModType raiden = AGRC::GIBuilder::raiden();
    AGRC::ModType boss(0, static_cast<int>(ModTypeId::RaidenBoss), "RaidenBoss");
    check(boss.hashes->replaceAll(raiden.hashes->getFromAssets()[0], std::nullopt, {}, std::nullopt, {}, false).empty(),
          "a mod type with an empty map remaps onto nothing");
}


static void testRegisterAllPopulatesTheRegistry() {
    std::printf("testRegisterAllPopulatesTheRegistry\n");

    ModTypeIdTools::clear();

    // Nothing resolves before the explicit call -- registration is deliberately not automatic.
    check(!ModTypeIdTools::getModType(static_cast<int>(ModTypeId::Raiden)).has_value(),
          "before registerAll(), the registry is empty");

    AGRC::GlobalModTypes::registerAll();

    std::optional<AGRC::ModType> raiden = ModTypeIdTools::getModType(static_cast<int>(ModTypeId::Raiden));
    check(raiden.has_value(), "after registerAll(), Raiden resolves by id");
    if (raiden.has_value()) {
        check(raiden->name == "Raiden", "...and it is the right mod type");
    }

    std::optional<ModTypeId> byName = ModTypeIdTools::findByName("Raiden");
    check(byName.has_value() && *byName == ModTypeId::Raiden, "findByName resolves a registered name");

    // Aliases go into the same lookup -- "Shogun" is one of Raiden's.
    std::optional<ModTypeId> byAlias = ModTypeIdTools::findByName("Shogun");
    check(byAlias.has_value() && *byAlias == ModTypeId::Raiden, "findByName resolves a registered alias");

    // Case-insensitively, and ignoring surrounding whitespace. Both are the pure-Python 'ModTypes'
    // behaviour this stands in for -- it files its DFA from 'name.lower()' and searches with
    // 'txt.lower().strip()' -- and the --help text promises it out loud: "The names/aliases for the
    // mod types are not case sensitive". RemapServiceCLI is what made it matter: a name typed on a
    // command line arrives in whatever case the user felt like.
    std::optional<ModTypeId> lowerName = ModTypeIdTools::findByName("raiden");
    check(lowerName.has_value() && *lowerName == ModTypeId::Raiden, "findByName ignores case on a name");

    std::optional<ModTypeId> upperAlias = ModTypeIdTools::findByName("SHOGUN");
    check(upperAlias.has_value() && *upperAlias == ModTypeId::Raiden, "and on an alias");

    std::optional<ModTypeId> padded = ModTypeIdTools::findByName("  Raiden  ");
    check(padded.has_value() && *padded == ModTypeId::Raiden, "and ignores surrounding whitespace");

    // The two target-only ids have no factory, so registerAll never files them.
    check(!ModTypeIdTools::getModType(static_cast<int>(ModTypeId::RaidenBoss)).has_value(),
          "registerAll does not register RaidenBoss -- nothing builds it");

    // Idempotent.
    AGRC::GlobalModTypes::registerAll();
    check(ModTypeIdTools::getModType(static_cast<int>(ModTypeId::Raiden)).has_value(),
          "registerAll() twice leaves the registry intact");

    ModTypeIdTools::clear();
}


int main() {
    testEveryRowMatchesPython();
    testRaidenRemapsByHashOnly();
    testBossIdsAreTargetsOnly();
    testBuiltModTypesCarryTheirMap();
    testEachBuiltModTypeOwnsItsTables();
    testHashesActuallyRemapNow();
    testRegisterAllPopulatesTheRegistry();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
