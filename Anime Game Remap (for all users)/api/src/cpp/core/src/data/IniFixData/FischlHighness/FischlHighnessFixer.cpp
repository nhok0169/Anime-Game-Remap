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

    IniFixBuilder::Factory IniFixBuilderFuncs::fischlHighness4_0() {
        // THE 4.0 FIX for FischlHighness -> Fischl. Kept for the historical record: the game cannot be
        // rolled back, so an A/B against the old script at --version 4.0 --fromVersion 4.0 is
        // the whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT, the inverse of fischl4_0's merge.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // Her head drops ps-t2 and pulls ps-t3 into its place.
        config.objRegRemovals = {{"head", {"ps-t2"}}};
        config.objRegRemaps = {{"head", {{"ps-t3", {{"ps-t2"}}, true}}}};

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
    IniFixBuilder::Factory IniFixBuilderFuncs::fischlHighness5_7() {
        // THE 5.7 FIX, which is fischlHighness4_0 plus ONE thing: the draw call moves.
        // Verified only against the old script at --version 5.7 --fromVersion 5.7;
        // the game cannot be rolled back to play it.

        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT, the inverse of fischl4_0's merge.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // Her head drops ps-t2 and pulls ps-t3 into its place.
        config.objRegRemovals = {{"head", {"ps-t2"}}};
        config.objRegRemaps = {{"head", {{"ps-t3", {{"ps-t2"}}, true}}}};

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

        // AND ONE THING fischlHighness4_0 DOES NOT DO: the split's dress copy gets a null ib.
        // It draws no geometry of its own, so leaving it bound to the source's index buffer has
        // it draw the body again. The 4.0 row carries no such RegNewVals.
        config.objNewRegVals = {{"dress", {{"ib", "null"}}}};

        // ---- 5.7: THE DRAW CALL MOVES ----
        //
        // The pure-Python row says this as IbRemapData + IbDrawIndexedRename +
        // IbTempToDrawIndexed with a postModel RegRemove of the original 'ib' -- stash the key
        // on a temp register, rename that register's VALUE to 'drawindexed', remap the temp
        // register onto the real key, drop what was there. One flag here.
        //
        // Applies to head and body, over the 4.0 split.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }

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
