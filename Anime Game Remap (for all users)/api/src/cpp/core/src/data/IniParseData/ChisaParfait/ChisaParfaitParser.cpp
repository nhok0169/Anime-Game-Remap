#include "AGRemapCore/data/IniParseData/ChisaParfait/ChisaParfaitParser.h"

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
#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitTextures.h"


namespace AGRemapCore {
    IniParseBuilder::Factory IniParseBuilderFuncs::chisaParfait3_5() {
        // The WWMI shape -- see makeWWMIParser. Nothing is ChisaParfait-specific beyond her id: her
        // EIGHT draw slots come off the library's Indices rows, her hashes off HashData.
        //
        // The version is the SKIN's, 3.5, which is the one her assets are filed at and the one
        // downloadVersionFolder has to agree with.
        WWMIParserConfig config{};
        config.version = "3.5";
        // Her own textures, so a mod of hers is sorted into roles at PARSE time.
        config.textures = chisaParfaitTextureFacts();
        return makeWWMIParser(std::move(config));
    }


    IniParseBuilder::Factory ChisaParfaitParser::v3_5() {
        return IniParseBuilderFuncs::chisaParfait3_5();
    }
}
