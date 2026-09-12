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

#include "AGRemapCore/data/IniFixData/Barbara/BarbaraFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::barbara6_1ToBarbaraSummertime() {
        // Remapped onto BarbaraSummertime, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what Barbara does differently lives here.
        //
        // THE SIMPLE SHAPE OF THIS BATCH: both draw head, body and dress, so every object maps
        // to its own counterpart and there is no objSplits at all. Compare the Klee and Lisa
        // pairs, where one side is missing an object entirely.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // SHE DOES CARRY THE Ib* ENTRIES, unlike MonaCN and unlike what an earlier draft of this
        // file claimed: barbara6_1 has IbRemapData, IbDrawIndexedRename and IbTempToDrawIndexed on
        // all three objects, which is exactly the combination this flag stands for (compare
        // amber6_1, which has them, against monaCN6_1, which does not).
        //
        // Worth knowing that the A/B did NOT catch this: its section check compares the SET of
        // remapped section NAMES, and where `drawindexed` sits is a line inside a section.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory BarbaraFixer::v6_1ToBarbaraSummertime() {
        return IniFixBuilderFuncs::barbara6_1ToBarbaraSummertime();
    }
}
