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

#include "AGRemapCore/data/IniParseData/XianglingCheer/XianglingCheerParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::xianglingCheer5_3() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what XianglingCheer does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::XianglingCheer;
        config.downloadCharFolder = "XianglingCheer";
        config.downloadVersionFolder = "5_3";
        config.downloadPrefix = "XianglingCheer";
        config.drawnObjs = {"head", "body"};

        // 12, where Xiangling herself is 20 -- the pair does not share a stride.
        config.texcoordStride = 12;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory XianglingCheerParser::v5_3() {
        return IniParseBuilderFuncs::xianglingCheer5_3();
    }
}
