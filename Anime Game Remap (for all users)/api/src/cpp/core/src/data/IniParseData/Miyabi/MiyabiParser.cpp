#include "AGRemapCore/data/IniParseData/Miyabi/MiyabiParser.h"

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

#include "AGRemapCore/data/IniFixData/Miyabi/MiyabiFixer.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/ZZMIParser.h"


namespace AGRemapCore {
    IniParseBuilder::Factory IniParseBuilderFuncs::miyabi3_2() {
        return makeZZMIParser(miyabiBlossomConfig());
    }


    IniParseBuilder::Factory MiyabiParser::v3_2() {
        return IniParseBuilderFuncs::miyabi3_2();
    }
}
