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

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


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
        // UNCONDITIONAL, and that is the point. These rules were briefly written with
        // RegValChecks on them -- move ps-t1 only if it looks like a diffuse, drop ps-t0 only if
        // it looks like a normal map -- so that a mod already in the modern layout would not be
        // shifted twice. That guards on the wrong thing: RegValChecks reads the RESOURCE NAME,
        // which the modder picks and which a mod ported forward keeps long after the content
        // behind it has moved. LisaStudent1 binds ps-t0 to a section called
        // 'ResourceLisaStudentHeadDiffuse' that holds her NORMAL MAP. The register position is
        // the contract; the name is a label.
        //
        // ALL THREE TARGETS, including the dress. The dress is the second copy of the body
        // graph that the split makes, so it arrives with the same registers bound the same way
        // -- and objRegRemovals/objRegRemaps are keyed by TARGET, so leaving it out shifted the
        // body copy and left the dress copy exactly as it came in. Half the model in one layout
        // and half in the other, which no section-name diff can see.
        config.objRegRemovals = {{"head", {"ps-t0", "ps-t3"}},
                                 {"body", {"ps-t0", "ps-t3"}},
                                 {"dress", {"ps-t0", "ps-t3"}}};

        config.objRegRemaps = {{"head", {{"ps-t1", {"ps-t0"}}, {"ps-t2", {"ps-t1"}}}},
                               {"body", {{"ps-t1", {"ps-t0"}}, {"ps-t2", {"ps-t1"}}}},
                               {"dress", {{"ps-t1", {"ps-t0"}}, {"ps-t2", {"ps-t1"}}}}};

        // NNFix on all three, which is the default and so needs no objFixCalls entry at all.
        // It is what fixes up Lisa's REFLECTION, and dropping it -- which this row did briefly --
        // costs that. Only the other direction differs: lisa6_1ToLisaStudent re-issues ORFix,
        // because that direction gains a normal map and ORFix is the library that reads one.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory LisaStudentFixer::v6_1ToLisa() {
        return IniFixBuilderFuncs::lisaStudent6_1ToLisa();
    }
}
