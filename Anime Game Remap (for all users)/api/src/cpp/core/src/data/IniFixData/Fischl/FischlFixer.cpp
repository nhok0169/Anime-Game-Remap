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

#include "AGRemapCore/data/IniFixData/Fischl/FischlFixer.h"

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::fischl6_1ToFischlHighness() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE, with the head duplicated into both .ini files exactly as DilucFlamme's is --
        // fischl6_1's {"head": ["head", "head"]}. Her dress comes through FischlHighness's body.
        config.objSplits = {{"head", {"head", "head"}}, {"body", {"body"}}, {"dress", {"body"}}};

        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // No register edits and no texture edits at all: fischl6_1 carries the Ib* entries and
        // nothing else beyond the NNFix re-issue the template does by default.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory FischlFixer::v6_1ToFischlHighness() {
        return IniFixBuilderFuncs::fischl6_1ToFischlHighness();
    }
}
