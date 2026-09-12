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

#include "AGRemapCore/data/IniFixData/BarbaraSummertime/BarbaraSummertimeFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::barbaraSummertime6_1ToBarbara() {
        // Remapped onto Barbara, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what BarbaraSummertime does differently lives here.
        //
        // The mirror of BarbaraFixer, and equally plain: head, body and dress on both sides.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // Same Ib* entries as Barbara's row -- see BarbaraFixer for why this is true and for
        // why the A/B cannot see it.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory BarbaraSummertimeFixer::v6_1ToBarbara() {
        return IniFixBuilderFuncs::barbaraSummertime6_1ToBarbara();
    }
}
