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

#include "AGRemapCore/data/IniFixData/BarbaraSummertime/BarbaraSummertimeFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::barbaraSummertime4_0() {
        // THE 4.0 FIX, and its pure-Python row's body is empty -- which means a plain
        // one-to-one remap, not an absent one: hashes swapped, indices forward-looked-up,
        // sections renamed. Everything the 6.1 row adds on top (the NNFix/ORFix re-issue, any
        // register shuffle, moveDrawIndexed) arrived in later versions and is deliberately
        // absent here. Kept for the historical record; the game cannot be rolled back to 4.0
        // to confirm it in play, so the A/B against the old script at --version 4.0 is the
        // whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

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
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }
    IniFixBuilder::Factory IniFixBuilderFuncs::barbaraSummertime5_7() {
        // THE 5.7 FIX, which is barbaraSummertime4_0 plus ONE thing: the draw call moves.
        // Verified only against the old script at --version 5.7 --fromVersion 5.7;
        // the game cannot be rolled back to play it.

        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

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
        // Applies to head, body and dress.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory BarbaraSummertimeFixer::v4_0() {
        return IniFixBuilderFuncs::barbaraSummertime4_0();
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::barbaraSummertime6_1ToBarbara() {
        // Remapped onto Barbara, a genuinely different model -- see makeGIMICharFixer for what
        // that shape does. Only what BarbaraSummertime does differently lives here.
        //
        // The mirror of BarbaraFixer, and equally plain: head, body and dress on both sides.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // Same Ib* entries as Barbara's row -- see BarbaraFixer for why this is true and for
        // why the A/B cannot see it.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory BarbaraSummertimeFixer::v6_1ToBarbara() {
        return IniFixBuilderFuncs::barbaraSummertime6_1ToBarbara();
    }
}
