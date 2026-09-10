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

#include "AGRemapCore/data/IniParseData/ShenheFrostFlower/ShenheFrostFlowerParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::shenheFrostFlower4_4() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what ShenheFrostFlower does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::ShenheFrostFlower;
        config.downloadCharFolder = "ShenheFrostFlower";
        config.downloadVersionFolder = "4_4";
        config.downloadPrefix = "ShenheFrostFlower";
        config.drawnObjs = {"head", "body", "dress", "extra"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory ShenheFrostFlowerParser::v4_4() {
        return IniParseBuilderFuncs::shenheFrostFlower4_4();
    }
}
