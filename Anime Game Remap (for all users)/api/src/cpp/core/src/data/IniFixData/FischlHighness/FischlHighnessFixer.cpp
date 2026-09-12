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

#include "AGRemapCore/data/IniFixData/FischlHighness/FischlHighnessFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::fischlHighness6_1ToFischl() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT -- her body carries Fischl's body AND his dress.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // Fischl's head reads one slot lower than hers: her ps-t2 goes and her ps-t3 takes its
        // place. Only the head; the body keeps its registers where they are.
        config.objRegRemovals = {{"head", {"ps-t2"}}};
        config.objRegRemaps = {{"head", {{"ps-t3", {"ps-t2"}}}}};

        // The dress copy inherited the body's ib and must not draw with it -- fischlHighness6_1's
        // RegNewVals({"dress": {"ib": "null"}}). This is the Jean-style null that most splits do
        // NOT have; it is here because the row has it.
        config.objNewRegVals = {{"dress", {{"ib", "null"}}}};

        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory FischlHighnessFixer::v6_1ToFischl() {
        return IniFixBuilderFuncs::fischlHighness6_1ToFischl();
    }
}
