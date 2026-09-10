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

#include "AGRemapCore/data/IniFixData/AmberCN/AmberCNFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::amberCN6_1() {
        // Remapped onto Amber, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what AmberCN does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // AmberCN's fix DOES move the shared draw call onto her drawn objects. Mona's and Rosaria's do
        // not, despite their .ini files having the same shape -- see GIMICharFixerConfig.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory AmberCNFixer::v6_1() {
        return IniFixBuilderFuncs::amberCN6_1();
    }
}
