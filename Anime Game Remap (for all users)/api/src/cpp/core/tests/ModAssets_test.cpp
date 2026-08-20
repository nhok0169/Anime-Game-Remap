// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::ModAssets<K, T, KeyEqual> -- the
// C++ port of model/assets/ModAssets.py, the sibling to ModDictAssets/
// ModMappedAssets specifically for tables with more than one version column
// (this project's real VGRemaps resolves a fromVersion AND a toVersion
// independently and sequentially).
//
// Data shape mirrors VGRemaps: indices = [fromVersion(v), fromChar, fromComp,
// toVersion(v), toChar, toComp], with 4 rows spanning fromVersion in {1.0, 2.0}
// x toVersion in {4.0, 5.0}, all for the same (fromChar, fromComp, toChar,
// toComp) = ("A", "", "ACN", "") -- isolates the sequential version-column
// narrowing logic from any non-version filtering noise.
//
// Also covers the zero-version-column case (this project's real
// PositionEditors, which ends up with an empty versionIndices set once its
// custom `indices = ["from", "to"]` doesn't include "version" at all).
//
// This file has NO dependency on the project's build system (CMake/pybind11),
// Z3, utf8proc, ordered-map, or xxHash. Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /I <core>/include ^
//      ModAssets_test.cpp <core>/src/model/Version.cpp ^
//      /Fe:test.exe
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModAssets.h"
#include "AGRemapCore/model/assets/Row.h"

#include <cstdio>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using AGRemapCore::ModAssets;
using AGRemapCore::Row;
using AGRemapCore::Version;

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (condition) {
        std::printf("  [PASS] %s\n", description);
    } else {
        std::printf("  [FAIL] %s\n", description);
        ++failures;
    }
}

std::optional<Version> parseVersion(const std::string& raw) {
    return Version::parse(raw);
}

using VGRemapLikeAssets = ModAssets<std::string, std::string>;

Row<std::string, std::string> makeVGRow(const std::string& fromVersion, const std::string& toVersion, const std::string& value) {
    return Row<std::string, std::string>{{fromVersion, "A", "", toVersion, "ACN", ""}, value};
}

VGRemapLikeAssets makeVGRemapLikeAssets() {
    std::vector<Row<std::string, std::string>> rows = {
        makeVGRow("1.0", "4.0", "v1-4"),
        makeVGRow("1.0", "5.0", "v1-5"),
        makeVGRow("2.0", "4.0", "v2-4"),
        makeVGRow("2.0", "5.0", "v2-5"),
    };
    // indices: fromVersion(v), fromChar, fromComp, toVersion(v), toChar, toComp
    return VGRemapLikeAssets({true, false, false, true, false, false}, parseVersion, std::move(rows));
}

void testSequentialVersionNarrowing() {
    std::printf("-- ModAssets::get (sequential multi-version-column narrowing) --\n");

    VGRemapLikeAssets assets = makeVGRemapLikeAssets();
    check(assets.size() == 4, "size() reflects all 4 rows");
    check(assets.getTotalIndices() == 6, "getTotalIndices() == 6");
    check(assets.getVersionColumnCount() == 2, "getVersionColumnCount() == 2");
    check(assets.getNonVersionColumnCount() == 4, "getNonVersionColumnCount() == 4");

    std::vector<std::optional<std::string>> nonVersion = {std::string("A"), std::string(""), std::string("ACN"), std::string("")};

    // Both exact matches.
    auto exact = assets.get(nonVersion, {*Version::parse("2.0"), *Version::parse("5.0")});
    check(exact.has_value() && *exact == "v2-5", "both version columns exact match -> v2-5");

    // Both floor between available versions.
    auto floored = assets.get(nonVersion, {*Version::parse("1.5"), *Version::parse("4.5")});
    check(floored.has_value() && *floored == "v1-4", "fromVersion floors to 1.0, toVersion (within that narrowed set) floors to 4.0 -> v1-4");

    // Both latest (nullopt) -- toVersion's "latest" must be computed within the fromVersion=2.0
    // narrowed set, not globally, proving the sequential (not independent) resolution order.
    auto latest = assets.get(nonVersion, {std::nullopt, std::nullopt});
    check(latest.has_value() && *latest == "v2-5", "both nullopt -> latest fromVersion (2.0), then latest toVersion within that subset (5.0) -> v2-5");

    // fromVersion below everything (falls back to smallest = 1.0), toVersion above everything
    // within that narrowed subset (floors to the largest available there = 5.0).
    auto fallback = assets.get(nonVersion, {*Version::parse("0.5"), *Version::parse("10.0")});
    check(fallback.has_value() && *fallback == "v1-5", "fromVersion below everything falls back to smallest (1.0), toVersion above everything floors to largest available (5.0) -> v1-5");

    // Only fromVersion pinned, toVersion latest within that subset.
    auto mixedLatest = assets.get(nonVersion, {*Version::parse("1.0"), std::nullopt});
    check(mixedLatest.has_value() && *mixedLatest == "v1-5", "fromVersion=1.0 exact, toVersion=latest within the 1.0 subset -> v1-5");
}

void testErrorPaths() {
    std::printf("-- ModAssets::get (error paths) --\n");

    VGRemapLikeAssets assets = makeVGRemapLikeAssets();
    std::vector<std::optional<std::string>> nonVersion = {std::string("A"), std::string(""), std::string("ACN"), std::string("")};

    auto notFound = assets.get({std::string("nope"), std::string(""), std::string("ACN"), std::string("")}, {std::nullopt, std::nullopt}, false);
    check(!notFound.has_value(), "unknown non-version key with errorOnNotFound=false returns nullopt");

    bool threwNotFound = false;
    try {
        assets.get({std::string("nope"), std::string(""), std::string("ACN"), std::string("")}, {std::nullopt, std::nullopt}, true);
    } catch (const std::out_of_range&) {
        threwNotFound = true;
    }
    check(threwNotFound, "unknown non-version key with errorOnNotFound=true throws std::out_of_range");

    bool threwBadNonVersionSize = false;
    try {
        assets.get({std::string("A")}, {std::nullopt, std::nullopt});
    } catch (const std::invalid_argument&) {
        threwBadNonVersionSize = true;
    }
    check(threwBadNonVersionSize, "wrong-sized nonVersionVals throws std::invalid_argument");

    bool threwBadVersionSize = false;
    try {
        assets.get(nonVersion, {std::nullopt});
    } catch (const std::invalid_argument&) {
        threwBadVersionSize = true;
    }
    check(threwBadVersionSize, "wrong-sized versionVals throws std::invalid_argument");
}

void testAddRows() {
    std::printf("-- ModAssets::addRows --\n");

    VGRemapLikeAssets assets = makeVGRemapLikeAssets();
    std::vector<std::optional<std::string>> nonVersion = {std::string("A"), std::string(""), std::string("ACN"), std::string("")};

    // Overwrite an existing exact full-key row.
    assets.addRows({makeVGRow("2.0", "5.0", "v2-5-UPDATED")});
    check(assets.size() == 4, "overwriting an existing row doesn't change the total row count");
    auto updated = assets.get(nonVersion, {*Version::parse("2.0"), *Version::parse("5.0")});
    check(updated.has_value() && *updated == "v2-5-UPDATED", "overwrite replaced the value at the exact same full key");

    // Add a genuinely new row (a new toVersion).
    assets.addRows({makeVGRow("2.0", "6.0", "v2-6")});
    check(assets.size() == 5, "adding a new row grows the table");
    auto latest = assets.get(nonVersion, {*Version::parse("2.0"), std::nullopt});
    check(latest.has_value() && *latest == "v2-6", "the newly-added, later toVersion is now the latest");
}

void testZeroVersionColumns() {
    std::printf("-- ModAssets::get (zero version columns -- PositionEditors shape) --\n");

    std::vector<Row<std::string, std::string>> rows = {
        Row<std::string, std::string>{{"body", "head"}, "editor-body-to-head"},
        Row<std::string, std::string>{{"body", "dress"}, "editor-body-to-dress"},
    };
    ModAssets<std::string, std::string> assets({false, false}, parseVersion, std::move(rows));

    check(assets.getVersionColumnCount() == 0, "getVersionColumnCount() == 0");

    auto result = assets.get({std::string("body"), std::string("head")}, {});
    check(result.has_value() && *result == "editor-body-to-head", "pure non-version lookup works with an empty versionVals");

    auto notFound = assets.get({std::string("body"), std::string("nope")}, {}, false);
    check(!notFound.has_value(), "unknown non-version key returns nullopt (errorOnNotFound=false)");
}

}  // namespace

int main() {
    testSequentialVersionNarrowing();
    testErrorPaths();
    testAddRows();
    testZeroVersionColumns();

    if (failures == 0) {
        std::printf("\nAll checks passed.\n");
        return 0;
    }

    std::printf("\n%d check(s) FAILED.\n", failures);
    return 1;
}
