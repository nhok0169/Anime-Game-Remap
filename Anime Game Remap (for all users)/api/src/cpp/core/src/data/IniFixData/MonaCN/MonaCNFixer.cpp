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

#include "AGRemapCore/data/IniFixData/MonaCN/MonaCNFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::monaCN6_1() {
        // Remapped onto Mona, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what MonaCN does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // moveDrawIndexed stays false: the pure-Python row for MonaCN carries none of the Ib*
        // entries Amber's does, and a mod the old script has already fixed keeps
        // 'drawindexed = auto' on the remapped IB section untouched.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory MonaCNFixer::v6_1() {
        return IniFixBuilderFuncs::monaCN6_1();
    }
}
