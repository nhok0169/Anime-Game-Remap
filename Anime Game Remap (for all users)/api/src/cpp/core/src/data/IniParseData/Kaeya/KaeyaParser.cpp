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

#include "AGRemapCore/data/IniParseData/Kaeya/KaeyaParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::kaeya4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Kaeya does differently lives here.

        // THREE drawn objects, and FOUR indices. IndexData gives Kaeya an 'extra' at 47727 that no
        // Kaeya .ini file ever declares -- it exists only as the second half of KaeyaSailwind's dress
        // split, which is why it is not listed here. See KaeyaSailwindFixer.

        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Kaeya;
        config.downloadCharFolder = "Kaeya";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Kaeya";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KaeyaParser::v4_0() {
        return IniParseBuilderFuncs::kaeya4_0();
    }
}
