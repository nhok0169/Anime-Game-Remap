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
        // Both rules ask what is actually BOUND before moving it, so a mod already hand-fixed
        // for 6.1 -- one whose author did this shift themselves -- is left alone rather than
        // shifted twice. See RegValChecks for why that guard exists.
        config.objRegRemovals = {{"head", {"ps-t0", "ps-t3"}},
                                 {"body", {"ps-t0", "ps-t3"}}};

        config.objRegRemaps = {{"head", {{"ps-t1", {{"ps-t0", &RegValChecks::isDiffuse}}, true},
                                         {"ps-t2", {{"ps-t1", &RegValChecks::isLightMap}}, true}}},
                               {"body", {{"ps-t1", {{"ps-t0", &RegValChecks::isDiffuse}}, true},
                                         {"ps-t2", {{"ps-t1", &RegValChecks::isLightMap}}, true}}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory LisaStudentFixer::v6_1ToLisa() {
        return IniFixBuilderFuncs::lisaStudent6_1ToLisa();
    }
}
