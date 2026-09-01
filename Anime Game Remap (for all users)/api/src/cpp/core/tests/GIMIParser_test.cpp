// -----------------------------------------------------------------------------
// Standalone regression test for the iniParsers subsystem's plain-C++
// (<std::string, std::string>) instantiation:
//
//   * AGRemapCore::GIMISectionClassifier  (classify a section by its hash /
//     match_first_index KVPs)
//   * AGRemapCore::GIMIParser             (classify by TextureOverride name,
//     build the command graphs, synthesize download resources)
//   * AGRemapCore::DownloadData           (the plain-C++ IniParseDownloadData)
//
// WHY THIS FILE EXISTS: GIMIParser is reachable from Python (test_GIMIParser.py /
// test_GIMISectionClassifier.py cover it there), but ONLY as
// GIMIParser<py::object, ...>. Nothing in the Python suite ever instantiates the
// std::string one, and so nothing there exercises
// GIMISectionClassifier::defaultConfig / GIMIParser::defaultConfig /
// AGRemapCore::DownloadData at all -- those exist purely for a standalone C++
// caller. This file is that caller.
//
// It supplies its own IniParseContext (TestIniParseContext below), the same way
// the pybind11 layer supplies PyIniParseContext -- see IniParseContext.h's own
// note on why a parser can't just take an AGRemapCore::IniFile*.
//
// NOT wired into any build target (core/tests/*.cpp never is -- no CMake entry,
// no CTest, not run by CI or by main.py). Compile and run it by hand:
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      GIMIParser_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// (build AGRemapCore.lib first: `cd cbuild && ninja AGRemapCore`, and copy
//  cext/z3/bin/libz3.dll next to test.exe before running it)
// -----------------------------------------------------------------------------

#include "AGRemapCore/data/HashData.h"
#include "AGRemapCore/data/HashToModObjData.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseDownloadData.h"

#include <cstdio>
#include <memory>
#include <optional>
#include <string>
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
        ++failures;
    }
}


using Section = IfTemplate<std::string, std::string>;
using Graph = IniSectionGraph<std::string, std::string>;
using ModObj = std::pair<std::string, std::string>;
using KVPs = tsl::ordered_map<std::string, std::vector<std::pair<long long, std::string>>>;
using RawPart = std::pair<int, std::variant<std::string, KVPs>>;


IfTemplateRunConfig<std::string, std::string> runConfig() {
    return IfTemplateRunConfig<std::string, std::string>{
        IniKeywords::Run,
        [](const std::string& val) { return val; },
        [](const std::string& name) { return name; }
    };
}


std::unique_ptr<Section> makeSection(const std::string& name, const std::vector<std::pair<std::string, std::string>>& kvps,
                                      Z3Context* z3Ctx) {
    KVPs src;
    long long index = 0;
    for (const auto& kvp : kvps) {
        src[kvp.first].emplace_back(index, kvp.second);
        ++index;
    }

    std::vector<RawPart> rawParts;
    rawParts.emplace_back(0, src);
    return Section::build(rawParts, runConfig(), name, nullptr, z3Ctx);
}


using Colouring = GIMISectionClassifier<>::Colouring;


// AGRemapCore::IfContentPartColouring has only a default constructor (the 'src'-dict shape the
// pybind11 layer exposes is a binding-only convenience), so build one key at a time.
Colouring makeColouring(const std::vector<std::pair<std::string, std::vector<std::pair<long long, std::string>>>>& src) {
    Colouring result;
    for (const auto& entry : src) {
        result.set(entry.first, entry.second);
    }
    return result;
}


// A minimal, in-memory ".ini file" -- the plain-C++ counterpart of the binding layer's
// PyIniParseContext, and the only thing this test needs beyond the classes under test themselves.
class TestIniParseContext: public IniParseContext<std::string, std::string> {
    public:
        using Base = IniParseContext<std::string, std::string>;

        explicit TestIniParseContext(Z3Context* z3Ctx): z3Ctx_(z3Ctx), groupsView_(groupsStorage_, runConfig()) {
            groupsView_.insertGroup(0);
        }

        std::string folder = "C:/Mods/TestMod";
        DownloadMode mode = DownloadMode::Normal;
        std::optional<Version> iniVersion;
        std::string modName;
        Assets* hashes = nullptr;
        Assets* indices = nullptr;
        std::vector<std::unique_ptr<IniResource>> fileDownloads;

        void add(std::unique_ptr<Section> section) {
            std::string name = section->name;
            addSection(name, std::move(section));
        }

        bool hasIni() const override { return true; }
        std::string iniFolder() const override { return folder; }
        std::optional<Version> version() const override { return iniVersion; }
        DownloadMode downloadMode() const override { return mode; }
        Z3Context* z3Ctx() const override { return z3Ctx_; }

        std::unordered_map<std::string, Section*> sectionIfTemplates() const override {
            std::unordered_map<std::string, Section*> result;
            for (const auto& entry : sections_) {
                result[entry.first] = entry.second.get();
            }
            return result;
        }

        std::vector<std::string> sectionNames() const override {
            std::vector<std::string> result;
            for (const auto& entry : sections_) {
                result.push_back(entry.first);
            }
            return result;
        }

        Section* getSection(const std::string& name) const override {
            auto found = sections_.find(name);
            return found == sections_.end() ? nullptr : found->second.get();
        }

        Section* addSection(const std::string& name, std::unique_ptr<Section> section) override {
            Section* result = section.get();
            sections_[name] = std::move(section);
            return result;
        }

        void removeSection(const std::string& name) override {
            sections_.erase(name);
        }

        void addFileDownload(std::unique_ptr<IniResource> download) override {
            fileDownloads.push_back(std::move(download));
        }

        bool hasModType() const override { return !modName.empty(); }
        std::string modTypeName() const override { return modName; }
        Assets* modTypeHashes() const override { return hashes; }
        Assets* modTypeIndices() const override { return indices; }
        GraphGroups& graphGroups() override { return groupsView_; }

    private:
        // tsl::ordered_map so sectionNames() is genuinely declaration-ordered, matching what a real
        // .ini file (and the Python dict the binding layer reads) gives a parser.
        tsl::ordered_map<std::string, std::unique_ptr<Section>> sections_;
        Z3Context* z3Ctx_;
        std::vector<IniGraphGroup<>> groupsStorage_;
        IniGraphGroupsVec<std::string, std::string> groupsView_;
};


ModMappedAssets<std::string, std::string> makeAssets(std::size_t totalIndices, std::vector<Row<std::string, std::string>> rows) {
    ModDictAssets<std::string, std::string> repo(totalIndices, 0, [](const std::string& raw) { return Version::parse(raw); },
                                                  std::move(rows));
    return ModMappedAssets<std::string, std::string>(std::move(repo));
}


// ---------------------------------------------------------------------------------------
// GIMISectionClassifier
// ---------------------------------------------------------------------------------------

void testClassifier() {
    std::printf("\n--- GIMISectionClassifier ---\n");

    // Hashes' index columns: version, name, type
    auto hashes = makeAssets(3, {{{"99.0", "testrika", "blend_vb"}, "test-hash-blend"},
                                  {{"50.0", "testrika", "blend_vb"}, "test-hash-blend-old"},
                                  {{"99.0", "testrika", "ib"}, "test-hash-ib"}});
    // Indices' index columns: version, name, component, object
    auto indices = makeAssets(4, {{{"99.0", "testrika", "", "body"}, "test-index-body"},
                                   {{"99.0", "testrika", "", "head"}, "test-index-head"}});

    GIMISectionClassifier<> classifier({{"blend_vb", ModObj("", "blend")}}, &hashes,
                                        {{"ib", {{{"", "body"}, ModObj("", "body")},
                                                 {{"", "head"}, ModObj("", "head")}}}}, &indices);

    Colouring blendOnly = makeColouring({{"hash", {{0, "test-hash-blend"}}}});
    auto result = classifier.classify("SomeSection", nullptr, blendOnly);
    check(result.size() == 1 && result[0] == ModObj("", "blend"), "a hash-only mod object is classified by its hash alone");

    Colouring unknown = makeColouring({{"hash", {{0, "not-a-real-hash"}}}});
    check(classifier.classify("SomeSection", nullptr, unknown).empty(), "an unrecognized hash classifies to nothing");

    Colouring ibBody = makeColouring({{"hash", {{0, "test-hash-ib"}}}, {"match_first_index", {{1, "test-index-body"}}}});
    result = classifier.classify("SomeSection", nullptr, ibBody);
    check(result.size() == 1 && result[0] == ModObj("", "body"), "a shared hash is disambiguated by its match_first_index");

    Colouring ibHead = makeColouring({{"hash", {{0, "test-hash-ib"}}}, {"match_first_index", {{1, "test-index-head"}}}});
    result = classifier.classify("SomeSection", nullptr, ibHead);
    check(result.size() == 1 && result[0] == ModObj("", "head"), "a different match_first_index picks a different mod object");

    // The index sits before the hash it would otherwise qualify, so it belongs to an earlier hash.
    Colouring outOfWindow = makeColouring({{"hash", {{5, "test-hash-ib"}}}, {"match_first_index", {{1, "test-index-body"}}}});
    check(classifier.classify("SomeSection", nullptr, outOfWindow).empty(),
          "a match_first_index before its hash is outside that hash's window");

    // ModDictAssets resolves a version by inclusive floor-match (see its own class note), so a
    // 60.0 .ini file sees the 50.0 row's hash, not the 99.0 one's.
    Colouring oldBlend = makeColouring({{"hash", {{0, "test-hash-blend-old"}}}});
    classifier.version = Version::parse("60.0");
    result = classifier.classify("SomeSection", nullptr, oldBlend);
    check(result.size() == 1 && result[0] == ModObj("", "blend"), "an older .ini file resolves against that version's own row");
    // ModMappedAssets::getKey is a reverse-index lookup: a hash resolves to the key that owns it
    // whatever version is pinned (the version only picks *which* row, when one asset appears at
    // several). So the 99.0-only hash still classifies here -- pinning a version narrows the row,
    // it does not hide an asset.
    check(classifier.classify("SomeSection", nullptr, blendOnly).size() == 1,
          "a hash still resolves to its key under a pinned version (getKey is a reverse-index lookup)");
    classifier.version = std::nullopt;
    check(classifier.classify("SomeSection", nullptr, blendOnly).size() == 1,
          "with no version pinned, the latest row is used");

    GIMISectionClassifier<> noAssets({{"blend_vb", ModObj("", "blend")}}, nullptr);
    check(noAssets.classify("SomeSection", nullptr, blendOnly).empty(), "a classifier with no hash assets classifies to nothing");

    auto built = GIMISectionClassifier<>::buildDefaultClassifier(&hashes, &indices, Version::parse("3.0"));
    check(built->hashes() == &hashes && built->indices() == &indices && built->version.has_value(),
          "buildDefaultClassifier keeps the assets and the version");
}


// ---------------------------------------------------------------------------------------
// The default mod object mappings every mod type shares
// ---------------------------------------------------------------------------------------

void testDefaultModObjMappings() {
    std::printf("\n--- Data::getHashKeyOnlyToModObj / getIndexKeyToModObj ---\n");

    check(Data::parseModObjKey("blend_vb") == std::make_pair(std::string(""), std::string("blend_vb")),
          "a bare key names an object with no component");
    check(Data::parseModObjKey("someComp;head") == std::make_pair(std::string("someComp"), std::string("head")),
          "a ';'-qualified key names a component and an object");
    check(Data::parseModObjKey("a;b;c") == std::make_pair(std::string("a"), std::string("b;c")),
          "only the first ';' separates -- an object name may hold more of them");

    const auto& hashOnly = Data::getHashKeyOnlyToModObj();
    const auto& indexKeys = Data::getIndexKeyToModObj();

    check(hashOnly.count("blend_vb") == 1 && hashOnly.at("blend_vb") == std::make_pair(std::string(""), std::string("blend_vb")),
          "a buffer hash type maps to the mod object its own key names");
    check(hashOnly.count("tex_head_diffuse") == 1 && hashOnly.count("draw_vb") == 1 && hashOnly.count("position_vb") == 1
              && hashOnly.count("texcoord_vb") == 1,
          "so does every other non-'ib' hash type the table ships");

    check(hashOnly.count("ib") == 0, "'ib' is the one hash type a hash value alone can't resolve...");
    check(indexKeys.count("ib") == 1 && indexKeys.size() == 1, "...so it is the only key on the index side");

    const auto& ibModObjs = indexKeys.at("ib");
    check(ibModObjs.count(std::make_pair(std::string(""), std::string("head"))) == 1
              && ibModObjs.count(std::make_pair(std::string(""), std::string("body"))) == 1
              && ibModObjs.count(std::make_pair(std::string(""), std::string("dress"))) == 1
              && ibModObjs.count(std::make_pair(std::string(""), std::string("extra"))) == 1,
          "an 'ib' hash reaches every mod object the index table names");
    check(ibModObjs.at(std::make_pair(std::string(""), std::string("head"))) == std::make_pair(std::string(""), std::string("head")),
          "an index row already ends in a (component, object) pair, so it maps to itself");

    // Every hash type the table ships lands on exactly one side of the split.
    std::unordered_set<std::string> allKeys;
    for (const auto& row : Data::getHashDataRows()) {
        if (!row.first.empty()) {
            allKeys.insert(row.first.back());
        }
    }

    check(allKeys.size() == hashOnly.size() + indexKeys.size(),
          "every hash type is on exactly one side of the hash-only/index split");

    // The mappings the classifier is actually handed are these, keyed as K.
    auto built = GIMISectionClassifier<>::buildDefaultClassifier(nullptr, nullptr);
    check(built->hashKeyOnlyToModObj.size() == hashOnly.size() && built->indexKeyToModObj.size() == indexKeys.size(),
          "buildDefaultClassifier hands them straight to the classifier");
    check(built->indexKeyToModObj.at("ib").size() == ibModObjs.size(),
          "including the inner index-key mappings");
}


// ---------------------------------------------------------------------------------------
// GIMIParser -- classification by TextureOverride name
// ---------------------------------------------------------------------------------------

void testClassifyByName() {
    std::printf("\n--- GIMIParser::classifyByTextureOverrideName ---\n");

    Z3Context z3Ctx;
    TestIniParseContext ctx(&z3Ctx);
    GIMIParser<> parser(&ctx, {ModObj("", "blend"), ModObj("bang", "B")});

    auto result = GIMIParser<>::classifyByTextureOverrideName(parser, "TextureOverrideRaidenShogunBlend");
    check(result.size() == 1 && result[0] == ModObj("", "blend"), "a matching name suffix classifies the section");

    result = GIMIParser<>::classifyByTextureOverrideName(parser, "   textureoverrideRAIDENSHOGUNblend   ");
    check(result.size() == 1 && result[0] == ModObj("", "blend"), "the name match is case- and whitespace-insensitive");

    check(GIMIParser<>::classifyByTextureOverrideName(parser, "TextureOverrideRaidenShogunRemapBlend").empty(),
          "a section this software already wrote ('remap' in the name) is never classified");

    check(GIMIParser<>::classifyByTextureOverrideName(parser, "CommandListRaidenShogunBlend").empty(),
          "a section that isn't a TextureOverride is never classified");

    check(GIMIParser<>::classifyByTextureOverrideName(parser, "TextureOverrideRaidenBlendExtras").empty(),
          "the match has to be a suffix, not just an occurrence");

    result = GIMIParser<>::classifyByTextureOverrideName(parser, "TextureOverrideYelanBangB");
    check(result.size() == 1 && result[0] == ModObj("bang", "B"), "component and object are concatenated for the search text");

    std::vector<ModObj> explicitModObjs{ModObj("", "Head")};
    result = GIMIParser<>::classifyByTextureOverrideName(parser, "TextureOverrideHuTaoHead", true, &explicitModObjs);
    check(result.size() == 1 && result[0] == ModObj("", "Head"), "an explicit mod object list overrides the parser's own");

    check(parser.globalGraph() != nullptr, "fromRoots builds the global graph on first use");
}


// ---------------------------------------------------------------------------------------
// GIMIParser -- a full parse, including a synthesized download resource
// ---------------------------------------------------------------------------------------

void testParse() {
    std::printf("\n--- GIMIParser::parse ---\n");

    Z3Context z3Ctx;
    TestIniParseContext ctx(&z3Ctx);
    ctx.modName = "Raiden";

    ctx.add(makeSection("TextureOverrideRaidenShogunBlend", {{"run", "CommandListRaidenShogunBlend"}}, &z3Ctx));
    ctx.add(makeSection("CommandListRaidenShogunBlend", {{"vb1", "ResourceRaidenShogunBlend"}}, &z3Ctx));
    ctx.add(makeSection("ResourceRaidenShogunBlend", {{"filename", "RaidenShogunBlend.buf"}}, &z3Ctx));

    DownloadData<>::DownloadConfig downloadConfig{IniKeywords::Filename,
                                                   [](const std::string& path) { return path; },
                                                   runConfig()};
    DownloadData<> diffuse("testDiffuse", std::make_unique<FileDownload>("someServer.com/diffuse.dds", "diffuse.dds"),
                            downloadConfig);
    DownloadData<> texcoord("testPosition", std::make_unique<FileDownload>("someServer.com/position.buf", "position.buf"),
                             downloadConfig);

    GIMIParser<> parser(&ctx, {ModObj("", "blend"), ModObj("", "texcoord")});
    // No global graph / no KVP tracking -> the by-name strategy, which is what these section names
    // are written for.
    parser.makeGlobalGraph = false;
    parser.trackKeys = false;
    parser.downloads[ModObj("", "blend")]["ps-t0"] = &diffuse;
    parser.downloads[ModObj("", "texcoord")]["vb0"] = &texcoord;

    std::vector<IniGraphGroup<>> parsed = parser.parse();

    auto targets = parser.sectionTargets();
    check(targets.find(ModObj("", "blend")) != targets.end()
              && targets.at(ModObj("", "blend")) == std::vector<std::string>{"TextureOverrideRaidenShogunBlend"},
          "the blend mod object's entry point is found by name");
    check(targets.find(ModObj("", "texcoord")) != targets.end() && targets.at(ModObj("", "texcoord")).empty(),
          "a mod object with no matching section gets an empty entry-point list");

    Graph* blendGraph = parser.getCommandGraph(ModObj("", "blend"));
    // Two, not three: a graph edge is a `run =` call, and the Resource section is reached by a
    // plain `vb1 =` KVP, which is a resource reference rather than a command call.
    check(blendGraph != nullptr && blendGraph->sections().size() == 2,
          "the blend command graph follows the entry point's run = calls");

    Graph* texcoordGraph = parser.getCommandGraph(ModObj("", "texcoord"));
    check(texcoordGraph != nullptr && !texcoordGraph->isEmpty(),
          "an initially-empty command graph is given a synthesized TextureOverride...RemapFix section");

    check(ctx.getSection("TextureOverrideRaidenTexcoordRemapFix") != nullptr,
          "the synthesized command section is added to the .ini file");
    check(ctx.getSection("ResourceRaidenTestPositionRemapDL") != nullptr,
          "the synthesized download resource section is added to the .ini file");
    check(ctx.getSection("ResourceRaidenTestDiffuseRemapDL") != nullptr,
          "a download resource is synthesized for the non-empty graph's missing register too");

    check(ctx.fileDownloads.size() == 2, "one file download is recorded per download resource");

    const auto& resourceGraphs = parser.downloadResourceGraphs();
    check(resourceGraphs.size() == 2, "each mod object with a download gets its own resource graph map");
    check(resourceGraphs.at(ModObj("", "texcoord")).at("vb0")->sections().count("ResourceRaidenTestPositionRemapDL") == 1,
          "the resource graph actually holds the section it was built for");

    // The pure-Python original wrote these through read-only views and silently lost them -- see
    // GIMIParser::createDownloadResource's own comment.
    check(!resourceGraphs.at(ModObj("", "texcoord")).at("vb0")->targetSectionNames().empty(),
          "the resource graph's target section names survive (they were discarded in the original)");

    // ---- what parse() itself hands back ----

    check(parsed.size() == 1, "parse() returns exactly one group");

    const IniGraphGroup<>& group = parsed[0];
    check(group.size() == 4, "the group holds both command graphs and both download resource graphs");

    std::vector<IniGraphGroup<>::ModObj> expectedKeys{
        ModObj("", "blend"), ModObj("", "texcoord"),
        ModObj(IniGraphModObjKeywords::Download, "testDiffuse"),
        ModObj(IniGraphModObjKeywords::Download, "testPosition")};
    check(group.modObjs() == expectedKeys,
          "the group is keyed by mod object first, then by ('download', the download's name), in that order");

    Graph* groupedBlend = group.getGraph(ModObj("", "blend"));
    check(groupedBlend != nullptr && groupedBlend != blendGraph,
          "the group's graphs are deep copies, not the parser's own borrowed ones");
    check(groupedBlend != nullptr && groupedBlend->sections().size() == blendGraph->sections().size(),
          "a copied graph carries the same sections as the one it was copied from");

    Graph* groupedDownload = group.getGraph(ModObj(IniGraphModObjKeywords::Download, "testPosition"));
    check(groupedDownload != nullptr && groupedDownload->sections().count("ResourceRaidenTestPositionRemapDL") == 1,
          "a download resource graph in the group holds the section it was built for");

    parser.clear();
    check(parser.commandGraphs().empty() && parser.downloadResourceGraphs().empty() && parser.globalGraph() == nullptr,
          "clear() drops every graph the parse produced");
    check(group.size() == 4, "the group parse() returned survives clear(), because it owns its graphs");
}


// One DownloadData reached from several registers: the resource section is named after the
// download, and a .ini file can only hold one section of a given name -- so it is built (and
// downloaded) once, and every register's resource graph points at that same one.
void testSharedDownload() {
    std::printf("\n--- GIMIParser::parse (one download shared by two registers) ---\n");

    Z3Context z3Ctx;
    TestIniParseContext ctx(&z3Ctx);
    ctx.modName = "Raiden";
    ctx.add(makeSection("TextureOverrideRaidenShogunBlend", {{"vb1", "ResourceRaidenShogunBlend"}}, &z3Ctx));

    DownloadData<>::DownloadConfig downloadConfig{IniKeywords::Filename,
                                                   [](const std::string& path) { return path; },
                                                   runConfig()};
    DownloadData<> shared("sharedDownload", std::make_unique<FileDownload>("someServer.com/shared.buf", "shared.buf"),
                           downloadConfig);

    GIMIParser<> parser(&ctx, {ModObj("", "blend")});
    parser.makeGlobalGraph = false;
    parser.trackKeys = false;
    parser.downloads[ModObj("", "blend")]["ps-t0"] = &shared;
    parser.downloads[ModObj("", "blend")]["ps-t1"] = &shared;

    std::vector<IniGraphGroup<>> parsed = parser.parse();

    std::string resourceName = IniNamingTools::getRemapDLResourceName(
        TextTools::capitalize("Raiden") + TextTools::capitalize("sharedDownload"));

    check(ctx.fileDownloads.size() == 1, "a download shared by two registers is only downloaded once");
    check(ctx.getSection(resourceName) != nullptr, "its resource section is added to the .ini file");

    const auto& resourceGraphs = parser.downloadResourceGraphs();
    Graph* first = resourceGraphs.at(ModObj("", "blend")).at("ps-t0");
    Graph* second = resourceGraphs.at(ModObj("", "blend")).at("ps-t1");

    check(first != nullptr && second != nullptr && first != second,
          "each register still gets its own resource graph");
    check(first->getSection(resourceName, false) == second->getSection(resourceName, false)
              && first->getSection(resourceName, false) == ctx.getSection(resourceName),
          "and both hold the one section the .ini file has, not a copy each");

    check(parsed.size() == 1 && parsed[0].getGraph(ModObj(IniGraphModObjKeywords::Download, "sharedDownload")) != nullptr,
          "parse()'s group carries it under its download name");
    check(parsed[0].size() == 2, "and carries it exactly once, alongside the one command graph");
}


void testDownloadModeDisabled() {
    std::printf("\n--- GIMIParser::setupDownloads (DownloadMode::Disabled) ---\n");

    Z3Context z3Ctx;
    TestIniParseContext ctx(&z3Ctx);
    ctx.modName = "Raiden";
    ctx.mode = DownloadMode::Disabled;
    ctx.add(makeSection("TextureOverrideRaidenShogunBlend", {{"vb1", "ResourceRaidenShogunBlend"}}, &z3Ctx));

    DownloadData<>::DownloadConfig downloadConfig{IniKeywords::Filename,
                                                   [](const std::string& path) { return path; },
                                                   runConfig()};
    DownloadData<> diffuse("testDiffuse", std::make_unique<FileDownload>("someServer.com/diffuse.dds", "diffuse.dds"),
                            downloadConfig);

    GIMIParser<> parser(&ctx, {ModObj("", "blend")});
    parser.makeGlobalGraph = false;
    parser.trackKeys = false;
    parser.downloads[ModObj("", "blend")]["ps-t0"] = &diffuse;

    std::vector<IniGraphGroup<>> parsed = parser.parse();

    check(ctx.fileDownloads.empty(), "no downloads are recorded at all when downloads are disabled");
    check(parser.downloadResourceGraphs().empty(), "no download resource graphs are built when downloads are disabled");
    check(parsed.size() == 1 && parsed[0].size() == 1 && parsed[0].getGraph(ModObj("", "blend")) != nullptr,
          "the returned group holds just the command graph when there are no downloads to add");
}

}


int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    testClassifier();
    testDefaultModObjMappings();
    testClassifyByName();
    testParse();
    testSharedDownload();
    testDownloadModeDisabled();

    if (failures == 0) {
        std::printf("\nALL PASSED\n");
        return 0;
    }

    std::printf("\n%d CHECK(S) FAILED\n", failures);
    return 1;
}
