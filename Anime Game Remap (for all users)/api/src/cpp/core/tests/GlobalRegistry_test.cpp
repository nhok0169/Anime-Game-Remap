// -----------------------------------------------------------------------------
// Standalone regression test for the two bugs in how AGRemapCore::GlobalIniClassifiers
// populates AGRemapCore::ModTypeIdTools's global registry
// (constants/GlobalIniClassifiers.h, constants/GlobalModTypes.h,
// constants/ModTypeId.h).
//
// Both had one root cause: the registry population sat INSIDE classifier()'s
// one-shot lazy initializer, so it fired exactly once per process at whatever
// moment something first classified, and never again.
//
//   1. clear() permanently broke classification. ModTypeIdTools::clear() empties
//      the registry, but the classifier's own keyword DFA is untouched -- so it
//      kept finding mod type ids that nothing could resolve, for the rest of the
//      process. Every .ini file came back isMod == true with ZERO mod types.
//      (Confirmed pre-fix from Python: a Raiden .ini classified as {35: Raiden},
//      then after a clear() as {} with isMod still true.)
//
//   2. The implicit population OVERWROTE caller registrations. A caller doing
//      clear() + registerModType(<some shipped id>) + classify() had its own
//      ModType silently replaced by the shipped one, because the population ran
//      registerAll() (which overwrites) at first-classify time rather than at
//      any point the caller could see.
//
// The fix: ModTypeIdTools grew a generation counter bumped by clear();
// classifier() re-files whenever that generation moved; and it re-files with
// registerMissing() (fills gaps) rather than registerAll() (overwrites).
//
// NOTE ON ORDER: these tests deliberately run BEFORE any other use of the
// default classifier in this process, and each one leaves the registry cleared.
//
// Needs the full static lib. Build AGRemapCore first ("cd cbuild && ninja
// AGRemapCore"), then compile as described in IniFile_resources_test.cpp.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <string>

#include "AGRemapCore/constants/GlobalIniClassifiers.h"
#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"

using namespace AGRemapCore;

static int failures = 0;


static void check(bool condition, const std::string& what) {
    std::printf("%s %s\n", condition ? "[PASS]" : "[FAIL]", what.c_str());
    if (!condition) {
        ++failures;
    }
}


// A .ini file the shipped classifier recognizes, so the registry lookup actually has to resolve
// something.
static const char* RAIDEN_INI = "[TextureOverrideRaidenBody]\nhash = abcdabcd\n";


static void testClassifyResolvesModTypesAtAll() {
    std::printf("testClassifyResolvesModTypesAtAll\n");

    ModTypeIdTools::clear();

    IniFile ini(std::nullopt, RAIDEN_INI);
    ini.classify();

    check(!ini.getModTypes().empty(), "a shipped .ini file resolves at least one mod type");
    check(ini.getIsMod(), "and is a mod");
}


static void testClearDoesNotPermanentlyBreakClassification() {
    std::printf("testClearDoesNotPermanentlyBreakClassification\n");

    // First use populates.
    IniFile before(std::nullopt, RAIDEN_INI);
    before.classify();
    const std::size_t beforeCount = before.getModTypes().size();
    check(beforeCount > 0, "classifies before the clear");

    // THE BUG: this used to leave the registry empty for the rest of the process, because the
    // population was welded to a one-shot static.
    ModTypeIdTools::clear();

    IniFile after(std::nullopt, RAIDEN_INI);
    after.classify();

    check(!after.getModTypes().empty(), "still classifies AFTER ModTypeIdTools::clear()");
    check(after.getModTypes().size() == beforeCount, "and resolves the same number of mod types");

    // The specific broken shape the bug produced, called out on its own because it is the one a
    // caller would actually notice: a mod that is a mod but is of no type.
    check(!(after.getIsMod() && after.getModTypes().empty()),
          "never reports isMod with zero mod types");
}


static void testImplicitPopulationDoesNotOverwriteCallerRegistrations() {
    std::printf("testImplicitPopulationDoesNotOverwriteCallerRegistrations\n");

    // Find a shipped id, so the collision is real rather than hypothetical.
    ModTypeIdTools::clear();
    GlobalModTypes::registerAll();

    int shippedId = -1;
    std::string shippedName;
    for (int candidate = 0; candidate < 200; ++candidate) {
        std::optional<ModType> modType = ModTypeIdTools::getModType(candidate);
        if (modType.has_value()) {
            shippedId = candidate;
            shippedName = modType->name;
            break;
        }
    }

    check(shippedId >= 0, "found a shipped mod type id to collide with");
    if (shippedId < 0) {
        return;
    }

    // Now: start empty, claim that id for ourselves, and classify.
    ModTypeIdTools::clear();
    ModTypeIdTools::registerModType(ModType(0, shippedId, "CallerOwnedType"));

    IniFile ini(std::nullopt, RAIDEN_INI);
    ini.classify();

    std::optional<ModType> afterClassify = ModTypeIdTools::getModType(shippedId);
    check(afterClassify.has_value(), "the caller's mod type is still registered after classifying");
    check(afterClassify.has_value() && afterClassify->name == "CallerOwnedType",
          "and is still the CALLER's, not the shipped one it collided with");
    check(shippedName != "CallerOwnedType", "(sanity: the shipped one really has a different name)");

    // The rest of the registry was still filled in around it -- gap-filling, not no-op.
    std::size_t registered = 0;
    for (int candidate = 0; candidate < 200; ++candidate) {
        if (ModTypeIdTools::getModType(candidate).has_value()) {
            ++registered;
        }
    }

    check(registered > 1, "and the other shipped mod types were still filled in around it");
}


static void testRegisterAllStillOverwrites() {
    std::printf("testRegisterAllStillOverwrites\n");

    // The EXPLICIT call keeps its documented overwrite semantics -- only the implicit path yields.
    ModTypeIdTools::clear();
    GlobalModTypes::registerAll();

    int shippedId = -1;
    for (int candidate = 0; candidate < 200; ++candidate) {
        if (ModTypeIdTools::getModType(candidate).has_value()) {
            shippedId = candidate;
            break;
        }
    }

    if (shippedId < 0) {
        check(false, "found a shipped mod type id");
        return;
    }

    ModTypeIdTools::registerModType(ModType(0, shippedId, "CallerOwnedType"));
    GlobalModTypes::registerAll();

    std::optional<ModType> afterRegisterAll = ModTypeIdTools::getModType(shippedId);
    check(afterRegisterAll.has_value() && afterRegisterAll->name != "CallerOwnedType",
          "registerAll() still overwrites, unlike registerMissing()");

    ModTypeIdTools::registerModType(ModType(0, shippedId, "CallerOwnedType"));
    GlobalModTypes::registerMissing();

    std::optional<ModType> afterRegisterMissing = ModTypeIdTools::getModType(shippedId);
    check(afterRegisterMissing.has_value() && afterRegisterMissing->name == "CallerOwnedType",
          "registerMissing() leaves it alone");
}


static void testGenerationCountsClearsOnly() {
    std::printf("testGenerationCountsClearsOnly\n");

    const unsigned long long start = ModTypeIdTools::generation();

    ModTypeIdTools::registerModType(ModType(0, 12345, "SomeType"));
    check(ModTypeIdTools::generation() == start, "registerModType does NOT bump the generation");

    ModTypeIdTools::clear();
    check(ModTypeIdTools::generation() == start + 1, "clear() bumps it by one");

    ModTypeIdTools::clear();
    check(ModTypeIdTools::generation() == start + 2, "and again");
}


int main() {
    // Order matters: the first two tests are about what happens around the FIRST use of the
    // default classifier in this process.
    testClassifyResolvesModTypesAtAll();
    testClearDoesNotPermanentlyBreakClassification();
    testImplicitPopulationDoesNotOverwriteCallerRegistrations();
    testRegisterAllStillOverwrites();
    testGenerationCountsClearsOnly();

    ModTypeIdTools::clear();

    if (failures > 0) {
        std::printf("\n%d check(s) FAILED.\n", failures);
        return 1;
    }

    std::printf("\nAll tests passed.\n");
    return 0;
}
