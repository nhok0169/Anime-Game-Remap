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

#include "AGRemapCore/data/IniParseData/Xiangling/XianglingParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::xiangling4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Xiangling does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Xiangling;
        config.downloadCharFolder = "Xiangling";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Xiangling";
        config.drawnObjs = {"head", "body", "dress"};

        // Per character, and worth checking rather than copying -- her own skin is 12.
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory XianglingParser::v4_0() {
        return IniParseBuilderFuncs::xiangling4_0();
    }
}
