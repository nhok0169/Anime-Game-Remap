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

#include "AGRemapCore/data/IniParseData/Yelan/YelanParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::yelan4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Yelan does differently lives here.
        //
        // Four drawn objects, in draw order: her identity mod (the game's own model, built from
        // GI-Model-Importer-Assets' PlayerCharacterData/Yelan by Tools/Misc/Prototypes/identityMod.py)
        // draws all four, a downloaded mod any subset -- one hid the dress with `ib = null`, one
        // hung a cape on the extra.
        //
        // The downloads are that identity: Data/Mod Downloads/GI/Yelan/4_0 holds the game's own
        // buffers and textures, so a mod that names no lightmap for an object still draws with one.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Yelan;
        config.downloadCharFolder = "Yelan";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Yelan";
        config.drawnObjs = {"head", "body", "dress", "extra"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory YelanParser::v4_0() {
        return IniParseBuilderFuncs::yelan4_0();
    }
}
