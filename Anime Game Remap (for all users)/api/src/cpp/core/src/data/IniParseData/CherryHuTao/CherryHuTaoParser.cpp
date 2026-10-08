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

        // HER HEAD AND DRESS SIT ONE SLOT HIGHER, because ps-t0 is their normal map (efe5e3ed /
        // e66b5b37); her body and glasses are on the modern ps-t0 / ps-t1. That is what the
        // pure-Python download table says at 5.3, what every mod of hers binds, and what both fix
        // rows assume when they drop the head's and dress's ps-t0 and shift ps-t1 / ps-t2 down.
        // Left on the default slots, a downloaded head or dress diffuse was deleted as the "normal
        // map" and its lightmap shifted into HuTao's diffuse slot.
        config.objDownloadRegs = {{"head", "ps-t1", "ps-t2"}, {"dress", "ps-t1", "ps-t2"}};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory CherryHuTaoParser::v5_3() {
        return IniParseBuilderFuncs::cherryHutao5_3();
    }
}
