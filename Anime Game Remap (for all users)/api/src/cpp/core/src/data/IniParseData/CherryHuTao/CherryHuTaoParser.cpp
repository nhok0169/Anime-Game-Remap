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

#include "AGRemapCore/data/IniParseData/CherryHuTao/CherryHuTaoParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::cherryHutao5_3() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what CherryHuTao does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::CherryHuTao;
        config.downloadCharFolder = "CherryHuTao";
        config.downloadVersionFolder = "5_3";
        config.downloadPrefix = "CherryHuTao";
        config.drawnObjs = {"head", "body", "dress", "extra"};

        // Her glasses have a diffuse and no lightmap -- the only object in the whole download tree
        // that does. Declared anyway, the fix names a [Resource...ExtraLightMapRemapDL] file that
        // nothing will ever fetch.
        config.objsWithoutLightMap = {"extra"};

        // 28 -- the largest in the table, and nothing else uses it. Do not copy a 20 here.
        config.texcoordStride = 28;

        // HER HEAD AND DRESS ARE THE NORMAL-MAP LAYOUT (2026-10-07): normal map at ps-t0, diffuse at
        // ps-t1, light map at ps-t2 -- what every CherryHuTao mod binds, and what both
        // cherryHuTao5_3 and cherryHuTao6_1 assume when they drop ps-t0 and shift the rest down.
        // Registered at the default ps-t0 / ps-t1 instead, a downloaded head or dress diffuse was the
        // texture the fix then deleted and its light map took the diffuse's slot: a recolour of
        // CherryHutao6 drew Hu Tao's hair and skirt glowing green in game, and moving the head's two
        // textures by hand fixed it. Her body and extra are the plain layout and keep the default.
        // No normal map is registered: the asset folder has none, and HuTao reads none.
        config.objDownloadRegs = {{"head", "ps-t1", "ps-t2"}, {"dress", "ps-t1", "ps-t2"}};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory CherryHuTaoParser::v5_3() {
        return IniParseBuilderFuncs::cherryHutao5_3();
    }
}
