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

    IniFixBuilder::Factory IniFixBuilderFuncs::fischl4_0() {
        // THE 4.0 FIX for Fischl -> FischlHighness. Kept for the historical record: the game cannot be
        // rolled back, so an A/B against the old script at --version 4.0 --fromVersion 4.0 is
        // the whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE: FischlHighness has no dress, so Fischl's body and dress both land on its
        // body. Read objSplits the other way round for a merge -- it is keyed by SOURCE.
        //
        // The head is listed TWICE, and that is what an OMITTED object means in the pure-Python
        // merge. Its map here is {"body": ["body", "dress"]} -- the head is not mentioned at all --
        // and an object the merge map does not name is emitted into EVERY generated file. Naming it
        // once, as jeanSea6_1 does, is what restricts it to the first. This template's objSplits is
        // all-or-nothing, so "in both files" has to be spelled out as two entries (2026-09-13).
        config.objSplits = {{"head", {"head", "head"}}, {"body", {"body"}}, {"dress", {"body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

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
                              {"body", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }
    IniFixBuilder::Factory IniFixBuilderFuncs::fischl5_7() {
        // THE 5.7 FIX, which is fischl4_0 plus ONE thing: the draw call moves.
        // Verified only against the old script at --version 5.7 --fromVersion 5.7;
        // the game cannot be rolled back to play it.

        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE: FischlHighness has no dress, so Fischl's body and dress both land on its
        // body. Read objSplits the other way round for a merge -- it is keyed by SOURCE.
        //
        // The head is listed TWICE, and that is what an OMITTED object means in the pure-Python
        // merge. Its map here is {"body": ["body", "dress"]} -- the head is not mentioned at all --
        // and an object the merge map does not name is emitted into EVERY generated file. Naming it
        // once, as jeanSea6_1 does, is what restricts it to the first. This template's objSplits is
        // all-or-nothing, so "in both files" has to be spelled out as two entries (2026-09-13).
        config.objSplits = {{"head", {"head", "head"}}, {"body", {"body"}}, {"dress", {"body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

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
                              {"body", std::vector<std::string>{}}};

        // ---- 5.7: THE DRAW CALL MOVES ----
        //
        // The pure-Python row says this as IbRemapData + IbDrawIndexedRename +
        // IbTempToDrawIndexed with a postModel RegRemove of the original 'ib' -- stash the key
        // on a temp register, rename that register's VALUE to 'drawindexed', remap the temp
        // register onto the real key, drop what was there. One flag here.
        //
        // Applies to head, body and dress, over the 4.0 merge.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }

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
