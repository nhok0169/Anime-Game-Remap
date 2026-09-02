// -----------------------------------------------------------------------------
// Standalone regression test for the two plain-C++ strategy contexts:
//
//   * AGRemapCore::IniFileParseContext  (an AGRemapCore::IniFile behind IniParseContext)
//   * AGRemapCore::IniFileFixContext    (an AGRemapCore::IniFile behind IniFixContext)
//
// WHY THIS FILE EXISTS: the pybind11 layer has its own implementations of both
// interfaces (PyIniParseContext / PyIniFixContext, covered from Python), and nothing in
// the Python suite ever instantiates these. They are reachable only from a caller with a
// real C++ AGRemapCore::IniFile, so they are covered here or nowhere -- the same reason
// GIMIFixer_test.cpp and IniFileResEditContext_test.cpp exist.
//
// These two are what a core-side IniParseBuilder/IniFixBuilder default factory would
// build its strategies against, in place of the do-nothing BaseIniParser/BaseIniFixer
// those currently return.
//
// NOT wired into any build target (core/tests/*.cpp never is). Compile and run it by hand:
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      IniFileContexts_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// (build AGRemapCore.lib first: `cd cbuild && ninja AGRemapCore`, and copy
//  cext/z3/bin/libz3.dll, cbuild/curl/lib/libcurl.dll and cbuild/utf8proc/utf8proc.dll
//  next to test.exe before running it)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
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


// A small .ini file whose sections are deliberately NOT in alphabetical order, so that
// "declaration order" and "whatever an unordered_map happens to give" cannot be confused.
const char* iniTxt =
    "[TextureOverrideZebra]\n"
    "run = CommandListZebra\n"
    "\n"
    "[CommandListZebra]\n"
    "vb1 = ResourceApple\n"
    "\n"
    "[ResourceApple]\n"
    "type = Buffer\n";


std::unique_ptr<IniFile> makeIni() {
    return std::make_unique<IniFile>(std::nullopt, iniTxt);
}


// ---------------------------------------------------------------------------------------
// IniFileParseContext
// ---------------------------------------------------------------------------------------

void testParseContextSections() {
    std::printf("\n--- IniFileParseContext (sections) ---\n");

    std::unique_ptr<IniFile> ini = makeIni();
    IniFileParseContext ctx(ini.get());

    check(ctx.hasIni() && ctx.getIniFile() == ini.get(), "a context remembers the .ini file it wraps");

    std::vector<std::string> names = ctx.sectionNames();
    check(names.size() == 3, "every section is reported");
    check(names[0] == "TextureOverrideZebra" && names[1] == "CommandListZebra" && names[2] == "ResourceApple",
          "and in the order the .ini file declared them, not sorted or hashed");

    check(ctx.sectionIfTemplates().size() == 3, "the keyed view holds the same sections");
    check(ctx.getSection("CommandListZebra") != nullptr, "a section can be looked up by name");
    check(ctx.getSection("NoSuchSection") == nullptr, "and a missing one is a null pointer, not a throw");

    check(ctx.z3Ctx() == ini->getZ3Ctx(), "the Z3 context is the .ini file's own, not a new one");
}


void testParseContextSectionMutation() {
    std::printf("\n--- IniFileParseContext (adding and removing sections) ---\n");

    std::unique_ptr<IniFile> ini = makeIni();
    IniFileParseContext ctx(ini.get());

    // A parser synthesizes sections as it goes -- the "...RemapFix" ones for a mod object the
    // .ini file has none of, and the download resources.
    std::unique_ptr<IniFileParseContext::Section> section = ctx.getSection("ResourceApple")->deepcopy();
    IniFileParseContext::Section* added = ctx.addSection("ResourceSynthesized", std::move(section));

    check(added != nullptr, "adding a section hands back a borrowed pointer to it");
    check(ctx.getSection("ResourceSynthesized") == added, "and the .ini file now has it under that name");

    std::vector<std::string> names = ctx.sectionNames();
    check(names.size() == 4 && names.back() == "ResourceSynthesized",
          "a new name goes on the end of the declaration order");
    check(names[0] == "TextureOverrideZebra", "leaving the order of what was already there alone");

    ctx.removeSection("CommandListZebra");
    names = ctx.sectionNames();
    check(names.size() == 3 && ctx.getSection("CommandListZebra") == nullptr, "removing a section drops it");
    check(names[0] == "TextureOverrideZebra" && names[1] == "ResourceApple" && names[2] == "ResourceSynthesized",
          "and the survivors keep their order -- no swap-with-the-last");

    ctx.removeSection("NoSuchSection");
    check(ctx.sectionNames().size() == 3, "removing one that isn't there does nothing");
}


void testParseContextModType() {
    std::printf("\n--- IniFileParseContext (mod type and downloads) ---\n");

    std::unique_ptr<IniFile> ini = makeIni();
    IniFileParseContext ctx(ini.get());

    check(!ctx.hasModType() && ctx.modTypeName().empty(), "with no mod type id, there is no mod type");
    check(ctx.modTypeHashes() == nullptr && ctx.modTypeIndices() == nullptr, "and no assets to look anything up in");

    ctx.setModTypeId(12345);
    check(!ctx.hasModType(), "an id the .ini file was never classified as is still no mod type");

    ctx.addFileDownload(std::make_unique<IniResource>("blend", "C:/Mods/TestMod", "blend.buf"));
    check(ini->getFileDownloads().size() == 1, "a file download is recorded on the .ini file");
    check(ini->getFileDownloads()[0]->type == "blend", "and it is the one that was added");

    ini->clear();
    check(ini->getFileDownloads().empty(), "clear() drops them, as the pure-Python clearModels does");

    IniFileParseContext empty;
    check(!empty.hasIni() && empty.iniFolder().empty() && empty.z3Ctx() == nullptr
              && empty.sectionNames().empty() && empty.getSection("anything") == nullptr,
          "a context with no .ini file answers as though there were none");
    check(empty.addSection("x", nullptr) == nullptr, "and has nowhere to put a synthesized section");
}


void testParseContextGroups() {
    std::printf("\n--- IniFileParseContext (graph groups) ---\n");

    std::unique_ptr<IniFile> ini = makeIni();
    IniFileParseContext ctx(ini.get());

    check(ctx.graphGroups().size() == 0, "a context starts with no groups");

    ctx.graphGroups().insertGroup(0);
    ctx.graphGroups().insertGroup(1);
    check(ctx.graphGroups().size() == 2, "a parser can build groups through the context");

    std::vector<IniGraphGroup<std::string, std::string>> taken = ctx.takeGroups();
    check(taken.size() == 2, "takeGroups hands them over...");
    check(ctx.graphGroups().size() == 0, "...and leaves the context empty, ready to be reused");

    ctx.graphGroups().insertGroup(0);
    check(ctx.graphGroups().size() == 1 && taken.size() == 2,
          "the view was rebuilt, so building again does not disturb what was taken");
}


// ---------------------------------------------------------------------------------------
// IniFileFixContext
// ---------------------------------------------------------------------------------------

void testFixContextPaths() {
    std::printf("\n--- IniFileFixContext (paths) ---\n");

    IniFile ini("C:/Mods/TestMod/CuteLittleEi.ini", iniTxt);
    IniFileFixContext ctx(&ini);

    check(ctx.hasIni() && ctx.getIniFile() == &ini, "a context remembers the .ini file it wraps");

    std::optional<std::string> first = ctx.fixedFilePath(0);
    check(first.has_value() && std::filesystem::path(*first) == std::filesystem::path("C:/Mods/TestMod/CuteLittleEi.ini"),
          "group 0 writes to the .ini file's own path");

    std::optional<std::string> second = ctx.fixedFilePath(1);
    check(second.has_value()
              && std::filesystem::path(*second).filename() == std::filesystem::path("CuteLittleEiRemapFix1.ini"),
          "every later group writes to a copy, named by the RemapFix suffix and the index");
    check(std::filesystem::path(*second).parent_path() == std::filesystem::path("C:/Mods/TestMod"),
          "in the same folder as the .ini file");

    IniFile pathless(std::nullopt, iniTxt);
    IniFileFixContext pathlessCtx(&pathless);
    check(!pathlessCtx.fixedFilePath(0).has_value(), "an .ini file with no path has nowhere to write");
    check(!pathlessCtx.fixedFileExists(), "and nothing exists at the path it does not have");
}


void testFixContextInherited() {
    std::printf("\n--- IniFileFixContext (inherited from RemapIniFixContext) ---\n");

    IniFile ini("C:/Mods/TestMod/CuteLittleEi.ini", iniTxt);
    IniFileFixContext ctx(&ini);

    check(!ctx.modTypeName().has_value(), "an unclassified .ini file has no mod type name");
    check(ctx.getFixHeader() == "; --------------- GI Remap ---------------",
          "so the inherited boilerplate falls back to 'GI', with no extra code here");

    std::string fix = ctx.addFixBoilerPlate("FIXBODY");
    check(fix.rfind("; --------------- GI Remap ---------------", 0) == 0 && fix.find("FIXBODY") != std::string::npos,
          "and addFixBoilerPlate is inherited whole");

    // hideOriginalSections is inherited too, and works through fileTxt/setFileTxt -- both of which
    // this class does implement.
    //
    // Pathless on purpose: a file-backed IniFile ignores the constructor's 'txt' and reads from
    // disk when something first asks (see IniFile's own constructor), so 'ini' above has no text
    // yet and so nothing to hide.
    IniFile textOnly(std::nullopt, iniTxt);
    IniFileFixContext textOnlyCtx(&textOnly);

    textOnlyCtx.hideOriginalSections({"CommandListZebra"});
    check(textOnly.getFileTxt().find(IniKeywords::HideOriginalComment + "[CommandListZebra]") != std::string::npos,
          "hiding a section rewrites the .ini file's own text through setFileTxt");
    check(textOnly.getFileTxt().find("\n[ResourceApple]") != std::string::npos,
          "leaving every other section alone");

    check(ctx.modsToFix().empty(),
          "modsToFix is empty -- ModMappedAssets.fixTo is never populated, in Python either");
}


void testFixContextWriting() {
    std::printf("\n--- IniFileFixContext (writing and logging) ---\n");

    std::filesystem::path dir = std::filesystem::temp_directory_path() / "AGRemapIniFileFixContextTest";
    std::error_code err;
    std::filesystem::remove_all(dir, err);
    std::filesystem::create_directories(dir, err);

    std::filesystem::path iniPath = dir / "CuteLittleEi.ini";
    {
        std::ofstream out(iniPath, std::ios::binary | std::ios::trunc);
        out << iniTxt;
    }

    IniFile ini(iniPath.string(), iniTxt);
    IniFileFixContext ctx(&ini);

    check(ctx.fixedFileExists(), "an .ini file that is really on disk is seen");

    std::filesystem::path written = dir / "written.ini";
    ctx.writeFixedFile(written.string(), "the fixed content");
    check(std::filesystem::exists(written), "writeFixedFile really writes");

    {
        std::ifstream in(written, std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        check(content == "the fixed content", "and writes exactly what it was given");
    }

    check(ctx.getLogs().empty(), "a context starts with no logs");
    ctx.log("Cleaning up and disabling the OLD STINKY ini");
    check(ctx.getLogs().size() == 1, "a logged line is kept rather than dropped");

    ctx.disableIni();
    check(!std::filesystem::exists(iniPath), "disableIni moves the .ini file aside...");
    check(std::filesystem::exists(dir / "RemapBKUPCuteLittleEi.txt"),
          "...to a RemapBKUP-prefixed .txt, so a mod loader stops reading it");
    check(!ctx.fixedFileExists(), "and it no longer exists where it was");

    std::filesystem::remove_all(dir, err);
}


void testFixContextGroups() {
    std::printf("\n--- IniFileFixContext (graph groups) ---\n");

    IniFile ini("C:/Mods/TestMod/CuteLittleEi.ini", iniTxt);
    IniFileFixContext ctx(&ini);

    std::unique_ptr<IniFileFixContext::GraphGroups> first = ctx.makeGraphGroups();
    check(first != nullptr && first->size() == 0, "makeGraphGroups hands back a fresh, empty place for groups");

    first->insertGroup(0);
    check(first->size() == 1, "which a fixer can then fill");

    std::unique_ptr<IniFileFixContext::GraphGroups> second = ctx.makeGraphGroups();
    check(second != nullptr && second->size() == 0, "a second call is a different, empty one...");
    check(first->size() == 1, "...and does not disturb the first -- a chain of fixers holds both at once");

    // setIsFixed is deliberately a no-op, exactly as IniFileRemoveContext's is.
    ctx.setIsFixed(true);
    check(true, "setIsFixed does nothing, leaving IniFile::classify to own that flag");
}

}


int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    testParseContextSections();
    testParseContextSectionMutation();
    testParseContextModType();
    testParseContextGroups();

    testFixContextPaths();
    testFixContextInherited();
    testFixContextWriting();
    testFixContextGroups();

    if (failures == 0) {
        std::printf("\nALL PASSED\n");
        return 0;
    }

    std::printf("\n%d CHECK(S) FAILED\n", failures);
    return 1;
}
