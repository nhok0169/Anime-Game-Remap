// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::Version, AGRemapCore::VersionSet,
// and AGRemapCore::LruCache<K, V> -- the Phase 0 prerequisite building blocks
// for porting ModDictAssets/ModMappedAssets (see model/assets/ModDictAssets.py
// and model/assets/ModMappedAssets.py's own Version/LruCache dependencies).
//
// Version is a full PEP 440 port of Python's packaging.version.Version (not
// just a dotted-numeric subset) -- epoch, pre/post/dev-release segments, and
// local version segments are all supported, with the same comparison-key
// algorithm. This was verified empirically against the real `packaging`
// library (not just read from its source) via a throwaway side-by-side sweep
// during development: every accept/reject decision, every extracted field
// (epoch/release/pre/post/dev/local/is_prerelease/.../public/base_version),
// and the full pairwise ordering matrix over PEP 440's own canonical ascending
// example list (reproduced in testVersionPEP440Ordering below) came back
// byte-identical between packaging.version.Version and this port across a
// 65-case corpus. That sweep isn't part of this file (it depended on a real
// Python + packaging install, which this standalone regression test
// deliberately doesn't need) -- the cases below are a hand-picked subset
// baked in as plain assertions so this file stays self-contained.
//
// Covers, against the pure-Python originals' documented semantics
// (model/Version.py, tools/caches/LRUCache.py):
//   * Version::parse: full PEP 440 grammar (epoch, pre/post/dev, local,
//     v-prefix, whitespace, case-insensitivity, letter-spelling
//     normalization) plus rejection of malformed input
//   * Version::compare/operators: PEP 440's exact comparison-key rules --
//     trailing-zero-stripped release ("1.0" == "1.0.0"), the pre-release
//     sentinel trick (dev-only sorts before any pre-release, no-pre-release
//     sorts after any pre-release), post/local segment presence rules, and
//     the full PEP 440 canonical ordering example list
//   * std::hash<Version>: equal versions (by PEP 440 ==) hash equal too --
//     required for VersionSet's internal LruCache<Version, Version> to behave
//   * VersionSet::add + findClosest: floor-match, "smaller than everything"
//     fallback-to-smallest (NOT std::nullopt -- matches
//     Version.findClosestFromSortedList's own documented fallback), latest
//     version for a std::nullopt query, and cache-vs-no-cache agreement
//   * Cache: the plain, non-evicting base LruCache extends -- mirrors the
//     pure-Python Cache/LruCache split (tools/caches/Cache.py /
//     LRUCache.py), where Cache stores a capacity but never enforces it,
//     and LruCache is the one subclass that actually evicts
//   * LruCache: eviction order, promotion on get(), update-in-place on put(),
//     and capacity == 0 disabling caching entirely
//
// This file has NO dependency on the project's build system (CMake/pybind11),
// Z3, utf8proc, ordered-map, or xxHash -- Version/VersionSet/Cache/LruCache
// are pure standard-library code. Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /I <core>/include ^
//      Version_VersionSet_LruCache_test.cpp ^
//      <core>/src/model/Version.cpp <core>/src/model/VersionSet.cpp ^
//      /Fe:test.exe
//
// (g++/clang++ equivalent: swap /std:c++latest /EHsc /I /Fe: for
//  -std=c++23 -I ... -o test.exe)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/VersionSet.h"
#include "AGRemapCore/tools/caches/Cache.h"
#include "AGRemapCore/tools/caches/LruCache.h"

#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

using AGRemapCore::Cache;
using AGRemapCore::LruCache;
using AGRemapCore::Version;
using AGRemapCore::VersionSet;

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

void testVersionParsing() {
    std::printf("-- Version::parse (plain dotted-numeric) --\n");

    auto v = Version::parse("3.7");
    check(v.has_value() && v->getRelease().size() == 2 && v->getRelease()[0] == 3 && v->getRelease()[1] == 7, "parses \"3.7\"");

    auto v2 = Version::parse("10.20.3");
    check(v2.has_value() && v2->getRelease().size() == 3 && v2->getRelease()[2] == 3, "parses multi-component \"10.20.3\"");

    check(!Version::parse("").has_value(), "rejects empty string");
    check(!Version::parse("a.b").has_value(), "rejects non-numeric release with no valid grammar element");
    check(!Version::parse("1..2").has_value(), "rejects empty component (double dot)");
    check(!Version::parse("1.2.").has_value(), "rejects trailing dot");
    check(!Version::parse(".1.2").has_value(), "rejects leading dot");
    check(!Version::parse("-1.0").has_value(), "rejects a leading dash (release must start with a digit)");

    check(Version::parse("4.0")->toString() == "4.0", "toString round-trips \"4.0\"");
    check(Version::parse("01.02")->toString() == "1.2", "leading zeros normalize away (\"01.02\" -> \"1.2\")");
}

void testVersionPEP440() {
    std::printf("-- Version::parse (full PEP 440) --\n");

    // "1.2a" is a real, valid PEP 440 version (pre-release "a", implicit number 0) -- NOT
    // rejected, unlike the narrower dotted-numeric-only parser this replaced. Confirmed against
    // the real `packaging` library during development (see the file header comment).
    auto preNoNumber = Version::parse("1.2a");
    check(preNoNumber.has_value() && preNoNumber->getPre().has_value() && preNoNumber->getPre()->first == "a" && preNoNumber->getPre()->second == 0, "\"1.2a\" parses as pre-release \"a0\", not rejected");

    // Epoch.
    auto epoch = Version::parse("1!2.0");
    check(epoch.has_value() && epoch->getEpoch() == 1 && epoch->toString() == "1!2.0", "epoch (\"1!2.0\") parses and round-trips");
    check(Version::parse("2.0")->getEpoch() == 0, "no epoch specified -> epoch 0");
    check(*Version::parse("2!1.0") > *Version::parse("1!100.0"), "epoch dominates comparison over release");

    // Pre-release spelling normalization: alpha->a, beta->b, c/pre/preview->rc.
    check(Version::parse("1.0alpha1")->getPre() == Version::parse("1.0a1")->getPre(), "\"alpha\" normalizes to \"a\"");
    check(Version::parse("1.0beta1")->getPre() == Version::parse("1.0b1")->getPre(), "\"beta\" normalizes to \"b\"");
    check(Version::parse("1.0c1")->getPre() == Version::parse("1.0rc1")->getPre(), "\"c\" normalizes to \"rc\"");
    check(Version::parse("1.0pre1")->getPre() == Version::parse("1.0rc1")->getPre(), "\"pre\" normalizes to \"rc\"");
    check(Version::parse("1.0preview1")->getPre() == Version::parse("1.0rc1")->getPre(), "\"preview\" normalizes to \"rc\"");
    check(Version::parse("1.0RC1")->getPre() == Version::parse("1.0rc1")->getPre(), "letters are case-insensitive (\"RC1\" == \"rc1\")");

    // Post-release: word form, "rev"/"r" synonyms, and the implicit "-N" shorthand.
    check(Version::parse("1.0.post1")->getPost() == 1u, "\".post1\" parses post=1");
    check(Version::parse("1.0.rev1")->getPost() == 1u, "\"rev\" is a synonym for \"post\"");
    check(Version::parse("1.0.r1")->getPost() == 1u, "\"r\" is a synonym for \"post\"");
    check(Version::parse("1.0-1")->getPost() == 1u, "implicit \"-1\" shorthand means post=1");
    check(*Version::parse("1.0-1") == *Version::parse("1.0.post1"), "\"1.0-1\" and \"1.0.post1\" are equal versions");

    // Dev-release.
    check(Version::parse("1.0.dev1")->getDev() == 1u, "\".dev1\" parses dev=1");
    check(Version::parse("1.0dev1")->getDev() == 1u, "dev-release separator is optional");

    // Local version segment.
    auto local = Version::parse("1.0+abc.1.twelve");
    check(local.has_value() && local->getLocal() == "abc.1.twelve", "local segment parses and joins with '.'");
    check(Version::parse("1.0+ABC")->getLocal() == "abc", "local segment text is lowercased");
    check(!Version::parse("1.0+").has_value(), "bare '+' with no local content is rejected");

    // v-prefix and surrounding whitespace.
    check(Version::parse("v1.0").has_value() && Version::parse("v1.0")->toString() == "1.0", "leading 'v' is accepted and stripped");
    check(Version::parse(" 1.0 ").has_value() && Version::parse(" 1.0 ")->toString() == "1.0", "surrounding whitespace is trimmed");

    // Still-invalid input under the full grammar.
    check(!Version::parse("1.0..dev1").has_value(), "double-dot before a suffix is still rejected");
    check(!Version::parse("vv1.0").has_value(), "only one leading 'v' is allowed");
}

void testVersionPEP440ComparisonKeyRules() {
    std::printf("-- Version comparison-key rules (PEP 440 sentinel logic) --\n");

    // A dev-only version (no pre, no post) sorts BEFORE its own release; a pre-release with no
    // dev/post sorts before the final release; a version with neither sorts after any pre-release.
    check(*Version::parse("1.0.dev1") < *Version::parse("1.0a1"), "1.0.dev1 < 1.0a1 (dev-only sentinel trick)");
    check(*Version::parse("1.0a1") < *Version::parse("1.0"), "1.0a1 < 1.0 (pre-release sorts before final release)");
    check(*Version::parse("1.0") < *Version::parse("1.0.post1"), "1.0 < 1.0.post1 (post-release sorts after final release)");

    // Local segment ordering: alphanumeric parts sort before numeric parts; no-local sorts before
    // has-local; shorter sorts before longer on a matching prefix.
    check(*Version::parse("1.0") < *Version::parse("1.0+abc"), "no local segment sorts before any local segment");
    check(*Version::parse("1.0+abc") < *Version::parse("1.0+1"), "alphanumeric local segment sorts before numeric local segment");
    check(*Version::parse("1.0+abc.5") < *Version::parse("1.0+abc.7"), "numeric local sub-segments compare numerically");
    check(*Version::parse("1.0+abc") < *Version::parse("1.0+abc.1"), "shorter local segment sorts before a longer one sharing its prefix");

    // The full PEP 440 canonical ascending ordering example (from the PEP 440 spec itself) --
    // verified byte-identical against real packaging.version.Version during development.
    const char* ascending[] = {
        "1.0.dev456", "1.0a1", "1.0a2.dev456", "1.0a12.dev456", "1.0a12",
        "1.0b1.dev456", "1.0b2", "1.0b2.post345.dev456", "1.0b2.post345",
        "1.0rc1.dev456", "1.0rc1", "1.0", "1.0+abc.5", "1.0+abc.7", "1.0+5",
        "1.0.post456.dev34", "1.0.post456", "1.0.15", "1.1.dev1",
    };
    constexpr std::size_t n = sizeof(ascending) / sizeof(ascending[0]);

    std::vector<Version> versions;
    for (const char* raw : ascending) {
        auto v = Version::parse(raw);
        check(v.has_value(), raw);
        versions.push_back(*v);
    }

    bool strictlyIncreasing = true;
    for (std::size_t i = 0; i + 1 < n; ++i) {
        if (!(versions[i] < versions[i + 1])) {
            strictlyIncreasing = false;
        }
    }
    check(strictlyIncreasing, "PEP 440's full canonical ascending example list sorts strictly increasing end-to-end");
}

void testVersionComparison() {
    std::printf("-- Version comparison/equality --\n");

    Version v1_0 = *Version::parse("1.0");
    Version v1_0_0 = *Version::parse("1.0.0");
    Version v1_1 = *Version::parse("1.1");
    Version v0_9 = *Version::parse("0.9");
    Version v2 = *Version::parse("2");

    check(v1_0 == v1_0_0, "\"1.0\" == \"1.0.0\" (zero-padded equality)");
    check(!(v1_0 != v1_0_0), "\"1.0\" != \"1.0.0\" is false");
    check(v0_9 < v1_0, "\"0.9\" < \"1.0\"");
    check(v1_1 > v1_0, "\"1.1\" > \"1.0\"");
    check(v2 > v1_1, "\"2\" > \"1.1\"");
    check(v1_0 <= v1_0_0 && v1_0 >= v1_0_0, "\"1.0\" <= and >= \"1.0.0\"");
}

void testVersionHashConsistency() {
    std::printf("-- std::hash<Version> consistency --\n");

    Version v1_0 = *Version::parse("1.0");
    Version v1_0_0 = *Version::parse("1.0.0");
    Version v0_0 = *Version::parse("0.0");
    Version v0 = *Version::parse("0");

    std::unordered_set<Version> set;
    set.insert(v1_0);
    set.insert(v1_0_0);
    check(set.size() == 1, "\"1.0\" and \"1.0.0\" collapse to one unordered_set entry (equal hash + equal ==)");

    set.insert(v0_0);
    set.insert(v0);
    check(set.size() == 2, "\"0.0\"/\"0\" collapse together but stay distinct from \"1.0\"");
    check(set.count(*Version::parse("1.0.0.0")) == 1, "\"1.0.0.0\" hashes/compares equal to the stored \"1.0\" entry");
}

void testVersionSetFindClosest() {
    std::printf("-- VersionSet::add / findClosest --\n");

    VersionSet versions;

    check(!versions.findClosest(std::nullopt).has_value(), "findClosest on empty set returns nullopt");

    // Add out of order to also exercise the front/back/middle insert branches in add().
    versions.add(*Version::parse("3.7"));
    versions.add(*Version::parse("1.0"));
    versions.add(*Version::parse("4.0"));
    versions.add(*Version::parse("2.0"));
    versions.add(*Version::parse("3.7"));  // duplicate -- must not create a second entry

    check(versions.getVersions().size() == 4, "duplicate add() is deduped");
    check(versions.getLatestVersion().has_value() && *versions.getLatestVersion() == *Version::parse("4.0"), "latest version is 4.0");

    check(*versions.findClosest(std::nullopt) == *Version::parse("4.0"), "findClosest(nullopt) returns latest (4.0)");
    check(*versions.findClosest(*Version::parse("3.7")) == *Version::parse("3.7"), "findClosest(3.7) exact match");
    check(*versions.findClosest(*Version::parse("3.9")) == *Version::parse("3.7"), "findClosest(3.9) floors to 3.7");
    check(*versions.findClosest(*Version::parse("100.0")) == *Version::parse("4.0"), "findClosest(100.0) floors to latest (4.0)");
    check(*versions.findClosest(*Version::parse("0.1")) == *Version::parse("1.0"), "findClosest(0.1), below everything, falls back to smallest (1.0) -- not nullopt");

    // Same queries with the cache disabled must agree with the cached path.
    check(*versions.findClosest(*Version::parse("3.9"), false) == *Version::parse("3.7"), "findClosest(3.9, fromCache=false) agrees with cached result");
    check(*versions.findClosest(*Version::parse("0.1"), false) == *Version::parse("1.0"), "findClosest(0.1, fromCache=false) agrees with cached result");

    versions.clear();
    check(!versions.getLatestVersion().has_value(), "clear() resets latest version");
    check(!versions.findClosest(std::nullopt).has_value(), "clear() empties the set (findClosest -> nullopt again)");
}

void testCache() {
    std::printf("-- Cache (the plain, non-evicting base LruCache extends) --\n");

    // Capacity of 1 -- if Cache enforced it like LruCache does, put(2, ...) below would evict
    // key 1. It shouldn't: Cache stores capacity but never acts on it, matching Cache.py exactly
    // (only LruCache.py's overridden __getitem__/__setitem__ do any eviction).
    Cache<int, std::string> cache(1);

    check(cache.getCapacity() == 1, "getCapacity() reflects the constructor argument");
    check(!cache.contains(1), "empty cache doesn't contain key 1");
    check(!cache.get(1).has_value(), "get() on a missing key returns nullopt");

    cache.put(1, "a");
    cache.put(2, "b");
    cache.put(3, "c");
    check(cache.size() == 3, "put() past 'capacity' never evicts -- size grows unbounded, matching the pure-Python Cache base");
    check(cache.get(1).has_value() && *cache.get(1) == "a", "key 1 (the oldest entry) is still present, not evicted");
    check(cache.contains(2), "contains() finds a present key");

    cache.put(2, "b-updated");
    check(cache.size() == 3, "update-in-place doesn't grow the cache");
    check(*cache.get(2) == "b-updated", "update-in-place overwrites the value");

    cache.clear();
    check(cache.size() == 0, "clear() empties the cache");
    check(!cache.contains(1), "clear() removes previously-present keys");
}

void testLruCache() {
    std::printf("-- LruCache --\n");

    LruCache<int, std::string> cache(2);

    cache.put(1, "a");
    cache.put(2, "b");
    check(cache.size() == 2, "size after 2 puts at capacity 2");

    cache.put(3, "c");  // evicts key 1 (LRU); order is now MRU=3, LRU=2
    check(cache.size() == 2, "size stays at capacity after eviction");
    check(!cache.get(1).has_value(), "key 1 was evicted");

    std::optional<std::string> v2 = cache.get(2);  // single touch: promotes 2 to MRU (order now MRU=2, LRU=3)
    check(v2.has_value() && *v2 == "b", "key 2 still present");

    std::optional<std::string> v3 = cache.get(3);  // single touch: promotes 3 to MRU (order now MRU=3, LRU=2)
    check(v3.has_value() && *v3 == "c", "key 3 present");

    // The get(3) touch above made 3 MRU and 2 LRU. Inserting a 4th key should evict 2, not 3.
    cache.put(4, "d");
    check(!cache.get(2).has_value(), "key 2 (LRU after key 3 was touched last) was evicted, not key 3");
    check(cache.get(3).has_value(), "key 3 (touched last via get()) survived the eviction");

    // Update-in-place must not change cache size or evict anything.
    cache.put(4, "d-updated");
    check(cache.size() == 2, "update-in-place doesn't grow the cache");
    check(*cache.get(4) == "d-updated", "update-in-place overwrites the value");

    cache.clear();
    check(cache.size() == 0, "clear() empties the cache");

    LruCache<int, int> disabled(0);
    disabled.put(1, 100);
    check(disabled.size() == 0, "capacity-0 cache never stores anything");
    check(!disabled.get(1).has_value(), "capacity-0 cache never returns a hit");
}

}  // namespace

int main() {
    testVersionParsing();
    testVersionPEP440();
    testVersionPEP440ComparisonKeyRules();
    testVersionComparison();
    testVersionHashConsistency();
    testCache();
    testVersionSetFindClosest();
    testLruCache();

    if (failures == 0) {
        std::printf("\nAll checks passed.\n");
        return 0;
    }

    std::printf("\n%d check(s) FAILED.\n", failures);
    return 1;
}
