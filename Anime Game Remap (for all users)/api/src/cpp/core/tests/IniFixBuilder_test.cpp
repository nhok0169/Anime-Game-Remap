// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniFixBuilder
// (model/strategies/iniFixers/IniFixBuilder.h) and the IniFile/ModType rewiring
// that goes with it -- ModType now holds a *builder* rather than a single shared
// fixer instance, and IniFile::fix builds one fixer per mod type, per file,
// bound to that file's own built parser and picked using the file's own
// IniFile::version.
//
// The exact sibling of IniParseBuilder_test.cpp; read that one first. The
// behaviours below are checked against the pure-Python IniFixBuilder
// (model/strategies/iniFixers/IniFixBuilder.py) + IniFixBuilderArgs
// (model/assets/IniFixBuilderArgs.py) pair.
//
// Covers:
//   * Fixed-factory flavour: same factory every build, key args ignored
//   * An empty std::function falls back to defaultFactory(); so does a nullptr
//     ArgsRepo and the default constructor
//   * Version-dependent flavour: the FOUR-column
//     {fromVersion, fromModName, toVersion, toModName} lookup, with TWO version
//     columns, inclusive floor-matching, and nullopt-version-means-latest
//   * buildAll(): the fan-out that returns one fixer PER TARGET MOD for a given
//     (fromVersion, fromModName, toVersion), each target resolving its own
//     toVersion independently -- plus the filteredToModNames filter
//   * errorOnNotFound false (default) falls back / true throws std::out_of_range
//   * Each build() produces a fresh fixer, already bound to the parser it was
//     given -- the fix-side counterpart of "no setIniFile rebinding"
//   * ModType's null-fallback for iniFixBuilder, and that copying a ModType
//     shares the builder rather than cloning it
//   * IniFile end-to-end: fix() builds one fixer per classified ModType using
//     the file's own version, binds it to that file's built parser, caches it
//     (a second fix() reuses it), and clear() drops the cache
//   * A mod type whose parser cannot be built gets no fixer either -- the
//     equivalent of the original's _getFixer refusing while _iniParser is None
//
// Same build story as IniParseBuilder_test.cpp: it drives IniFile::parse/fix, so
// it cannot be built from a hand-picked source list (that pulls in the whole
// Z3/IfTemplate half of the core). Link the already-built static lib instead
// (`cd cbuild && ninja AGRemapCore`):
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      IniFixBuilder_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags
// are load-bearing. Copy libz3.dll next to test.exe before running.
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"

#include <cstdio>
#include <map>
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

// Reports which row of the args table built it, standing in for the concrete GIMIFixer/
// GIMIObjRegEditFixer subclasses the pure-Python IniFixBuilderData picks between.
class TaggedFixer: public BaseIniFixer<> {
    public:
        TaggedFixer(BaseIniParser<>* parser, std::string tag): BaseIniFixer<>(parser), tag(std::move(tag)) {}

        std::string tag;

        // Counts how many times this instance was run, so a test can prove IniFile reuses one
        // built fixer rather than building a second.
        int fixCount = 0;

    protected:
        // The 7th parameter (IniFixingContext) was added to BaseIniFixer::fixImpl after this
        // file was written; nothing builds core/tests/*, so it went unnoticed until a rebuild.
        FixResult fixImpl(ParseData&, bool, bool, bool, bool, bool, IniFixingContext) override {
            ++fixCount;
            return {};
        }
};

class TaggedParser: public BaseIniParser<> {
    public:
        using BaseIniParser<>::BaseIniParser;

        std::vector<IniGraphGroup<>> parse() override {
            return {};
        }
};

std::string tagOf(const std::shared_ptr<BaseIniFixer<>>& fixer) {
    TaggedFixer* tagged = dynamic_cast<TaggedFixer*>(fixer.get());
    return (tagged != nullptr) ? tagged->tag : std::string("<untagged>");
}

IniFixBuilder::Factory taggedFactory(std::string tag) {
    return [tag](BaseIniParser<>* parser) {
        return std::make_shared<TaggedFixer>(parser, tag);
    };
}

Version ver(const std::string& raw) {
    std::optional<Version> parsed = Version::parse(raw);
    if (!parsed.has_value()) {
        std::printf("[FATAL] test bug: '%s' is not a parsable version\n", raw.c_str());
        std::exit(1);
    }
    return *parsed;
}

// Four columns: fromVersion, fromModName, toVersion, toModName -- with the TWO version columns at
// positions 0 and 2, which is why this is a ModAssets rather than a ModDictAssets.
std::shared_ptr<IniFixBuilder::ArgsRepo> makeArgsRepo(std::vector<Row<std::string, IniFixBuilder::Factory>> rows) {
    return std::make_shared<IniFixBuilder::ArgsRepo>(
        std::vector<bool>{true, false, true, false},
        [](const std::string& raw) { return Version::parse(raw); },
        std::move(rows));
}

// Shorthand for a row: fromVersion "1.0" throughout, matching how IniFixBuilderData is laid out.
Row<std::string, IniFixBuilder::Factory> row(const std::string& fromMod, const std::string& toVer,
                                             const std::string& toMod, IniFixBuilder::Factory f) {
    return Row<std::string, IniFixBuilder::Factory>{{"1.0", fromMod, toVer, toMod}, std::move(f)};
}

// ---------------------------------------------------------------------------

void testFixedFactory() {
    std::printf("\n== fixed-factory flavour ==\n");

    IniFixBuilder builder(taggedFactory("fixed"));
    check(builder.getBuilderArgs() == nullptr, "a fixed-factory builder has no ArgsRepo");
    check(tagOf(builder.build(nullptr, "Amber", "AmberCN")) == "fixed", "fixed factory used regardless of mod name");
    check(tagOf(builder.build(nullptr, "Jean", "JeanCN", ver("1.0"), ver("5.7"))) == "fixed", "fixed factory used regardless of version");

    std::shared_ptr<BaseIniFixer<>> first = builder.build(nullptr, "Amber", "AmberCN");
    std::shared_ptr<BaseIniFixer<>> second = builder.build(nullptr, "Amber", "AmberCN");
    check(first != second, "each build() produces a fresh fixer instance");

    // A fixer is built from the *parser*, and arrives already bound to it -- the fix-side
    // counterpart of the parse side's "already bound to the IniFile".
    IniFile file(std::nullopt, "x\n");
    TaggedParser parser(&file);
    std::shared_ptr<BaseIniFixer<>> bound = builder.build(&parser, "Amber", "AmberCN");
    check(bound->getParser() == &parser, "the built fixer is already bound to the given parser");
    check(bound->getIniFile() == &file, "the built fixer picks up the parser's own IniFile");
    check(builder.build(nullptr, "Amber", "AmberCN")->getParser() == nullptr, "a nullptr parser builds an unbound fixer");
}

void testDefaultFallbacks() {
    std::printf("\n== defaultFactory fallbacks ==\n");

    IniFixBuilder defaulted;
    std::shared_ptr<BaseIniFixer<>> fromDefault = defaulted.build(nullptr, "Amber", "AmberCN");
    check(fromDefault != nullptr, "the default constructor still builds a fixer");
    check(dynamic_cast<TaggedFixer*>(fromDefault.get()) == nullptr, "the default constructor builds a plain BaseIniFixer");

    IniFixBuilder emptyFactory{IniFixBuilder::Factory{}};
    check(emptyFactory.build(nullptr, "Amber", "AmberCN") != nullptr, "an empty factory falls back to defaultFactory rather than crashing");

    IniFixBuilder nullRepo{std::shared_ptr<const IniFixBuilder::ArgsRepo>{}};
    check(nullRepo.build(nullptr, "Amber", "AmberCN") != nullptr, "a nullptr ArgsRepo degrades to the default factory");
}

void testVersionDependentLookup() {
    std::printf("\n== version-dependent flavour (4 columns, 2 version columns) ==\n");

    // Shaped like the real IniFixBuilderData: fromVersion 1.0 throughout, a 4.0 baseline row per
    // target, and a later toVersion row for the one that changed. Jean fixes to TWO targets.
    auto repo = makeArgsRepo({
        row("Amber", "4.0", "AmberCN", taggedFactory("amber4_0")),
        row("Amber", "5.7", "AmberCN", taggedFactory("amber5_7")),
        row("Jean", "4.0", "JeanCN", taggedFactory("jean4_0-CN")),
        row("Jean", "4.0", "JeanSea", taggedFactory("jean4_0-Sea")),
        row("Jean", "5.5", "JeanCN", taggedFactory("jean5_5-CN")),
    });

    IniFixBuilder builder(repo);
    check(builder.getBuilderArgs() == repo, "the builder exposes the ArgsRepo it was given");
    check(!builder.getErrorOnNotFound(), "errorOnNotFound defaults to false");
    check(repo->getVersionColumnCount() == 2, "the table really has two version columns");
    check(repo->getNonVersionColumnCount() == 2, "and two non-version columns");

    // Exact key: both mod names and both versions participate.
    check(tagOf(builder.build(nullptr, "Amber", "AmberCN", ver("1.0"), ver("4.0"))) == "amber4_0",
          "an exact 4-column key picks that row");
    check(tagOf(builder.build(nullptr, "Amber", "AmberCN", ver("1.0"), ver("5.7"))) == "amber5_7",
          "a later toVersion supersedes the older row");
    check(tagOf(builder.build(nullptr, "Amber", "AmberCN", ver("1.0"), ver("4.5"))) == "amber4_0",
          "a toVersion between rows floor-matches the older one");

    // The TARGET mod is part of the key, not just the source.
    check(tagOf(builder.build(nullptr, "Jean", "JeanCN", ver("1.0"), ver("4.0"))) == "jean4_0-CN",
          "the target mod selects among rows sharing a source and version");
    check(tagOf(builder.build(nullptr, "Jean", "JeanSea", ver("1.0"), ver("4.0"))) == "jean4_0-Sea",
          "a different target resolves to a genuinely different fixer");

    // Each target floor-matches its own toVersion: JeanCN has a 5.5 row, JeanSea does not.
    check(tagOf(builder.build(nullptr, "Jean", "JeanCN", ver("1.0"), ver("5.5"))) == "jean5_5-CN",
          "JeanCN picks up its own 5.5 row");
    check(tagOf(builder.build(nullptr, "Jean", "JeanSea", ver("1.0"), ver("5.5"))) == "jean4_0-Sea",
          "while JeanSea, having no 5.5 row, stays on its 4.0 one");

    check(tagOf(builder.build(nullptr, "Amber", "AmberCN")) == "amber5_7",
          "nullopt versions resolve to the latest listed row");
}

void testBuildAllFanOut() {
    std::printf("\n== buildAll: one fixer per target mod ==\n");

    auto repo = makeArgsRepo({
        row("Jean", "4.0", "JeanCN", taggedFactory("jean4_0-CN")),
        row("Jean", "4.0", "JeanSea", taggedFactory("jean4_0-Sea")),
        row("Jean", "5.5", "JeanCN", taggedFactory("jean5_5-CN")),
        row("Amber", "4.0", "AmberCN", taggedFactory("amber4_0")),
    });
    IniFixBuilder builder(repo);

    auto tagsOf = [](const std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>>& built) {
        std::map<std::string, std::string> out;
        for (const auto& e : built) { out[e.first] = tagOf(e.second); }
        return out;
    };

    // THE point of buildAll: Jean fixes to two targets, so both come back from one call.
    std::map<std::string, std::string> jean = tagsOf(builder.buildAll(nullptr, "Jean", ver("1.0"), ver("4.0")));
    check(jean.size() == 2, "a source mod with two targets yields two fixers");
    check(jean["JeanCN"] == "jean4_0-CN", "keyed by target mod name, JeanCN");
    check(jean["JeanSea"] == "jean4_0-Sea", "and JeanSea");

    // Each target resolves its own toVersion -- JeanCN has a 5.5 row, JeanSea does not. A single
    // global version resolution would have dropped JeanSea entirely.
    std::map<std::string, std::string> jean55 = tagsOf(builder.buildAll(nullptr, "Jean", ver("1.0"), ver("5.5")));
    check(jean55.size() == 2, "at 5.5 BOTH targets still come back");
    check(jean55["JeanCN"] == "jean5_5-CN", "JeanCN advances to its own 5.5 row");
    check(jean55["JeanSea"] == "jean4_0-Sea", "while JeanSea stays on 4.0 rather than disappearing");

    // A single-target mod yields exactly one.
    check(builder.buildAll(nullptr, "Amber", ver("1.0"), ver("4.0")).size() == 1,
          "a source mod with one target yields one fixer");

    // The filter narrows the fan-out.
    std::unordered_set<std::string> onlySea = {"JeanSea"};
    std::map<std::string, std::string> filtered = tagsOf(builder.buildAll(nullptr, "Jean", ver("1.0"), ver("4.0"), onlySea));
    check(filtered.size() == 1 && filtered.count("JeanSea") == 1, "filteredToModNames narrows to just the named target");

    std::unordered_set<std::string> none;
    check(builder.buildAll(nullptr, "Jean", ver("1.0"), ver("4.0"), none).empty(),
          "an EMPTY filter set excludes everything -- distinct from nullopt meaning no filter");
    check(builder.buildAll(nullptr, "Jean", ver("1.0"), ver("4.0"), std::nullopt).size() == 2,
          "while nullopt means no filtering at all");

    // Nothing matched is an empty vector, not a throw.
    check(builder.buildAll(nullptr, "NotAMod", ver("1.0"), ver("4.0")).empty(),
          "an unknown source mod yields an empty vector rather than throwing");

    // A fixed-factory builder has no targets; one entry under the empty name keeps callers uniform.
    IniFixBuilder fixed(taggedFactory("fixed"));
    std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>> fixedAll = fixed.buildAll(nullptr, "Jean");
    check(fixedAll.size() == 1, "a fixed-factory builder yields exactly one entry");
    check(fixedAll.size() == 1 && fixedAll[0].first.empty(), "keyed by the empty string, since it has no targets");
}

void testNotFoundBehaviour() {
    std::printf("\n== unlisted mod names ==\n");

    auto repo = makeArgsRepo({row("Amber", "4.0", "AmberCN", taggedFactory("amber4_0"))});

    IniFixBuilder lenient(repo);
    std::shared_ptr<BaseIniFixer<>> fallback = lenient.build(nullptr, "NotListed", "AmberCN", ver("1.0"), ver("4.0"));
    check(fallback != nullptr, "an unlisted mod name falls back to a fixer rather than returning null");
    check(dynamic_cast<TaggedFixer*>(fallback.get()) == nullptr, "the fallback is the plain default fixer, not some other row");

    IniFixBuilder strict(repo, true);
    check(strict.getErrorOnNotFound(), "errorOnNotFound is reported back");

    bool threw = false;
    try {
        strict.build(nullptr, "NotListed", "AmberCN", ver("1.0"), ver("4.0"));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    check(threw, "errorOnNotFound = true throws std::out_of_range for an unlisted mod name");
    check(tagOf(strict.build(nullptr, "Amber", "AmberCN", ver("1.0"), ver("4.0"))) == "amber4_0", "errorOnNotFound = true still resolves a listed mod name");
}

void testModTypeFallbacks() {
    std::printf("\n== ModType null-fallbacks ==\n");

    ModType bare(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber");
    check(bare.iniFixBuilder != nullptr, "a ModType with no fix builder gets a default-constructed IniFixBuilder");
    check(bare.iniFixBuilder->getBuilderArgs() == nullptr, "that fallback builder is the fixed-factory flavour");

    // The remover has its own builder -- covered properly in IniRemoveBuilder_test.cpp; just
    // checked here for the ModType fallback's sake.
    check(bare.iniRemoveBuilder != nullptr, "a ModType with no remove builder still gets one");

    ModType copy = bare;
    check(copy.iniFixBuilder == bare.iniFixBuilder, "copying a ModType shares its fix builder rather than cloning it");
}

// Exposes IniFile's protected modTypes so a test can reach the ModType it was given.
class TestableIniFile: public IniFile {
    public:
        using IniFile::IniFile;

        // modTypes is a tsl::ordered_map now -- insertion order decides which mod type takes
        // the .ini file's backup and which hides the original (see IniFile::fix).
        const tsl::ordered_map<int, ModType>& testModTypes() const { return modTypes; }
};

// What a fix factory recorded each time IniFile asked it to build. Observing built fixers this way
// keeps the test off IniFile's private members.
struct BuildRecord {
    std::string tag;
    BaseIniParser<>* boundTo = nullptr;
    TaggedFixer* built = nullptr;
};

void testIniFileUsesTheBuilder() {
    std::printf("\n== IniFile end-to-end ==\n");

    auto log = std::make_shared<std::vector<BuildRecord>>();
    auto loggingFactory = [log](std::string tag) {
        // The Factory takes the target mod now, so the default fixer can tell its GIMIFixer which
        // mod it is fixing onto. Ignored here -- this fake only records which factory ran.
        return IniFixBuilder::Factory{[log, tag](BaseIniParser<>* parser, const std::string&) {
            auto fixer = std::make_shared<TaggedFixer>(parser, tag);
            log->push_back(BuildRecord{tag, parser, fixer.get()});
            return fixer;
        }};
    };

    auto repo = makeArgsRepo({
        row("Amber", "4.0", "AmberCN", loggingFactory("amber4_0")),
        row("Amber", "5.7", "AmberCN", loggingFactory("amber5_7")),
    });

    const int modTypeId = static_cast<int>(ModTypeId::Amber);

    auto makeOverrides = [&]() {
        std::unordered_map<int, ModType> overrides;
        overrides.emplace(modTypeId, ModType(static_cast<int>(GameTypeId::GI), modTypeId, "Amber", std::vector<std::string>{},
                                             /*hashes*/ nullptr, /*indices*/ nullptr,
                                             /*vertexCounts*/ nullptr, /*vgRemaps*/ nullptr,
                                             /*iniParseBuilder*/ nullptr,
                                             std::make_shared<IniFixBuilder>(repo)));
        return overrides;
    };

    std::unordered_set<int> forced = {modTypeId};

    // fromVersion is 1.0 on both -- matching every row in the table -- so it is the *toVersion*
    // that selects which fixer each file gets. That is the whole point of the two version columns:
    // the source version and the target version are independent axes.
    TestableIniFile oldFile(std::nullopt, "x\n", std::nullopt, std::nullopt, forced, makeOverrides(), nullptr,
                            std::nullopt, DownloadMode::Normal, ver("1.0"), ver("4.0"));
    TestableIniFile newFile(std::nullopt, "x\n", std::nullopt, std::nullopt, forced, makeOverrides(), nullptr,
                            std::nullopt, DownloadMode::Normal, ver("1.0"), ver("5.7"));

    // parse() alone must not build any fixer -- only fix() does.
    oldFile.parse();
    check(log->empty(), "parse() alone builds no fixer");

    oldFile.fix();
    newFile.fix();

    check(log->size() == 2, "one fixer built per .ini file");
    check(log->size() == 2 && (*log)[0].tag == "amber4_0", "a 4.0 .ini file gets the 4.0 fixer");
    check(log->size() == 2 && (*log)[1].tag == "amber5_7", "a 5.7 .ini file of the same mod type gets the 5.7 fixer");

    // Each fixer is bound to its own file's parser, so two IniFiles of the same mod type no longer
    // stomp each other -- the problem the old shared-instance + setParser design had.
    check(log->size() == 2 && (*log)[0].boundTo != nullptr && (*log)[0].boundTo->getIniFile() == &oldFile,
          "each file's fixer is bound to that file's own parser");
    check(log->size() == 2 && (*log)[1].boundTo != nullptr && (*log)[1].boundTo->getIniFile() == &newFile,
          "the second file's fixer is bound to the second file's parser");
    check(log->size() == 2 && (*log)[0].boundTo != (*log)[1].boundTo, "the two files got distinct parsers");
    check(log->size() == 2 && (*log)[0].built != (*log)[1].built, "the two files got distinct fixer instances");

    TaggedFixer* oldFixer = (*log)[0].built;

    // Cached per mod type, the analogue of the original's self._iniFixer.
    int fixesBefore = oldFixer->fixCount;
    oldFile.fix();
    check(log->size() == 2, "a second fix() reuses the cached fixer rather than building another");
    check(oldFixer->fixCount > fixesBefore, "the cached fixer is re-run rather than skipped");

    // clear() drops the cache, matching the original's "self._iniFixer = None".
    oldFile.clear();
    oldFile.fix();
    check(log->size() == 3, "clear() drops the cache so the next fix builds a fresh fixer");
    check(log->size() == 3 && (*log)[2].built != oldFixer, "the post-clear fixer is a genuinely new instance");
}

void testNoParserMeansNoFixer() {
    std::printf("\n== no parser, no fixer ==\n");

    auto log = std::make_shared<std::vector<BuildRecord>>();
    auto repo = makeArgsRepo({row("Amber", "4.0", "AmberCN", IniFixBuilder::Factory{[log](BaseIniParser<>* parser, const std::string&) {
        auto fixer = std::make_shared<TaggedFixer>(parser, "amber4_0");
        log->push_back(BuildRecord{"amber4_0", parser, fixer.get()});
        return fixer;
    }})});

    const int modTypeId = static_cast<int>(ModTypeId::Amber);

    // A ModType whose parse builder is explicitly nulled out AFTER construction -- the constructor
    // fills in a fallback, so this is the only way to reach the "no parser at all" state that the
    // pure-Python original's '_getFixer' guards against with "self._iniParser is not None".
    std::unordered_map<int, ModType> overrides;
    ModType modType(static_cast<int>(GameTypeId::GI), modTypeId, "Amber", std::vector<std::string>{},
                    /*hashes*/ nullptr, /*indices*/ nullptr, /*vertexCounts*/ nullptr,
                    /*vgRemaps*/ nullptr, /*iniParseBuilder*/ nullptr,
                    std::make_shared<IniFixBuilder>(repo));
    modType.iniParseBuilder = nullptr;
    overrides.emplace(modTypeId, std::move(modType));

    std::unordered_set<int> forced = {modTypeId};
    TestableIniFile file(std::nullopt, "x\n", std::nullopt, std::nullopt, forced, std::move(overrides), nullptr,
                         std::nullopt, DownloadMode::Normal, ver("4.0"));

    std::unordered_map<std::string, std::string> result = file.fix();
    check(log->empty(), "a mod type with no parse builder builds no fixer either");
    check(result.empty(), "and contributes nothing to the fix result, rather than aborting the run");
}

}  // namespace

int main() {
    testFixedFactory();
    testDefaultFallbacks();
    testVersionDependentLookup();
    testBuildAllFanOut();
    testNotFoundBehaviour();
    testModTypeFallbacks();
    testIniFileUsesTheBuilder();
    testNoParserMeansNoFixer();

    std::printf("\n%s (%d failure(s))\n", (failures == 0 ? "ALL PASSED" : "FAILURES"), failures);
    return (failures == 0) ? 0 : 1;
}
