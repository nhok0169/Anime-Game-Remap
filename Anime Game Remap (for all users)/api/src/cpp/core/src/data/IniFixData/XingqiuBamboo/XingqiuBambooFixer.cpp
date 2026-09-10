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

#include "AGRemapCore/data/IniFixData/XingqiuBamboo/XingqiuBambooFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::xingqiuBamboo6_1() {
        // Remapped onto Xingqiu -- the MERGE that undoes xingqiu6_1's split.
        //
        // The skin's head and its outer robe ('dress') both come back through Xingqiu's head, which
        // is two .ini files. His body takes the skin's body, and has to appear in BOTH files or the
        // second one draws a head with no body under it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // 'body' listed twice against the same target is not a typo: the first lands in group 0 and
        // the second collides and lands in group 1, which is the same mechanism that creates the
        // extra group in the first place and the only way to ask for a graph in every file.
        config.objSplits = {{"head", {"head"}}, {"dress", {"head"}}, {"body", {"body", "body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // ---- the head's register shift ----
        //
        // SOURCE-keyed, and this is the smallest example of why that field exists. The skin's HEAD
        // binds a ps-t2 Xingqiu does not read, with what he wants on ps-t3; its DRESS does not, and
        // both land on his head in different groups. Keyed by target, the dress copy would lose a
        // ps-t2 it needs and gain a ps-t3 it does not have.
        config.srcObjRegRemovals = {{"head", {"ps-t2"}}};
        config.srcObjRegRemaps = {{"head", {{"ps-t3", {"ps-t2"}}}}};

        // objFixCalls left alone: NNFix on every object, which is the 6.1 default and what the
        // pure-Python row spells out by hand.
        //
        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries.
        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory XingqiuBambooFixer::v6_1() {
        return IniFixBuilderFuncs::xingqiuBamboo6_1();
    }
}
