// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::ModMappedAssets<K, T, ...> --
// Phase 2 of the ModDictAssets/ModMappedAssets port (see
// model/assets/ModMappedAssets.py).
//
// Data shape: a two-character (A, B) x two-type (x, y) x two-version (1.0,
// 2.0) asset table, deliberately constructed so "shared" is the SAME hash
// value for BOTH A.x and B.x at version 1.0 -- this is the exact shape that
// exposed a real, confirmed bug in the live pure-Python ModMappedAssets
// during development: its updateKeys() nested-dict stack traversal corrupts
// one of the two sibling candidates into NaN/NaN and silently drops it from
// the reverse index. This class's reverse index is built from
// ModDictAssets::forEachEntry's already-flat data instead (see
// ModMappedAssets.h's class-level note), which structurally can't hit that
// bug -- testFindsBothSharedCandidates below is the regression test proving
// both candidates really are reachable, not just one.
//
//   version  name  type  value
//   1.0      A     x     shared
//   1.0      A     y     a-y-1.0
//   1.0      B     x     shared
//   1.0      B     y     b-y-1.0
//   2.0      A     x     a-x-2.0
//   2.0      A     y     a-y-2.0
//   2.0      B     x     b-x-2.0
//   2.0      B     y     b-y-2.0
//
//   map: A -> [B]
//
// This file has NO dependency on the project's build system (CMake/pybind11),
// Z3, utf8proc, ordered-map, or xxHash. Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /I <core>/include ^
//      ModMappedAssets_test.cpp <core>/src/model/Version.cpp ^
//      /Fe:test.exe
//
// (g++/clang++ equivalent: swap /std:c++latest /EHsc /I /Fe: for
//  -std=c++23 -I ... -o test.exe)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModDictAssets.h"
#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/assets/Row.h"

#include <cstdio>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using AGRemapCore::ModDictAssets;
using AGRemapCore::ModMappedAssets;
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

using DictAssets = ModDictAssets<std::string, std::string>;
using MappedAssets = ModMappedAssets<std::string, std::string>;

Row<std::string, std::string> makeRow(const std::string& version, const std::string& name, const std::string& type, const std::string& value) {
    return Row<std::string, std::string>{{version, name, type}, value};
}

MappedAssets makeSampleAssets() {
    std::vector<Row<std::string, std::string>> rows = {
        makeRow("1.0", "A", "x", "shared"),
        makeRow("1.0", "A", "y", "a-y-1.0"),
        makeRow("1.0", "B", "x", "shared"),
        makeRow("1.0", "B", "y", "b-y-1.0"),
        makeRow("2.0", "A", "x", "a-x-2.0"),
        makeRow("2.0", "A", "y", "a-y-2.0"),
        makeRow("2.0", "B", "x", "b-x-2.0"),
        makeRow("2.0", "B", "y", "b-y-2.0"),
    };
    DictAssets repo(3, 0, parseVersion, std::move(rows));

    std::unordered_map<std::string, std::vector<std::string>> map = {{"A", {"B"}}};
    return MappedAssets(std::move(repo), std::move(map));
}

void testFindsBothSharedCandidates() {
    std::printf("-- ModMappedAssets::getKey (regression: both sibling candidates reachable) --\n");

    MappedAssets assets = makeSampleAssets();

    auto keyA = assets.getKey("shared", *Version::parse("1.0"), {"A", std::nullopt});
    check(keyA.has_value() && *keyA == std::vector<std::string>{"A", "x"}, "filtering by name=A finds the (A, x) candidate");

    auto keyB = assets.getKey("shared", *Version::parse("1.0"), {"B", std::nullopt});
    check(keyB.has_value() && *keyB == std::vector<std::string>{"B", "x"}, "filtering by name=B finds the (B, x) candidate -- NOT corrupted/dropped like the live pure-Python original");
}

void testGetKey() {
    std::printf("-- ModMappedAssets::getKey (version resolution + disambiguation) --\n");

    MappedAssets assets = makeSampleAssets();

    auto latest = assets.getKey("a-x-2.0", std::nullopt, {});
    check(latest.has_value() && *latest == std::vector<std::string>{"A", "x"}, "fromVersion=nullopt resolves to the latest available version for the asset");

    // getKey() deliberately only returns the resolved key, not the version it was resolved at
    // (matching the pure-Python original's contract exactly -- see ModMappedAssets.h's note on
    // GIMIParser.py's real destructuring dependency on this). Floor-vs-exact version resolution
    // is still exercised, just observed indirectly: "dual" resolves to a DIFFERENT key depending
    // on which version it's queried from, so which key comes back proves which version won.
    std::vector<Row<std::string, std::string>> dualRows = {
        makeRow("1.0", "A", "x", "dual"),
        makeRow("2.0", "B", "y", "dual"),
    };
    DictAssets dualRepo(3, 0, parseVersion, dualRows);
    MappedAssets dualAssets(std::move(dualRepo), std::unordered_map<std::string, std::vector<std::string>>{});

    auto dualLatest = dualAssets.getKey("dual", std::nullopt, {});
    check(dualLatest.has_value() && *dualLatest == std::vector<std::string>{"B", "y"}, "fromVersion=nullopt resolves to the latest bucket (2.0 -> (B, y))");

    auto dualFloored = dualAssets.getKey("dual", *Version::parse("1.5"), {});
    check(dualFloored.has_value() && *dualFloored == std::vector<std::string>{"A", "x"}, "fromVersion=1.5 floors to the 1.0 bucket (-> (A, x)), not the corrected version's own bug");

    auto dualExact = dualAssets.getKey("dual", *Version::parse("2.0"), {});
    check(dualExact.has_value() && *dualExact == std::vector<std::string>{"B", "y"}, "fromVersion=2.0 (exact match) resolves to the 2.0 bucket");

    auto notFound = assets.getKey("does-not-exist", std::nullopt, {}, false);
    check(!notFound.has_value(), "unknown asset with errorOnNotFound=false returns nullopt");

    bool threwNotFound = false;
    try {
        assets.getKey("does-not-exist", std::nullopt, {}, true);
    } catch (const std::out_of_range&) {
        threwNotFound = true;
    }
    check(threwNotFound, "unknown asset with errorOnNotFound=true throws std::out_of_range");

    auto filteredOut = assets.getKey("shared", *Version::parse("1.0"), {"does-not-exist", std::nullopt}, false);
    check(!filteredOut.has_value(), "a non-matching non-version filter returns nullopt (errorOnNotFound=false)");

    bool threwBadFilterSize = false;
    try {
        assets.getKey("shared", std::nullopt, {"A"});  // only 1 element, 2 required
    } catch (const std::invalid_argument&) {
        threwBadFilterSize = true;
    }
    check(threwBadFilterSize, "a wrong-sized non-version filter throws std::invalid_argument");
}

void testHasFrom() {
    std::printf("-- ModMappedAssets::hasFrom --\n");

    MappedAssets assets = makeSampleAssets();
    check(assets.hasFrom("shared"), "hasFrom finds a real asset");
    check(!assets.hasFrom("does-not-exist"), "hasFrom returns false for an unknown asset, doesn't throw");
}

void testReplace() {
    std::printf("-- ModMappedAssets::replace (single target) --\n");

    MappedAssets assets = makeSampleAssets();

    auto latestReplace = assets.replace("shared", *Version::parse("1.0"), {"A", std::nullopt}, std::nullopt, std::string("B"));
    check(latestReplace.has_value() && *latestReplace == "b-x-2.0", "replace to the latest toVersion resolves B's latest x (b-x-2.0)");

    auto exactReplace = assets.replace("shared", *Version::parse("1.0"), {"A", std::nullopt}, *Version::parse("1.0"), std::string("B"));
    check(exactReplace.has_value() && *exactReplace == "shared", "replace to an exact toVersion=1.0 resolves B's own 1.0 value (shared)");

    auto notMapped = assets.replace("shared", *Version::parse("1.0"), {"B", std::nullopt}, std::nullopt, std::string("A"), false);
    check(!notMapped.has_value(), "replacing from B (not a key in the map at all) with errorOnNotFound=false returns nullopt");

    bool threwNotMapped = false;
    try {
        assets.replace("shared", *Version::parse("1.0"), {"B", std::nullopt}, std::nullopt, std::string("A"), true);
    } catch (const std::out_of_range&) {
        threwNotMapped = true;
    }
    check(threwNotMapped, "replacing from B (not a key in the map at all) with errorOnNotFound=true throws -- unlike the pure-Python original, this respects errorOnNotFound consistently (see ModMappedAssets.h's note on this deliberate deviation)");

    auto notActuallyMapped = assets.replace("shared", *Version::parse("1.0"), {"A", std::nullopt}, std::nullopt, std::string("C"));
    check(!notActuallyMapped.has_value(), "requesting a toAssetName that A doesn't actually map to returns nullopt, doesn't throw");
}

void testReplaceAll() {
    std::printf("-- ModMappedAssets::replaceAll (every mapped target) --\n");

    MappedAssets assets = makeSampleAssets();

    auto all = assets.replaceAll("shared", *Version::parse("1.0"), {"A", std::nullopt}, std::nullopt);
    check(all.size() == 1 && all.count("B") == 1 && all.at("B") == "b-x-2.0", "empty toAssetNames filter uses every mapped target (just B here)");

    auto filtered = assets.replaceAll("shared", *Version::parse("1.0"), {"A", std::nullopt}, std::nullopt, {"B"});
    check(filtered.size() == 1 && filtered.at("B") == "b-x-2.0", "explicit toAssetNames filter matching B still works");

    auto filteredOutAll = assets.replaceAll("shared", *Version::parse("1.0"), {"A", std::nullopt}, std::nullopt, {"does-not-exist"});
    check(filteredOutAll.empty(), "a toAssetNames filter matching nothing real returns an empty map, doesn't throw");

    auto emptyOnUnmapped = assets.replaceAll("does-not-exist", std::nullopt, {}, std::nullopt, {}, false);
    check(emptyOnUnmapped.empty(), "unknown asset with errorOnNotFound=false returns an empty map");
}

void testAddRepoRowsAndAddMap() {
    std::printf("-- ModMappedAssets::addRepoRows / addMap --\n");

    MappedAssets assets = makeSampleAssets();

    assets.addRepoRows({makeRow("3.0", "A", "x", "brand-new")});
    check(assets.get({"A", "x"}, *Version::parse("3.0")) == "brand-new", "addRepoRows makes the new row queryable via get()");
    check(assets.hasFrom("brand-new"), "addRepoRows rebuilds the reverse index -- the new value is immediately findable via hasFrom");

    assets.addMap({{"A", {"B", "C"}}});  // "B" already present -- should be deduped, not duplicated
    const std::vector<std::string>& aMap = assets.getMap().at("A");
    check(aMap.size() == 2 && aMap[0] == "B" && aMap[1] == "C", "addMap unions in new entries, preserving order, without duplicating the already-present B");

    assets.addRepoRows({makeRow("1.0", "C", "x", "c-x-1.0")});
    auto viaC = assets.replaceAll("shared", *Version::parse("1.0"), {"A", std::nullopt}, *Version::parse("1.0"));
    check(viaC.size() == 2 && viaC.at("B") == "shared" && viaC.at("C") == "c-x-1.0", "the newly-mapped C target is usable by replaceAll immediately after addMap");
}

void testGetFromAssetsAndAccessors() {
    std::printf("-- ModMappedAssets accessors --\n");

    MappedAssets assets = makeSampleAssets();

    std::vector<std::string> fromAssets = assets.getFromAssets();
    check(fromAssets.size() == 7, "getFromAssets returns every distinct leaf value (8 rows, but A.x and B.x share \"shared\" at 1.0, so 7 distinct values)");

    check(assets.getRepo().size() == 8, "getRepo() exposes the underlying ModDictAssets with all 8 rows");
    check(assets.getMap().size() == 1 && assets.getMap().at("A").size() == 1, "getMap() exposes the initial adjacency list");
}

}  // namespace

int main() {
    testFindsBothSharedCandidates();
    testGetKey();
    testHasFrom();
    testReplace();
    testReplaceAll();
    testAddRepoRowsAndAddMap();
    testGetFromAssetsAndAccessors();

    if (failures == 0) {
        std::printf("\nAll checks passed.\n");
        return 0;
    }

    std::printf("\n%d check(s) FAILED.\n", failures);
    return 1;
}
