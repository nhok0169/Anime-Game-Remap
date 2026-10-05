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

#include "AGRemapCore/data/IniFixData/JeanSea/JeanSeaFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {
    namespace {
        /**
         * The merge, shared by both targets -- Jean and JeanCN draw the same objects as each other,
         * so what JeanSea has to collapse is the same either way.
         *
         * 'head' is listed even though it maps to itself: objSplits is all-or-nothing, and an object
         * left out of it is dropped from the remap entirely.
         *
         * ORDER IS LOAD-BEARING. The first claimant of a target keeps the main .ini file and later
         * ones go to generated copies, so listing 'dress' before 'body' would put the cape in the
         * mod's own file and the body in the copy -- working, but backwards from what a reader
         * opening the mod expects to find.
         */
        GIMICharFixerConfig makeConfig() {
            GIMICharFixerConfig config{};
            config.drawnObjs = {"head", "body", "dress"};
            config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}};

            // The generated second file explains itself -- see IniComments::GIMIObjMergerPreamble,
            // which is the paragraph the pure-Python merge wrote for exactly this.
            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            // No texEdits: the ShadeLightMap edit belongs to the fixes that remap ONTO JeanSea (see
            // JeanFixer), not to the ones that remap off her. The pure-Python jeanSea6_1 row has no
            // RegTexEdit either.
            //
            // moveDrawIndexed stays false, as for the rest of the Jean family.
            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::jeanSea4_0() {
        // THE 4.0 FIX for JeanSea -> Jean. Kept for the historical record: the game cannot be
        // rolled back, so an A/B against the old script at --version 4.0 --fromVersion 4.0 is
        // the whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE onto a target with no dress. Serves both JeanSea -> Jean and
        // JeanSea -> JeanCN: the pure-Python row is one function for both targets.
        //
        // No texEdits: the ShadeLightMap edit belongs to the fixes that remap ONTO JeanSea, and
        // the 4.0 row carries none in either direction.
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

    IniFixBuilder::Factory IniFixBuilderFuncs::jeanSea6_1ToJean() {
        return makeGIMICharFixer(makeConfig());
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::jeanSea6_1ToJeanCN() {
        return makeGIMICharFixer(makeConfig());
    }


    IniFixBuilder::Factory JeanSeaFixer::v6_1ToJean() {
        return IniFixBuilderFuncs::jeanSea6_1ToJean();
    }


    IniFixBuilder::Factory JeanSeaFixer::v6_1ToJeanCN() {
        return IniFixBuilderFuncs::jeanSea6_1ToJeanCN();
    }
}
