// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniFile::parse() actually parsing.
//
// Until IniParseBuilder::defaultFactory was wired to a real GIMIParser it returned
// a do-nothing BaseIniParser, so parse() completed and produced NOTHING for every
// .ini file. This pins that it now produces graph groups.
//
// Note what parse() does and does not do: it builds the graph groups, keyed by
// mod type id. Resources (IniFile::getResources) are populated by the resource
// EDITS that run during fix(), not here -- a parse finding 0 resources is
// correct, which is why this test measures graph groups instead.
//
// Needs the full static lib. Build AGRemapCore first ("cd cbuild && ninja
// AGRemapCore"), then compile as described in IniFile_resources_test.cpp.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <optional>
#include <string>

#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"

namespace AGRC = AGRemapCore;

static int failures = 0;


static void check(bool condition, const std::string& what) {
    if (condition) {
        return;
    }

    std::printf("  FAILED: %s\n", what.c_str());
    ++failures;
}


static std::string raidenIni() {
    return "[Constants]\n"
           "global $active = 0\n"
           "\n"
           "[TextureOverrideRaidenBody]\n"
           "hash = 1a495487\n"
           "run = CommandListRaidenBody\n"
           "\n"
           "[CommandListRaidenBody]\n"
           "vb0 = ResourceRaidenBody\n"
           "ib = ResourceRaidenBodyIB\n"
           "\n"
           "[ResourceRaidenBody]\n"
           "type = Buffer\n"
           "stride = 40\n"
           "filename = RaidenBody.buf\n";
}


static void testDefaultParserIsARealOne() {
    std::printf("testDefaultParserIsARealOne\n");

    // The default used to be a bare BaseIniParser whose parse produced nothing. It is a GIMIParser
    // now, matching the pure-Python ModType.__init__'s own IniParseBuilder(GIMIParser) default.
    AGRC::IniFile ini(std::nullopt, raidenIni());
    ini.classify();

    check(ini.getIsMod(), "the .ini file classifies as a mod");
    check(!ini.getModTypes().empty(), "...and names a mod type");
    if (ini.getModTypes().empty()) {
        return;
    }

    check(ini.getModTypes().begin()->second.name == "Raiden", "specifically Raiden");
}


static void testParseProducesGraphGroups() {
    std::printf("testParseProducesGraphGroups\n");

    AGRC::IniFile ini(std::nullopt, raidenIni());
    ini.classify();

    AGRC::IniFile::ParseData& parsed = ini.parse();

    check(!parsed.empty(), "parse() produces an entry for the mod type it was classified as");

    std::size_t groups = 0;
    for (const auto& entry : parsed) {
        groups += entry.second.size();
    }

    std::printf("  (%zu mod type(s) parsed, %zu graph group(s))\n", parsed.size(), groups);
    check(groups > 0, "...and that entry holds at least one graph group -- the do-nothing "
                      "BaseIniParser default produced none at all");
}


static void testParseIsCachedAcrossCalls() {
    std::printf("testParseIsCachedAcrossCalls\n");

    AGRC::IniFile ini(std::nullopt, raidenIni());
    ini.classify();

    const std::size_t first = ini.parse().size();
    const std::size_t second = ini.parse().size();

    check(first == second, "parsing twice yields the same shape rather than accumulating");
}


static void testUnclassifiedFileParsesToNothing() {
    std::printf("testUnclassifiedFileParsesToNothing\n");

    AGRC::IniFile ini(std::nullopt, "[SomethingElse]\nkey = value\n");
    ini.classify();

    check(ini.getModTypes().empty(), "nothing classifies it");
    check(ini.parse().empty(), "and so there is no mod type to parse for");
}


static void testResourcesAreNotAParseOutput() {
    std::printf("testResourcesAreNotAParseOutput\n");

    // Guards the thing that made this look broken: resources come from the resource EDITS that run
    // during fix(), so an empty getResources() after a parse is correct, not a missing feature.
    AGRC::IniFile ini(std::nullopt, raidenIni());
    ini.classify();
    ini.parse();

    check(ini.getResources().empty(), "parse() populates graph groups, not resources");
}


int main() {
    // The .ini file has to be classifiable before any of this means anything, and that needs the
    // shipped mod types filed in the registry -- IniFile::classify resolves ids through it.
    AGRC::GlobalModTypes::registerAll();

    testDefaultParserIsARealOne();
    testParseProducesGraphGroups();
    testParseIsCachedAcrossCalls();
    testUnclassifiedFileParsesToNothing();
    testResourcesAreNotAParseOutput();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
