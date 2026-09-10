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

#include "AGRemapCore/data/IniFixData/NingguangOrchid/NingguangOrchidFixer.h"

#include <utility>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::ningguangOrchid6_1() {
        // Remapped onto Ningguang -- the plainest shape this template has. See makeGIMICharFixer.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // NOTHING ELSE, and the asymmetry with NingguangFixer is real rather than an omission:
        //
        //  * no objRegRemovals -- the ps-t3 strip goes the other way. Ningguang binds a ps-t3 that
        //    Orchid does not read; Orchid binds none, so there is nothing to take away
        //  * no texEdits -- the DarkDiffuse edit is declared against Ningguang's head diffuse. The
        //    pure-Python ningguangOrchid6_1 row has no RegTexEdit either
        //
        // moveDrawIndexed stays false, as for Ningguang.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory NingguangOrchidFixer::v6_1() {
        return IniFixBuilderFuncs::ningguangOrchid6_1();
    }
}
