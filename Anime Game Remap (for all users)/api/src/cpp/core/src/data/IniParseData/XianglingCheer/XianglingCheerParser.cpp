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

        // HER DIFFUSE AND LIGHTMAP SIT ONE SLOT HIGHER, because ps-t0 is her normal map: ps-t0
        // normal map (2725cfa6 head / 25260201 body), ps-t1 diffuse, ps-t2 lightmap. That is what
        // the pure-Python download table says at 5.3, what a mod dumped from the game binds
        // (XianglingCheer3 names those very hashes on those slots), and what both of her fix rows
        // assume when they drop ps-t0 and shift ps-t1 / ps-t2 down. Left on the default modern
        // slots, a downloaded diffuse was deleted as the "normal map" and her lightmap landed in
        // Xiangling's diffuse slot. No normal map is fetched: every fix of hers drops it.
        config.objDownloadRegs = {{"head", "ps-t1", "ps-t2"}, {"body", "ps-t1", "ps-t2"}};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory XianglingCheerParser::v5_3() {
        return IniParseBuilderFuncs::xianglingCheer5_3();
    }
}
