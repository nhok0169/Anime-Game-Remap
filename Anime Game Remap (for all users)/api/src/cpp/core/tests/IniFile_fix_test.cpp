// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniFile::fix() actually producing a
// fix, and for the renderer that lets it.
//
// Two things had to be true before this could pass, and each failed silently on
// its own:
//   * IniFixBuilder::defaultFactory returned a do-nothing BaseIniFixer, so no
//     GIMIFixer was ever built. fix() returned an empty map for every .ini file.
//   * A GIMIFixer with no 'sectionToStr' in its config renders an EMPTY string --
//     it builds its groups correctly and then writes nothing. Neither IfTemplate
//     nor IfContentPart has a toStr of its own (deliberately), so the renderer
//     is handed in as a callback: renderIfTemplate.
//
// Also covers the renderer directly, since it is the piece with real formatting
// behaviour: KVP lines, the section header, prefix/suffix, and the if/elif/else/
// endif indentation, which dedents BEFORE writing a closer and indents AFTER
// writing an opener.
//
// ...and GIMIFixer::fixKey, which is what keeps "where does this get written"
// separate from "what is it keyed by". Only the former can have no answer.
//
// Needs the full static lib. Build AGRemapCore first ("cd cbuild && ninja
// AGRemapCore"), then compile as described in IniFile_resources_test.cpp.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"

namespace AGRC = AGRemapCore;

static int failures = 0;


static void check(bool condition, const std::string& what) {
    if (condition) {
        return;
    }

    std::printf("  FAILED: %s\n", what.c_str());
    ++failures;
}


static void checkEqual(const std::string& got, const std::string& expected, const std::string& what) {
    if (got == expected) {
        return;
    }

    std::printf("  FAILED: %s\n    expected: '%s'\n    got:      '%s'\n",
                what.c_str(), expected.c_str(), got.c_str());
    ++failures;
}


static std::string raidenIni() {
    return "[TextureOverrideRaidenBody]\n"
           "hash = 1a495487\n"
           "run = CommandListRaidenBody\n"
           "\n"
           "[CommandListRaidenBody]\n"
           "vb0 = ResourceRaidenBody\n"
           "\n"
           "[ResourceRaidenBody]\n"
           "type = Buffer\n"
           "filename = RaidenBody.buf\n";
}


// fix() keys its result by file path, so anything exercising it needs a real one.
static std::string writeTempIni() {
    const std::string path = (std::filesystem::temp_directory_path() / "agremap_fix_test.ini").string();
    std::ofstream out(path, std::ios::binary);
    out << raidenIni();
    out.close();
    return path;
}


static void testFileLessIniStillProducesAFix() {
    std::printf("testFileLessIniStillProducesAFix\n");

    // Where a fix is WRITTEN and what it is KEYED BY are the fixer's two separate decisions, and
    // only the first can be unanswerable. A text-only .ini file has nowhere to write, but its fix
    // still exists -- GIMIFixer::fixKey falls back to the group index. This used to come back as
    // an empty map, with the fix produced and then dropped for want of a key.
    AGRC::IniFile ini(std::nullopt, raidenIni());
    ini.classify();

    check(ini.getAvailableType() != nullptr, "it still classifies");

    std::unordered_map<std::string, std::string> fix = ini.fix(false, true, false);

    check(!fix.empty(), "a file-less .ini file still produces a fix");
    check(fix.count("0") == 1, "...keyed by the group index, since there is no path to key it by");
    check(!fix.at("0").empty(), "...and that entry has real content");
}


static void testRenderContentPart() {
    std::printf("testRenderContentPart\n");

    AGRC::IfContentPart<std::string, std::string> part;
    part.addKVP("hash", "abc123");
    part.addKVP("vb0", "ResourceBody");

    checkEqual(AGRC::renderIfContentPart(part), "hash = abc123\nvb0 = ResourceBody",
               "one 'key = value' line per KVP, no trailing newline");
    checkEqual(AGRC::renderIfContentPart(part, "; "), "; hash = abc123\n; vb0 = ResourceBody",
               "the line prefix goes on every line -- what hiding a section relies on");

    AGRC::IfContentPart<std::string, std::string> empty;
    checkEqual(AGRC::renderIfContentPart(empty), "", "an empty part renders to nothing");
}


static void testRenderSection() {
    std::printf("testRenderSection\n");

    // Round-tripping a real .ini file is the strongest check available here: whatever the parser
    // read back out has to render to the same text it came from.
    AGRC::IniFile ini(std::nullopt, raidenIni());

    AGRC::IfTemplate<std::string, std::string>* section = ini.getSection("ResourceRaidenBody");
    check(section != nullptr, "the section was read");
    if (section == nullptr) {
        return;
    }

    checkEqual(AGRC::renderIfTemplate(*section),
               "[ResourceRaidenBody]\ntype = Buffer\nfilename = RaidenBody.buf",
               "a section round-trips through the renderer unchanged");
}


static void testRenderSectionIndentsConditionals() {
    std::printf("testRenderSectionIndentsConditionals\n");

    AGRC::IniFile ini(std::nullopt,
                      "[TextureOverrideRaidenBody]\n"
                      "if $active == 1\n"
                      "hash = abc123\n"
                      "else\n"
                      "hash = def456\n"
                      "endif\n");

    AGRC::IfTemplate<std::string, std::string>* section = ini.getSection("TextureOverrideRaidenBody");
    check(section != nullptr, "the section was read");
    if (section == nullptr) {
        return;
    }

    // 'else' and 'endif' dedent before they are written, so they line up with the 'if'.
    const std::string rendered = AGRC::renderIfTemplate(*section);
    check(rendered.find("\n\thash = abc123") != std::string::npos,
          "the body of an if block is indented one tab");
    check(rendered.find("\nelse") != std::string::npos, "'else' lines up with its 'if', not the body");
    check(rendered.find("\nendif") != std::string::npos, "and so does 'endif'");

    checkEqual(AGRC::renderIfTemplate(*section, "", false).find("\t") == std::string::npos ? "none" : "some",
               "none", "autoindent = false writes no tabs at all");
}


static void testFixProducesAFix() {
    std::printf("testFixProducesAFix\n");

    // A REAL file on disk, not a txt-only IniFile: fix() returns its result keyed by file path,
    // and IniFileFixContext::fixedFilePath answers std::nullopt when there is no path -- so a
    // file-less .ini file legitimately fixes to nothing. That is a contract, not a gap.
    const std::string path = writeTempIni();

    AGRC::IniFile ini(path);
    ini.classify();

    check(ini.getAvailableType() != nullptr, "the .ini file classifies as a mod type");

    std::unordered_map<std::string, std::string> fix = ini.fix(false, true, false);

    // Before the default fixer was a real GIMIFixer with a renderer, this was empty for every
    // input -- first because no fixer was built, then because one with no sectionToStr renders
    // nothing.
    check(!fix.empty(), "fix() produces at least one entry");

    std::size_t nonEmpty = 0;
    for (const auto& entry : fix) {
        if (!entry.second.empty()) {
            ++nonEmpty;
        }
    }

    std::printf("  (%zu entr(y/ies), %zu with content)\n", fix.size(), nonEmpty);
    check(nonEmpty > 0, "...and at least one of them has actual rendered content");
}


static void testUnclassifiedFileFixesToNothing() {
    std::printf("testUnclassifiedFileFixesToNothing\n");

    AGRC::IniFile ini(std::nullopt, "[SomethingElse]\nkey = value\n");
    ini.classify();

    check(ini.fix(false, true, false).empty(), "nothing classifies it, so there is nothing to fix");
}


int main() {
    // classify() resolves ids through the registry, so the shipped mod types have to be in it.
    AGRC::GlobalModTypes::registerAll();

    testRenderContentPart();
    testRenderSection();
    testRenderSectionIndentsConditionals();
    testFixProducesAFix();
    testFileLessIniStillProducesAFix();
    testUnclassifiedFileFixesToNothing();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
