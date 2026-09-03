// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniRemoveBuilder
// (model/strategies/iniRemovers/IniRemoveBuilder.h), AGRemapCore::
// GlobalIniRemoveBuilders, and the IniFile/ModType rewiring that goes with them.
//
// This builder is now the same shape as IniParseBuilder / IniFixBuilder: two
// flavours (fixed factory, or a version-dependent ArgsRepo), a fresh strategy
// per build(), and nothing else. It did NOT start that way -- it was a
// flyweight, mirroring the pure-Python IniRemoveBuilder's FlyweightBuilder base
// (cache / id / clearCache / one shared instance per key). The maintainer's
// call was to drop all of that, so the first thing this file pins down is that
// build() really does construct every time.
//
// The ArgsRepo flavour is a deliberate extension beyond the Python original
// rather than a port of it (there is no IniRemoveBuilderData.py -- see
// IniRemoveBuilder.h's own note), so that section is checked against this
// codebase's contract rather than against Python.
//
// Covers, against the documented contract in IniRemoveBuilder.h:
//   * every build() constructs a NEW remover -- no caching, no sharing, and two
//     IniFiles never end up holding the same instance
//   * every build() binds the result to the given IniFile, including when the
//     factory hands one back unbound
//   * an empty factory falls back to defaultFactory(); a fixed-factory builder
//     ignores modName/version entirely
//   * ArgsRepo flavour: {version, modName} selection and floor-matching, the
//     not-found fallback, and errorOnNotFound = true throwing instead
//   * GlobalIniRemoveBuilders::removeBuilder() is a single shared instance, and
//     is what ModType falls back to -- the BUILDER is shared, the removers it
//     hands out are not
//   * IniFile::removeFix asks the builder per call and keeps nothing
//   * defaultFactory() builds a real RemapIniRemover, already carrying its own
//     IniFileRemoveContext -- so the whole unmocked chain (ModType ->
//     GlobalIniRemoveBuilders -> defaultFactory -> RemapIniRemover) really strips a
//     fix out of an IniFile
//   * Exactly one of a multi-mod-type file's passes is given
//     IniRemovalContext::ignoreModType, and it is the last
//   * An UNCLASSIFIED file falls back to GlobalIniRemoveBuilders for one sweeping
//     pass, while a classified mod type that deliberately has no remove builder
//     is left alone -- the two cases are not the same, see removeFix's own note
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
#include "AGRemapCore/model/strategies/iniRemovers/RemapIniRemover.h"

#include <cstdio>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
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

// Counts constructions, and records every removal that ran.
//
// The removals live HERE rather than on the remover on purpose: build() constructs a fresh remover
// every call, so anything a test wants to observe about a removal has to outlive the remover that
// did it. (Back when this builder was a flyweight, build() handed back the very instance
// removeFix had used, and a counter on the remover was enough.)
struct BuildLog {
    int constructions = 0;

    // One entry per remove() call: which factory's remover ran, and whether it was given the sweep.
    std::vector<std::pair<std::string, bool>> removals;
};

// Reports which factory built it, and records its removals into the shared log.
class TaggedRemover: public BaseIniRemover<> {
    public:
        TaggedRemover(IniFile* iniFile, std::string tag, std::shared_ptr<BuildLog> log = nullptr):
            BaseIniRemover<>(iniFile), tag(std::move(tag)), log(std::move(log)) {}

        std::string tag;
        std::shared_ptr<BuildLog> log;

        // The IniRemovalContext is recorded rather than ignored, so a test can prove which pass
        // IniFile::removeFix handed the sweep to.
        std::string remove(bool, bool, IniRemovalContext context) override {
            if (log != nullptr) {
                log->removals.emplace_back(tag, context.ignoreModType);
            }

            return "removed-by-" + tag;
        }
};

IniRemoveBuilder::Factory countingFactory(std::shared_ptr<BuildLog> log, std::string tag) {
    return [log, tag](IniFile* iniFile) {
        ++(log->constructions);
        return std::make_shared<TaggedRemover>(iniFile, tag, log);
    };
}

// ---------------------------------------------------------------------------

void testNoCaching() {
    std::printf("\n== no caching ==\n");

    auto log = std::make_shared<BuildLog>();
    IniRemoveBuilder builder(countingFactory(log, "tagged"));

    IniFile fileA(std::nullopt, "a\n");
    IniFile fileB(std::nullopt, "b\n");

    std::shared_ptr<BaseIniRemover<>> first = builder.build(&fileA);
    check(log->constructions == 1, "the first build constructs a remover");

    std::shared_ptr<BaseIniRemover<>> second = builder.build(&fileB);
    check(log->constructions == 2, "a second build constructs another one");
    check(first != second, "the two files got DIFFERENT instances -- this is not a flyweight");

    // The whole point of dropping the flyweight: a remover handed out stays bound to the file it
    // was built for, however many builds happen afterwards.
    check(first->getIniFile() == &fileA, "the first remover is still bound to its own file");
    check(second->getIniFile() == &fileB, "and the second to its own");

    builder.build(&fileB);
    check(first->getIniFile() == &fileA, "a later build for another file does not rebind an earlier remover");

    // Same file twice is still two removers -- there is no key to hit.
    std::shared_ptr<BaseIniRemover<>> againA = builder.build(&fileA);
    check(againA != first, "even the same .ini file gets a fresh remover each time");
    check(log->constructions == 4, "so every call constructed exactly once");
}

void testBuildAlwaysBinds() {
    std::printf("\n== build() binds ==\n");

    // A factory that ignores its IniFile argument entirely -- build() has to bind the result
    // itself, which is what the setIniFile call after the factory is for.
    IniRemoveBuilder builder(IniRemoveBuilder::Factory{[](IniFile*) {
        return std::make_shared<TaggedRemover>(nullptr, "unbound");
    }});

    IniFile file(std::nullopt, "x\n");
    std::shared_ptr<BaseIniRemover<>> r = builder.build(&file);

    check(r != nullptr, "a remover comes back");
    check(r->getIniFile() == &file, "and build() bound it even though the factory did not");

    // nullptr is a legitimate .ini file to build for -- BaseIniRemover allows an unbound remover.
    check(builder.build(nullptr)->getIniFile() == nullptr, "building for nullptr leaves it unbound");
}

void testDefaults() {
    std::printf("\n== defaults ==\n");

    IniRemoveBuilder defaulted;
    IniFile file(std::nullopt, "x\n");

    std::shared_ptr<BaseIniRemover<>> r = defaulted.build(&file);
    check(r != nullptr, "the default constructor still hands back a remover");
    check(dynamic_cast<TaggedRemover*>(r.get()) == nullptr, "the default constructor does not build the test's own remover");
    // defaultFactory hands out the real RemapIniRemover now, not a bare BaseIniRemover -- this is what
    // makes IniFile::removeFix actually remove anything.
    check(dynamic_cast<RemapIniRemover<>*>(r.get()) != nullptr, "the default constructor builds a real RemapIniRemover");
    check(dynamic_cast<RemapIniRemover<>*>(r.get())->getContext() != nullptr,
          "and it comes back already bound, with an IniFileRemoveContext of its own");
    check(defaulted.getBuilderArgs() == nullptr, "the default constructor is the fixed-factory flavour");

    IniRemoveBuilder emptyFactory{IniRemoveBuilder::Factory{}};
    check(emptyFactory.build(&file) != nullptr, "an empty factory falls back to defaultFactory rather than crashing");
    check(dynamic_cast<RemapIniRemover<>*>(emptyFactory.build(&file).get()) != nullptr,
          "and that fallback is the real RemapIniRemover too");

    IniRemoveBuilder nullArgs{std::shared_ptr<const IniRemoveBuilder::ArgsRepo>{}};
    check(nullArgs.build(&file) != nullptr, "a nullptr ArgsRepo degrades to the default factory rather than crashing");
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

    auto tagOf = [](const std::shared_ptr<BaseIniRemover<>>& r) {
        TaggedRemover* t = dynamic_cast<TaggedRemover*>(r.get());
        return (t != nullptr) ? t->tag : std::string("<untagged>");
    };

    // Version selection works exactly as on the parse/fix sides.
    check(tagOf(builder.build(&file, "Amber", ver("4.0"))) == "amber4_0", "an exact version match picks that row");
    check(tagOf(builder.build(&file, "Jean", ver("4.0"))) == "jean4_0", "the mod name selects among rows at the same version");
    check(tagOf(builder.build(&file, "Jean", ver("5.7"))) == "jean4_0", "a mod with only an old row keeps using it later");

    // Two mod names resolving to different rows obviously get different removers -- and so do two
    // builds for the SAME mod name, since nothing is cached.
    check(tagOf(builder.build(&file, "Amber", ver("4.0"))) != tagOf(builder.build(&file, "Jean", ver("4.0"))),
          "two mod names resolving to different factories get different removers");

    int before = log->constructions;
    builder.build(&file, "Amber", ver("4.0"));
    check(log->constructions == before + 1, "a repeat build for the same mod constructs again");

    // A mod with no row falls back rather than throwing.
    check(builder.build(&file, "NotListed", ver("4.0")) != nullptr, "an unlisted mod name falls back to a plain remover");

    IniRemoveBuilder strict(repo, true);
    bool threw = false;
    try {
        strict.build(&file, "NotListed", ver("4.0"));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    check(threw, "errorOnNotFound = true throws std::out_of_range for an unlisted mod name");

    // Binding happens on every call, table or not.
    IniFile other(std::nullopt, "y\n");
    std::shared_ptr<BaseIniRemover<>> amber = builder.build(&other, "Amber", ver("4.0"));
    check(amber->getIniFile() == &other, "a table-resolved remover is bound to the caller's file");
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

    // Sharing the BUILDER does not mean sharing removers -- that is the whole difference from the
    // flyweight this used to be.
    IniFile file(std::nullopt, "x\n");
    check(a.iniRemoveBuilder->build(&file) != b.iniRemoveBuilder->build(&file),
          "but each build() still hands back its own remover");

    ModType copy = a;
    check(copy.iniRemoveBuilder == a.iniRemoveBuilder, "copying a ModType shares its remove builder rather than cloning it");
}

// Exposes IniFile's protected modTypes so a test can reach the ModType it was given.
class TestableIniFile: public IniFile {
    public:
        using IniFile::IniFile;

        // modTypes is a tsl::ordered_map now -- insertion order decides which mod type takes
        // the .ini file's backup and which hides the original (see IniFile::fix).
        const tsl::ordered_map<int, ModType>& testModTypes() const { return modTypes; }
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
    check(log->constructions == 2, "a second file builds its own remover rather than reusing one");
    check(resultB == "removed-by-tagged", "and still gets a working remover");

    // removeFix keeps nothing: a second removal on the same file builds again.
    fileA.removeFix(false, false);
    check(log->constructions == 3, "a repeat removal on the same file builds again -- nothing is cached on the IniFile");

    // And a remover built for one file is never handed to another, so there is no binding to
    // re-point. This is what the flyweight made fragile and what dropping it fixed.
    std::shared_ptr<BaseIniRemover<>> ownA = builder->build(&fileA);
    std::shared_ptr<BaseIniRemover<>> ownB = builder->build(&fileB);
    check(ownA != ownB, "two files' removers are distinct objects");
    check(ownA->getIniFile() == &fileA && ownB->getIniFile() == &fileB, "and each stays bound to its own file");
}

// A whole fix, boilerplate and all, for the end-to-end tests below.
const std::string FixedIni =
    "[TextureOverrideFooBlend]" + std::string(1, '\n') +
    "vb1 = ResourceFooBlend\n"
    "\n"
    "[ResourceFooBlend]\n"
    "filename = FooBlend.buf\n"
    "\n"
    "; --------------- Raiden Remap ---------------\n"
    "\n"
    "[TextureOverrideFooRemapBlend]\n"
    "vb1 = ResourceFooRemapBlend\n"
    "\n"
    "[ResourceFooRemapBlend]\n"
    "filename = FooRemapBlend.buf\n"
    "\n"
    "; --------------------------------------------\n";


void testExactlyOnePassSweeps() {
    std::printf("\n== the sweep goes to the last pass ==\n");

    // Two mod types, two builders, two removers over one file. Every pass but the last asks
    // RemapIniRemover's strict question; the last is handed IniRemovalContext::ignoreModType.
    const int amberId = static_cast<int>(ModTypeId::Amber);
    const int ayakaId = static_cast<int>(ModTypeId::Ayaka);

    auto log = std::make_shared<BuildLog>();
    auto amberBuilder = std::make_shared<IniRemoveBuilder>(countingFactory(log, "amber"));
    auto ayakaBuilder = std::make_shared<IniRemoveBuilder>(countingFactory(log, "ayaka"));

    std::unordered_map<int, ModType> overrides;
    overrides.emplace(amberId, ModType(static_cast<int>(GameTypeId::GI), amberId, "Amber", std::vector<std::string>{},
                                       nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, amberBuilder));
    overrides.emplace(ayakaId, ModType(static_cast<int>(GameTypeId::GI), ayakaId, "Ayaka", std::vector<std::string>{},
                                       nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, ayakaBuilder));

    std::unordered_set<int> forced = {amberId, ayakaId};
    TestableIniFile file(std::nullopt, "x\n", std::nullopt, std::nullopt, forced, std::move(overrides), nullptr);

    file.removeFix(false, false);

    check(log->constructions == 2, "one remover was built per mod type");
    check(log->removals.size() == 2, "and each of them ran exactly once");

    int amberRuns = 0;
    int ayakaRuns = 0;
    int sweeps = 0;
    for (const std::pair<std::string, bool>& removal : log->removals) {
        if (removal.first == "amber") {
            ++amberRuns;
        } else if (removal.first == "ayaka") {
            ++ayakaRuns;
        }

        if (removal.second) {
            ++sweeps;
        }
    }

    check(amberRuns == 1 && ayakaRuns == 1, "every mod type's remover ran, and only once");

    // Which of the two draws the sweep is unordered_map iteration order, so the assertion is on the
    // count, not on the identity -- see removeFix's own note.
    check(sweeps == 1, "exactly one of the two passes was given the sweep");
    check(log->removals.back().second, "and it was the LAST pass that got it");
}


void testDefaultChainReallyRemoves() {
    std::printf("\n== the default chain is live ==\n");

    // No factory override anywhere: a stock ModType, whose iniRemoveBuilder is the global default,
    // whose defaultFactory is RemapIniRemover<>::factory(). This is the whole chain, unmocked.
    const int modTypeId = static_cast<int>(ModTypeId::Amber);
    std::unordered_map<int, ModType> overrides;
    overrides.emplace(modTypeId, ModType(static_cast<int>(GameTypeId::GI), modTypeId, "Amber"));

    std::unordered_set<int> forced = {modTypeId};
    TestableIniFile file(std::nullopt, FixedIni, std::nullopt, std::nullopt, forced, std::move(overrides), nullptr);

    std::string result = file.removeFix(false, false);

    check(result.find("Remap") == std::string::npos, "the fix is gone from the returned text");
    check(result.find("[TextureOverrideFooBlend]") != std::string::npos, "the original mod is left alone");
    check(result.find("Raiden Remap") == std::string::npos, "and so is the boilerplate heading");
    check(result != FixedIni, "removeFix is no longer a no-op");
}


void testUnclassifiedFileFallsBackToTheGlobalBuilder() {
    std::printf("\n== unclassified fallback ==\n");

    // No forced types and no overrides: classify() finds nothing, so modTypes stays empty -- the
    // pure-Python "availableType is None" state.
    IniFile file(std::nullopt, FixedIni);
    std::string result = file.removeFix(false, false);

    check(file.getModTypes().empty(), "the file really is unclassified");
    check(result.find("Remap") == std::string::npos,
          "an unclassified file still gets its fix stripped, through the global builder");
    check(result.find("[TextureOverrideFooBlend]") != std::string::npos, "and keeps the original mod");
}


void testNoBuilderMeansNoRemoval() {
    std::printf("\n== no remove builder ==\n");

    const int modTypeId = static_cast<int>(ModTypeId::Amber);

    // The constructor fills in the global fallback, so nulling it afterwards is the only way to
    // reach the "no remove builder at all" state removeFix guards against.
    //
    // Note this is NOT the unclassified case above: this file has a mod type, that mod type says it
    // has no remover to offer, and removeFix takes it at its word rather than reaching for the
    // global one. See removeFix's own note.
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
    testNoCaching();
    testBuildAlwaysBinds();
    testArgsRepoFlavour();
    testDefaults();
    testGlobalRemoveBuilder();
    testIniFileUsesTheBuilder();
    testExactlyOnePassSweeps();
    testDefaultChainReallyRemoves();
    testUnclassifiedFileFallsBackToTheGlobalBuilder();
    testNoBuilderMeansNoRemoval();

    std::printf("\n%s (%d failure(s))\n", (failures == 0 ? "ALL PASSED" : "FAILURES"), failures);
    return (failures == 0) ? 0 : 1;
}
