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

#include "AGRemapCore/data/IniFixData/NilouBreeze/NilouBreezeFixer.h"

#include <utility>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::nilouBreeze6_1() {
        // Remapped onto Nilou, a genuinely different model -- see makeGIMICharFixer for what that
        // shape does. Only what NilouBreeze does differently lives here.
        //
        // The plainest fix in this batch, and the counterpart to her base's: Nilou -> NilouBreeze
        // re-issues ORFix and therefore needs no 6.1 row, while this direction re-issues NNFix and
        // is registered at 6.1 precisely because NNFix is what GI 6.1 introduced.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // Nilou does not read ps-t3 on any of the three. NilouBreeze binds one, and a register the
        // target's shader never samples is at best ignored and at worst read as something else.
        config.objRegRemovals = {{"head", {"ps-t3"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t3"}}};

        // No objFixCalls: every object re-issues NNFix, which is makeGIMICharFixer's default. The
        // pure-Python row says the same thing the long way round, threading a 'tempNNFix' register
        // through a remap and renaming it to 'run' -- the template owns that now, including
        // stripping the mod's own ORFix/NNFix calls first (the row's ORFixCompleteRemoval).

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory NilouBreezeFixer::v6_1() {
        return IniFixBuilderFuncs::nilouBreeze6_1();
    }
}
