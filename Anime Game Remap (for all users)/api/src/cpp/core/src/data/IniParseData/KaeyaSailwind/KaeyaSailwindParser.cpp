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

#include "AGRemapCore/data/IniParseData/KaeyaSailwind/KaeyaSailwindParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::kaeyaSailwind4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what KaeyaSailwind does differently lives here.

        // Same three objects as Kaeya. The asymmetry is entirely on the fix side: his model has a
        // fourth index to fill and hers does not.

        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::KaeyaSailwind;
        config.downloadCharFolder = "KaeyaSailwind";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "KaeyaSailwind";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KaeyaSailwindParser::v4_0() {
        return IniParseBuilderFuncs::kaeyaSailwind4_0();
    }
}
