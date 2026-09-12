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

#include "AGRemapCore/data/IniFixData/Lisa/LisaFixer.h"

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::lisa6_1ToLisaStudent() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE. LisaStudent draws head and body only, so Lisa's dress has nowhere of its
        // own to go and is drawn through LisaStudent's BODY. Same shape as
        // KleeBlossomingStarlight -> Klee, with the characters the other way round.
        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}};

        // The generated second file explains itself -- see IniComments::GIMIObjMergerPreamble,
        // which is the paragraph the pure-Python merge wrote for exactly this.
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // Three registers LisaStudent has no use for, one per object and each a DIFFERENT slot --
        // lisa6_1's RegRemove(remove = {"head": {"ps-t2"}, "body": {"ps-t3"}, "dress": {"ps-t2"}}).
        // The asymmetry is real rather than a transcription slip; her body is the odd one out.
        config.objRegRemovals = {{"head", {"ps-t2"}},
                                 {"body", {"ps-t3"}},
                                 {"dress", {"ps-t2"}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory LisaFixer::v6_1ToLisaStudent() {
        return IniFixBuilderFuncs::lisa6_1ToLisaStudent();
    }
}
