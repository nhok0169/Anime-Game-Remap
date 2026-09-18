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

#include "AGRemapCore/data/IniFixData/Diluc/DilucFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::diluc4_0() {
        // THE 4.0 FIX, and its pure-Python row's body is empty -- which means a plain
        // one-to-one remap, not an absent one: hashes swapped, indices forward-looked-up,
        // sections renamed. Everything the 6.1 row adds on top (the NNFix/ORFix re-issue, any
        // register shuffle, moveDrawIndexed) arrived in later versions and is deliberately
        // absent here. Kept for the historical record; the game cannot be rolled back to 4.0
        // to confirm it in play, so the A/B against the old script at --version 4.0 is the
        // whole of its verification.
        // THE SPLIT is the only thing diluc4_0 carries -- her dress comes later, and the
        // Ib* entries that become moveDrawIndexed arrive in 5.7.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // AND NO FACE REGISTER SWAP. That swap corrects something GI 6.x did to the shader; at
        // 4.0 the diffuse still belongs on ps-t0, so applying it moves a correct binding to the
        // wrong register. It has no pure-Python equivalent and the old script produces none.
        config.swapFaceRegs = false;

        // AND THE MOD KEEPS ITS OWN ORFix/NNFix CALLS. The default strip exists because the fix
        // re-issues those itself and a survivor would duplicate; this row re-issues nothing, so
        // the strip would just delete the modder's line. The pure-Python 6.1 rows each carry an
        // explicit RegRemove for it and the 4.0 rows carry none -- this flag is that difference.
        config.removeSrcFixCalls = false;

        // NO FIX CALL ON ANY OBJECT. makeGIMICharFixer re-issues NNFix by default, which is
        // 6.1-era behaviour -- this row predates the whole NNFix/ORFix layer and its
        // pure-Python body carries none of it, so every drawn object gets an empty list. An
        // empty list REPLACES the default rather than adding to it.
        // ALL THREE TARGETS, not just the two he draws. objFixCalls is keyed by TARGET and
        // the split makes a dress out of his body, so leaving it out lets that copy keep the
        // default NNFix while the other two have none -- which the old script does not do.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }
    IniFixBuilder::Factory IniFixBuilderFuncs::diluc5_7() {
        // THE 5.7 FIX, which is diluc4_0 plus ONE thing: the draw call moves.
        // Verified only against the old script at --version 5.7 --fromVersion 5.7;
        // the game cannot be rolled back to play it.

        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // AND NO FACE REGISTER SWAP. That swap corrects something GI 6.x did to the shader; at
        // 4.0 the diffuse still belongs on ps-t0, so applying it moves a correct binding to the
        // wrong register. It has no pure-Python equivalent and the old script produces none.
        config.swapFaceRegs = false;

        // AND THE MOD KEEPS ITS OWN ORFix/NNFix CALLS. The default strip exists because the fix
        // re-issues those itself and a survivor would duplicate; this row re-issues nothing, so
        // the strip would just delete the modder's line. The pure-Python 6.1 rows each carry an
        // explicit RegRemove for it and the 4.0 rows carry none -- this flag is that difference.
        config.removeSrcFixCalls = false;

        // NO FIX CALL ON ANY OBJECT. makeGIMICharFixer re-issues NNFix by default, which is
        // 6.1-era behaviour -- this row predates the whole NNFix/ORFix layer and its
        // pure-Python body carries none of it, so every drawn object gets an empty list. An
        // empty list REPLACES the default rather than adding to it.
        // ALL THREE TARGETS, not just the two he draws. objFixCalls is keyed by TARGET and
        // the split makes a dress out of his body, so leaving it out lets that copy keep the
        // default NNFix while the other two have none -- which the old script does not do.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

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


    IniFixBuilder::Factory DilucFixer::v4_0() {
        return IniFixBuilderFuncs::diluc4_0();
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::diluc6_1ToDilucFlamme() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT. DilucFlamme's dress is geometry Diluc does not have, so his body graph is
        // emitted twice -- once under DilucFlamme's body index and once under her dress index.
        //
        // 'head' is listed even though it maps to itself: objSplits is all-or-nothing, and an
        // object left out of it is dropped from the remap entirely.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // diluc6_1 carries IbRemapData / IbDrawIndexedRename / IbTempToDrawIndexed on both objects
        // plus a postModel drawindexed removal, which together are what this flag does.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory DilucFixer::v6_1ToDilucFlamme() {
        return IniFixBuilderFuncs::diluc6_1ToDilucFlamme();
    }
}
