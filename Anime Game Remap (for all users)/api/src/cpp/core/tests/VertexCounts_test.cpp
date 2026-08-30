// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::VertexCounts
// (model/assets/VertexCounts.h) and the ModType::vertexCounts attribute it
// backs.
//
// The third of the ModType asset tables, after Hashes_test.cpp and
// Indices_test.cpp -- but NOT simply a third copy of them. VertexCounts differs
// from those two in three ways, all inherited from the pure-Python original:
//   * it derives from ModDictAssets, not ModMappedAssets, so there is no
//     fix-from/fix-to adjacency list and therefore no getMap()/hasFrom()
//   * its value type is int, not std::string
//   * its data (data/VertexCountData.h) was generated for this port rather than
//     already existing in C++ the way HashData/IndexData did, so this test
//     asserts the row count against the live Python dict's own 43
//
// Covers:
//   * A default-constructed VertexCounts is FULLY POPULATED, with 3 index
//     columns (version, name, component) and the version at position 0
//   * Every shipped row carries an EMPTY component, so lookups pass "" for it --
//     a real key value, not a "missing" marker
//   * Real lookups against real shipped data, returning ints, including
//     inclusive floor-matching across versions and the later-version rows
//   * A mod with no row does not resolve
//   * ModType::vertexCounts defaults to a populated table, is per-ModType, and
//     is shared when a ModType is copied -- the same ownership rules as
//     ModType::hashes/indices
//   * All three asset tables coexist on one ModType, each defaulting
//     independently
//
// Same build story as the other core tests -- link the already-built static lib
// (`cd cbuild && ninja AGRemapCore`):
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      VertexCounts_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags
// are load-bearing. Copy libz3.dll next to test.exe before running.
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/assets/VertexCounts.h"

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/VertexCountData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/strategies/ModType.h"

#include <cstdio>
#include <memory>
#include <optional>
#include <set>
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

Version ver(const std::string& raw) {
    return *Version::parse(raw);
}

std::string nameOf(ModTypeId id) {
    return ModTypeIdTools::getName(id);
}

// ---------------------------------------------------------------------------

void testPrePopulated() {
    std::printf("\n== a bare VertexCounts is fully populated ==\n");

    VertexCounts counts;
    check(counts.size() > 0, "a default-constructed VertexCounts is not empty");
    check(counts.size() == Data::getVertexCountDataRows().size(),
          "it holds exactly as many rows as the VertexCountData table it is built from");

    // Row count taken straight from the live pure-Python VertexCountData dict, so a row dropped or
    // duplicated during the extraction shows up here.
    check(counts.size() == 43, "43 rows, matching the pure-Python VertexCountData dict");

    // Same depth as Hashes now (3), one shallower than Indices (4).
    check(counts.getTotalIndices() == 3, "3 index columns (version, name, component)");
    check(counts.getVersionIndexPos() == 0, "with the version at position 0");

    VertexCounts other;
    check(other.size() == counts.size(), "a second instance holds the same rows");

    // The component column exists but is unused so far -- pin that, so the day a real
    // component-specific count is added this test says so rather than silently passing.
    bool allComponentsEmpty = true;
    counts.forEachEntry([&](const std::vector<std::string>& nonVersion, const Version&, const int&) {
        if (nonVersion.size() != 2 || !nonVersion[1].empty()) {
            allComponentsEmpty = false;
        }
    });
    check(allComponentsEmpty, "every shipped row has exactly two non-version keys, with an empty component");
}

void testVersionCoverage() {
    std::printf("\n== version coverage ==\n");

    std::set<std::string> versions;
    VertexCounts().forEachEntry([&](const std::vector<std::string>&, const Version& v, const int&) {
        versions.insert(v.toString());
    });

    const std::set<std::string> expected = {"4.0", "4.4", "4.6", "4.8", "5.3"};
    check(versions == expected, "covers exactly the 5 versions the Python dict lists");
}

void testLookups() {
    std::printf("\n== lookups against real shipped data ==\n");

    VertexCounts counts;

    // Straight out of VertexCountData.py: Amber @4.0 -> 10406, AmberCN @4.0 -> 10514.
    std::optional<int> amber = counts.get({nameOf(ModTypeId::Amber), ""}, ver("4.0"), false);
    check(amber.has_value(), "a known mod name at its own version resolves");
    check(amber.has_value() && *amber == 10406, "and returns the exact count -- as an int, not a string");

    std::optional<int> amberCN = counts.get({nameOf(ModTypeId::AmberCN), ""}, ver("4.0"), false);
    check(amberCN.has_value() && *amberCN == 10514, "a second mod resolves independently");

    // Later-version rows.
    std::optional<int> arlecchino = counts.get({nameOf(ModTypeId::Arlecchino), ""}, ver("4.6"), false);
    check(arlecchino.has_value() && *arlecchino == 22510, "a 4.6-only mod resolves at 4.6");

    std::optional<int> cherry = counts.get({nameOf(ModTypeId::CherryHuTao), ""}, ver("5.3"), false);
    check(cherry.has_value(), "a 5.3-only mod resolves at 5.3");

    // Inclusive floor-matching, same as the sibling tables.
    std::optional<int> later = counts.get({nameOf(ModTypeId::Amber), ""}, ver("5.7"), false);
    check(later.has_value() && *later == 10406, "the 4.0 row still floor-matches at a much later version");

    // Asking below a mod's earliest row does NOT miss -- ModDictAssets::get falls back to the
    // oldest row rather than returning nothing (its documented "smallest available value" rule,
    // and the same behaviour the builder tests assert). So Arlecchino, whose only row is 4.6,
    // still answers at 4.0.
    std::optional<int> tooEarly = counts.get({nameOf(ModTypeId::Arlecchino), ""}, ver("4.0"), false);
    check(tooEarly.has_value() && *tooEarly == 22510,
          "a version older than a mod's earliest row resolves to that oldest row, not to nothing");

    check(counts.get({nameOf(ModTypeId::Amber), ""}, std::nullopt, false).has_value(),
          "a nullopt version resolves to the latest listed row");
    check(!counts.get({"NotAMod", ""}, ver("4.0"), false).has_value(), "an unknown mod name does not resolve");
}

void testModTypeAttribute() {
    std::printf("\n== ModType::vertexCounts ==\n");

    ModType bare(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber");

    check(bare.vertexCounts != nullptr, "a ModType built with no vertexCounts still gets some");
    check(bare.vertexCounts->size() > 0, "and they are fully populated, not an empty table");

    std::optional<int> own = bare.vertexCounts->get({bare.name, ""}, ver("4.0"), false);
    check(own.has_value() && *own == 10406,
          "so looking up the mod type's own name works straight off a default-constructed ModType");

    // All three asset tables now sit side by side on the base class.
    check(bare.hashes != nullptr && bare.hashes->getRepo().size() > 0, "hashes is still there beside it");
    check(bare.indices != nullptr && bare.indices->getRepo().size() > 0, "and indices too");

    // Per-ModType by default, matching the original's own per-ModType VertexCounts().
    ModType second(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Jean), "Jean");
    check(second.vertexCounts != bare.vertexCounts, "two default ModTypes each get their own VertexCounts");

    ModType copy = bare;
    check(copy.vertexCounts == bare.vertexCounts, "copying a ModType shares its vertexCounts rather than cloning them");

    // Explicit sharing is opt-in, and does not disturb the other two tables' defaults.
    auto shared = std::make_shared<VertexCounts>();
    ModType a(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber",
              std::vector<std::string>{}, /*hashes*/ nullptr, /*indices*/ nullptr, shared);
    ModType b(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Jean), "Jean",
              std::vector<std::string>{}, /*hashes*/ nullptr, /*indices*/ nullptr, shared);
    check(a.vertexCounts == shared && a.vertexCounts == b.vertexCounts,
          "an explicitly passed VertexCounts is used as-is and genuinely shared");
    check(a.hashes != b.hashes && a.indices != b.indices,
          "while their defaulted hashes and indices stay independent");

    // GIBuilder passes nullptr for all three.
    ModType amber = GIBuilder::amber();
    ModType jean = GIBuilder::jean();
    check(amber.vertexCounts != nullptr && amber.vertexCounts->size() > 0, "GIBuilder mod types get populated vertexCounts");
    check(amber.vertexCounts != jean.vertexCounts, "and each GI mod type gets its own");

    // Every GI mod type should have a row of its own -- the table was built from the same 43.
    check(amber.vertexCounts->get({amber.name, ""}, std::nullopt, false).has_value(), "Amber has a row");
    check(jean.vertexCounts->get({jean.name, ""}, std::nullopt, false).has_value(), "Jean has a row");
}

}  // namespace

int main() {
    testPrePopulated();
    testVersionCoverage();
    testLookups();
    testModTypeAttribute();

    std::printf("\n%s (%d failure(s))\n", (failures == 0 ? "ALL PASSED" : "FAILURES"), failures);
    return (failures == 0) ? 0 : 1;
}
