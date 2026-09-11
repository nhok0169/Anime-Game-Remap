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

#include "AGRemapCore/data/IniParseData/Ayaka/AyakaParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::ayaka4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Ayaka does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Ayaka;
        config.downloadCharFolder = "Ayaka";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Ayaka";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        // No objDownloadRegs: alone in this batch, Ayaka's own downloads are already on the modern
        // ps-t0/ps-t1. It is her SKIN that sits a slot higher -- see AyakaSpringbloomParser -- and
        // the shift between the two is what her fixer spends most of its length on.

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory AyakaParser::v4_0() {
        return IniParseBuilderFuncs::ayaka4_0();
    }
}
