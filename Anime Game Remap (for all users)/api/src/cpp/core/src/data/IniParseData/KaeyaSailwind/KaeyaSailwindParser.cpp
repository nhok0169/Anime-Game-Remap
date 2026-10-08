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

#include "AGRemapCore/data/IniParseData/KaeyaSailwind/KaeyaSailwindParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::kaeyaSailwind4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what KaeyaSailwind does differently lives here.

        // Same three objects as Kaeya. The asymmetry is entirely on the fix side: his model has a
        // fourth index to fill and hers does not.

        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::KaeyaSailwind;
        config.downloadCharFolder = "KaeyaSailwind";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "KaeyaSailwind";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        // HER BODY'S DIFFUSE AND LIGHTMAP SIT ONE SLOT HIGHER, because ps-t0 is its normal map
        // (1077694d); her head and dress are on the modern ps-t0 / ps-t1. HashData says so, all
        // three of her mods bind a normal map there and these very download files on ps-t1 /
        // ps-t2, and every fix row of hers shifts the body's ps-t1 / ps-t2 down. The pure-Python
        // table put the body on ps-t0 / ps-t1 after GI-Model-Importer-Assets' 4.x labels, which
        // are one slot off (Diffuse / LightMap / Shadow for normal map / diffuse / lightmap). Left
        // there, the shift put the lightmap on ps-t0 beside the downloaded diffuse -- two ps-t0
        // lines -- and nothing on ps-t1.
        config.objDownloadRegs = {{"body", "ps-t1", "ps-t2"}};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KaeyaSailwindParser::v4_0() {
        return IniParseBuilderFuncs::kaeyaSailwind4_0();
    }
}
