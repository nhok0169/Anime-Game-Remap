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

#include "AGRemapCore/data/IniParseData/Lisa/LisaParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::lisa4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Lisa does differently lives here.

        // Lisa has the dress and LisaStudent does not -- the mirror of the Klee pair. Remapping
        // off Lisa MERGES her dress into LisaStudent's body; remapping onto her SPLITS
        // LisaStudent's body in two.

        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Lisa;
        config.downloadCharFolder = "Lisa";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Lisa";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory LisaParser::v4_0() {
        return IniParseBuilderFuncs::lisa4_0();
    }
}
