// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::GlobalRemapIniRemover -- the general-use
// remover, ie. a RemapIniRemover that always behaves as though it were handed
// IniRemovalContext::ignoreModType.
//
// It exists for one state: a .ini file the classifier says belongs to a mod
// (IniClassifyStats::isMod) but could not attribute to any ModTypeId. On that
// file RemapIniRemover's strict rule can only ever recognize the boilerplate
// half -- the other half asks whether a leftover's `hash` belongs to one of the
// file's ModTypes, and there are none to ask -- so every Remap-named leftover
// outside the boilerplate would survive forever. See GlobalRemapIniRemover.h.
//
// What this pins down:
//   * the sweep really happens: a leftover the strict rule provably cannot
//     recognize (no boilerplate anywhere, no usable hash) is removed by this
//     class and kept by RemapIniRemover, over the SAME text
//   * a caller-supplied IniRemovalContext(false) does not turn it off, and the
//     caller's own object is not written through (it is taken by value)
//   * everything else is still RemapIniRemover's: the resources come out
//     classified the same way, and a non-Remap-named section outside any
//     boilerplate is still not a candidate
//   * GlobalIniRemoveBuilders::globalRemoveBuilder() is a single shared builder
//     that hands out a GlobalRemapIniRemover already bound with a context of its own,
//     while removeBuilder() still hands out a plain RemapIniRemover
//   * IniFile::removeFix's new 'readAllIni' parameter, in all four combinations
//     of (has mod types, readAllIni)
//
// ON WHAT IS **NOT** DIRECTLY OBSERVABLE HERE: removeFix's choice between the
// two global builders cannot be read back off the .ini file, because the two
// agree on the output by design -- the ordinary fallback pass is handed
// IniRemovalContext(true) anyway, so both sweep. The builders are function-local
// statics with no setter, so there is nothing to substitute either. What is
// checked instead is (a) each builder really builds the class it says it does,
// and (b) removeFix produces the right text in every one of the four states.
//
// Same build story as RemapIniRemover_test.cpp -- it reaches IniFile::getIfTemplates,
// so link the static library rather than a hand-picked source list:
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      GlobalRemapIniRemover_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// (build AGRemapCore.lib first: `cd cbuild && ninja AGRemapCore`, and copy
//  cext/z3/bin/libz3.dll, cbuild/curl/lib/libcurl.dll AND cbuild/utf8proc/utf8proc.dll
//  next to test.exe before running it -- a missing one exits -1073741515 with no
//  message at all)
// -----------------------------------------------------------------------------

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/constants/GlobalIniRemoveBuilders.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"
#include "AGRemapCore/model/strategies/iniRemovers/GlobalRemapIniRemover.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"
#include "AGRemapCore/model/strategies/iniRemovers/RemapIniRemover.h"

#include <cstdio>
#include <memory>
#include <optional>
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

void checkEqual(const std::string& actual, const std::string& expected, const char* description) {
    if (actual == expected) {
        std::printf("[PASS] %s\n", description);
        return;
    }

    std::printf("[FAIL] %s\n  expected: %s\n  actual:   %s\n", description, expected.c_str(), actual.c_str());
    failures++;
}

bool contains(const std::vector<std::string>& names, const std::string& name) {
    for (const std::string& current : names) {
        if (current == name) {
            return true;
        }
    }

    return false;
}

// The state this class exists for, spelled out: a mod's .ini file that no mod type owns.
class ModButNoTypeClassifier: public BaseIniClassifier {
    public:
        IniClassifyStats classify(const std::vector<std::string>&, std::optional<GameTypeId>) override {
            // No mod types at all, but isMod -- the second argument.
            return IniClassifyStats({}, true, true);
        }

        void checkIsFixedMod(const std::vector<std::string>&, bool* isFixedOut, bool* isModOut, std::optional<GameTypeId>) override {
            *isFixedOut = true;
            *isModOut = true;
        }
};

// A half-undone fix: the boilerplate is gone, and its Remap-named sections are the debris left
// behind. Nothing here carries a `hash`, so there is nothing for the strict rule to attribute even
// if the file HAD a mod type -- which is exactly the shape GlobalRemapIniRemover is for.
const std::string OrigMod =
    "[TextureOverrideFooBlend]\n"
    "vb1 = ResourceFooBlend\n"
    "\n"
    "[ResourceFooBlend]\n"
    "filename = FooBlend.buf\n";

const std::string LeftoverIni =
    OrigMod +
    "\n"
    "[TextureOverrideFooRemapBlend]\n"
    "vb1 = ResourceFooRemapBlend\n"
    "\n"
    "[ResourceFooRemapBlend]\n"
    "filename = FooRemapBlend.buf\n";

// One file-less IniFile with no mod type at all, plus a remover of the requested kind bound to it.
template <typename RemoverType>
struct Fixture {
    ModButNoTypeClassifier classifier;
    std::optional<IniFile> ini;
    std::optional<RemoverType> remover;

    explicit Fixture(const std::string& txt) {
        ini.emplace(std::nullopt, txt, std::nullopt, std::nullopt, std::optional<std::unordered_set<int>>{},
                    std::unordered_map<int, ModType>{}, &classifier);

        // Constructed unbound, then pointed at the .ini file -- which is what builds the
        // IniFileRemoveContext behind it, and the same path IniRemoveBuilder::build takes.
        remover.emplace();
        remover->setIniFile(&*ini);
    }
};

using Strict = RemapIniRemover<>;
using Global = GlobalRemapIniRemover<>;

// ---------------------------------------------------------------------------

void testSweepsWhatTheStrictRuleCannotSee() {
    std::printf("\n== the sweep ==\n");

    // First the strict rule over the same text, so the difference is the class and nothing else.
    Fixture<Strict> strict(LeftoverIni);
    std::string strictResult = strict.remover->remove(false, false);

    check(strictResult.find("[TextureOverrideFooRemapBlend]") != std::string::npos,
          "RemapIniRemover leaves an unattributable leftover standing");
    check(strict.remover->getTargetSectionNames().empty(),
          "and finds no targets at all on a file with no mod types and no boilerplate");

    Fixture<Global> global(LeftoverIni);
    std::string result = global.remover->remove(false, false);

    const std::vector<std::string>& targets = global.remover->getTargetSectionNames();
    check(contains(targets, "TextureOverrideFooRemapBlend") && contains(targets, "ResourceFooRemapBlend"),
          "GlobalRemapIniRemover makes every Remap-named leftover a target");

    check(result.find("Remap") == std::string::npos, "nothing Remap-named is left in the file");
    // OrigMod + "\n", not OrigMod: the final strip in this family is LEADING-only, so the blank
    // line that separated the original mod from the leftovers stays behind. See RemapIniRemover::remove.
    checkEqual(result, OrigMod + "\n", "exactly the original mod is left standing");
}


void testCandidateRuleIsUnchanged() {
    std::printf("\n== still only the candidates ==\n");

    // The sweep widens which CANDIDATES are targets, not what a candidate is. A section that is
    // neither inside a boilerplate region nor Remap-named is not a candidate, so it survives.
    Fixture<Global> fixture(LeftoverIni);
    std::string result = fixture.remover->remove(false, false);

    check(result.find("[TextureOverrideFooBlend]") != std::string::npos,
          "the original mod is not swept up with the fix");
    check(result.find("[ResourceFooBlend]") != std::string::npos,
          "nor is the resource it points at");
    check(!contains(fixture.remover->getTargetSectionNames(), "TextureOverrideFooBlend"),
          "and it was never a target to begin with");
}


void testCallerCannotTurnTheSweepOff() {
    std::printf("\n== the caller does not get a say ==\n");

    Fixture<Global> fixture(LeftoverIni);

    IniRemovalContext strictRequest(false);
    std::string result = fixture.remover->remove(false, false, strictRequest);

    check(result.find("Remap") == std::string::npos,
          "an explicit ignoreModType = false is ignored -- the sweep still happens");

    // The flag is set on GlobalRemapIniRemover's own copy (every remove() in this family takes the
    // context by value), so the caller's object comes back exactly as it went in.
    check(strictRequest.ignoreModType == false,
          "and the caller's own IniRemovalContext was not written through");
}


void testResourcesStillComeOut() {
    std::printf("\n== resources ==\n");

    Fixture<Global> fixture(LeftoverIni);
    fixture.remover->remove(false, false);

    const std::unordered_map<std::string, std::vector<std::unique_ptr<IniResource>>>& resources =
        fixture.remover->getRemovedResources();

    auto blends = resources.find(Global::ResourceType::Blend);
    check(blends != resources.end() && blends->second.size() == 1,
          "the removed section's .buf is collected, classified as a blend");

    if (blends != resources.end() && blends->second.size() == 1) {
        const std::string& srcPath = blends->second[0]->srcPath;
        check(srcPath.find("FooRemapBlend.buf") != std::string::npos, "and it is the right file");
        check(srcPath.find("FooBlend.buf") == std::string::npos || srcPath.find("FooRemapBlend.buf") != std::string::npos,
              "the surviving original's own .buf is not collected");
    }

    check(resources.find(Global::ResourceType::TexEdit) == resources.end(),
          "a kind nothing was found for is absent rather than empty");
}


void testGlobalRemoveBuilder() {
    std::printf("\n== GlobalIniRemoveBuilders::globalRemoveBuilder ==\n");

    const std::shared_ptr<IniRemoveBuilder>& first = GlobalIniRemoveBuilders::globalRemoveBuilder();
    const std::shared_ptr<IniRemoveBuilder>& second = GlobalIniRemoveBuilders::globalRemoveBuilder();

    check(first != nullptr, "a builder comes back");
    check(first.get() == second.get(), "and it is the same shared instance every time");
    check(first.get() != GlobalIniRemoveBuilders::removeBuilder().get(),
          "it is NOT the same builder as removeBuilder() -- the two hand out different removers");

    IniFile file(std::nullopt, LeftoverIni);

    std::shared_ptr<BaseIniRemover<>> global = first->build(&file);
    check(dynamic_cast<Global*>(global.get()) != nullptr, "globalRemoveBuilder builds a GlobalRemapIniRemover");
    check(dynamic_cast<Global*>(global.get())->getContext() != nullptr,
          "already bound, with an IniFileRemoveContext of its own");
    check(global->getIniFile() == &file, "and bound to the .ini file it was built for");

    std::shared_ptr<BaseIniRemover<>> strict = GlobalIniRemoveBuilders::removeBuilder()->build(&file);
    check(dynamic_cast<Strict*>(strict.get()) != nullptr, "removeBuilder still builds a RemapIniRemover");
    check(dynamic_cast<Global*>(strict.get()) == nullptr, "and that one is not a GlobalRemapIniRemover");

    // Fixed-factory, so the mod name and version it is asked about make no difference -- there is
    // no mod type to key on by definition, which is the state this builder exists for.
    check(dynamic_cast<Global*>(first->build(&file, "Amber").get()) != nullptr,
          "a mod name changes nothing -- it is the fixed-factory flavour");
    check(first->getBuilderArgs() == nullptr, "and it carries no lookup table");
}


void testRemoveFixReadAllIni() {
    std::printf("\n== IniFile::removeFix's readAllIni ==\n");

    // isMod, no mod types -- the state GlobalRemapIniRemover exists for. Both values of readAllIni end up
    // sweeping (see this file's header on why that is not directly observable), so what is checked
    // is that neither one regressed.
    ModButNoTypeClassifier classifier;

    IniFile swept(std::nullopt, LeftoverIni, std::nullopt, std::nullopt, std::optional<std::unordered_set<int>>{},
                  std::unordered_map<int, ModType>{}, &classifier);
    std::string sweptResult = swept.removeFix(false, false, true);

    check(swept.getIsMod(), "the file really is a mod's");
    check(swept.getModTypes().empty(), "and really has no mod types");
    checkEqual(sweptResult, OrigMod + "\n", "readAllIni = true: the leftover fix is stripped");

    IniFile notSwept(std::nullopt, LeftoverIni, std::nullopt, std::nullopt, std::optional<std::unordered_set<int>>{},
                     std::unordered_map<int, ModType>{}, &classifier);
    checkEqual(notSwept.removeFix(false, false, false), OrigMod + "\n",
               "readAllIni = false: the ordinary global fallback still strips it, exactly as before");

    // The default is false, and the two-argument call sites that predate this parameter still mean
    // what they always did.
    IniFile defaulted(std::nullopt, LeftoverIni, std::nullopt, std::nullopt, std::optional<std::unordered_set<int>>{},
                      std::unordered_map<int, ModType>{}, &classifier);
    checkEqual(defaulted.removeFix(false, false), OrigMod + "\n", "readAllIni defaults to false and changes nothing");
}


void testReadAllIniIsIgnoredWhenTheFileHasModTypes() {
    std::printf("\n== readAllIni only touches the no-mod-type fallback ==\n");

    // A classified file never reaches the fallback at all: it runs one pass per mod type, the last
    // of which sweeps. readAllIni has nothing to say about that, and must not change it.
    const int modTypeId = static_cast<int>(ModTypeId::Amber);
    std::unordered_map<int, ModType> overrides;
    overrides.emplace(modTypeId, ModType(static_cast<int>(GameTypeId::GI), modTypeId, "Amber"));

    std::unordered_set<int> forced = {modTypeId};

    IniFile withFlag(std::nullopt, LeftoverIni, std::nullopt, std::nullopt, forced, overrides, nullptr);
    IniFile withoutFlag(std::nullopt, LeftoverIni, std::nullopt, std::nullopt, forced, overrides, nullptr);

    checkEqual(withFlag.removeFix(false, false, true), withoutFlag.removeFix(false, false, false),
               "a classified file removes the same fix whether readAllIni was asked for or not");
    check(!withFlag.getModTypes().empty(), "and it really was classified");
}

}  // namespace


int main() {
    // Unbuffered, so a crash mid-run still leaves every line that already ran on screen -- an
    // access violation otherwise loses the lot and reads like a static-init failure.
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    testSweepsWhatTheStrictRuleCannotSee();
    testCandidateRuleIsUnchanged();
    testCallerCannotTurnTheSweepOff();
    testResourcesStillComeOut();
    testGlobalRemoveBuilder();
    testRemoveFixReadAllIni();
    testReadAllIniIsIgnoredWhenTheFileHasModTypes();

    std::printf("\n%s\n", failures == 0 ? "ALL CHECKS PASSED" : "THERE WERE FAILURES");
    return failures == 0 ? 0 : 1;
}
