// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "AGRemapCore/data/IniFixData/LisaStudent/LisaStudentFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/data/IniFixData/RegValChecks.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::lisaStudent6_1ToLisa() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT. Lisa's dress is an object LisaStudent has no geometry for, so LisaStudent's
        // body graph is emitted twice -- once carrying Lisa's body index (16815) and once her
        // dress index (45873), both under Lisa's own ib hash.
        //
        // 'head' is listed even though it maps to itself: objSplits is all-or-nothing, and an
        // object left out of it is dropped from the remap entirely.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // NO objNewRegVals: lisaStudent6_1 writes no "null" over the dress copy's ib, unlike
        // Jean's split. Ported rather than copied.

        // ---- SHE IS A 4.0-ERA MODEL AND LISA IS NOT ----
        //
        // Her diffuse sits on ps-t1 and her lightmap on ps-t2, one slot up, because ps-t0 is her
        // normal map (see LisaStudentParser, and HashData's note on that row). Lisa reads the
        // modern layout, so the whole thing shifts DOWN on the way across:
        //
        //     ps-t0  normal map  -> dropped, Lisa has none
        //     ps-t1  diffuse     -> ps-t0
        //     ps-t2  lightmap    -> ps-t1
        //     ps-t3              -> dropped
        //
        // EVERY rule here asks what is actually BOUND before touching it, so a mod already in
        // the modern layout -- one whose author did this shift themselves, or built the skin on
        // a base-character mod -- is left alone rather than shifted twice.
        //
        // The ps-t0 REMOVAL needs the guard just as much as the remaps do, and that took a real
        // mod to notice. LisaStudent2 binds ps-t0 to its diffuse, not to a normal map; an
        // unconditional removal there deleted the diffuse outright and the two guarded remaps
        // then correctly declined to fire, so the head came out with a lightmap, a shadow ramp
        // and NOTHING on ps-t0. It only became visible once that mod started classifying as
        // LisaStudent at all -- before the classifier learned its hashes it was fixed as Lisa,
        // and this row never ran on it.
        //
        // ps-t3 stays unconditional: it is dropped because LISA has no use for the slot,
        // whatever the source put there, which is a statement about the target and not about
        // the value.
        config.objRegRemovals = {{"head", {{"ps-t0", &RegValChecks::isNormalMap}, "ps-t3"}},
                                 {"body", {{"ps-t0", &RegValChecks::isNormalMap}, "ps-t3"}}};

        config.objRegRemaps = {{"head", {{"ps-t1", {{"ps-t0", &RegValChecks::isDiffuse}}, true},
                                         {"ps-t2", {{"ps-t1", &RegValChecks::isLightMap}}, true}}},
                               {"body", {{"ps-t1", {{"ps-t0", &RegValChecks::isDiffuse}}, true},
                                         {"ps-t2", {{"ps-t1", &RegValChecks::isLightMap}}, true}}}};

        // NO FIX CALL AT ALL on either object -- an empty list here replaces the default
        // NNFixPath rather than adding to it, and the mod's own NNFix/ORFix is stripped before
        // this runs, so the remapped head and body come out with no `run =` line.
        //
        // The mirror of lisa6_1ToLisaStudent re-issuing ORFix: that direction GAINS a normal map
        // and needs the library that reads one; this one drops it, and Lisa needs neither. The
        // pure-Python lisaStudent6_1 re-issues NNFix here, so this is a deliberate divergence
        // from it and not a transcription.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory LisaStudentFixer::v6_1ToLisa() {
        return IniFixBuilderFuncs::lisaStudent6_1ToLisa();
    }
}
