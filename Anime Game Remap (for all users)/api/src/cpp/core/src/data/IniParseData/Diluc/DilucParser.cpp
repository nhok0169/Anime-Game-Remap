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

#include "AGRemapCore/data/IniParseData/Diluc/DilucParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::diluc4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Diluc does differently lives here.

        // Diluc draws head and body; DilucFlamme adds the dress his coat becomes. Remapping off
        // Diluc SPLITS his body in two; remapping onto him MERGES DilucFlamme's dress back in.

        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Diluc;
        config.downloadCharFolder = "Diluc";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Diluc";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory DilucParser::v4_0() {
        return IniParseBuilderFuncs::diluc4_0();
    }
}
