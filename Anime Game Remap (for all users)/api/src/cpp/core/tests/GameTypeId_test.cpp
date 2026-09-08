// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::GameTypeIdTools -- the name/alias
// registry for GameTypeId, and the GameTypeId counterpart of
// ModTypeIdTools::findByName.
//
// WHY THIS FILE EXISTS: findByName/getAliases/getAll are bound to Python, so
// Testing/Unit Tester's test_GameTypeId.py covers them from that side. What it
// cannot cover is the thing this file exists to pin: GameType's constructor is
// the SINGLE SOURCE OF TRUTH for names, aliases, the declaration order and the
// Aho-Corasick DFA all at once, and the four are derived from one table
// precisely so they cannot drift apart. A test that only asks findByName
// questions would still pass if getName started answering out of a second,
// stale table -- which is exactly the arrangement that used to exist here (a
// hand-written switch in GameTypeId.cpp beside GameType's own vector).
//
// What it pins down:
//   * getName/getAliases/getAll agree with each other, and getAll is in
//     declaration order (the CLI's --help epilog prints in this order)
//   * every name AND every alias round-trips through findByName
//   * findByName ignores case and surrounding whitespace, matching
//     ModTypeIdTools::findByName
//   * a string naming nothing resolves to std::nullopt rather than guessing
//   * getEnum still rejects an int that is not a declared GameTypeId
//   * getHelpStr names the game and lists its aliases sorted
//
// NOT wired into any build target (core/tests/*.cpp never is -- no CMake entry,
// no CTest, not run by CI or by main.py). Compile and run it by hand; the
// static-lib link line in AI Agent Help/Building/CLAUDE.md works for it.
// -----------------------------------------------------------------------------

#include <algorithm>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/constants/GameTypeId.h"

using namespace AGRemapCore;

namespace {

int failures = 0;

void check(bool condition, const std::string& description) {
    if (condition) {
        std::printf("[PASS] %s\n", description.c_str());
    } else {
        std::printf("[FAIL] %s\n", description.c_str());
        failures++;
    }
}

void checkFinds(const std::string& txt, GameTypeId expected, const std::string& description) {
    const std::optional<GameTypeId> found = GameTypeIdTools::findByName(txt);

    if (found.has_value() && *found == expected) {
        std::printf("[PASS] %s\n", description.c_str());
        return;
    }

    std::printf("[FAIL] %s\n", description.c_str());
    std::printf("  ----- searched -----\n%s\n", txt.c_str());
    std::printf("  ----- expected -----\n%s\n", GameTypeIdTools::getName(expected).c_str());
    std::printf("  ----- actual -------\n%s\n",
                found.has_value() ? GameTypeIdTools::getName(*found).c_str() : "<nothing>");
    std::printf("  --------------------\n");
    failures++;
}


void testGetAllIsEveryGameInDeclarationOrder() {
    const std::vector<GameTypeId>& all = GameTypeIdTools::getAll();

    check(all.size() == 2, "getAll: both of the declared games are listed");
    check(all.size() == 2 && all[0] == GameTypeId::GI && all[1] == GameTypeId::WuWa,
          "getAll: in GameTypeId's own declaration order, not a hash order");
}

void testNamesAndAliases() {
    check(GameTypeIdTools::getName(GameTypeId::GI) == "GI", "getName: GI");
    check(GameTypeIdTools::getName(GameTypeId::WuWa) == "WuWa", "getName: WuWa");

    const std::vector<std::string>& giAliases = GameTypeIdTools::getAliases(GameTypeId::GI);
    check(std::find(giAliases.begin(), giAliases.end(), "Genshin") != giAliases.end(),
          "getAliases: GI answers to Genshin");
    check(std::find(giAliases.begin(), giAliases.end(), "GenshinImpact") != giAliases.end(),
          "getAliases: GI answers to GenshinImpact");

    const std::vector<std::string>& wuwaAliases = GameTypeIdTools::getAliases(GameTypeId::WuWa);
    check(std::find(wuwaAliases.begin(), wuwaAliases.end(), "WutheringWaves") != wuwaAliases.end(),
          "getAliases: WuWa answers to WutheringWaves");

    // A name is never repeated in its own alias list -- getHelpStr prints both, and a game whose
    // help text read "name: GI / aliases: GI" would be the tell that the two came apart.
    check(std::find(giAliases.begin(), giAliases.end(), "GI") == giAliases.end(),
          "getAliases: a game's own name is not also listed as one of its aliases");
}

// The drift guard: whatever getName/getAliases report has to be findable, for EVERY game, without
// this test naming any of them. A name added to GameType's table but left out of its DFA (or vice
// versa) fails here rather than at some user's command line.
void testEveryReportedNameRoundTrips() {
    for (GameTypeId gameTypeId : GameTypeIdTools::getAll()) {
        const std::string name = GameTypeIdTools::getName(gameTypeId);
        checkFinds(name, gameTypeId, "findByName round-trips getName's answer for " + name);

        for (const std::string& alias : GameTypeIdTools::getAliases(gameTypeId)) {
            checkFinds(alias, gameTypeId, "findByName round-trips the alias '" + alias + "' of " + name);
        }
    }
}

void testFindByNameNormalizesLikeModTypeIdToolsDoes() {
    checkFinds("gi", GameTypeId::GI, "findByName: lowercase name");
    checkFinds("GENSHIN", GameTypeId::GI, "findByName: uppercase alias");
    checkFinds("  WuWa  ", GameTypeId::WuWa, "findByName: surrounding whitespace is stripped");
    checkFinds("wUtHeRiNgWaVeS", GameTypeId::WuWa, "findByName: mixed case alias");
}

void testFindByNameRefusesToGuess() {
    check(!GameTypeIdTools::findByName("totally not a game").has_value(),
          "findByName: a string naming nothing resolves to nothing");
    check(!GameTypeIdTools::findByName("").has_value(),
          "findByName: the empty string resolves to nothing");
}

void testGetEnumStillValidates() {
    check(GameTypeIdTools::getEnum(static_cast<int>(GameTypeId::GI)) == GameTypeId::GI,
          "getEnum: a declared value converts");
    check(!GameTypeIdTools::getEnum(999999).has_value(),
          "getEnum: an undeclared value does not");
}

void testGetHelpStr() {
    const std::string giHelp = GameTypeIdTools::getHelpStr(GameTypeId::GI);

    check(giHelp.find("name: GI") != std::string::npos, "getHelpStr: names the game");

    // Sorted, matching ModType::getHelpStr -- "Genshin" before "GenshinImpact" regardless of the
    // order they were declared in.
    check(giHelp.find("aliases: Genshin, GenshinImpact") != std::string::npos,
          "getHelpStr: lists the aliases, sorted");
}

}


int main() {
    // Unbuffered, so a crash mid-run still shows which check it got to.
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    testGetAllIsEveryGameInDeclarationOrder();
    testNamesAndAliases();
    testEveryReportedNameRoundTrips();
    testFindByNameNormalizesLikeModTypeIdToolsDoes();
    testFindByNameRefusesToGuess();
    testGetEnumStillValidates();
    testGetHelpStr();

    if (failures == 0) {
        std::printf("\nAll GameTypeId tests passed.\n");
        return 0;
    }

    std::printf("\n%d GameTypeId test(s) FAILED.\n", failures);
    return 1;
}
