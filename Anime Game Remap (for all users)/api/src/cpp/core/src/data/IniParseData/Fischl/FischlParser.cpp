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

#include "AGRemapCore/data/IniParseData/Fischl/FischlParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::fischl4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Fischl does differently lives here.

        // Fischl is the one WITH the dress here and FischlHighness is the one without -- the opposite
        // way round from the Diluc pair, and the reason her 6.1 row is a merge where his is a split.

        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Fischl;
        config.downloadCharFolder = "Fischl";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Fischl";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory FischlParser::v4_0() {
        return IniParseBuilderFuncs::fischl4_0();
    }
}
