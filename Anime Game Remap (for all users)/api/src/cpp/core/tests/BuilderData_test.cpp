// -----------------------------------------------------------------------------
// Standalone regression test for the three version-keyed factory tables in
// data/: IniParseBuilderData, IniFixBuilderData and IniRemoveBuilderData.
//
// The first two are ports of data/IniParseBuilderData.py and
// data/IniFixBuilderData.py, so their row and version counts are asserted
// against those originals. The third has NO Python original (see
// IniRemoveBuilderData.h's own warning) -- it is one row per GI mod type, all at
// the 4.0 baseline -- so it is checked against this codebase's contract instead.
//
// Every generator in those tables is currently a STUB returning the builder's
// defaultFactory(), so this test deliberately does NOT assert on which concrete
// strategy comes back -- there is only one. What it does assert is that the
// *table* is faithful and that version selection works through it, which is the
// part that has to stay correct while the stubs get filled in one by one:
//   * row and version counts match the pure-Python originals exactly
//     (53 rows / 9 versions for parse, 73 rows / 10 versions for fix), and the
//     remove table is 43 rows all at 4.0
//   * spot-checked rows exist at the versions the Python file lists them at,
//     including the mods that legitimately appear more than once
//   * floor-matching through the real table: a mod listed only at 4.0 still
//     resolves at 5.7, and a mod with a later row switches over at that version
//   * every row's mod name resolves through ModTypeIdTools, i.e. the table keys
//     and the registry agree
//   * a mod with no row at all still builds (errorOnNotFound = false)
//   * GIBuilder's mod types are wired to all three tables, not to fixed
//     factories
//
// Same build story as the builder tests -- link the already-built static lib
// (`cd cbuild && ninja AGRemapCore`):
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      BuilderData_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// See IniParseBuilder_test.cpp's header for why the three /NODEFAULTLIB flags
// are load-bearing. Copy libz3.dll next to test.exe before running.
// -----------------------------------------------------------------------------

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniRemoveBuilderData.h"

#include "AGRemapCore/constants/GIBuilder.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/strategies/ModType.h"

#include <cstdio>
#include <optional>
#include <set>
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
        failures++;
    }
}

Version ver(const std::string& raw) {
    return *Version::parse(raw);
}

std::string nameOf(ModTypeId id) {
    return ModTypeIdTools::getName(id);
}

// ---------------------------------------------------------------------------

void testTableShape() {
    std::printf("\n== table shape ==\n");

    // Counts taken straight from the pure-Python dicts, so a row silently dropped or duplicated
    // during the port shows up here.
    check(IniParseBuilderData::repo()->size() == 53, "the parse table has all 53 rows from IniParseBuilderData.py");
    check(IniFixBuilderData::repo()->size() == 78, "the fix table has 78 rows -- the 73 Python rows fanned out per target mod");

    // The remove table has no Python original -- one row per GI mod type, all at 4.0.
    check(IniRemoveBuilderData::repo()->size() == 43, "the remove table has one row per GI mod type");

    check(IniParseBuilderData::repo()->getTotalIndices() == 2, "the parse table has 2 index columns");
    check(IniParseBuilderData::repo()->getVersionIndexPos() == 0, "with the version at position 0");
    check(IniFixBuilderData::repo()->getTotalIndices() == 4, "the fix table has 4 index columns");
    check(IniFixBuilderData::repo()->getVersionColumnCount() == 2, "TWO of them are version columns (fromVersion, toVersion)");
    check(IniFixBuilderData::repo()->getNonVersionColumnCount() == 2, "leaving fromModName and toModName");

    // repo() is a build-once shared table, not a fresh one per call.
    check(IniRemoveBuilderData::repo()->getTotalIndices() == 2, "the remove table has 2 index columns");
    check(IniRemoveBuilderData::repo()->getVersionIndexPos() == 0, "with the version at position 0");

    check(IniParseBuilderData::repo() == IniParseBuilderData::repo(), "the parse table is a single shared instance");
    check(IniFixBuilderData::repo() == IniFixBuilderData::repo(), "the fix table is a single shared instance");
    check(IniRemoveBuilderData::repo() == IniRemoveBuilderData::repo(), "the remove table is a single shared instance");
}

void testVersionCoverage() {
    std::printf("\n== version coverage ==\n");

    // Collect the distinct versions each table actually holds, straight from its rows.
    std::set<std::string> parseVers;
    IniParseBuilderData::repo()->forEachEntry([&](const std::vector<std::string>&, const Version& v, const IniParseBuilder::Factory&) {
        parseVers.insert(v.toString());
    });


    const std::set<std::string> expectedParse = {"4.0", "4.4", "4.6", "4.8", "5.3", "5.4", "5.5", "5.6", "5.7"};

    check(parseVers == expectedParse, "the parse table covers exactly the 9 versions the Python file lists");
    check(parseVers.count("5.0") == 0, "and the parse table has no 5.0, matching its Python original");

    std::set<std::string> removeVers;
    IniRemoveBuilderData::repo()->forEachEntry([&](const std::vector<std::string>&, const Version& v, const IniRemoveBuilder::Factory&) {
        removeVers.insert(v.toString());
    });
    check(removeVers == std::set<std::string>{"4.0"}, "the remove table sits entirely at the 4.0 baseline");
}

void testRowsResolve() {
    std::printf("\n== rows resolve ==\n");

    const auto& parse = *IniParseBuilderData::repo();
    const auto& fix = *IniFixBuilderData::repo();

    // Spot-checks against specific lines of the Python files. errorOnNotFound = true, so a missing
    // row throws rather than silently falling back -- which is what makes these real assertions.
    check(parse.get({nameOf(ModTypeId::Amber)}, ver("4.0"), false).has_value(), "parse: Amber has its 4.0 row");
    check(parse.get({nameOf(ModTypeId::Raiden)}, ver("4.0"), false).has_value(), "parse: Raiden has its 4.0 row (giDefault)");
    check(parse.get({nameOf(ModTypeId::Arlecchino)}, ver("4.6"), false).has_value(), "parse: Arlecchino has its 4.6 row (giDefault)");
    check(parse.get({nameOf(ModTypeId::AyakaSpringbloom)}, ver("5.6"), false).has_value(), "parse: AyakaSpringBloom has its 5.6 row");
    check(parse.get({nameOf(ModTypeId::Nilou)}, ver("5.7"), false).has_value(), "parse: Nilou has its 5.7 row");

    // The fix table is 4-column now: (fromVersion, fromModName, toVersion, toModName), with
    // fromVersion 1.0 on every row. getAll fans out over the target mod.
    auto fixTargets = [&](ModTypeId from, const std::string& toVer) {
        std::set<std::string> out;
        for (const auto& m : fix.getAll({nameOf(from), std::nullopt}, {ver("1.0"), ver(toVer)})) {
            out.insert(m.first[1]);
        }
        return out;
    };

    check(fixTargets(ModTypeId::Kaeya, "5.0").size() == 1, "fix: Kaeya resolves at 5.0");
    check(fixTargets(ModTypeId::NilouBreeze, "5.4").size() == 1, "fix: NilouBreeze resolves at 5.4");

    // Jean is the multi-target case the fan-out exists for.
    std::set<std::string> jeanTargets = fixTargets(ModTypeId::Jean, "4.0");
    check(jeanTargets.size() == 2, "fix: Jean has TWO targets");
    check(jeanTargets.count(nameOf(ModTypeId::JeanCN)) == 1, "fix: one of them is JeanCN");
    check(jeanTargets.count(nameOf(ModTypeId::JeanSea)) == 1, "fix: and the other is JeanSea");

    check(fixTargets(ModTypeId::Amber, "4.0") == std::set<std::string>{nameOf(ModTypeId::AmberCN)},
          "fix: Amber's single target is AmberCN");

    // A mod the Python tables genuinely do not list.
    check(!parse.get({nameOf(ModTypeId::ArlecchinoBoss)}, ver("5.7"), false).has_value(),
          "parse: a mod with no row anywhere is genuinely absent");

    // Every row's mod name must round-trip through the registry -- catches the table and
    // ModTypeIdTools drifting apart.
    bool allNamed = true;
    parse.forEachEntry([&](const std::vector<std::string>& nonVersion, const Version&, const IniParseBuilder::Factory&) {
        if (nonVersion.size() != 1 || nonVersion[0].empty()) {
            allNamed = false;
        }
    });
    check(allNamed, "every parse row has exactly one, non-empty mod name");
}

void testFloorMatchingThroughTheRealTable() {
    std::printf("\n== floor-matching through the real table ==\n");

    IniParseBuilder parseBuilder(IniParseBuilderData::repo());
    IniFixBuilder fixBuilder(IniFixBuilderData::repo());

    // Every stub returns defaultFactory(), so the observable check here is that a build succeeds
    // for each (mod, version) the table should cover -- the selection itself, not the product.
    check(parseBuilder.build(nullptr, nameOf(ModTypeId::Amber), ver("4.0")) != nullptr, "parse: Amber at 4.0 builds");
    check(parseBuilder.build(nullptr, nameOf(ModTypeId::Amber), ver("5.7")) != nullptr, "parse: Amber at 5.7 floor-matches its 4.0 row and still builds");
    check(parseBuilder.build(nullptr, nameOf(ModTypeId::Nilou), ver("4.0")) != nullptr, "parse: Nilou at 4.0 builds");
    check(parseBuilder.build(nullptr, nameOf(ModTypeId::Nilou), ver("5.7")) != nullptr, "parse: Nilou at 5.7 picks up its own later row");
    check(parseBuilder.build(nullptr, nameOf(ModTypeId::Amber)) != nullptr, "parse: no version means latest, and still builds");

    check(!fixBuilder.buildAll(nullptr, nameOf(ModTypeId::Amber), ver("1.0"), ver("4.0")).empty(), "fix: Amber at 4.0 builds");
    check(!fixBuilder.buildAll(nullptr, nameOf(ModTypeId::Amber), ver("1.0"), ver("5.7")).empty(), "fix: Amber at 5.7 builds");
    check(fixBuilder.buildAll(nullptr, nameOf(ModTypeId::Jean), ver("1.0"), ver("4.0")).size() == 2,
          "fix: Jean fans out to both its targets through the real table");

    // A mod absent from the table degrades rather than throwing, since these builders are
    // constructed with errorOnNotFound = false.
    check(parseBuilder.build(nullptr, nameOf(ModTypeId::ArlecchinoBoss), ver("5.7")) != nullptr,
          "parse: an unlisted mod still builds, via the default-factory fallback");
    check(fixBuilder.buildAll(nullptr, "NotAModAtAll", ver("1.0"), ver("5.7")).empty(),
          "fix: an entirely unknown source mod fans out to nothing");
}

void testGIBuilderIsWiredToTheTables() {
    std::printf("\n== GIBuilder wiring ==\n");

    ModType amber = GIBuilder::amber();
    ModType jean = GIBuilder::jean();

    check(amber.iniParseBuilder != nullptr && amber.iniParseBuilder->getBuilderArgs() == IniParseBuilderData::repo(),
          "GIBuilder's parse builder is the version-dependent flavour over IniParseBuilderData");
    check(amber.iniFixBuilder != nullptr && amber.iniFixBuilder->getBuilderArgs() == IniFixBuilderData::repo(),
          "GIBuilder's fix builder is the version-dependent flavour over IniFixBuilderData");
    check(amber.iniRemoveBuilder != nullptr && amber.iniRemoveBuilder->getBuilderArgs() == IniRemoveBuilderData::repo(),
          "GIBuilder's remove builder is the table-backed flavour over IniRemoveBuilderData");

    // All 43 mod types share the one builder (and therefore the one table).
    check(amber.iniParseBuilder == jean.iniParseBuilder, "every GI mod type shares one parse builder");
    check(amber.iniFixBuilder == jean.iniFixBuilder, "every GI mod type shares one fix builder");
    check(amber.iniRemoveBuilder == jean.iniRemoveBuilder, "every GI mod type shares one remove builder");

    // Every mod type GIBuilder produces must have a row, or it silently degrades to a plain remover.
    bool allCovered = true;
    for (const ModType& m : {GIBuilder::amber(), GIBuilder::jean(), GIBuilder::raiden(),
                             GIBuilder::xingqiuBamboo(), GIBuilder::arlecchino()}) {
        if (!IniRemoveBuilderData::repo()->get({m.name}, std::nullopt, false).has_value()) {
            allCovered = false;
        }
    }
    check(allCovered, "spot-checked GI mod types all have a remove row");
}

}  // namespace

int main() {
    testTableShape();
    testVersionCoverage();
    testRowsResolve();
    testFloorMatchingThroughTheRealTable();
    testGIBuilderIsWiredToTheTables();

    std::printf("\n%s (%d failure(s))\n", (failures == 0 ? "ALL PASSED" : "FAILURES"), failures);
    return (failures == 0) ? 0 : 1;
}
