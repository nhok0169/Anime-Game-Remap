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

#include "AGRemapCore/data/IniParseData/Klee/KleeParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::klee4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Klee does differently lives here.

        // NO DRESS: Klee draws head and body only, which is what makes her half of the
        // pair a SPLIT -- see KleeFixer for where her body becomes KleeBlossomingStarlight's
        // body AND dress. Her texcoord stride is 12 rather than the 20 the rest of this
        // batch uses.

        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Klee;
        config.downloadCharFolder = "Klee";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Klee";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KleeParser::v4_0() {
        return IniParseBuilderFuncs::klee4_0();
    }
}
