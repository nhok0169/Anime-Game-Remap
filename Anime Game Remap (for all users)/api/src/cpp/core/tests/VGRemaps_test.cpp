// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::VGRemaps
// (model/assets/VGRemaps.h), AGRemapCore::ModDataAssets, and the
// ModType::vgRemaps attribute they back.
//
// The last of the four ModType asset tables, and structurally the odd one out:
//   * it is built on ModAssets, NOT ModDictAssets, because it has TWO version
//     columns (fromVersion at 0 and toVersion at 3) -- the only table here that
//     ModDictAssets cannot express
//   * six index columns total: fromVersion, fromChar, fromComp, toVersion,
//     toChar, toComp -- so get() takes FOUR non-version values and TWO versions
//   * its value is a whole VGRemap object, not a scalar
//   * ModType::vgRemaps falls back to the SHARED ModDataAssets::vgRemaps, unlike
//     hashes/indices/vertexCounts which each get a fresh table. That asymmetry is
//     upstream (the pure-Python ModType defaults to ModDataAssets.VGRemaps.value)
//     and is asserted here so it cannot drift silently
//
// Covers:
//   * A default-constructed VGRemaps is FULLY POPULATED: 52 rows, matching the
//     live pure-Python vgRemapDataBuilder.build() output
//   * Its shape: 6 columns, version flags at 0 and 3
//   * Real lookups against real shipped remaps, keyed on both versions at once,
//     returning VGRemap objects whose contents match the Python data
//   * A remap that does not exist does not resolve
//   * ModType::vgRemaps defaults to the shared table -- two default ModTypes get
//     the SAME instance, unlike the other three attributes
//   * An explicitly passed VGRemaps is used as-is
//
// Same build story as the other core tests -- link the already-built static lib
// (`cd cbuild && ninja AGRemapCore`):
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      VGRemaps_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags
// are load-bearing. Copy libz3.dll next to test.exe before running.
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/assets/VGRemaps.h"

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/ModDataAssets.h"
#include "AGRemapCore/data/VGRemapData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/strategies/ModType.h"

#include <cstdio>
#include <memory>
#include <optional>
#include <string>
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

std::optional<Version> ver(const std::string& raw) {
    return Version::parse(raw);
}

std::string nameOf(ModTypeId id) {
    return ModTypeIdTools::getName(id);
}

// The four non-version values, in their relative order: fromChar, fromComp, toChar, toComp.
std::vector<std::optional<std::string>> keys(const std::string& fromChar, const std::string& toChar) {
    return {fromChar, std::string(""), toChar, std::string("")};
}

// ---------------------------------------------------------------------------

void testPrePopulated() {
    std::printf("\n== a bare VGRemaps is fully populated ==\n");

    VGRemaps remaps;
    check(remaps.getTotalIndices() == 6, "6 index columns");
    check(remaps.getVersionColumnCount() == 2, "TWO of them are version columns -- the only table like this");
    check(remaps.getNonVersionColumnCount() == 4, "leaving four non-version values to query with");

    check(Data::getVGRemapDataRows().size() == 52,
          "52 rows, matching the live pure-Python vgRemapDataBuilder.build() output");
}

void testRealLookups() {
    std::printf("\n== lookups against real shipped remaps ==\n");

    VGRemaps remaps;

    // Straight out of the Python data's very first entry:
    //   1.0 / Amber / "" -> 4.0 / AmberCN / ""   (78 index pairs, 0 -> 7 and 1 -> 6 first)
    std::optional<VGRemap> amberToCN = remaps.get(keys(nameOf(ModTypeId::Amber), nameOf(ModTypeId::AmberCN)),
                                                   {ver("1.0"), ver("4.0")}, false);
    check(amberToCN.has_value(), "a known from/to pair resolves, keyed on both versions at once");
    check(amberToCN.has_value() && amberToCN->getRemap().size() == 78, "and carries all 78 index pairs");
    check(amberToCN.has_value() && amberToCN->getRemap().at(0) == 7, "with 0 -> 7, straight from the Python data");
    check(amberToCN.has_value() && amberToCN->getRemap().at(1) == 6, "and 1 -> 6");
    check(amberToCN.has_value() && amberToCN->getMaxIndex().has_value() && *amberToCN->getMaxIndex() == 77,
          "getMaxIndex reflects the loaded remap");

    // The reverse direction is a separate row with different contents -- proving both the fromChar
    // and toChar columns really participate in the key.
    std::optional<VGRemap> cnToAmber = remaps.get(keys(nameOf(ModTypeId::AmberCN), nameOf(ModTypeId::Amber)),
                                                   {ver("4.0"), ver("4.0")}, false);
    check(cnToAmber.has_value(), "the reverse direction resolves too");
    check(cnToAmber.has_value() && cnToAmber->getRemap().at(0) == 15, "and is a genuinely different remap (0 -> 15)");

    // A pair that is not in the table at all.
    check(!remaps.get(keys("NotAMod", nameOf(ModTypeId::Amber)), {ver("4.0"), ver("4.0")}, false).has_value(),
          "an unknown mod name does not resolve");
}

void testSharedGlobal() {
    std::printf("\n== ModDataAssets::vgRemaps ==\n");

    check(ModDataAssets::vgRemaps() == ModDataAssets::vgRemaps(),
          "vgRemaps() returns the same shared instance every call");
    check(ModDataAssets::vgRemaps() != nullptr, "and it is never null");
    check(ModDataAssets::vgRemaps()->getTotalIndices() == 6, "the shared table has the same shape");
}

void testModTypeAttribute() {
    std::printf("\n== ModType::vgRemaps ==\n");

    ModType bare(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber");
    check(bare.vgRemaps != nullptr, "a ModType built with no vgRemaps still gets some");
    check(bare.vgRemaps->getTotalIndices() == 6, "and they are the real, populated table");

    std::optional<VGRemap> viaModType = bare.vgRemaps->get(keys(nameOf(ModTypeId::Amber), nameOf(ModTypeId::AmberCN)),
                                                            {ver("1.0"), ver("4.0")}, false);
    check(viaModType.has_value(), "so a real lookup works straight off a default-constructed ModType");

    // THE asymmetry: unlike hashes/indices/vertexCounts, the default here is the SHARED table.
    ModType second(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Jean), "Jean");
    check(bare.vgRemaps == ModDataAssets::vgRemaps(), "the default is the shared ModDataAssets table, not a fresh one");
    check(second.vgRemaps == bare.vgRemaps, "so two default ModTypes SHARE one vgRemaps");

    // ...whereas the other three do not. Pinned side by side so the difference is unmissable.
    check(second.hashes != bare.hashes, "while their hashes are still independent");
    check(second.indices != bare.indices, "and their indices");
    check(second.vertexCounts != bare.vertexCounts, "and their vertexCounts");

    // Explicit pass-through still works.
    auto own = std::make_shared<VGRemaps>();
    ModType custom(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber",
                   std::vector<std::string>{}, nullptr, nullptr, nullptr, own);
    check(custom.vgRemaps == own, "an explicitly passed VGRemaps is used as-is");
    check(custom.vgRemaps != ModDataAssets::vgRemaps(), "and is genuinely not the shared one");

    // GIBuilder passes nullptr, so all 43 GI mod types share the one table.
    ModType amber = GIBuilder::amber();
    ModType jean = GIBuilder::jean();
    check(amber.vgRemaps == ModDataAssets::vgRemaps(), "GIBuilder mod types use the shared table");
    check(amber.vgRemaps == jean.vgRemaps, "so every GI mod type shares one vgRemaps");
}

}  // namespace

int main() {
    testPrePopulated();
    testRealLookups();
    testSharedGlobal();
    testModTypeAttribute();

    std::printf("\n%s (%d failure(s))\n", (failures == 0 ? "ALL PASSED" : "FAILURES"), failures);
    return (failures == 0) ? 0 : 1;
}
