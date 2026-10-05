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

#include "AGRemapCore/data/IniParseData/DilucFlamme/DilucFlammeParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::dilucFlamme4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what DilucFlamme does differently lives here.

        // The dress is the object Diluc does not have -- the mirror of DilucParser. Her stride is 20
        // where his is 12, which is the one number in this file that is not guessable from the other.

        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::DilucFlamme;
        config.downloadCharFolder = "DilucFlamme";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "DilucFlamme";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory DilucFlammeParser::v4_0() {
        return IniParseBuilderFuncs::dilucFlamme4_0();
    }
}
