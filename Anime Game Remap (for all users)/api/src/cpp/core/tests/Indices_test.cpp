// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::Indices (model/assets/Indices.h)
// and the ModType::indices attribute it backs.
//
// The sibling of Hashes_test.cpp: Indices is a ModMappedAssets pre-populated
// from data/IndexData.h, and ModType::indices mirrors the pure-Python
// ModType.indices attribute -- which, like hashes, lives on ModType itself
// rather than in any game-specific subclass.
//
// The one shape difference from Hashes: 4 index columns
// (version, name, component, type) rather than 3, so lookups take THREE
// non-version values, and `component` is usually the empty string -- a real key
// value here, not a "missing" marker.
//
// Covers:
//   * A default-constructed Indices is FULLY POPULATED, with 4 index columns and
//     the version at position 0
//   * Real lookups against real shipped index data, including the empty-string
//     component and inclusive floor-matching across versions
//   * hasFrom() both ways, and that the constructor's `map` argument lands
//   * ModType::indices defaults to a populated table, is per-ModType, and is
//     shared when a ModType is copied -- the same ownership rules as
//     ModType::hashes
//   * GIBuilder's mod types each get their own populated indices
//
// Same build story as the other core tests -- link the already-built static lib
// (`cd cbuild && ninja AGRemapCore`):
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      Indices_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags
// are load-bearing. Copy libz3.dll next to test.exe before running.
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/assets/Indices.h"

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IndexData.h"
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

Version ver(const std::string& raw) {
    return *Version::parse(raw);
}

// ---------------------------------------------------------------------------

void testPrePopulated() {
    std::printf("\n== a bare Indices is fully populated ==\n");

    Indices indices;
    check(indices.getRepo().size() > 0, "a default-constructed Indices is not empty");
    check(indices.getRepo().size() == Data::getIndexDataRows().size(),
          "it holds exactly as many rows as the IndexData table it is built from");

    // The one shape difference from Hashes.
    check(indices.getRepo().getTotalIndices() == 4, "4 index columns (version, name, component, type)");
    check(indices.getRepo().getVersionIndexPos() == 0, "with the version at position 0");

    Indices other;
    check(other.getRepo().size() == indices.getRepo().size(), "a second instance holds the same rows");
    check(indices.getMap().empty(), "the map defaults to empty");

    Indices withMap({{"0", {"5670"}}});
    check(withMap.getMap().size() == 1, "a supplied map reaches getMap");
    check(withMap.getRepo().size() == indices.getRepo().size(), "and does not disturb the pre-populated rows");
}

void testLookups() {
    std::printf("\n== lookups against real shipped index data ==\n");

    Indices indices;

    // Straight out of IndexData.cpp's first rows: {"4.0", "Amber", "", "head"} -> "0" and
    // {"4.0", "Amber", "", "body"} -> "5670". Note the empty-string component is a real key.
    std::optional<std::string> head = indices.get({"Amber", "", "head"}, ver("4.0"), false);
    check(head.has_value(), "a known (name, component, type) triple resolves");
    check(head.has_value() && *head == "0", "and returns the exact index from the table");

    std::optional<std::string> body = indices.get({"Amber", "", "body"}, ver("4.0"), false);
    check(body.has_value() && *body == "5670", "a second row on the same mod resolves independently");

    check(indices.get({"Amber", "", "head"}, ver("5.7"), false).has_value(),
          "the 4.0 row still floor-matches at a much later version");
    check(!indices.get({"NotAMod", "", "head"}, ver("4.0"), false).has_value(),
          "an unknown mod name does not resolve");

    check(indices.hasFrom("5670"), "hasFrom recognises an index that is in the table");
    check(!indices.hasFrom("not-a-real-index"), "and rejects one that is not");
}

void testModTypeAttribute() {
    std::printf("\n== ModType::indices ==\n");

    ModType bare(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber");

    check(bare.indices != nullptr, "a ModType built with no indices still gets some");
    check(bare.indices->getRepo().size() > 0, "and they are fully populated, not an empty table");
    check(bare.indices->get({"Amber", "", "head"}, ver("4.0"), false).has_value(),
          "so a real lookup works straight off a default-constructed ModType");

    // hashes and indices sit side by side on the base class, exactly as in the pure-Python original.
    check(bare.hashes != nullptr && bare.hashes->getRepo().size() > 0,
          "hashes is still there beside it, also populated");

    // Per-ModType by default, matching the original's own per-ModType Indices().
    ModType second(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Jean), "Jean");
    check(second.indices != bare.indices, "two default ModTypes each get their own Indices");

    // ...but copying one shares, the way an ordinary Python attribute reference would.
    ModType copy = bare;
    check(copy.indices == bare.indices, "copying a ModType shares its indices rather than cloning them");
    check(copy.hashes == bare.hashes, "and shares its hashes too");

    // An explicitly shared table is shared, which is what makes that opt-in meaningful.
    auto shared = std::make_shared<Indices>();
    ModType a(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber",
              std::vector<std::string>{}, /*hashes*/ nullptr, shared);
    ModType b(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Jean), "Jean",
              std::vector<std::string>{}, /*hashes*/ nullptr, shared);
    check(a.indices == shared && a.indices == b.indices,
          "an explicitly passed Indices is used as-is and genuinely shared");
    check(a.hashes != b.hashes, "while their defaulted hashes stay independent");

    // GIBuilder passes nullptr for both, so every GI mod type gets its own populated tables.
    ModType amber = GIBuilder::amber();
    ModType jean = GIBuilder::jean();
    check(amber.indices != nullptr && amber.indices->getRepo().size() > 0, "GIBuilder mod types get populated indices");
    check(amber.indices != jean.indices, "and each GI mod type gets its own");
}

}  // namespace

int main() {
    testPrePopulated();
    testLookups();
    testModTypeAttribute();

    std::printf("\n%s (%d failure(s))\n", (failures == 0 ? "ALL PASSED" : "FAILURES"), failures);
    return (failures == 0) ? 0 : 1;
}
