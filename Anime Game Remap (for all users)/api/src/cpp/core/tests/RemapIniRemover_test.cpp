// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::RemapIniRemover -- the reachability-based
// fix remover (see its own class doc for why it is NOT a port of IniRemover.py).
//
// WHY THIS FILE EXISTS: this started as the only coverage the class had -- when it
// was written there was no pybind11 binding for the iniRemovers/ family, and the
// pure-Python IniRemover was still live, so a green Testing/Unit Tester run proved
// nothing about this code either way. Both of those have since changed (see
// PyRemapIniRemover, and test_RemapIniRemover.py), but this file still covers the
// plain <std::string, std::string> instantiation and the plain-C++ context, neither
// of which the Python suite touches.
//
// What it pins down:
//   * the boilerplate scan (both heading flavours, the asymmetric-border real
//     heading, and the near-miss comment lines that must NOT open a region)
//   * candidate collection (inside the boilerplate: everything; outside it: only
//     names containing "Remap")
//   * target selection -- inside the boilerplate by the marker alone, outside
//     it by IfContentPartColouring hash state
//   * the removal set closing over BOTH directions of the reference relation,
//     which is wider than the run = graph (vb1/vb0/ib/ps-t0 ... all count)
//   * the boilerplate going with the sections it wrapped
//   * the IniRemoveContext seam -- both the IniFile-backed one and a
//     caller-supplied implementation with no AGRemapCore::IniFile at all
//   * classifyResource's full decision tree, and the resources collected off
//     the sections that were actually removed
//
// NOT wired into any build target (core/tests/*.cpp never is -- no CMake entry,
// no CTest, not run by CI or by main.py). Compile and run it by hand -- note this
// one reaches IniFile::getIfTemplates, so the per-source-file recipes some of the
// older tests in this directory carry do NOT link; link the static library:
//
//   cl /std:c++latest /EHsc /nologo /MD ^
//      /I <core>/include /I <extern>/utf8proc /I <extern>/ordered-map/include ^
//      /I <repo>/cext/z3/include ^
//      RemapIniRemover_test.cpp /Fe:test.exe ^
//      /link /NODEFAULTLIB:libcpmt.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libucrt.lib ^
//      <repo>/cbuild/src/cpp/core/AGRemapCore.lib ^
//      <repo>/cbuild/utf8proc/utf8proc.lib ^
//      <repo>/cext/z3/lib/libz3.lib ^
//      <repo>/cbuild/curl/lib/libcurl_imp.lib
//
// (build AGRemapCore.lib first: `cd cbuild && ninja AGRemapCore`, and copy
//  cext/z3/bin/libz3.dll next to test.exe before running it)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniFileRemoveContext.h"
#include "AGRemapCore/model/strategies/iniRemovers/RemapIniRemover.h"

#include <cstdio>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace AGRemapCore;

namespace {

// The plain-C++ instantiation. The whole point of the template parameters is the
// pybind11 layer's <py::object, ...> one, which this file cannot reach.
using Remover = RemapIniRemover<>;

int failures = 0;

void check(bool condition, const std::string& description) {
    if (condition) {
        std::printf("[PASS] %s\n", description.c_str());
    } else {
        std::printf("[FAIL] %s\n", description.c_str());
        failures++;
    }
}

void checkEqual(const std::string& actual, const std::string& expected, const std::string& description) {
    if (actual == expected) {
        std::printf("[PASS] %s\n", description.c_str());
        return;
    }

    std::printf("[FAIL] %s\n", description.c_str());
    std::printf("  ----- expected -----\n%s\n", expected.c_str());
    std::printf("  ----- actual -------\n%s\n", actual.c_str());
    std::printf("  --------------------\n");
    failures++;
}

// A real hash the shipped Hashes table knows -- {"4.0", "Raiden", "blend_vb"}.
const std::string RaidenBlendHash = "1a495487";

// Another real one -- {"4.0", "RaidenBoss", "blend_vb"}. The "to" side of a Raiden remap, and so
// what a fix section really carries. Deliberately a DIFFERENT mod name than the one above, to pin
// down RemapIniRemover::hashNonVersionVals' documented empty default.
const std::string RaidenBossBlendHash = "fe5c0180";

// Not in HashData.cpp or IndexData.cpp at all (verified by grep).
const std::string UnknownHash = "deadbeef";

// The .ini file is classified by force, so the classifier is never asked to identify anything --
// it only has to exist and answer checkIsFixedMod.
class StubIniClassifier: public BaseIniClassifier {
    public:
        IniClassifyStats classify(const std::vector<std::string>&, std::optional<GameTypeId>) override {
            return IniClassifyStats({}, true, true);
        }

        void checkIsFixedMod(const std::vector<std::string>&, bool* isFixedOut, bool* isModOut, std::optional<GameTypeId>) override {
            *isFixedOut = true;
            *isModOut = true;
        }
};

// One file-less IniFile forced onto one ModType with the full default Hashes table, plus the
// remover bound to it. Grouped so each test case can spin up a fresh, independent pair.
struct Fixture {
    StubIniClassifier classifier;
    std::optional<IniFile> ini;
    std::optional<Remover> remover;

    explicit Fixture(const std::string& txt) {
        std::unordered_set<int> forced = {77};
        std::unordered_map<int, ModType> overrides;

        // nullptr hashes -> ModType builds its own fully-populated Hashes, same as every real
        // GIBuilder mod type does.
        overrides.emplace(77, ModType(0, 77, "TestMod"));

        ini.emplace(std::nullopt, txt, std::nullopt, std::nullopt, forced, overrides, &classifier);

        // Constructed unbound, then pointed at the .ini file -- which is what builds the
        // IniFileRemoveContext behind it, and the same path IniRemoveBuilder::build takes.
        remover.emplace();
        remover->setIniFile(&*ini);
    }
};

// The srcPaths collected under one resource kind, reduced to their file names so a test can assert
// on them without caring what absolute folder they resolved against.
std::vector<std::string> collectedNames(const Remover& remover, const std::string& resType) {
    std::vector<std::string> result;

    auto found = remover.getRemovedResources().find(resType);
    if (found == remover.getRemovedResources().end()) {
        return result;
    }

    for (const std::unique_ptr<IniResource>& resource : found->second) {
        std::size_t slash = resource->srcPath.find_last_of("/\\");
        result.push_back(slash == std::string::npos ? resource->srcPath : resource->srcPath.substr(slash + 1));
    }

    return result;
}


bool contains(const std::vector<std::string>& names, const std::string& name) {
    for (const std::string& current : names) {
        if (current == name) {
            return true;
        }
    }

    return false;
}


// ===== the .ini fixtures =====

// Everything before the boilerplate -- the original mod, which must always survive untouched.
const std::string OrigMod =
    "[TextureOverrideFooBlend]\n"
    "hash = " + RaidenBlendHash + "\n"
    "run = CommandListFooBlend\n"
    "\n"
    "[CommandListFooBlend]\n"
    "vb1 = ResourceFooBlend\n"
    "\n"
    "[ResourceFooBlend]\n"
    "type = Buffer\n"
    "filename = FooBlend.buf\n";

const std::string RemapHeadingOpen = "; --------------- TestMod Remap ---------------\n";
const std::string RemapHeadingClose = "; --------------------------------------------\n";

// The three sections one remap fix writes: a hashed TextureOverride, the CommandList it runs, and
// the Resource that CommandList points at.
const std::string FixBody =
    "[TextureOverrideFooRemapBlend]\n"
    "hash = " + RaidenBossBlendHash + "\n"
    "run = CommandListFooRemapBlend\n"
    "\n"
    "[CommandListFooRemapBlend]\n"
    "vb1 = ResourceFooRemapBlend\n"
    "run = ResourceFooRemapBlend\n"
    "\n"
    "[ResourceFooRemapBlend]\n"
    "type = Buffer\n"
    "filename = FooRemapBlend.buf\n";


void testWholeFixAndBoilerPlateRemoved() {
    std::string txt =
        OrigMod +
        "\n" +
        RemapHeadingOpen +
        "; TestMod remapped by Albert Gold#2696 and NK#1321.\n"
        "\n" +
        FixBody +
        "\n" +
        RemapHeadingClose;

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    const std::vector<std::string>& targets = fixture.remover->getTargetSectionNames();
    check(!targets.empty() && targets[0] == "TextureOverrideFooRemapBlend",
          "whole fix: the hashed TextureOverride is the first target, in the .ini file's own order");
    check(!contains(targets, "TextureOverrideFooBlend"),
          "whole fix: a hashed section outside the candidate pool is not a target");

    const std::vector<std::string>& removed = fixture.remover->getRemovedSectionNames();
    check(removed.size() == 3, "whole fix: the target and both sections it reaches are removed");
    check(contains(removed, "CommandListFooRemapBlend"), "whole fix: the CommandList a target runs is a descendant");
    check(contains(removed, "ResourceFooRemapBlend"), "whole fix: the Resource that CommandList points at is a descendant");

    check(!contains(removed, "TextureOverrideFooBlend"),
          "whole fix: the original's own hashed TextureOverride is never a candidate -- it is outside the boilerplate and unnamed");

    // Everything the boilerplate held is gone, so the boilerplate goes with it -- heading lines,
    // credit comment and all.
    checkEqual(result, "[TextureOverrideFooBlend]\n"
                       "hash = " + RaidenBlendHash + "\n"
                       "run = CommandListFooBlend\n"
                       "\n"
                       "[CommandListFooBlend]\n"
                       "vb1 = ResourceFooBlend\n"
                       "\n"
                       "[ResourceFooBlend]\n"
                       "type = Buffer\n"
                       "filename = FooBlend.buf\n\n",
               "whole fix: only the original mod is left, and the boilerplate is gone with it");
}




void testAncestorsAndOutsideLeftoversRemoved() {
    std::string txt =
        OrigMod +
        "\n"
        // An ancestor: outside the boilerplate, named for the fix, and it runs INTO the target.
        // Nothing reaches it and it carries no hash, so only the reverse walk can find it.
        "[CommandListFooRemapWrapper]\n"
        "run = TextureOverrideFooRemapBlend\n"
        "\n"
        // A leftover of a half-undone fix: outside the boilerplate, and a descendant of the target
        // purely by name. getIfTemplates keeps THIS copy (first occurrence wins), not the one
        // inside the boilerplate -- both spans still have to go.
        "[ResourceFooRemapBlend]\n"
        "type = Buffer\n"
        "filename = StaleFooRemapBlend.buf\n"
        "\n" +
        RemapHeadingOpen +
        "\n" +
        FixBody +
        "\n" +
        RemapHeadingClose;

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    const std::vector<std::string>& removed = fixture.remover->getRemovedSectionNames();
    check(contains(removed, "CommandListFooRemapWrapper"),
          "ancestors: a candidate that runs into a target is removed even though nothing reaches it");
    check(contains(removed, "ResourceFooRemapBlend"),
          "ancestors: an outside-the-boilerplate leftover reached from a target is removed");

    check(result.find("StaleFooRemapBlend.buf") == std::string::npos,
          "ancestors: the leftover copy's own lines go too, not just the copy getIfTemplates kept");
    check(result.find("Remap") == std::string::npos,
          "ancestors: nothing named for the fix survives anywhere");

    checkEqual(result, "[TextureOverrideFooBlend]\n"
                       "hash = " + RaidenBlendHash + "\n"
                       "run = CommandListFooBlend\n"
                       "\n"
                       "[CommandListFooBlend]\n"
                       "vb1 = ResourceFooBlend\n"
                       "\n"
                       "[ResourceFooBlend]\n"
                       "type = Buffer\n"
                       "filename = FooBlend.buf\n\n",
               "ancestors: only the original mod is left");
}








void testEverythingInTheBoilerPlateIsATarget() {
    // The boilerplate is this software's own marker, so what it surrounds needs no `hash` to
    // qualify -- which is what makes a GIMI fix matched purely by TextureOverride *name* (no hash
    // anywhere in it, the normal case) removable at all. The pure-Python original gets there a
    // different way: its _fixRemovalPattern deletes the whole region outright.
    std::string txt =
        OrigMod +
        "\n" +
        RemapHeadingOpen +
        "\n"
        "[TextureOverrideFooRemapBlend]\n"
        "run = CommandListFooRemapBlend\n"
        "\n"
        "[CommandListFooRemapBlend]\n"
        "vb1 = ResourceFooRemapBlend\n"
        "\n"
        "[ResourceFooRemapBlend]\n"
        "filename = FooRemapBlend.buf\n"
        "\n"
        // No hash, no Remap in the name, no reference from anything -- inside the boilerplate is
        // the only thing that makes this a target, and it is enough.
        "[CommandListUnrelated]\n"
        "$someVar = 1\n"
        "\n" +
        RemapHeadingClose;

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    const std::vector<std::string>& targets = fixture.remover->getTargetSectionNames();
    check(contains(targets, "TextureOverrideFooRemapBlend") && contains(targets, "CommandListFooRemapBlend")
              && contains(targets, "ResourceFooRemapBlend") && contains(targets, "CommandListUnrelated"),
          "boilerplate targets: every section the boilerplate surrounds is a target, hash or not");

    check(result.find("Remap") == std::string::npos, "boilerplate targets: the whole hash-less fix is removed");
    check(result.find("[CommandListUnrelated]") == std::string::npos,
          "boilerplate targets: so is a section that nothing else would have caught");
    check(result.find("; ---") == std::string::npos,
          "boilerplate targets: nothing inside survives, so the boilerplate goes too");

    check(collectedNames(*fixture.remover, Remover::ResourceType::Blend)
              == std::vector<std::string>{"FooRemapBlend.buf"},
          "boilerplate targets: and its resource is collected");
}


void testResourceReachedOnlyByVb1IsRemoved() {
    // The shape of a real fix -- see
    // Testing/.../expected_fullFix_modFixed/fullFix/RaidenShogun/Mod/ei.ini, whose TextureOverride
    // reaches its Resources through `vb1 =` and has no `run =` at all. IniSectionGraph walks `run =`
    // only, so the removal set closes over RemapIniRemover::buildReferences' wider relation instead.
    std::string txt =
        OrigMod +
        "\n" +
        RemapHeadingOpen +
        "\n"
        "[TextureOverrideFooRemapBlend]\n"
        "hash = " + RaidenBossBlendHash + "\n"
        "vb1 = ResourceFooRemapBlend\n"
        "ib = ResourceFooRemapIB\n"
        "ps-t0 = ResourceFooRemapTex\n"
        "\n"
        "[ResourceFooRemapBlend]\n"
        "filename = FooRemapBlend.buf\n"
        "\n"
        "[ResourceFooRemapIB]\n"
        "filename = FooRemapIB.buf\n"
        "\n"
        "[ResourceFooRemapTex]\n"
        "filename = FooRemapTex.dds\n"
        "\n" +
        RemapHeadingClose;

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    const std::vector<std::string>& removed = fixture.remover->getRemovedSectionNames();
    check(contains(removed, "ResourceFooRemapBlend"), "vb1: a Resource reached only by vb1 is removed");
    check(contains(removed, "ResourceFooRemapIB"), "ib: a Resource reached only by ib is removed");
    check(contains(removed, "ResourceFooRemapTex"), "ps-t0: a Resource reached only by ps-t0 is removed");

    check(result.find("Remap") == std::string::npos, "vb1: nothing of the fix is left in the file");
    check(collectedNames(*fixture.remover, Remover::ResourceType::Blend)
              == std::vector<std::string>{"FooRemapBlend.buf"}, "vb1: its .buf file is collected");
    check(collectedNames(*fixture.remover, Remover::ResourceType::Buf)
              == std::vector<std::string>{"FooRemapIB.buf"}, "ib: its .buf file is collected");
    check(collectedNames(*fixture.remover, Remover::ResourceType::TexEdit)
              == std::vector<std::string>{"FooRemapTex.dds"}, "ps-t0: its .dds file is collected");
}


void testOutsideTheBoilerPlateStillNeedsAHash() {
    // With no marker to go on, a Remap-named leftover outside the boilerplate is a target only if
    // its colouring really carries one of the ModType's hashes -- and it drags what it references
    // along, while an unrelated one is left alone.
    std::string txt =
        OrigMod +
        "\n"
        "[TextureOverrideStaleRemapBlend]\n"
        "hash = " + RaidenBossBlendHash + "\n"
        "vb1 = ResourceStaleRemapBlend\n"
        "\n"
        "[ResourceStaleRemapBlend]\n"
        "filename = StaleRemapBlend.buf\n"
        "\n"
        "[TextureOverrideUnknownRemapBlend]\n"
        "hash = " + UnknownHash + "\n"
        "vb1 = ResourceUnknownRemapBlend\n"
        "\n"
        "[ResourceUnknownRemapBlend]\n"
        "filename = UnknownRemapBlend.buf\n";

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    const std::vector<std::string>& targets = fixture.remover->getTargetSectionNames();
    check(contains(targets, "TextureOverrideStaleRemapBlend"),
          "outside: a leftover whose hash the ModType knows is a target");
    check(!contains(targets, "TextureOverrideUnknownRemapBlend"),
          "outside: a hash the ModType's table does not know makes no target");

    check(result.find("[TextureOverrideStaleRemapBlend]") == std::string::npos,
          "outside: the known-hash leftover is removed");
    check(result.find("[ResourceStaleRemapBlend]") == std::string::npos,
          "outside: and so is the Resource it points at");
    check(result.find("[TextureOverrideUnknownRemapBlend]") != std::string::npos
              && result.find("[ResourceUnknownRemapBlend]") != std::string::npos,
          "outside: the unknown-hash leftover and its Resource are both left alone");
}


void testIgnoreModTypeTakesEveryCandidate() {
    // The same file as the test above, plus a Remap-named leftover carrying no `hash` at all -- the
    // shape the strict rule provably cannot recognize, since there is nothing to attribute. With
    // IniRemovalContext::ignoreModType every candidate becomes a target regardless, which is what
    // IniFile::removeFix asks for on its last mod type.
    std::string txt =
        OrigMod +
        "\n"
        "[TextureOverrideStaleRemapBlend]\n"
        "hash = " + RaidenBossBlendHash + "\n"
        "vb1 = ResourceStaleRemapBlend\n"
        "\n"
        "[ResourceStaleRemapBlend]\n"
        "filename = StaleRemapBlend.buf\n"
        "\n"
        "[TextureOverrideUnknownRemapBlend]\n"
        "hash = " + UnknownHash + "\n"
        "vb1 = ResourceUnknownRemapBlend\n"
        "\n"
        "[ResourceUnknownRemapBlend]\n"
        "filename = UnknownRemapBlend.buf\n"
        "\n"
        "[TextureOverrideHashlessRemapBlend]\n"
        "vb1 = ResourceHashlessRemapBlend\n"
        "\n"
        "[ResourceHashlessRemapBlend]\n"
        "filename = HashlessRemapBlend.buf\n";

    // First the strict rule, so the difference is this flag and nothing else.
    Fixture strict(txt);
    std::string strictResult = strict.remover->remove(false, false);
    check(strictResult.find("[TextureOverrideUnknownRemapBlend]") != std::string::npos
              && strictResult.find("[TextureOverrideHashlessRemapBlend]") != std::string::npos,
          "ignoreModType: without it, an unattributable leftover survives");

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false, AGRemapCore::IniRemovalContext(true));

    const std::vector<std::string>& targets = fixture.remover->getTargetSectionNames();
    check(contains(targets, "TextureOverrideStaleRemapBlend")
              && contains(targets, "TextureOverrideUnknownRemapBlend")
              && contains(targets, "TextureOverrideHashlessRemapBlend"),
          "ignoreModType: every Remap-named leftover is a target, hash or no hash");
    check(contains(targets, "ResourceUnknownRemapBlend") && contains(targets, "ResourceHashlessRemapBlend"),
          "ignoreModType: their Remap-named Resources are targets in their own right too");

    check(result.find("Remap") == std::string::npos,
          "ignoreModType: nothing Remap-named is left in the file");
    checkEqual(result, OrigMod + "\n", "ignoreModType: exactly the original mod is left standing");

    // Still only the candidates -- this widens which candidates are targets, not what a candidate
    // is. The original mod is not Remap-named and sits outside the boilerplate, so it is untouched.
    check(result.find("[TextureOverrideFooBlend]") != std::string::npos,
          "ignoreModType: the original mod is not swept up with the fix");

    // The resources still come out, classified the same way.
    std::vector<std::string> blends = collectedNames(*fixture.remover, Remover::ResourceType::Blend);
    check(blends.size() == 3, "ignoreModType: every removed section's resource is still collected");
}


void testColouringCarriesAHashIntoWhatItRuns() {
    // "some IniColouring state", not "some KVP of this one section". Tested outside the boilerplate,
    // because inside it every section is a target anyway and the walk would prove nothing.
    std::string txt =
        OrigMod +
        "\n"
        "[TextureOverrideStaleRemapBlend]\n"
        "hash = " + RaidenBossBlendHash + "\n"
        "run = CommandListStaleRemapBlend\n"
        "\n"
        "[CommandListStaleRemapBlend]\n"
        "handling = skip\n";

    Fixture fixture(txt);
    fixture.remover->remove(false, false);

    const std::vector<std::string>& targets = fixture.remover->getTargetSectionNames();
    check(contains(targets, "TextureOverrideStaleRemapBlend"), "colouring: the section declaring the hash is a target");
    check(contains(targets, "CommandListStaleRemapBlend"),
          "colouring: a section run by a hashed one inherits that hash's state and is a target too");
}


void testOldBossFixHeadingWithAsymmetricBorder() {
    // Byte-for-byte the heading this software really wrote, asymmetric border included -- 15 '-' on
    // the left, 17 on the right. A whole-line match would miss it.
    std::string txt =
        OrigMod +
        "\n"
        "; --------------- TestMod Boss Fix -----------------\n"
        "\n" +
        FixBody +
        "\n"
        "; -------------------------------------------------\n";

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    check(fixture.remover->getRemovedSectionNames().size() == 3,
          "old heading: the ' .*Boss Fix ' flavour opens a region, asymmetric border and all");
    check(result.find("Boss Fix") == std::string::npos, "old heading: the region's own lines go with it");
    checkEqual(result, "[TextureOverrideFooBlend]\n"
                       "hash = " + RaidenBlendHash + "\n"
                       "run = CommandListFooBlend\n"
                       "\n"
                       "[CommandListFooBlend]\n"
                       "vb1 = ResourceFooBlend\n"
                       "\n"
                       "[ResourceFooBlend]\n"
                       "type = Buffer\n"
                       "filename = FooBlend.buf\n\n",
               "old heading: only the original mod is left");
}


void testUnterminatedBoilerPlateIsNotARegion() {
    // Byte-for-byte the case from test_IniFile.test_differentText_remapBlendSectionsAndScriptFixRemoved:
    // the closing rule carries 37 sideChars where "Raiden Boss Fix" needs 45, so the region is never
    // closed. The pure-Python original needs BOTH halves of its pattern to match, so it leaves the
    // whole block alone -- an opening line on its own is not a fix, and must not swallow the rest of
    // the file.
    std::string txt =
        "; --------------- Raiden Boss Fix ---------------\n"
        "\n"
        "FDFDFDFDF\n"
        "\n"
        "; -------------------------------------\n";

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    check(fixture.remover->getRemovedSectionNames().empty(), "unterminated: nothing is removed");
    checkEqual(result, "; --------------- Raiden Boss Fix ---------------\n\n"
                       "FDFDFDFDF\n\n"
                       "; -------------------------------------\n",
               "unterminated: the block survives exactly as written");
}


void testUnterminatedBoilerPlateSectionsJudgedByName() {
    // ...and the sections inside an unterminated region fall back to the outside-the-boilerplate
    // rule rather than being targets for free. Neither of these carries a ModType hash, so both
    // survive -- but the Remap-named one is at least a candidate, which is what proves the fallback.
    std::string txt =
        OrigMod +
        "\n"
        "; --------------- TestMod Remap ---------------\n"
        "\n"
        "[TextureOverrideFooRemapBlend]\n"
        "vb1 = ResourceFooRemapBlend\n";

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    check(fixture.remover->getTargetSectionNames().empty(),
          "unterminated: an unclosed region makes no targets of the sections under it");
    check(result.find("[TextureOverrideFooRemapBlend]") != std::string::npos,
          "unterminated: those sections survive");
    check(result.find("TestMod Remap") != std::string::npos, "unterminated: so does the heading line");
}


void testNearMissCommentIsNotABoilerPlate() {
    // Both lines are real, from Testing/.../PartiallyFixedRaiden.ini. The first is a hand-written
    // comment that must NOT open a region (6 border characters, not 15, and no matching title); the
    // second is a bare rule that must not be mistaken for a closing line while nothing is open.
    std::string txt =
        "; ------ some lines originally generated from the fix ---------\n"
        "\n"
        "[ResourceFooRemapBlend]\n"
        "type = Buffer\n"
        "filename = FooRemapBlend.buf\n"
        "\n"
        "; --------------------------------------------------------------\n"
        "\n" +
        OrigMod;

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    check(fixture.remover->getTargetSectionNames().empty(),
          "near miss: the Remap-named leftover has no hash, so nothing is a target");
    checkEqual(result, txt, "near miss: the file is left exactly as it was");
}


void testHideOriginalCommentRemoved() {
    // A fix applied with hideOrig comments the ORIGINAL mod out with this prefix. Removing the fix
    // has to take the prefix with it, or the .ini file is left with neither the fix nor the original
    // switched on -- the pure-Python original's own _removeFixComment step.
    const std::string hide = ";RemapFixHideOrig -->";

    std::string txt =
        hide + "[TextureOverrideFooBlend]\n"
        + hide + "vb1 = ResourceFooBlend\n"
        "\n"
        + hide + "[ResourceFooBlend]\n"
        + hide + "filename = FooBlend.buf\n"
        "\n"
        + RemapHeadingOpen +
        "\n"
        "[TextureOverrideFooRemapBlend]\n"
        "vb1 = ResourceFooRemapBlend\n"
        "\n"
        "[ResourceFooRemapBlend]\n"
        "filename = FooRemapBlend.buf\n"
        "\n"
        + RemapHeadingClose;

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, false);

    check(result.find(hide) == std::string::npos, "hideOrig: the hide-original comment prefix is stripped");
    checkEqual(result, "[TextureOverrideFooBlend]\n"
                       "vb1 = ResourceFooBlend\n\n"
                       "[ResourceFooBlend]\n"
                       "filename = FooBlend.buf\n\n",
               "hideOrig: the original mod is left uncommented and the fix is gone");

    // The knob turns it off.
    Fixture kept(txt);
    kept.remover->hideOriginalComment = "";
    std::string keptResult = kept.remover->remove(false, false);
    check(keptResult.find(hide) != std::string::npos,
          "hideOrig: an empty hideOriginalComment leaves the prefixes alone");
}


void testUnboundRemoverIsInert() {
    Remover remover;
    check(remover.getContext() == nullptr, "unbound: a fresh remover has no context");
    check(remover.remove() == "", "unbound: a remover with no .ini file returns an empty string");
    check(remover.getRemovalGraph() == nullptr, "unbound: no graph is built");

    // setIniFile(nullptr) has to undo the binding, not leave a context pointing at a dead file.
    IniFile ini(std::nullopt, "[Foo]\n");
    remover.setIniFile(&ini);
    check(remover.getContext() != nullptr, "unbound: setIniFile builds an IniFileRemoveContext");

    remover.setIniFile(nullptr);
    check(remover.getContext() == nullptr, "unbound: setIniFile(nullptr) drops it again");
}


// An IniRemoveContext with no AGRemapCore::IniFile anywhere behind it -- which is the entire reason
// the seam exists, since the .ini file the pybind11 layer hands a remover is the *Python* IniFile.
// Here it is just a std::string.
class FakeRemoveContext: public IniRemoveContext<> {
    public:
        std::string txt;
        std::unordered_map<std::string, std::unique_ptr<Section>> sections;
        int writeCount = 0;
        int clearReadCount = 0;
        int isFixed = -1;   // -1 = never told

        explicit FakeRemoveContext(std::string txt): txt(std::move(txt)) {}

        bool hasIni() const override { return true; }
        std::string iniFolder() const override { return "."; }
        std::optional<Version> version() const override { return std::nullopt; }
        std::vector<Assets*> modTypeHashes() const override { return {}; }

        std::vector<std::string> readFileLines() override {
            std::vector<std::string> result;
            std::size_t start = 0;

            while (start < txt.size()) {
                std::size_t newline = txt.find('\n', start);
                if (newline == std::string::npos) {
                    result.push_back(txt.substr(start));
                    break;
                }

                result.push_back(txt.substr(start, newline - start + 1));
                start = newline + 1;
            }

            return result;
        }

        std::unordered_map<std::string, Section*> sectionIfTemplates() const override {
            std::unordered_map<std::string, Section*> result;
            for (const auto& entry : sections) {
                result.emplace(entry.first, entry.second.get());
            }
            return result;
        }

        std::string fileTxt() const override { return txt; }
        void setFileTxt(std::string newTxt) override { txt = std::move(newTxt); }
        std::string write() override { writeCount++; return txt; }
        void clearRead() override { clearReadCount++; }
        void setIsFixed(bool newIsFixed) override { isFixed = newIsFixed ? 1 : 0; }
};


void testCallerSuppliedContext() {
    // No AGRemapCore::IniFile at all. Note modTypeHashes() is empty, so nothing outside the
    // boilerplate can ever be a target -- and the boilerplate rule alone still removes the fix.
    FakeRemoveContext ctx(
        "[TextureOverrideFooBlend]\n"
        "vb1 = ResourceFooBlend\n"
        "\n"
        + RemapHeadingOpen +
        "\n"
        "[TextureOverrideFooRemapBlend]\n"
        "vb1 = ResourceFooRemapBlend\n"
        "\n"
        + RemapHeadingClose);

    Remover remover(&ctx);
    check(remover.getContext() == &ctx, "context seam: the remover uses the context it was handed");

    std::string result = remover.remove(false, false);
    check(contains(remover.getTargetSectionNames(), "TextureOverrideFooRemapBlend"),
          "context seam: the boilerplate rule works with no ModType hashes at all");
    check(result.find("Remap") == std::string::npos, "context seam: the fix is removed through the seam");
    check(ctx.txt == result, "context seam: the new text went back through setFileTxt");
    check(ctx.writeCount == 0 && ctx.clearReadCount == 0, "context seam: writeBack = false writes nothing");
    check(ctx.isFixed == 0, "context seam: the .ini file is told it no longer holds a fix");

    remover.remove(false, true);
    check(ctx.writeCount == 1 && ctx.clearReadCount == 1, "context seam: writeBack = true calls write then clearRead");

    // setIniFile must not clobber a context the caller supplied.
    remover.setIniFile(nullptr);
    check(remover.getContext() == &ctx, "context seam: setIniFile leaves a caller-supplied context alone");
}


void testClassifyResource() {
    Remover remover;

    // A download is decided by the section name alone -- the file at the other end can be anything,
    // including nothing recognizable.
    check(remover.classifyResource("ResourceFooRemapDL", "whatever.zip") == Remover::ResourceType::Download,
          "classify: a RemapDL section is a download, whatever its file is");
    check(remover.classifyResource("ResourceFooRemapDL", "Foo.dds") == Remover::ResourceType::Download,
          "classify: RemapDL wins over the .dds branch");
    check(remover.classifyResource("ResourceFooRemapBlend.RemapDL", "Foo.buf") == Remover::ResourceType::Download,
          "classify: RemapDL wins over the .buf branch too");

    check(remover.classifyResource("ResourceFooRemapTex", "Foo.dds") == Remover::ResourceType::TexEdit,
          "classify: a .dds without the texAdd keyword is a texEdit");
    check(remover.classifyResource("ResourceFooRemapTexAdd", "Foo.dds") == Remover::ResourceType::TexAdd,
          "classify: a .dds with the texAdd keyword is a texAdd");
    check(remover.classifyResource("ResourceFooRemapTex", "Foo.DDS") == Remover::ResourceType::TexEdit,
          "classify: the extension check ignores case");

    check(remover.classifyResource("ResourceFooRemapBlend", "Foo.buf") == Remover::ResourceType::Blend,
          "classify: a RemapBlend .buf is a blend");
    check(remover.classifyResource("ResourceFooRemapPosition", "Foo.buf") == Remover::ResourceType::Position,
          "classify: a RemapPosition .buf is a position");
    check(remover.classifyResource("ResourceFooRemapTexcoord", "Foo.buf") == Remover::ResourceType::Texcoord,
          "classify: a RemapTexcoord .buf is a texcoord");
    check(remover.classifyResource("ResourceFooRemapIB", "Foo.buf") == Remover::ResourceType::Buf,
          "classify: any other .buf is a plain buf");

    check(remover.classifyResource("ResourceFooRemapBlend", "Foo.ini") == Remover::ResourceType::Other,
          "classify: neither a .dds nor a .buf is other, whatever the section is called");
    check(remover.classifyResource("ResourceFooRemapBlend", "Foo") == Remover::ResourceType::Other,
          "classify: an extensionless path is other");

    // The documented consequence of taking "RemapTexAdd" literally: nothing this software currently
    // writes carries that substring, so a real texture section lands in texEdit.
    check(remover.classifyResource(IniNamingTools::getRemapTexResourceName("Foo"), "Foo.dds")
              == Remover::ResourceType::TexEdit,
          "classify: a section named by IniNamingTools::getRemapTexResourceName is a texEdit, not a texAdd");

    // ...and the case that would silently break if texcoordKeyword were spelled "RemapTexCoord".
    check(remover.classifyResource(IniNamingTools::getRemapTexcoordName("ResourceFooBlend"), "Foo.buf")
              == Remover::ResourceType::Texcoord,
          "classify: a section named by IniNamingTools::getRemapTexcoordName really is a texcoord");
}


void testCollectedResources() {
    std::string txt =
        OrigMod +
        "\n" +
        RemapHeadingOpen +
        "\n"
        "[TextureOverrideFooRemapBlend]\n"
        "hash = " + RaidenBossBlendHash + "\n"
        "run = CommandListFooRemap\n"
        "\n"
        // Every branch is walked, so both of these are collected -- and the section itself has no
        // filename of its own, so it contributes nothing.
        "[CommandListFooRemap]\n"
        "if $swapvar == 0\n"
        "    run = ResourceFooRemapBlend\n"
        "else\n"
        "    run = ResourceFooRemapBlendAlt\n"
        "endif\n"
        "run = ResourceFooRemapPosition\n"
        "run = ResourceFooRemapTexcoord\n"
        "run = ResourceFooRemapIB\n"
        "run = ResourceFooRemapTex\n"
        "run = ResourceFooRemapDL\n"
        "run = ResourceFooRemapNotes\n"
        "\n"
        "[ResourceFooRemapBlend]\n"
        "if $swapvar == 0\n"
        "    filename = FooRemapBlend.buf\n"
        "else\n"
        "    filename = OtherFooRemapBlend.buf\n"
        "endif\n"
        "\n"
        "[ResourceFooRemapBlendAlt]\n"
        "filename = AltFooRemapBlend.buf\n"
        "\n"
        "[ResourceFooRemapPosition]\n"
        "filename = FooRemapPosition.buf\n"
        "\n"
        "[ResourceFooRemapTexcoord]\n"
        "filename = FooRemapTexcoord.buf\n"
        "\n"
        "[ResourceFooRemapIB]\n"
        "filename = FooRemapIB.buf\n"
        "\n"
        "[ResourceFooRemapTex]\n"
        "filename = FooRemapTex.dds\n"
        "\n"
        "[ResourceFooRemapDL]\n"
        "filename = FooRemapDL.zip\n"
        "\n"
        "[ResourceFooRemapNotes]\n"
        "filename = FooRemapNotes.txt\n"
        "\n" +
        RemapHeadingClose;

    Fixture fixture(txt);
    fixture.remover->remove(false, false);

    check(collectedNames(*fixture.remover, Remover::ResourceType::Blend)
              == std::vector<std::string>{"FooRemapBlend.buf", "OtherFooRemapBlend.buf", "AltFooRemapBlend.buf"},
          "collect: every branch of a section's filename contributes its own resource, in declaration order");
    check(collectedNames(*fixture.remover, Remover::ResourceType::Position)
              == std::vector<std::string>{"FooRemapPosition.buf"}, "collect: the position resource");
    check(collectedNames(*fixture.remover, Remover::ResourceType::Texcoord)
              == std::vector<std::string>{"FooRemapTexcoord.buf"}, "collect: the texcoord resource");
    check(collectedNames(*fixture.remover, Remover::ResourceType::Buf)
              == std::vector<std::string>{"FooRemapIB.buf"}, "collect: the catch-all buf resource");
    check(collectedNames(*fixture.remover, Remover::ResourceType::TexEdit)
              == std::vector<std::string>{"FooRemapTex.dds"}, "collect: the texture resource");
    check(collectedNames(*fixture.remover, Remover::ResourceType::Download)
              == std::vector<std::string>{"FooRemapDL.zip"}, "collect: the download resource");
    check(collectedNames(*fixture.remover, Remover::ResourceType::Other)
              == std::vector<std::string>{"FooRemapNotes.txt"}, "collect: the catch-all other resource");

    check(fixture.remover->getRemovedResources().find(Remover::ResourceType::TexAdd)
              == fixture.remover->getRemovedResources().end(),
          "collect: a kind nothing was found for is absent, not an empty vector");

    // The original mod's own Resource section was never removed, so its file is not collected.
    check(!contains(collectedNames(*fixture.remover, Remover::ResourceType::Blend), "FooBlend.buf"),
          "collect: a surviving section's file is never collected");

    // Every path resolves against the .ini file's folder -- absolute, per IniResource's contract.
    auto blends = fixture.remover->getRemovedResources().find(Remover::ResourceType::Blend);
    bool hasBlend = blends != fixture.remover->getRemovedResources().end() && !blends->second.empty();
    check(hasBlend && blends->second[0]->type == Remover::ResourceType::Blend,
          "collect: the resource keeps its kind on IniResource::type");
    check(hasBlend && std::filesystem::path(blends->second[0]->srcPath).is_absolute(),
          "collect: srcPath is resolved to an absolute path");
}




void testCollectedResourcesResetPerCall() {
    std::string txt = OrigMod + "\n" + RemapHeadingOpen + "\n" + FixBody + "\n" + RemapHeadingClose;

    Fixture fixture(txt);
    fixture.remover->remove(false, false);
    check(collectedNames(*fixture.remover, Remover::ResourceType::Blend)
              == std::vector<std::string>{"FooRemapBlend.buf"}, "reset: the first call collects the fix's resource");

    // Nothing is left to remove the second time round, so the previous call's collection must not
    // still be sitting there.
    fixture.remover->remove(false, false);
    check(fixture.remover->getRemovedResources().empty(), "reset: a later call clears what the previous one collected");
}


void testWriteBackReturnsTheNewText() {
    std::string txt = OrigMod + "\n" + RemapHeadingOpen + "\n" + FixBody + "\n" + RemapHeadingClose;

    Fixture fixture(txt);
    std::string result = fixture.remover->remove(false, true);

    // A file-less .ini file has nowhere to write to, so IniFile::write hands its own text straight
    // back, and clearRead leaves that text alone (it is the only source of data there is).
    check(result.find("Remap") == std::string::npos, "writeBack: the returned text is the stripped one");
    check(fixture.ini->getFileTxt() == result, "writeBack: a file-less .ini file keeps the new text");
}

}


int main() {
    // Unbuffered, so a crash mid-run still shows which check it got to.
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    testWholeFixAndBoilerPlateRemoved();
    testAncestorsAndOutsideLeftoversRemoved();
    testEverythingInTheBoilerPlateIsATarget();
    testResourceReachedOnlyByVb1IsRemoved();
    testOutsideTheBoilerPlateStillNeedsAHash();
    testIgnoreModTypeTakesEveryCandidate();
    testColouringCarriesAHashIntoWhatItRuns();
    testOldBossFixHeadingWithAsymmetricBorder();
    testNearMissCommentIsNotABoilerPlate();
    testUnterminatedBoilerPlateIsNotARegion();
    testUnterminatedBoilerPlateSectionsJudgedByName();
    testClassifyResource();
    testCollectedResources();
    testCollectedResourcesResetPerCall();
    testHideOriginalCommentRemoved();
    testUnboundRemoverIsInert();
    testCallerSuppliedContext();
    testWriteBackReturnsTheNewText();

    if (failures == 0) {
        std::printf("\nAll RemapIniRemover tests passed.\n");
        return 0;
    }

    std::printf("\n%d RemapIniRemover test(s) FAILED.\n", failures);
    return 1;
}
