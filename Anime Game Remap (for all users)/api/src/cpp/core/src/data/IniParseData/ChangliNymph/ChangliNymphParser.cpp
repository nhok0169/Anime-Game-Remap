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

#include "AGRemapCore/data/IniParseData/ChangliNymph/ChangliNymphParser.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/WWMIParser.h"
#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphTextures.h"


namespace AGRemapCore {
    IniParseBuilder::Factory IniParseBuilderFuncs::changliNymph2_4() {
        // The WWMI shape -- see makeWWMIParser. Her eight draw slots come off the library's Indices
        // rows, her hashes off HashData; 2.4 is the skin's version, unchanged since.
        WWMIParserConfig config{};
        config.version = "2.4";
        // Her own textures, so a mod of hers can be sorted into roles at PARSE time
        config.textures = changliNymphTextureFacts();
        return makeWWMIParser(std::move(config));
    }


    IniParseBuilder::Factory ChangliNymphParser::v2_4() {
        return IniParseBuilderFuncs::changliNymph2_4();
    }
}
