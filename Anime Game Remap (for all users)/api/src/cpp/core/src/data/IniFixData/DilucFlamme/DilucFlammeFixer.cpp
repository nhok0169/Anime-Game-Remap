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

#include "AGRemapCore/data/IniFixData/DilucFlamme/DilucFlammeFixer.h"

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/ColourReplaceFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/InvertAlphaFilter.h"
#include "AGRemapCore/model/textures/Colour.h"
#include "AGRemapCore/model/textures/ColourRange.h"


namespace AGRemapCore {

    namespace {
        // Her BODY diffuse: alpha inverted, then anything black in the narrow 125-130 alpha band
        // replaced with (0, 0, 0, 177). Straight from dilucFlamme4_0's TexEditor, whose two filters
        // run in that order -- the band and the 177 are the maintainer's numbers, not derived.
        void transparentBodyDiffuse(TextureFile& texFile) {
            InvertAlphaFilter invert;
            invert.transform(texFile);

            ColourReplaceFilter replace(
                Colour(0, 0, 0, 177),
                ColourOrRangeSet{ColourRange(Colour(0, 0, 0, 125), Colour(0, 0, 0, 130))});
            replace.transform(texFile);
        }


        // Her DRESS diffuse gets the alpha inversion and nothing else. Two sources landing on one
        // target, edited DIFFERENTLY -- which is the whole reason TexEdit has a srcObj.
        void transparentDressDiffuse(TextureFile& texFile) {
            InvertAlphaFilter invert;
            invert.transform(texFile);
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::dilucFlamme4_0() {
        // THE 4.0 FIX for DilucFlamme -> Diluc. Kept for the historical record: the game cannot be
        // rolled back, so an A/B against the old script at --version 4.0 --fromVersion 4.0 is
        // the whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE: Diluc has no dress, so DilucFlamme's body and dress both land on his body.
        // The head is listed TWICE -- see fischl4_0 for what an omitted object means here.
        config.objSplits = {{"head", {"head", "head"}}, {"body", {"body"}}, {"dress", {"body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // One edit per SOURCE, both onto the merged body's ps-t0.
        config.texEdits = {{"body", "ps-t0", "TransparentBodyDiffuse", &transparentBodyDiffuse, true, "body"},
                           {"body", "ps-t0", "TransparentDressDiffuse", &transparentDressDiffuse, true, "dress"}};

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
    IniFixBuilder::Factory IniFixBuilderFuncs::dilucFlamme5_7() {
        // THE 5.7 FIX, which is dilucFlamme4_0 plus ONE thing: the draw call moves.
        // Verified only against the old script at --version 5.7 --fromVersion 5.7;
        // the game cannot be rolled back to play it.

        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE: Diluc has no dress, so DilucFlamme's body and dress both land on his body.
        // The head is listed TWICE -- see fischl4_0 for what an omitted object means here.
        config.objSplits = {{"head", {"head", "head"}}, {"body", {"body"}}, {"dress", {"body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // One edit per SOURCE, both onto the merged body's ps-t0.
        config.texEdits = {{"body", "ps-t0", "TransparentBodyDiffuse", &transparentBodyDiffuse, true, "body"},
                           {"body", "ps-t0", "TransparentDressDiffuse", &transparentDressDiffuse, true, "dress"}};

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
        // Applies to head and body, over the 4.0 merge.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::dilucFlamme6_1ToDiluc() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE, and note what the head does. Diluc draws head and body, so her dress has
        // nowhere of its own to go and comes through his BODY -- the ordinary merge. But her HEAD
        // is listed TWICE, which makes it appear in BOTH generated .ini files rather than only the
        // first. That is dilucFlamme6_1's own {"head": ["head", "head"]}, and it is not what the
        // Lisa or Keqing merges do; without it the second file draws a body with no head.
        config.objSplits = {{"head", {"head", "head"}}, {"body", {"body"}}, {"dress", {"body"}}};

        // The generated second file explains itself -- see IniComments::GIMIObjMergerPreamble.
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // TWO EDITS OF ONE TARGET, TOLD APART BY SOURCE. Both write to Diluc's body ps-t0, because
        // both of her objects land there -- so each entry names the srcObj it came from, and each
        // gets its own file name. Declared in dilucFlamme6_1's preRegEditFilters, i.e. before the
        // merge, which is exactly what srcObj means here.
        config.texEdits = {{"body", "ps-t0", "TransparentBodyDiffuse", &transparentBodyDiffuse, true, "body"},
                           {"body", "ps-t0", "TransparentDressDiffuse", &transparentDressDiffuse, true, "dress"}};

        // The Ib* entries again, on both objects, plus a per-.ini postModel drawindexed removal.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory DilucFlammeFixer::v6_1ToDiluc() {
        return IniFixBuilderFuncs::dilucFlamme6_1ToDiluc();
    }
}
