// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniParseBuilder
// (model/strategies/iniParsers/IniParseBuilder.h) and the IniFile/ModType
// rewiring that goes with it -- ModType now holds a *builder* rather than a
// single shared parser instance, and IniFile::parse builds one parser per mod
// type, per file, passing that file's own IniFile::version along.
//
// This is the C++ counterpart to the pure-Python IniParseBuilder
// (model/strategies/iniParsers/IniParseBuilder.py) + IniParseBuilderArgs
// (model/assets/IniParseBuilderArgs.py) pair, so the behaviours below are
// checked against that original's documented contract.
//
// Covers:
//   * Fixed-factory flavour: the same factory is used for every build, and
//     modName/version are ignored entirely (the original's "this argument has
//     no effect if _buildCls is not None")
//   * An empty std::function passed as the factory falls back to
//     defaultFactory(), mirroring the original's "if (iniParseBuilder is None)"
//   * Version-dependent flavour: a {version, modName} ArgsRepo picks a
//     different factory per mod name, and per version
//   * Inclusive floor-matching on version -- a 4.0 row keeps applying at 4.5,
//     a 5.7 row supersedes it from 5.7 on, and a version older than every row
//     still resolves to the oldest row (matching ModDictAssets::get)
//   * A std::nullopt version resolves to the latest listed row
//   * errorOnNotFound = false (the default) falls back to defaultFactory() for
//     a mod name with no row at any version; errorOnNotFound = true throws
//     std::out_of_range instead, matching the pure-Python original
//   * A nullptr ArgsRepo degrades to the default-constructor behaviour
//   * Each build() produces a *fresh* parser, already bound to the IniFile it
//     was given -- no setIniFile rebinding, which is the whole point of holding
//     a builder rather than a shared parser instance
//   * Whatever a row's lambda captures is shared across builds (the observable
//     equivalent of the original's @lru_cache'd (cls, args, kwargs) triple)
//   * ModType's null-fallback: a ModType built with no builder gets a
//     default-constructed IniParseBuilder, and its fixer is left unbound
//   * IniFile end-to-end: parse() builds one parser per classified ModType
//     using the file's own version, caches it (a second parse()/fix() reuses
//     the same parser instance), and clear() drops the cache so the next parse
//     builds fresh ones
//
// Unlike IniFile_classify_test.cpp, this one cannot be built from a hand-picked
// source list: it drives IniFile::parse/fix, which pull in getIfTemplates and so
// the whole Z3/IfTemplate half of the core. Link the already-built static lib
// instead (see Building/CLAUDE.md's "cd cbuild && ninja AGRemapCore" shortcut):
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      IniParseBuilder_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// The three /NODEFAULTLIB flags matter: AGRemapCore.lib is built against the
// DLL CRT (/MD), while utf8proc.lib/libz3.lib drag in /DEFAULTLIB directives for
// the static one, and the link fails with a wall of LNK2005 "already defined in
// libcpmt.lib" (or, with /MT, the mirror-image LNK2019 __imp_ std:: symbols)
// until the static variants are excluded. Copy libz3.dll and utf8proc's DLL next
// to test.exe before running it.
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"

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

// A parser that reports which "flavour" built it, so a test can tell which row of the args table
// actually won. Stands in for the concrete GIMIParser/GIMIObjParser subclasses the pure-Python
// IniParseBuilderData picks between, none of which exist in C++ yet.
class TaggedParser: public BaseIniParser<> {
    public:
        TaggedParser(IniFile* iniFile, std::string tag): BaseIniParser<>(iniFile), tag(std::move(tag)) {}

        std::string tag;

        // Counts how many times this specific instance was parsed, so a test can prove IniFile
        // reuses one built parser rather than building a second.
        int parseCount = 0;

        std::vector<IniGraphGroup<>> parse() override {
            ++parseCount;
            return {};
        }
};

std::string tagOf(const std::shared_ptr<BaseIniParser<>>& parser) {
    TaggedParser* tagged = dynamic_cast<TaggedParser*>(parser.get());
    return (tagged != nullptr) ? tagged->tag : std::string("<untagged>");
}

IniParseBuilder::Factory taggedFactory(std::string tag) {
    return [tag](IniFile* iniFile) {
        return std::make_shared<TaggedParser>(iniFile, tag);
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

// The version index sits at position 0 and the mod name at position 1, matching the pure-Python
// IniParseBuilderArgs' own ["version", "name"] index order.
std::shared_ptr<IniParseBuilder::ArgsRepo> makeArgsRepo(std::vector<Row<std::string, IniParseBuilder::Factory>> rows) {
    return std::make_shared<IniParseBuilder::ArgsRepo>(
        /*totalIndices*/ 2, /*versionIndexPos*/ 0,
        [](const std::string& raw) { return Version::parse(raw); },
        std::move(rows));
}

// ---------------------------------------------------------------------------

void testFixedFactory() {
    std::printf("\n== fixed-factory flavour ==\n");

    IniParseBuilder builder(taggedFactory("fixed"));
    check(builder.getBuilderArgs() == nullptr, "a fixed-factory builder has no ArgsRepo");

    // modName/version are documented as having no effect for this flavour -- the original's
    // "this argument has no effect if _buildCls is not None".
    check(tagOf(builder.build(nullptr, "Amber")) == "fixed", "fixed factory used regardless of mod name");
    check(tagOf(builder.build(nullptr, "Jean", ver("5.7"))) == "fixed", "fixed factory used regardless of version");

    // Each build is a distinct object -- this is what replaces the old shared-instance design.
    std::shared_ptr<BaseIniParser<>> first = builder.build(nullptr, "Amber");
    std::shared_ptr<BaseIniParser<>> second = builder.build(nullptr, "Amber");
    check(first != second, "each build() produces a fresh parser instance");

    // The IniFile the factory was handed is what the parser ends up bound to, so nothing has to
    // rebind it afterwards.
    IniFile file(std::nullopt, "x\n");
    check(builder.build(&file, "Amber")->getIniFile() == &file, "the built parser is already bound to the given IniFile");
    check(builder.build(nullptr, "Amber")->getIniFile() == nullptr, "a nullptr IniFile builds an unbound parser");
}

void testDefaultFallbacks() {
    std::printf("\n== defaultFactory fallbacks ==\n");

    IniParseBuilder defaulted;
    std::shared_ptr<BaseIniParser<>> fromDefault = defaulted.build(nullptr, "Amber");
    check(fromDefault != nullptr, "the default constructor still builds a parser");
    check(dynamic_cast<TaggedParser*>(fromDefault.get()) == nullptr, "the default constructor builds a plain BaseIniParser");

    // An empty std::function is this API's version of the original being handed None.
    IniParseBuilder emptyFactory{IniParseBuilder::Factory{}};
    check(emptyFactory.build(nullptr, "Amber") != nullptr, "an empty factory falls back to defaultFactory rather than crashing");

    // A nullptr table degrades the same way, rather than dereferencing it in build().
    IniParseBuilder nullRepo{std::shared_ptr<const IniParseBuilder::ArgsRepo>{}};
    check(nullRepo.build(nullptr, "Amber") != nullptr, "a nullptr ArgsRepo degrades to the default factory");
}

void testVersionDependentLookup() {
    std::printf("\n== version-dependent flavour ==\n");

    // Deliberately shaped like the real IniParseBuilderData: a 4.0 baseline row for every mod, plus
    // a later row for the one mod whose parser actually changed.
    auto repo = makeArgsRepo({
        {{"4.0", "Amber"}, taggedFactory("amber4_0")},
        {{"5.7", "Amber"}, taggedFactory("amber5_7")},
        {{"4.0", "Jean"}, taggedFactory("jean4_0")},
    });

    IniParseBuilder builder(repo);
    check(builder.getBuilderArgs() == repo, "the builder exposes the ArgsRepo it was given");
    check(!builder.getErrorOnNotFound(), "errorOnNotFound defaults to false");

    check(tagOf(builder.build(nullptr, "Amber", ver("4.0"))) == "amber4_0", "an exact version match picks that row");
    check(tagOf(builder.build(nullptr, "Jean", ver("4.0"))) == "jean4_0", "the mod name selects among rows at the same version");

    // Inclusive floor-match: this is what makes "the 4.0 row keeps applying until 5.7 supersedes
    // it" work, and is the behaviour ModDictAssets::get documents.
    check(tagOf(builder.build(nullptr, "Amber", ver("4.5"))) == "amber4_0", "a version between rows floor-matches the older row");
    check(tagOf(builder.build(nullptr, "Amber", ver("5.7"))) == "amber5_7", "a later row supersedes the older one from its own version on");
    check(tagOf(builder.build(nullptr, "Amber", ver("6.2"))) == "amber5_7", "a version past every row floor-matches the newest row");
    check(tagOf(builder.build(nullptr, "Amber", ver("3.1"))) == "amber4_0", "a version older than every row still resolves to the oldest");
    check(tagOf(builder.build(nullptr, "Jean", ver("5.7"))) == "jean4_0", "a mod with only an old row keeps using it at later versions");

    // "No version" means "latest", matching the original's version = None.
    check(tagOf(builder.build(nullptr, "Amber")) == "amber5_7", "a nullopt version resolves to the latest listed row");
    check(tagOf(builder.build(nullptr, "Jean")) == "jean4_0", "a nullopt version resolves per mod name, not globally");
}

void testNotFoundBehaviour() {
    std::printf("\n== unlisted mod names ==\n");

    auto repo = makeArgsRepo({{{"4.0", "Amber"}, taggedFactory("amber4_0")}});

    // Default: degrade to a plain parser, matching how IniFile::parse/fix skip a mod type with no
    // strategy rather than aborting the whole run.
    IniParseBuilder lenient(repo);
    std::shared_ptr<BaseIniParser<>> fallback = lenient.build(nullptr, "NotListed", ver("4.0"));
    check(fallback != nullptr, "an unlisted mod name falls back to a parser rather than returning null");
    check(dynamic_cast<TaggedParser*>(fallback.get()) == nullptr, "the fallback is the plain default parser, not some other row");

    // Opt-in: match the pure-Python original, whose ModAssets.get defaults to errorOnNotFound=True.
    IniParseBuilder strict(repo, true);
    check(strict.getErrorOnNotFound(), "errorOnNotFound is reported back");

    bool threw = false;
    try {
        strict.build(nullptr, "NotListed", ver("4.0"));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    check(threw, "errorOnNotFound = true throws std::out_of_range for an unlisted mod name");

    // A listed name must still resolve normally under the strict flag.
    check(tagOf(strict.build(nullptr, "Amber", ver("4.0"))) == "amber4_0", "errorOnNotFound = true still resolves a listed mod name");
}

void testCapturedStateIsShared() {
    std::printf("\n== captured row state ==\n");

    // The observable half of the original's @lru_cache(maxsize = 64): the *arguments* a row carries
    // are constructed once and shared across every build, while the parser itself is fresh each
    // time. Here the shared counter stands in for such an argument object.
    auto shared = std::make_shared<int>(0);
    auto repo = makeArgsRepo({{{"4.0", "Amber"}, [shared](IniFile* iniFile) {
        ++(*shared);
        return std::make_shared<TaggedParser>(iniFile, "amber4_0");
    }}});

    IniParseBuilder builder(repo);
    std::shared_ptr<BaseIniParser<>> a = builder.build(nullptr, "Amber", ver("4.0"));
    std::shared_ptr<BaseIniParser<>> b = builder.build(nullptr, "Amber", ver("4.0"));

    check(a != b, "each build() produces a fresh parser even from an args table");
    check(*shared == 2, "the row's captured state persists across builds rather than being rebuilt");
}

void testModTypeFallbacks() {
    std::printf("\n== ModType null-fallbacks ==\n");

    ModType bare(static_cast<int>(GameTypeId::GI), static_cast<int>(ModTypeId::Amber), "Amber");
    check(bare.iniParseBuilder != nullptr, "a ModType with no builder gets a default-constructed IniParseBuilder");
    check(bare.iniParseBuilder->getBuilderArgs() == nullptr, "that fallback builder is the fixed-factory flavour");
    check(bare.iniRemoveBuilder != nullptr, "a ModType with no remove builder still gets one");

    // A ModType stays copyable -- ModTypeIdTools::getModType returns one by value and
    // IniFile::modTypes stores them by value, and the builder is shared rather than cloned.
    ModType copy = bare;
    check(copy.iniParseBuilder == bare.iniParseBuilder, "copying a ModType shares its builder rather than cloning it");
}

// Exposes IniFile's protected modTypes so a test can reach the ModType it was given.
class TestableIniFile: public IniFile {
    public:
        using IniFile::IniFile;

        const std::unordered_map<int, ModType>& testModTypes() const { return modTypes; }
};

// What a factory recorded each time IniFile asked it to build something. Observing the built
// parsers this way keeps the test off IniFile's private members -- nothing about the production
// API is widened just so a test can look inside.
struct BuildRecord {
    std::string tag;
    IniFile* boundTo = nullptr;
    TaggedParser* built = nullptr;
};

void testIniFileUsesTheBuilder() {
    std::printf("\n== IniFile end-to-end ==\n");

    auto log = std::make_shared<std::vector<BuildRecord>>();
    auto loggingFactory = [log](std::string tag) {
        return IniParseBuilder::Factory{[log, tag](IniFile* iniFile) {
            auto parser = std::make_shared<TaggedParser>(iniFile, tag);
            log->push_back(BuildRecord{tag, iniFile, parser.get()});
            return parser;
        }};
    };

    auto repo = makeArgsRepo({
        {{"4.0", "Amber"}, loggingFactory("amber4_0")},
        {{"5.7", "Amber"}, loggingFactory("amber5_7")},
    });

    const int modTypeId = static_cast<int>(ModTypeId::Amber);

    auto makeOverrides = [&]() {
        std::unordered_map<int, ModType> overrides;
        overrides.emplace(modTypeId, ModType(static_cast<int>(GameTypeId::GI), modTypeId, "Amber", std::vector<std::string>{},
                                             /*hashes*/ nullptr, /*indices*/ nullptr, /*vertexCounts*/ nullptr,
                                             /*vgRemaps*/ nullptr,
                                             std::make_shared<IniParseBuilder>(repo)));
        return overrides;
    };

    // 'forcedModTypeIds' skips the classifier's own classification entirely, so these files land on
    // exactly the overridden ModType above without needing a fake classifier -- see
    // IniFile::classify.
    std::unordered_set<int> forced = {modTypeId};

    // The file's own version is what decides which row wins -- the whole point of the change.
    TestableIniFile oldFile(std::nullopt, "x\n", std::nullopt, std::nullopt, forced, makeOverrides(), nullptr,
                            std::nullopt, DownloadMode::Normal, ver("4.0"));
    oldFile.parse();

    TestableIniFile newFile(std::nullopt, "x\n", std::nullopt, std::nullopt, forced, makeOverrides(), nullptr,
                            std::nullopt, DownloadMode::Normal, ver("5.7"));
    newFile.parse();

    check(log->size() == 2, "one parser built per .ini file");
    check(log->size() == 2 && (*log)[0].tag == "amber4_0", "a 4.0 .ini file gets the 4.0 parser");
    check(log->size() == 2 && (*log)[1].tag == "amber5_7", "a 5.7 .ini file of the same mod type gets the 5.7 parser");

    // Two files of the same mod type no longer share (and stomp) one parser instance: each one's
    // parser arrives already bound to it.
    check(log->size() == 2 && (*log)[0].boundTo == &oldFile, "each file's parser is bound to that file, not the last one parsed");
    check(log->size() == 2 && (*log)[1].boundTo == &newFile, "the second file's parser is bound to the second file");
    check(log->size() == 2 && (*log)[0].built != (*log)[1].built, "the two files got distinct parser instances");

    TaggedParser* oldParser = (*log)[0].built;

    // The built parser is cached per mod type, the analogue of the original's self._iniParser:
    // a second parse()/fix() runs the same instance again rather than building a second one.
    int parsesBefore = oldParser->parseCount;
    oldFile.parse();
    oldFile.fix();
    check(log->size() == 2, "a second parse()/fix() reuses the cached parser rather than building another");
    check(oldParser->parseCount > parsesBefore, "the cached parser is re-run rather than skipped");

    // clear() drops the cache, matching the original's "self._iniParser = None".
    oldFile.clear();
    oldFile.parse();
    check(log->size() == 3, "clear() drops the cache so the next parse builds a fresh parser");
    check(log->size() == 3 && (*log)[2].built != oldParser, "the post-clear parser is a genuinely new instance");
    check(log->size() == 3 && (*log)[2].boundTo == &oldFile, "the post-clear parser is bound to the same file");
}

}  // namespace

int main() {
    testFixedFactory();
    testDefaultFallbacks();
    testVersionDependentLookup();
    testNotFoundBehaviour();
    testCapturedStateIsShared();
    testModTypeFallbacks();
    testIniFileUsesTheBuilder();

    std::printf("\n%s (%d failure(s))\n", (failures == 0 ? "ALL PASSED" : "FAILURES"), failures);
    return (failures == 0) ? 0 : 1;
}
