#include "AGRemapCore/data/IniParseData/LynaePeppermint/LynaePeppermintParser.h"

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

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintTextures.h"


namespace AGRemapCore {
    IniParseBuilder::Factory IniParseBuilderFuncs::lynaePeppermint3_7() {
        // The WWMI shape -- see makeWWMIParser. Her eight draw slots come off the library's Indices
        // rows, her hashes off HashData; 3.7 is the only version the skin has.
        WWMIParserConfig config{};
        config.version = "3.7";
        // Her own textures, so a mod of hers is sorted into roles at PARSE time
        config.textures = lynaePeppermintTextureFacts();
        return makeWWMIParser(std::move(config));
    }


    IniParseBuilder::Factory LynaePeppermintParser::v3_7() {
        return IniParseBuilderFuncs::lynaePeppermint3_7();
    }
}
