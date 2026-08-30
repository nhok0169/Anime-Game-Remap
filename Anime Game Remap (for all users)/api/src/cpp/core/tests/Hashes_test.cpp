// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::Hashes
// (model/assets/Hashes.h) and the ModType::hashes attribute it backs.
//
// Hashes is the core-side counterpart of the pure-Python `Hashes` (which now
// lives only as the pybind layer's PyHashes): a ModMappedAssets pre-populated
// with data/HashData.h's table, keyed (version, name, type). ModType::hashes
// mirrors the pure-Python ModType.hashes attribute, including its
// "if (hashes is None): hashes = Hashes()" default.
//
// Covers:
//   * A default-constructed Hashes is FULLY POPULATED, not empty -- the property
//     the Python default depends on, and the whole reason HashData had to move
//     into the core
//   * Its shape: 3 index columns with the version at position 0
//   * Real lookups against real shipped data, by (name, type) and version,
//     including inclusive floor-matching across versions
//   * hasFrom() recognises a hash that is actually in the table and rejects one
//     that is not
//   * The constructor's `map` argument reaches getMap()
//   * ModType::hashes defaults to a populated table, is per-ModType (two default
//     ModTypes do NOT share one), and IS shared when a ModType is copied or when
//     the same shared_ptr is handed to two ModTypes
//   * GIBuilder's mod types each get their own populated hashes
//
// Same build story as the builder tests -- link the already-built static lib
// (`cd cbuild && ninja AGRemapCore`):
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      Hashes_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags
// are load-bearing. Copy libz3.dll next to test.exe before running.
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/assets/Hashes.h"

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/HashData.h"
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
    std::printf("\n== a bare Hashes is fully populated ==\n");

    Hashes hashes;

    // The property the whole design rests on: Python's Hashes() ships with every hash, so
    // ModType's "no hashes given" default has to as well.
    check(hashes.getRepo().size() > 0, "a default-constructed Hashes is not empty");
    check(hashes.getRepo().size() == Data::getHashDataRows().size(),
          "it holds exactly as many rows as the HashData table it is built from");

    check(hashes.getRepo().getTotalIndices() == 3, "3 index columns (version, name, type)");
    check(hashes.getRepo().getVersionIndexPos() == 0, "with the version at position 0");

    // Two instances are independent tables (see ModType::hashes' second note), but built from the
    // same shared prototype, so they must agree.
    Hashes other;
    check(other.getRepo().size() == hashes.getRepo().size(), "a second instance holds the same rows");
}

void testRealLookups() {
    std::printf("\n== lookups against real shipped data ==\n");

    Hashes hashes;

    // Straight out of HashData.cpp's very first row: {"1.0", "Barbara", "draw_vb"} -> "f41c47cf".
    std::optional<std::string> barbara = hashes.get({"Barbara", "draw_vb"}, ver("1.0"), false);
    check(barbara.has_value(), "a known (name, type) pair at its own version resolves");
    check(barbara.has_value() && *barbara == "f41c47cf", "and returns the exact hash from the table");

    // Inclusive floor-match: the 1.0 row keeps applying at later versions unless superseded.
    std::optional<std::string> later = hashes.get({"Barbara", "draw_vb"}, ver("5.7"), false);
    check(later.has_value(), "the same pair still resolves at a much later version");

    // No version at all means "latest listed".
    check(hashes.get({"Barbara", "draw_vb"}, std::nullopt, false).has_value(),
          "a nullopt version resolves to the latest listed row");

    // A pair that genuinely is not in the table.
    check(!hashes.get({"NotAMod", "draw_vb"}, ver("1.0"), false).has_value(),
          "an unknown mod name does not resolve");

    // hasFrom asks the reverse question -- is this hash one we know about?
    check(hashes.hasFrom("f41c47cf"), "hasFrom recognises a hash that is in the table");
    check(!hashes.hasFrom("not-a-real-hash"), "and rejects one that is not");
}

void testMapArgument() {
    std::printf("\n== the map argument ==\n");

    Hashes bare;
    check(bare.getMap().empty(), "the map defaults to empty");

    Hashes withMap({{"aaaaaaaa", {"bbbbbbbb", "cccccccc"}}});
    check(withMap.getMap().size() == 1, "a supplied map reaches getMap");
    check(withMap.getMap().count("aaaaaaaa") == 1 && withMap.getMap().at("aaaaaaaa").size() == 2,
          "with its entries intact");
    check(withMap.getRepo().size() == bare.getRepo().size(),
          "and supplying a map does not disturb the pre-populated rows");
}

void testModTypeAttribute() {
    std::printf("\n== ModType::hashes ==\n");

    ModType bare(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber");
    check(bare.hashes != nullptr, "a ModType built with no hashes still gets some");
    check(bare.hashes->getRepo().size() > 0, "and they are fully populated, not an empty table");
    check(bare.hashes->get({"Barbara", "draw_vb"}, ver("1.0"), false).has_value(),
          "so a real lookup works straight off a default-constructed ModType");

    // Per-ModType by default, matching the original's own per-ModType Hashes().
    ModType second(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Jean), "Jean");
    check(second.hashes != bare.hashes, "two default ModTypes each get their own Hashes");

    // ...but copying one shares, the way an ordinary Python attribute reference would.
    ModType copy = bare;
    check(copy.hashes == bare.hashes, "copying a ModType shares its hashes rather than cloning them");

    // An explicitly shared table is shared, which is what makes that opt-in meaningful.
    auto shared = std::make_shared<Hashes>();
    ModType a(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber", std::vector<std::string>{}, shared);
    ModType b(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Jean), "Jean", std::vector<std::string>{}, shared);
    check(a.hashes == shared && b.hashes == shared, "an explicitly passed Hashes is used as-is");
    check(a.hashes == b.hashes, "and two ModTypes handed the same one genuinely share it");

    // GIBuilder passes nullptr, so every GI mod type gets its own populated table.
    ModType amber = GIBuilder::amber();
    ModType jean = GIBuilder::jean();
    check(amber.hashes != nullptr && amber.hashes->getRepo().size() > 0, "GIBuilder mod types get populated hashes");
    check(amber.hashes != jean.hashes, "and each GI mod type gets its own");
}

}  // namespace

int main() {
    testPrePopulated();
    testRealLookups();
    testMapArgument();
    testModTypeAttribute();

    std::printf("\n%s (%d failure(s))\n", (failures == 0 ? "ALL PASSED" : "FAILURES"), failures);
    return (failures == 0) ? 0 : 1;
}
