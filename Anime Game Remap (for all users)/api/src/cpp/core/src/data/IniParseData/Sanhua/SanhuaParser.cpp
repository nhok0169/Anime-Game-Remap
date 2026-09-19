#include "AGRemapCore/data/IniParseData/Sanhua/SanhuaParser.h"

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


namespace AGRemapCore {
    IniParseBuilder::Factory IniParseBuilderFuncs::sanhua2_5() {
        // The WWMI shape -- see makeWWMIParser for what that means and for every mod object this
        // produces. Nothing is Sanhua-specific beyond her id: her seven draw slots come off the
        // library's Indices rows, her hashes off HashData.
        WWMIParserConfig config{};
        config.modTypeId = ModTypeId::Sanhua;
        config.version = "2.5";
        return makeWWMIParser(std::move(config));
    }


    IniParseBuilder::Factory SanhuaParser::v2_5() {
        return IniParseBuilderFuncs::sanhua2_5();
    }
}
