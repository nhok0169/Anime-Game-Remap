// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniRemoveBuilder
// (model/strategies/iniRemovers/IniRemoveBuilder.h), AGRemapCore::
// GlobalIniRemoveBuilders, and the IniFile/ModType rewiring that goes with them.
//
// NOT simply a third copy of IniParseBuilder_test.cpp / IniFixBuilder_test.cpp.
// The pure-Python IniRemoveBuilder derives from FlyweightBuilder rather than
// Builder, so two of its semantics are genuinely different and the test is
// shaped around them:
//   * it CACHES: the same instance is handed back to every caller sharing a
//     cache key, and that cache lives on the builder, not on the IniFile
//   * the returned remover is re-bound to the caller's IniFile on every call,
//     including on a cache hit -- which is what makes the sharing workable
//
// It ALSO has an ArgsRepo flavour, which is a deliberate extension beyond the
// Python original rather than a port of it (there is no IniRemoveBuilderData.py
// -- see IniRemoveBuilder.h's own note), so the args-repo section below is
// checked against this codebase's contract rather than against Python.
//
// Covers, against the documented contract in IniRemoveBuilder.h (its flyweight
// half matched against model/strategies/iniRemovers/IniRemoveBuilder.py +
// tools/FlyweightBuilder.py):
//   * A cache hit returns the SAME instance, and a miss constructs one
//   * Distinct ids get distinct instances within one builder
//   * Every build() rebinds the result to the given IniFile -- hits included
//   * cache = false constructs a fresh remover every call and neither reads nor
//     fills the cache (the original's "if (not cache)" early-out), and flipping
//     it back does not lose what was already cached
//   * clearCache()/getCacheSize()
//   * An empty factory falls back to defaultFactory(); an empty id falls back to
//     defaultId(); a fixed-factory builder ignores modName entirely
//   * ArgsRepo flavour: {version, modName} selection and floor-matching, and --
//     the load-bearing one -- that the cache is keyed by MOD NAME there, so two
//     mod types resolving to different factories cannot collide on one cached
//     remover the way id-only keying would have made them
//   * GlobalIniRemoveBuilders::removeBuilder() is a single shared instance, and
//     is what ModType falls back to -- so two ModTypes built with no remove
//     builder genuinely share one builder AND one cached remover
//   * IniFile::removeFix asks the builder per call rather than caching, so a
//     remover shared between two IniFiles is correctly re-bound each time (the
//     deliberate divergence from the original's self._iniRemover cache)
//
// Same build story as the other two builder tests -- it drives
// IniFile::removeFix, so link the already-built static lib rather than a
// hand-picked source list (`cd cbuild && ninja AGRemapCore`):
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      IniRemoveBuilder_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags
// are load-bearing. Copy libz3.dll next to test.exe before running.
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalIniRemoveBuilders.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/Row.h"
#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"

#include <cstdio>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
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

// Reports which factory built it, and counts its own removals.
class TaggedRemover: public BaseIniRemover {
    public:
        TaggedRemover(IniFile* iniFile, std::string tag): BaseIniRemover(iniFile), tag(std::move(tag)) {}

        std::string tag;
        int removeCount = 0;

        std::string remove(bool, bool) override {
            ++removeCount;
            return "removed-by-" + tag;
        }
};

// Counts constructions, so a test can prove a cache hit did NOT construct anything.
struct BuildLog {
    int constructions = 0;
};

IniRemoveBuilder::Factory countingFactory(std::shared_ptr<BuildLog> log, std::string tag) {
    return [log, tag](IniFile* iniFile) {
        ++(log->constructions);
        return std::make_shared<TaggedRemover>(iniFile, tag);
    };
}

// ---------------------------------------------------------------------------

void testFlyweightCaching() {
    std::printf("\n== flyweight caching ==\n");

    auto log = std::make_shared<BuildLog>();
    IniRemoveBuilder builder(countingFactory(log, "tagged"));

    check(builder.cache, "caching is on by default");
    check(builder.getCacheSize() == 0, "the cache starts empty");

    IniFile fileA(std::nullopt, "a\n");
    IniFile fileB(std::nullopt, "b\n");

    std::shared_ptr<BaseIniRemover> first = builder.build(&fileA);
    check(log->constructions == 1, "the first build constructs a remover");
    check(builder.getCacheSize() == 1, "and files it in the cache");

    std::shared_ptr<BaseIniRemover> second = builder.build(&fileB);
    check(log->constructions == 1, "a second build for a different file constructs nothing new");
    check(first == second, "both files got the very same shared instance -- this is a flyweight");

    // The whole reason sharing one instance works: every call re-points it.
    check(second->getIniFile() == &fileB, "build() rebinds the cached remover to the caller's file");
    check(builder.build(&fileA)->getIniFile() == &fileA, "and rebinds it back on the next call");
    check(log->constructions == 1, "rebinding on a cache hit still constructs nothing");
}

void testDistinctIds() {
    std::printf("\n== ids ==\n");

    auto log = std::make_shared<BuildLog>();
    IniRemoveBuilder builder(countingFactory(log, "tagged"));

    IniFile file(std::nullopt, "x\n");

    // The explicit id is build()'s 4th argument -- 'modName'/'version' before it are ignored by a
    // fixed-factory builder like this one.
    std::shared_ptr<BaseIniRemover> defaulted = builder.build(&file);
    std::shared_ptr<BaseIniRemover> named = builder.build(&file, "", std::nullopt, std::string("other"));

    check(log->constructions == 2, "a different id is a cache miss");
    check(defaulted != named, "distinct ids get distinct instances");
    check(builder.getCacheSize() == 2, "both are cached");

    check(builder.build(&file, "", std::nullopt, std::string("other")) == named, "the same id keeps returning its own instance");
    check(log->constructions == 2, "and constructs nothing further");

    // An explicit id equal to the default is the same slot as passing none.
    check(builder.build(&file, "", std::nullopt, builder.getId()) == defaulted, "passing the default id explicitly hits the same slot");
    check(log->constructions == 2, "still nothing further constructed");

    // Without a table, modName never affects the key -- it is ignored entirely.
    check(builder.build(&file, "Amber") == defaulted, "a fixed-factory builder ignores modName for the cache key");
    check(builder.build(&file, "Jean") == defaulted, "including for a second, different mod name");
    check(log->constructions == 2, "so neither constructs anything new");
}

void testCacheDisabled() {
    std::printf("\n== cache = false ==\n");

    auto log = std::make_shared<BuildLog>();
    IniRemoveBuilder builder(countingFactory(log, "tagged"), "", false);

    check(!builder.cache, "the constructor's cache argument is honoured");

    IniFile file(std::nullopt, "x\n");
    std::shared_ptr<BaseIniRemover> a = builder.build(&file);
    std::shared_ptr<BaseIniRemover> b = builder.build(&file);

    check(log->constructions == 2, "every build constructs a fresh remover");
    check(a != b, "and they are distinct instances");
    check(builder.getCacheSize() == 0, "nothing is filed in the cache");
    check(a->getIniFile() == &file && b->getIniFile() == &file, "an uncached remover is still bound to the caller's file");

    // Flipping caching back on starts caching again from whatever is there.
    builder.cache = true;
    std::shared_ptr<BaseIniRemover> c = builder.build(&file);
    check(log->constructions == 3, "re-enabling caching constructs once more to fill the empty slot");
    check(builder.build(&file) == c, "and reuses it from then on");

    // Turning it off does not throw away what was already cached.
    builder.cache = false;
    check(builder.getCacheSize() == 1, "disabling caching does not empty the cache");
    builder.cache = true;
    check(builder.build(&file) == c, "so re-enabling finds the old entry still there");
}

void testClearCache() {
    std::printf("\n== clearCache ==\n");

    auto log = std::make_shared<BuildLog>();
    IniRemoveBuilder builder(countingFactory(log, "tagged"));

    IniFile file(std::nullopt, "x\n");
    std::shared_ptr<BaseIniRemover> before = builder.build(&file);
    check(builder.getCacheSize() == 1, "one entry cached");

    builder.clearCache();
    check(builder.getCacheSize() == 0, "clearCache empties it");

    std::shared_ptr<BaseIniRemover> after = builder.build(&file);
    check(log->constructions == 2, "the next build constructs afresh");
    check(after != before, "and is a genuinely new instance");
}

void testDefaults() {
    std::printf("\n== defaults ==\n");

    IniRemoveBuilder defaulted;
    IniFile file(std::nullopt, "x\n");

    std::shared_ptr<BaseIniRemover> r = defaulted.build(&file);
    check(r != nullptr, "the default constructor still hands back a remover");
    check(dynamic_cast<TaggedRemover*>(r.get()) == nullptr, "the default constructor builds a plain BaseIniRemover");
    check(defaulted.getId() == IniRemoveBuilder::defaultId(), "the default constructor uses defaultId");

    IniRemoveBuilder emptyFactory{IniRemoveBuilder::Factory{}};
    check(emptyFactory.build(&file) != nullptr, "an empty factory falls back to defaultFactory rather than crashing");

    IniRemoveBuilder emptyId{IniRemoveBuilder::defaultFactory(), ""};
    check(emptyId.getId() == IniRemoveBuilder::defaultId(), "an empty id falls back to defaultId");

    IniRemoveBuilder namedId{IniRemoveBuilder::defaultFactory(), "custom"};
    check(namedId.getId() == "custom", "a non-empty id is kept as given");
}

// The version index sits at position 0 and the mod name at position 1, matching every other table.
std::shared_ptr<IniRemoveBuilder::ArgsRepo> makeArgsRepo(std::vector<Row<std::string, IniRemoveBuilder::Factory>> rows) {
    return std::make_shared<IniRemoveBuilder::ArgsRepo>(
        /*totalIndices*/ 2, /*versionIndexPos*/ 0,
        [](const std::string& raw) { return Version::parse(raw); },
        std::move(rows));
}

Version ver(const std::string& raw) {
    return *Version::parse(raw);
}

void testArgsRepoFlavour() {
    std::printf("\n== args-repo flavour ==\n");

    auto log = std::make_shared<BuildLog>();
    auto tagged = [log](std::string tag) {
        return IniRemoveBuilder::Factory{[log, tag](IniFile* f) {
            ++(log->constructions);
            return std::make_shared<TaggedRemover>(f, tag);
        }};
    };

    auto repo = makeArgsRepo({
        {{"4.0", "Amber"}, tagged("amber4_0")},
        {{"5.7", "Amber"}, tagged("amber5_7")},
        {{"4.0", "Jean"}, tagged("jean4_0")},
    });

    IniRemoveBuilder builder(repo);
    check(builder.getBuilderArgs() == repo, "the builder exposes the ArgsRepo it was given");
    check(!builder.getErrorOnNotFound(), "errorOnNotFound defaults to false");

    IniFile file(std::nullopt, "x\n");

    auto tagOf = [](const std::shared_ptr<BaseIniRemover>& r) {
        TaggedRemover* t = dynamic_cast<TaggedRemover*>(r.get());
        return (t != nullptr) ? t->tag : std::string("<untagged>");
    };

    // Version selection works exactly as on the parse/fix sides.
    check(tagOf(builder.build(&file, "Amber", ver("4.0"))) == "amber4_0", "an exact version match picks that row");
    check(tagOf(builder.build(&file, "Jean", ver("4.0"))) == "jean4_0", "the mod name selects among rows at the same version");
    check(tagOf(builder.build(&file, "Jean", ver("5.7"))) == "jean4_0", "a mod with only an old row keeps using it later");

    // THE point of keying the cache by mod name: Amber and Jean must not collide. With the old
    // id-only keying both would land on defaultId() and the second would silently get the first's
    // remover.
    check(builder.getCacheSize() == 2, "each mod name gets its own cache slot");
    check(builder.build(&file, "Amber", ver("4.0")) != builder.build(&file, "Jean", ver("4.0")),
          "two mod types resolving to different factories do not share one cached remover");

    // Caching still applies within a mod name.
    int before = log->constructions;
    builder.build(&file, "Amber", ver("4.0"));
    check(log->constructions == before, "a repeat build for the same mod is a cache hit");

    // A mod with no row falls back rather than throwing, and an explicit id still overrides.
    check(builder.build(&file, "NotListed", ver("4.0")) != nullptr, "an unlisted mod name falls back to a plain remover");

    IniRemoveBuilder strict(repo, true);
    bool threw = false;
    try {
        strict.build(&file, "NotListed", ver("4.0"));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    check(threw, "errorOnNotFound = true throws std::out_of_range for an unlisted mod name");

    // Rebinding still happens on every call, table or not.
    IniFile other(std::nullopt, "y\n");
    std::shared_ptr<BaseIniRemover> amber = builder.build(&other, "Amber", ver("4.0"));
    check(amber->getIniFile() == &other, "a table-resolved remover is still rebound to the caller's file");
}

void testGlobalRemoveBuilder() {
    std::printf("\n== GlobalIniRemoveBuilders ==\n");

    check(GlobalIniRemoveBuilders::removeBuilder() == GlobalIniRemoveBuilders::removeBuilder(),
          "removeBuilder() returns the same shared instance every call");
    check(GlobalIniRemoveBuilders::removeBuilder() != nullptr, "and it is never null");

    // ModType's null-fallback is that same shared builder -- mirroring the pure-Python ModType's
    // own "iniRemoveBuilder = GlobalIniRemoveBuilders.RemoveBuilder.value".
    ModType a(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber");
    ModType b(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Jean), "Jean");

    check(a.iniRemoveBuilder != nullptr, "a ModType with no remove builder still gets one");
    check(a.iniRemoveBuilder == GlobalIniRemoveBuilders::removeBuilder(), "and it is the global shared builder");
    check(a.iniRemoveBuilder->getBuilderArgs() == nullptr, "the global fallback is the fixed-factory flavour, matching Python");
    check(a.iniRemoveBuilder == b.iniRemoveBuilder, "two such ModTypes share that one builder");

    // Sharing the builder means sharing its flyweight cache, which is the documented consequence.
    IniFile file(std::nullopt, "x\n");
    check(a.iniRemoveBuilder->build(&file) == b.iniRemoveBuilder->build(&file),
          "so they also share the one cached remover");

    ModType copy = a;
    check(copy.iniRemoveBuilder == a.iniRemoveBuilder, "copying a ModType shares its remove builder rather than cloning it");
}

// Exposes IniFile's protected modTypes so a test can reach the ModType it was given.
class TestableIniFile: public IniFile {
    public:
        using IniFile::IniFile;

        const std::unordered_map<int, ModType>& testModTypes() const { return modTypes; }
};

void testIniFileUsesTheBuilder() {
    std::printf("\n== IniFile end-to-end ==\n");

    auto log = std::make_shared<BuildLog>();
    auto builder = std::make_shared<IniRemoveBuilder>(countingFactory(log, "tagged"));

    const int modTypeId = static_cast<int>(ModTypeId::Amber);

    auto makeOverrides = [&]() {
        std::unordered_map<int, ModType> overrides;
        overrides.emplace(modTypeId, ModType(static_cast<int>(GameTypeId::GI), modTypeId, "Amber", std::vector<std::string>{},
                                             /*hashes*/ nullptr, /*indices*/ nullptr,
                                             /*vertexCounts*/ nullptr, /*vgRemaps*/ nullptr,
                                             /*iniParseBuilder*/ nullptr, /*iniFixBuilder*/ nullptr,
                                             builder));
        return overrides;
    };

    std::unordered_set<int> forced = {modTypeId};

    TestableIniFile fileA(std::nullopt, "a\n", std::nullopt, std::nullopt, forced, makeOverrides(), nullptr);
    TestableIniFile fileB(std::nullopt, "b\n", std::nullopt, std::nullopt, forced, makeOverrides(), nullptr);

    std::string resultA = fileA.removeFix(false, false);
    check(log->constructions == 1, "removeFix builds the remover through the builder");
    check(resultA == "removed-by-tagged", "and returns what that remover produced");

    std::string resultB = fileB.removeFix(false, false);
    check(log->constructions == 1, "a second file reuses the cached flyweight rather than constructing another");
    check(resultB == "removed-by-tagged", "and still gets a working remover");

    // The payoff of asking the builder every call instead of caching per-file: after fileB has
    // rebound the shared remover, fileA's next removeFix re-points it back at fileA rather than
    // acting through a remover bound to fileB (which is what the pure-Python original does).
    std::shared_ptr<BaseIniRemover> shared = builder->build(&fileA);
    TaggedRemover* tagged = dynamic_cast<TaggedRemover*>(shared.get());
    check(tagged != nullptr, "the shared instance is the tagged one");

    fileB.removeFix(false, false);
    check(tagged != nullptr && tagged->getIniFile() == &fileB, "the shared remover is currently bound to fileB");

    fileA.removeFix(false, false);
    check(tagged != nullptr && tagged->getIniFile() == &fileA, "fileA's next removeFix re-binds it back, rather than acting through fileB's binding");

    check(log->constructions == 1, "none of that constructed a second remover");
}

void testNoBuilderMeansNoRemoval() {
    std::printf("\n== no remove builder ==\n");

    const int modTypeId = static_cast<int>(ModTypeId::Amber);

    // The constructor fills in the global fallback, so nulling it afterwards is the only way to
    // reach the "no remove builder at all" state removeFix guards against.
    std::unordered_map<int, ModType> overrides;
    ModType modType(static_cast<int>(GameTypeId::GI), modTypeId, "Amber");
    modType.iniRemoveBuilder = nullptr;
    overrides.emplace(modTypeId, std::move(modType));

    std::unordered_set<int> forced = {modTypeId};
    TestableIniFile file(std::nullopt, "original\n", std::nullopt, std::nullopt, forced, std::move(overrides), nullptr);

    check(file.removeFix(false, false) == "original\n",
          "a mod type with no remove builder leaves the file's text unchanged rather than aborting");
}

}  // namespace

int main() {
    testFlyweightCaching();
    testDistinctIds();
    testCacheDisabled();
    testClearCache();
    testArgsRepoFlavour();
    testDefaults();
    testGlobalRemoveBuilder();
    testIniFileUsesTheBuilder();
    testNoBuilderMeansNoRemoval();

    std::printf("\n%s (%d failure(s))\n", (failures == 0 ? "ALL PASSED" : "FAILURES"), failures);
    return (failures == 0) ? 0 : 1;
}
