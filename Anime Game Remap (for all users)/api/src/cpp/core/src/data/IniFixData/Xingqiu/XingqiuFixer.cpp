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

#include "AGRemapCore/data/IniFixData/Xingqiu/XingqiuFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::xingqiu4_0() {
        // THE 4.0 FIX for Xingqiu -> XingqiuBamboo. Kept for the historical record: the game cannot be
        // rolled back, so an A/B against the old script at --version 4.0 --fromVersion 4.0 is
        // the whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT: XingqiuBamboo has a dress Xingqiu has no geometry for, so his head graph
        // is emitted twice.
        config.objSplits = {{"head", {"head", "dress"}}, {"body", {"body"}}};

        // ...and the head copy's ps-t2 moves down to ps-t3. A postRegEditFilter in the
        // pure-Python row, which is this template's ordinary objRegRemaps.
        config.objRegRemaps = {{"head", {{"ps-t2", {{"ps-t3"}}, true}}}};

        // ---- the three 6.1-era defaults this row predates ----
        //
        // The face register swap corrects something GI 6.x did to the shader; at 4.0 the diffuse
        // still belongs on faceDiffuseReg.
        config.swapFaceRegs = false;

        // Nothing in this row's pure-Python body removes the mod's own ORFix/NNFix calls, and
        // nothing re-issues them, so both halves of that machinery stay off. Leaving the removal
        // on would delete the modder's call with nothing putting it back.
        config.removeSrcFixCalls = false;

        // ...and the default NNFix re-issue. An objFixCalls entry REPLACES the default
        // for its target, including with an empty list, and it is keyed by TARGET -- a
        // split's second copy is its own target and needs its own entry.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::xingqiu6_1() {
        // Remapped onto XingqiuBamboo -- a SPLIT of the head alone, and the smallest fix in this
        // batch.
        //
        // Xingqiu draws head and body; the skin draws head, body and dress, its outer robe being
        // geometry Xingqiu simply does not have. So the head graph is emitted twice, once at the
        // skin's head index and once at its dress index, both under the skin's own ib hash.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // 'body' is listed even though it maps to itself: objSplits is all-or-nothing, and an object
        // left out of it is dropped from the remap entirely.
        config.objSplits = {{"head", {"head", "dress"}}, {"body", {"body"}}};

        // The target's head reads a shadow ramp a slot higher than Xingqiu binds it. Only the head:
        // the dress copy, which came off the same graph, is NOT shifted, and neither is the body --
        // this is the one place in this file where the target-keyed default is exactly what is
        // wanted rather than an approximation.
        config.objRegRemaps = {{"head", {{"ps-t2", {"ps-t3"}}}}};

        // objFixCalls left alone: NNFix on every object, which is the 6.1 default and what the
        // pure-Python row spells out by hand.
        //
        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries.
        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory XingqiuFixer::v6_1() {
        return IniFixBuilderFuncs::xingqiu6_1();
    }
}
