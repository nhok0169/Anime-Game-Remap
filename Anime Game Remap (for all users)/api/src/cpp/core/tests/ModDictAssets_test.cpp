// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::ModDictAssets<K, T, KeyHash, KeyEqual>
// -- Phase 1 of the ModDictAssets/ModMappedAssets port (see model/assets/ModDictAssets.py).
//
// Data shape mirrors the real model/assets/Hashes.py usage: indices =
// ["version", "name", "type"], versionIndexPos = 0, leaf value = a hash
// string. K = T = std::string here (K's raw version-index values get parsed
// via Version::parse).
//
// Covers:
//   * get(): latest-version query (version = nullopt), exact-match query
//     (the corrected, non-buggy semantics -- see ModDictAssets.h's class-level
//     note and the live side-by-side check against the real pure-Python
//     ModDictAssets done during development, which confirmed the pure-Python
//     original returns the *previous* version's data on an exact match,
//     except when that match is the smallest version), floor-match between
//     two versions, and the below-everything fallback-to-smallest case
//   * get() error paths: unknown non-version key (errorOnNotFound true/false),
//     wrong-sized nonVersionVals (std::invalid_argument)
//   * addRows(): overwriting an existing (nonVersionVals, version) entry,
//     adding a new version to an existing group, mismatched row width, and
//     an unparseable version value -- all as std::invalid_argument
//   * independent non-version groups don't interfere with each other
//
// This file has NO dependency on the project's build system (CMake/pybind11),
// Z3, utf8proc, ordered-map, or xxHash. Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /I <core>/include ^
//      ModDictAssets_test.cpp <core>/src/model/Version.cpp ^
//      /Fe:test.exe
//
// (g++/clang++ equivalent: swap /std:c++latest /EHsc /I /Fe: for
//  -std=c++23 -I ... -o test.exe)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModDictAssets.h"
#include "AGRemapCore/model/assets/Row.h"

#include <cstdio>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using AGRemapCore::ModDictAssets;
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

using Assets = ModDictAssets<std::string, std::string>;

Row<std::string, std::string> makeRow(const std::string& version, const std::string& name, const std::string& type, const std::string& value) {
    return Row<std::string, std::string>{{version, name, type}, value};
}

Assets makeSampleAssets() {
    // Mirrors HashData.py's shape: {version: {name: {type: hash}}}, flattened.
    //   1.0: A.x = "v1-Ax"
    //   2.0: A.x = "v2-Ax"
    //   4.0: A.x = "v4-Ax"
    //   3.7: B.y = "v3.7-By"   (a separate, independent group)
    std::vector<Row<std::string, std::string>> rows = {
        makeRow("1.0", "A", "x", "v1-Ax"),
        makeRow("2.0", "A", "x", "v2-Ax"),
        makeRow("4.0", "A", "x", "v4-Ax"),
        makeRow("3.7", "B", "y", "v3.7-By"),
    };
    return Assets(3, 0, parseVersion, std::move(rows));
}

void testBasicGet() {
    std::printf("-- ModDictAssets::get (basic + corrected exact-match semantics) --\n");

    Assets assets = makeSampleAssets();
    check(assets.size() == 4, "size() reflects all rows across all groups");
    check(assets.getTotalIndices() == 3, "getTotalIndices() == 3");
    check(assets.getVersionIndexPos() == 0, "getVersionIndexPos() == 0");

    check(assets.get({"A", "x"}, std::nullopt) == "v4-Ax", "version=nullopt returns the latest (4.0)");

    // Corrected semantics: exact match returns itself, not the previous version's data
    // (unlike the live pure-Python ModDictAssets, confirmed empirically during development).
    check(assets.get({"A", "x"}, *Version::parse("1.0")) == "v1-Ax", "exact match on the smallest version (1.0)");
    check(assets.get({"A", "x"}, *Version::parse("2.0")) == "v2-Ax", "exact match on a middle version (2.0) -- NOT the previous entry");
    check(assets.get({"A", "x"}, *Version::parse("4.0")) == "v4-Ax", "exact match on the largest version (4.0) -- NOT the previous entry");

    check(assets.get({"A", "x"}, *Version::parse("3.0")) == "v2-Ax", "between 2.0 and 4.0 floors to 2.0");
    check(assets.get({"A", "x"}, *Version::parse("0.5")) == "v1-Ax", "below every available version falls back to the smallest (1.0)");
    check(assets.get({"A", "x"}, *Version::parse("100.0")) == "v4-Ax", "above every available version floors to the largest (4.0)");

    check(assets.get({"B", "y"}, std::nullopt) == "v3.7-By", "an independent group (B.y) resolves on its own");
}

void testGetErrorPaths() {
    std::printf("-- ModDictAssets::get (error paths) --\n");

    Assets assets = makeSampleAssets();

    bool threw = false;
    try {
        assets.get({"nope", "x"}, std::nullopt, true);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    check(threw, "unknown non-version key with errorOnNotFound=true throws std::out_of_range");

    auto missing = assets.get({"nope", "x"}, std::nullopt, false);
    check(!missing.has_value(), "unknown non-version key with errorOnNotFound=false returns nullopt");

    bool threwArgSize = false;
    try {
        assets.get({"A"}, std::nullopt);  // only 1 value, but 2 are required (totalIndices=3, minus version=2)
    } catch (const std::invalid_argument&) {
        threwArgSize = true;
    }
    check(threwArgSize, "wrong-sized nonVersionVals throws std::invalid_argument");
}

void testAddRows() {
    std::printf("-- ModDictAssets::addRows --\n");

    Assets assets = makeSampleAssets();

    // Overwrite an existing exact (nonVersionVals, version) entry.
    assets.addRows({makeRow("2.0", "A", "x", "v2-Ax-UPDATED")});
    check(assets.size() == 4, "overwriting an existing row doesn't change the total row count");
    check(assets.get({"A", "x"}, *Version::parse("2.0")) == "v2-Ax-UPDATED", "overwrite replaced the value at the exact same key");

    // Add a new version to an existing group.
    assets.addRows({makeRow("3.0", "A", "x", "v3-Ax")});
    check(assets.size() == 5, "adding a new version grows the row count");
    check(assets.get({"A", "x"}, *Version::parse("3.0")) == "v3-Ax", "the newly-added version is retrievable exactly");
    check(assets.get({"A", "x"}, *Version::parse("3.5")) == "v3-Ax", "the group is still correctly sorted after insertion (floors to the new 3.0 entry)");
    check(assets.get({"A", "x"}, std::nullopt) == "v4-Ax", "latest is still 4.0 after inserting a version in the middle");

    bool threwWidth = false;
    try {
        assets.addRows({Row<std::string, std::string>{{"5.0", "A"}, "bad"}});  // only 2 index values, expected 3
    } catch (const std::invalid_argument&) {
        threwWidth = true;
    }
    check(threwWidth, "a row with the wrong number of index values throws std::invalid_argument");

    bool threwVersion = false;
    try {
        assets.addRows({makeRow("not-a-version", "A", "x", "bad")});
    } catch (const std::invalid_argument&) {
        threwVersion = true;
    }
    check(threwVersion, "a row with an unparseable version value throws std::invalid_argument");
}

}  // namespace

int main() {
    testBasicGet();
    testGetErrorPaths();
    testAddRows();

    if (failures == 0) {
        std::printf("\nAll checks passed.\n");
        return 0;
    }

    std::printf("\n%d check(s) FAILED.\n", failures);
    return 1;
}
