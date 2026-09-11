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

#include "AGRemapCore/data/IniParseData/NilouBreeze/NilouBreezeParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::nilouBreeze4_8() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what NilouBreeze does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::NilouBreeze;
        config.downloadCharFolder = "NilouBreeze";
        config.downloadVersionFolder = "4_8";
        config.downloadPrefix = "NilouBreeze";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        // No objDownloadRegs: the skin shipped after GI moved the diffuse and lightmap down, so
        // ps-t0/ps-t1 -- makeGIMICharParser's default -- is already right. Her BASE, Nilou, is the
        // one that needs the 4.0-era layout spelled out.

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory NilouBreezeParser::v4_8() {
        return IniParseBuilderFuncs::nilouBreeze4_8();
    }
}
