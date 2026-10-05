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

#include "AGRemapCore/data/IniParseData/BarbaraSummertime/BarbaraSummertimeParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::barbaraSummertime4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what BarbaraSummertime does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::BarbaraSummertime;
        config.downloadCharFolder = "BarbaraSummertime";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "BarbaraSummertime";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory BarbaraSummertimeParser::v4_0() {
        return IniParseBuilderFuncs::barbaraSummertime4_0();
    }
}
