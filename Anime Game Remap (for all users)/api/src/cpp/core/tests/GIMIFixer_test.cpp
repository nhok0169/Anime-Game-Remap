// -----------------------------------------------------------------------------
// Standalone regression test for the iniFixers subsystem's plain-C++
// (<std::string, std::string>) instantiation:
//
//   * AGRemapCore::BaseIniFixer  (the parser/.ini binding, and the fix -> fixImpl hop)
//   * AGRemapCore::GIMIFixer     (build the groups, edit them per mod, render and write)
//
// WHY THIS FILE EXISTS: the same reason GIMIParser_test.cpp does -- GIMIFixer is reachable from
// Python (test_GIMIFixer.py covers it there), but only as GIMIFixer<py::object, ...>. Nothing in
// the Python suite instantiates the std::string one, and the pybind11 layer overrides getFix
// outright, so the core's own group-building and prevFixer handover are covered here or nowhere.
//
// It supplies its own IniFixContext (TestIniFixContext below), the same way the pybind11 layer
// supplies PyIniFixContext -- see IniFixContext.h's own note on why a fixer can't just take an
// AGRemapCore::IniFile*.
//
// NOT wired into any build target (core/tests/*.cpp never is). Compile and run it by hand:
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      GIMIFixer_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// (build AGRemapCore.lib first: `cd cbuild && ninja AGRemapCore`, and copy cext/z3/bin/libz3.dll,
//  cbuild/curl/lib/libcurl.dll and cbuild/utf8proc/utf8proc.dll next to test.exe before running it)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/RemapIniFixContext.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"

#include <cstdio>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
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
using Group = IniGraphGroup<>;
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


// A minimal, in-memory ".ini file" -- the plain-C++ counterpart of the binding layer's
// PyIniFixContext.
// Deliberately a RemapIniFixContext rather than a bare IniFixContext: addFixBoilerPlate is the one
// method on the interface a plain C++ caller does *not* have to write for itself, and the fix this
// file checks should be the one this software really ships.
class TestIniFixContext: public RemapIniFixContext<std::string, std::string> {
    public:
        std::optional<std::string> typeName = "Raiden";
        std::string path = "C:/Mods/TestMod/CuteLittleEi.ini";
        bool hasPath = true;
        bool existsOnDisk = false;
        std::string txt = "; the original .ini file\n";
        std::vector<std::string> mods{"Raiden"};

        // What the fix did, recorded rather than done.
        std::unordered_map<std::string, std::string> written;
        std::vector<std::string> logs;
        bool hidOriginalSections = false;
        bool disabledIni = false;
        bool isFixed = false;

        bool hasIni() const override { return true; }
        std::vector<std::string> modsToFix() const override { return mods; }

        std::optional<std::string> fixedFilePath(std::size_t groupInd) const override {
            if (!hasPath) {
                return std::nullopt;
            }
            if (groupInd == 0) {
                return path;
            }
            return path + ".RemapFix" + std::to_string(groupInd);
        }

        bool fixedFileExists() const override { return existsOnDisk; }
        std::string fileTxt() const override { return txt; }
        void setFileTxt(std::string newTxt) override { txt = std::move(newTxt); }
        void hideOriginalSections() override { hidOriginalSections = true; txt = "; hidden\n"; }
        void disableIni() override { disabledIni = true; }
        void log(const std::string& message) override { logs.push_back(message); }

        std::optional<std::string> modTypeName() const override { return typeName; }

        void writeFixedFile(const std::string& filePath, const std::string& content) override {
            written[filePath] = content;
        }

        void setIsFixed(bool newIsFixed) override { isFixed = newIsFixed; }

        std::unique_ptr<GraphGroups> makeGraphGroups() override {
            // One storage vector per call, kept alive for this context's lifetime -- an
            // IniGraphGroupsVec is only a *view* over a caller-owned vector.
            groupStorages_.push_back(std::make_unique<std::vector<Group>>());
            return std::make_unique<IniGraphGroupsVec<std::string, std::string>>(*groupStorages_.back(), runConfig());
        }

    private:
        std::deque<std::unique_ptr<std::vector<Group>>> groupStorages_;
};


// Records which mods it was run for, and hands the groups straight back.
class SpyGroupEdit: public BaseIniGraphGroupEdit<> {
    public:
        std::vector<std::string> calls;

        GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName) override {
            (void)modType;
            calls.push_back(modName);
            return graphGroups;
        }
};


GIMIFixer<>::FixerConfig fixerConfig() {
    GIMIFixer<>::FixerConfig result{};
    // Just the header line -- what a section really looks like belongs to the pybind11 layer's own
    // IfTemplate.toStr, and none of these assertions care about the body.
    result.sectionToStr = [](Section& section, const std::string& linePrefix, bool) {
        return linePrefix + "[" + section.name + "]";
    };
    return result;
}


// One group holding two named, empty-ish graphs -- stands in for what GIMIParser::parse hands back.
Group makeParsedGroup(TestIniFixContext& ctx, Z3Context& z3Ctx,
                       std::vector<std::unique_ptr<Section>>& sectionStorage) {
    (void)ctx;
    Group group;

    for (const ModObj& modObj : {ModObj("", "blend"), ModObj(IniGraphModObjKeywords::Download, "testPosition")}) {
        std::string sectionName = "TextureOverride" + modObj.second;
        sectionStorage.push_back(makeSection(sectionName, {{"vb1", "ResourceFoo"}}, &z3Ctx));

        std::unordered_map<std::string, Section*> sections{{sectionName, sectionStorage.back().get()}};
        group.addGraph(modObj, std::make_unique<Graph>(sections, std::vector<std::string>{sectionName},
                                                        runConfig(), true, false, &z3Ctx));
    }

    return group;
}


// ---------------------------------------------------------------------------------------
// BaseIniFixer
// ---------------------------------------------------------------------------------------

void testBaseIniFixer() {
    std::printf("\n--- BaseIniFixer ---\n");

    BaseIniParser<> parser(nullptr);
    BaseIniFixer<> fixer(&parser);

    check(fixer.getParser() == &parser, "a fixer remembers the parser it was constructed with");
    check(fixer.getIniFile() == nullptr, "an unbound parser means an unbound .ini file");

    BaseIniFixer<> unbound;
    check(unbound.getParser() == nullptr, "a fixer can be constructed with no parser at all");

    unbound.setParser(&parser);
    check(unbound.getParser() == &parser, "setParser rebinds a fixer");

    BaseIniFixer<>::ParseData empty;
    check(fixer.fix(empty).empty(), "the base fixer fixes nothing");
}


// ---------------------------------------------------------------------------------------
// GIMIFixer::getFix
// ---------------------------------------------------------------------------------------

void testGetFix() {
    std::printf("\n--- GIMIFixer::getFix ---\n");

    Z3Context z3Ctx;
    TestIniFixContext ctx;
    std::vector<std::unique_ptr<Section>> sections;

    GIMIFixer<>::ParseData parseData;
    parseData.push_back(makeParsedGroup(ctx, z3Ctx, sections));

    SpyGroupEdit edit;
    GIMIFixer<> fixer(nullptr, &ctx, {&edit}, std::nullopt, nullptr, fixerConfig());

    check(fixer.graphGroups() == nullptr, "a fresh fixer holds no groups");
    check(fixer.getModsToFix() == std::vector<std::string>{"Raiden"},
          "with no explicit modsToFix, the .ini file's own list is used");

    GIMIFixer<>::FixTargets targets = fixer.getFix(parseData, false);

    check(fixer.graphGroups() != nullptr && fixer.graphGroups()->size() == 1, "getFix builds exactly one group");
    check(fixer.graphGroups()->graphCount(0) == 2, "the group holds every graph the parse data had");
    check(fixer.graphGroups()->getGraph(0, ModObj("", "blend")) != parseData[0].getGraph(ModObj("", "blend")),
          "the fixer's graphs are copies, so editing them leaves the parse data alone");
    check(fixer.graphGroups()->getGraph(0, ModObj(IniGraphModObjKeywords::Download, "testPosition")) != nullptr,
          "a download resource graph keeps its ('download', name) key through the copy");

    check(edit.calls == std::vector<std::string>{"Raiden"}, "each edit runs once per mod being fixed to");
    check(targets.size() == 1 && targets[0].has_value() && *targets[0] == ctx.path,
          "the only group is written back over the .ini file itself");

    // explicit modsToFix wins over the .ini file's own
    edit.calls.clear();
    fixer.modsToFix = std::vector<std::string>{"JeanCN", "JeanSea"};
    fixer.getFix(parseData, false);
    check(edit.calls == (std::vector<std::string>{"JeanCN", "JeanSea"}),
          "an explicit modsToFix replaces the .ini file's own list");

    // onlyEditObjGraphs stops before working out destinations
    edit.calls.clear();
    check(fixer.getFix(parseData, true).empty(), "onlyEditObjGraphs returns no destinations");
    check(fixer.graphGroups() != nullptr && fixer.graphGroups()->size() == 1,
          "onlyEditObjGraphs still leaves the edited groups behind");

    fixer.clear();
    check(fixer.graphGroups() == nullptr, "clear drops the groups");
}


void testPrevFixer() {
    std::printf("\n--- GIMIFixer::prevFixer ---\n");

    Z3Context z3Ctx;
    TestIniFixContext ctx;
    std::vector<std::unique_ptr<Section>> sections;

    GIMIFixer<>::ParseData parseData;
    parseData.push_back(makeParsedGroup(ctx, z3Ctx, sections));

    SpyGroupEdit prevEdit;
    SpyGroupEdit ownEdit;
    GIMIFixer<> prev(nullptr, &ctx, {&prevEdit}, std::vector<std::string>{"Raiden"}, nullptr, fixerConfig());
    GIMIFixer<> fixer(nullptr, &ctx, {&ownEdit}, std::vector<std::string>{"Raiden"}, &prev, fixerConfig());

    fixer.getFix(parseData, false);

    check(prevEdit.calls == std::vector<std::string>{"Raiden"}, "the previous fixer runs its own edit pass first");
    check(ownEdit.calls == std::vector<std::string>{"Raiden"}, "this fixer then runs its own over the same groups");
    check(fixer.graphGroups() != nullptr && fixer.graphGroups()->graphCount(0) == 2,
          "this fixer ends up holding the groups");
    check(prev.graphGroups() == nullptr, "the previous fixer is left empty after handing them over");
}


// ---------------------------------------------------------------------------------------
// GIMIFixer::fix
// ---------------------------------------------------------------------------------------

void testFix() {
    std::printf("\n--- GIMIFixer::fix ---\n");

    Z3Context z3Ctx;
    TestIniFixContext ctx;
    std::vector<std::unique_ptr<Section>> sections;

    GIMIFixer<>::ParseData parseData;
    parseData.push_back(makeParsedGroup(ctx, z3Ctx, sections));

    std::string originalTxt = ctx.txt;
    GIMIFixer<> fixer(nullptr, &ctx, {}, std::vector<std::string>{"Raiden"}, nullptr, fixerConfig());

    GIMIFixer<>::FixResult result = fixer.fix(parseData);

    check(result.size() == 1 && result.count(ctx.path) == 1, "the fix is keyed by the .ini file's own path");
    check(ctx.written.size() == 1 && ctx.written.count(ctx.path) == 1, "and is actually written there");
    check(ctx.written.at(ctx.path) == result.at(ctx.path), "what was written is what was returned");

    const std::string& content = ctx.written.at(ctx.path);
    check(content.rfind(originalTxt, 0) == 0, "the .ini file's own text leads the result");
    check(content.find("; --------------- Raiden Remap ---------------") != std::string::npos,
          "RemapIniFixContext's real boilerplate header is included");
    check(content.find("Albert Gold#2696") != std::string::npos, "and its credit line");
    check(content.find("[TextureOverrideblend]") != std::string::npos, "every graph in the group is rendered into it");
    check(content.find("[TextureOverridetestPosition]") != std::string::npos,
          "including the download resource graphs");

    check(ctx.isFixed, "the .ini file is marked as fixed");
    check(fixer.fixedContents().size() == 1 && fixer.fixedContents()[0] == content,
          "fixedContents mirrors what was written, per group");

    check(fixer.groupToStr(0).find("[TextureOverrideblend]") != std::string::npos,
          "groupToStr renders the group on its own, without boilerplate or source");
}


// ---------------------------------------------------------------------------------------
// RemapIniFixContext -- the boilerplate a fix is wrapped in
// ---------------------------------------------------------------------------------------

// A RemapIniFixContext with nothing but a mod type name -- every other method on the interface is
// stubbed out, since none of them takes part in building the boilerplate.
class BoilerPlateCtx: public RemapIniFixContext<std::string, std::string> {
    public:
        using RemapIniFixContext<std::string, std::string>::RemapIniFixContext;

        std::optional<std::string> typeName;

        std::optional<std::string> modTypeName() const override { return typeName; }

        bool hasIni() const override { return true; }
        std::vector<std::string> modsToFix() const override { return {}; }
        std::optional<std::string> fixedFilePath(std::size_t) const override { return std::nullopt; }
        bool fixedFileExists() const override { return false; }
        std::string fileTxt() const override { return ""; }
        void setFileTxt(std::string) override {}
        void hideOriginalSections() override {}
        void disableIni() override {}
        void log(const std::string&) override {}
        void writeFixedFile(const std::string&, const std::string&) override {}
        void setIsFixed(bool) override {}
        std::unique_ptr<GraphGroups> makeGraphGroups() override { return nullptr; }
};


void testRemapBoilerPlate() {
    std::printf("\n--- RemapIniFixContext::addFixBoilerPlate ---\n");

    // The three expected strings below are what the real pure-Python IniFile.addFixBoilerPlate
    // produces for the same three mod type names -- captured from it, not written by hand. This is
    // the whole point of the class: a fix a plain C++ caller writes has to be one the still-
    // pure-Python IniRemover can find again, and that means matching to the byte.
    BoilerPlateCtx named;
    named.typeName = "Raiden";
    check(named.addFixBoilerPlate("FIXBODY") == "; --------------- Raiden Remap ---------------\n; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for \"Albert Gold#2696\" and \"Nhok0169\"\n; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support\n\nFIXBODY\n\n; --------------------------------------------",
          "a classified .ini file's boilerplate matches the pure-Python original's, to the byte");

    BoilerPlateCtx unclassified;
    check(unclassified.addFixBoilerPlate("FIXBODY") == "; --------------- GI Remap ---------------\n; Mod remapped by Albert Gold#2696 and NK#1321. If you used it to remap your mods pls give credit for \"Albert Gold#2696\" and \"Nhok0169\"\n; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support\n\nFIXBODY\n\n; ----------------------------------------",
          "so does an unclassified one's -- 'GI' in the heading, 'Mod' in the credit");

    BoilerPlateCtx emptyName;
    emptyName.typeName = "";
    // An empty name is a real name, not a missing one: only "never classified" hits the fallbacks,
    // which is what the original's own `is None` checks do.
    check(emptyName.addFixBoilerPlate("FIXBODY") == "; --------------- Remap ---------------\n; remapped by Albert Gold#2696 and NK#1321. If you used it to remap your mods pls give credit for \"Albert Gold#2696\" and \"Nhok0169\"\n; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support\n\nFIXBODY\n\n; -------------------------------------",
          "an empty mod type name is a name, not a missing one");

    check(named.getFixHeader() == "; --------------- Raiden Remap ---------------", "the header is the heading, opened");
    check(named.getFixFooter() == "\n\n; " + std::string(named.getFixHeader().size() - 2, '-'),
          "and the footer closes to exactly the width the header opened to, after a blank line");

    BoilerPlateCtx overridden("; MY HEADER", "\n; MY FOOTER");
    overridden.typeName = "Raiden";
    check(overridden.getFixHeader() == "; MY HEADER" && overridden.getFixFooter() == "\n; MY FOOTER",
          "an explicitly given header/footer is used verbatim");
    check(overridden.addFixBoilerPlate("FIXBODY") == "; MY HEADER" + named.getFixCredit() + "\n\nFIXBODY\n; MY FOOTER",
          "...including in the boilerplate, whose credit is still the shipped one");

    BoilerPlateCtx headerOnly("; MY HEADER");
    check(headerOnly.getFixFooter() == unclassified.getFixFooter(),
          "overriding only the header leaves the default footer alone");

    BoilerPlateCtx dirty;
    dirty.typeName = "Rai\nden\tX";
    check(dirty.getFixHeader().find("RaidenX") != std::string::npos
              && dirty.getFixHeader().find('\n') == std::string::npos
              && dirty.getFixHeader().find('\t') == std::string::npos,
          "newlines and tabs are stripped out of the name -- this all lives inside a ';' comment");
}


void testFixNoPath() {
    std::printf("\n--- GIMIFixer::fix (no file path) ---\n");

    Z3Context z3Ctx;
    TestIniFixContext ctx;
    ctx.hasPath = false;
    std::vector<std::unique_ptr<Section>> sections;

    GIMIFixer<>::ParseData parseData;
    parseData.push_back(makeParsedGroup(ctx, z3Ctx, sections));

    GIMIFixer<> fixer(nullptr, &ctx, {}, std::vector<std::string>{"Raiden"}, nullptr, fixerConfig());
    GIMIFixer<>::FixResult result = fixer.fix(parseData);

    check(result.empty(), "a pathless .ini file contributes nothing to the path-keyed result");
    check(ctx.written.empty(), "and nothing is written");
    check(fixer.fixedContents().size() == 1, "its content is still reachable through fixedContents");
}


void testFixHideOrigAndBackup() {
    std::printf("\n--- GIMIFixer::fix (hideOrig / keepBackup) ---\n");

    Z3Context z3Ctx;
    TestIniFixContext ctx;
    ctx.existsOnDisk = true;
    std::vector<std::unique_ptr<Section>> sections;

    GIMIFixer<>::ParseData parseData;
    parseData.push_back(makeParsedGroup(ctx, z3Ctx, sections));

    std::string originalTxt = ctx.txt;
    GIMIFixer<> fixer(nullptr, &ctx, {}, std::vector<std::string>{"Raiden"}, nullptr, fixerConfig());

    fixer.fix(parseData, true, true, true);

    check(ctx.hidOriginalSections, "hideOrig comments out the original sections");
    check(ctx.txt == originalTxt, "and the .ini file's own text is put back afterwards");
    check(ctx.disabledIni, "keepBackup + fixOnly over an existing file disables the old one first");
    check(!ctx.logs.empty(), "and says so in the log");
}

}


int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    testBaseIniFixer();
    testGetFix();
    testPrevFixer();
    testFix();
    testRemapBoilerPlate();
    testFixNoPath();
    testFixHideOrigAndBackup();

    if (failures == 0) {
        std::printf("\nALL PASSED\n");
        return 0;
    }

    std::printf("\n%d CHECK(S) FAILED\n", failures);
    return 1;
}
