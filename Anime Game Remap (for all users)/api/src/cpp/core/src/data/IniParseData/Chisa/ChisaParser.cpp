#include "AGRemapCore/data/IniParseData/Chisa/ChisaParser.h"

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
#include "AGRemapCore/data/IniFixData/Chisa/ChisaTextures.h"


namespace AGRemapCore {
    IniParseBuilder::Factory IniParseBuilderFuncs::chisa2_8() {
        // The WWMI shape -- see makeWWMIParser for what that means and for every mod object this
        // produces. Nothing is Chisa-specific beyond her id: her seven draw slots come off the
        // library's Indices rows, her hashes off HashData.
        WWMIParserConfig config{};
        config.version = "2.8";
        // Her own textures, so a mod of hers can be sorted into roles at PARSE time -- which is
        // where deciding what a section is belongs.
        config.textures = chisaTextureFacts();
        return makeWWMIParser(std::move(config));
    }


    IniParseBuilder::Factory ChisaParser::v2_8() {
        return IniParseBuilderFuncs::chisa2_8();
    }
}
