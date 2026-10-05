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

#include "AGRemapCore/data/IniFixData/NingguangOrchid/NingguangOrchidFixer.h"

#include <utility>

#include <string>
#include <vector>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::ningguangOrchid4_0() {
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


    IniFixBuilder::Factory NingguangOrchidFixer::v4_0() {
        return IniFixBuilderFuncs::ningguangOrchid4_0();
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::ningguangOrchid6_1() {
        // Remapped onto Ningguang -- the plainest shape this template has. See makeGIMICharFixer.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // NOTHING ELSE, and the asymmetry with NingguangFixer is real rather than an omission:
        //
        //  * no objRegRemovals -- the ps-t3 strip goes the other way. Ningguang binds a ps-t3 that
        //    Orchid does not read; Orchid binds none, so there is nothing to take away
        //  * no texEdits -- the DarkDiffuse edit is declared against Ningguang's head diffuse. The
        //    pure-Python ningguangOrchid6_1 row has no RegTexEdit either
        //
        // moveDrawIndexed stays false, as for Ningguang.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory NingguangOrchidFixer::v6_1() {
        return IniFixBuilderFuncs::ningguangOrchid6_1();
    }
}
