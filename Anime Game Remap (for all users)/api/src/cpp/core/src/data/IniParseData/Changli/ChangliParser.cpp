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

#include "AGRemapCore/data/IniParseData/Changli/ChangliParser.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/data/IniFixData/Changli/ChangliTextures.h"
#include "AGRemapCore/data/IniFixData/Changli/ChangliSkeleton.h"


namespace AGRemapCore {
    IniParseBuilder::Factory IniParseBuilderFuncs::changli1_1() {
        // The WWMI shape -- see makeWWMIParser. Her eight draw slots come off the library's Indices
        // rows, her hashes off HashData; 1.1 is her oldest mesh, so a mod of any of her three mesh
        // generations classifies.
        WWMIParserConfig config{};
        config.version = "1.1";
        // Her own textures, so a mod of hers can be sorted into roles at PARSE time
        config.textures = changliTextureFacts();
        // the numberings her skeleton has had, and her own bones' centroids -- see WWMIParserConfig::skeletonNumberings
        config.skeletonNumberings = {{"1.1", changliRenumbered11To14()}, {"1.2", changliRenumbered12To14()}, {"1.4", {}}};
        config.referenceBoneCentroids = changliBoneCentroids();
        // her blindfold, which several mods hide: its sections go to a graph of their own for the fix to copy
        config.hashOnlyObjs.emplace_back("blindfold", "blindfold");
        return makeWWMIParser(std::move(config));
    }


    IniParseBuilder::Factory ChangliParser::v1_1() {
        return IniParseBuilderFuncs::changli1_1();
    }
}
