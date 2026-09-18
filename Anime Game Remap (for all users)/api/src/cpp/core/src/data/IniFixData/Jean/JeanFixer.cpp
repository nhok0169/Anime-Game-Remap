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

#include "AGRemapCore/data/IniFixData/Jean/JeanFixer.h"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/data/IniFixData/JeanShading.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::jean5_5ToJeanSea() {
        // THE 5.5 FIX for Jean -> JeanSea, verified only against the old script at
        // --version 5.5 --fromVersion 5.5 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // WHAT 5.5 ADDS over jean4_0ToJeanSea, and it is all on this target: the split's dress
        // copy gets a null ib, and the body's lightmap gets JeanSea's shading lift.
        config.objNewRegVals = {{"dress", {{"ib", "null"}}}};
        config.texEdits = {{"body", "ps-t1", "ShadeLightMap", &JeanShading::liftLowAlpha}};

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;

        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::jean5_5ToJeanCN() {
        // THE 5.5 FIX for Jean -> JeanCN, verified only against the old script at
        // --version 5.5 --fromVersion 5.5 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // A plain one-to-one remap, exactly as at 4.0 -- the MultiModFixer hands this target a
        // bare GIMIObjRegEditFixer with empty kwargs.

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;

        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::jean4_0ToJeanCN() {
        // THE 4.0 FIX for Jean -> JeanCN. Kept for the historical record: the game cannot be
        // rolled back, so an A/B against the old script at --version 4.0 --fromVersion 4.0 is
        // the whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // A plain one-to-one remap -- the pure-Python MultiModFixer hands this target a bare
        // GIMIObjRegEditFixer with no arguments at all. No split: JeanCN draws what Jean draws.

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

    IniFixBuilder::Factory IniFixBuilderFuncs::jean4_0ToJeanSea() {
        // THE 4.0 FIX for Jean -> JeanSea. Kept for the historical record: the game cannot be
        // rolled back, so an A/B against the old script at --version 4.0 --fromVersion 4.0 is
        // the whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT half of the same MultiModFixer: JeanSea has a dress Jean does not.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

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

    IniFixBuilder::Factory IniFixBuilderFuncs::jean6_1ToJeanCN() {
        // The ordinary CN-skin remap -- a genuinely different model, so see makeGIMICharFixer for
        // what that shape does. Nothing here is Jean-specific beyond her drawn objects.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // moveDrawIndexed stays false: the pure-Python row for Jean carries none of the Ib*
        // entries Amber's does.

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::jean6_1ToJeanSea() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT. JeanSea's cape is a 'dress' object Jean has no geometry for at all, so Jean's
        // body graph is emitted twice -- once carrying JeanSea's body index (7662) and once her
        // dress index (52542), both under JeanSea's own ib hash.
        //
        // 'head' is listed even though it maps to itself: objSplits is all-or-nothing, and an
        // object left out of it is dropped from the remap entirely.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // The dress copy inherited Jean's body 'ib', which is not its own. The pure-Python row
        // writes "null" over it and so does this.
        config.objNewRegVals = {{"dress", {{IniKeywords::Ib, "null"}}}};

        // The body's lightmap, shaded so JeanSea's cape does not read as a hard edge against the
        // body underneath it. Deliberately NOT applied to the dress copy: the pure-Python original
        // declares this edit against the mod's own 'body' and its RegTexEdit fires per NEW object,
        // so the dress -- which is not an object Jean has an edit for -- keeps the untouched
        // lightmap. The integration golden pins that asymmetry
        // (Testing/Integration Tester/.../expected_fullFix_someFix/multiFix/select/Jean/), where
        // the body section points at ...ShadeLightMapJeanSeaRemapTex0 and the dress section at the
        // original ...BodyLightMap.
        config.texEdits = {{"body", "ps-t1", "ShadeLightMap", &JeanShading::liftLowAlpha}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory JeanFixer::v6_1ToJeanCN() {
        return IniFixBuilderFuncs::jean6_1ToJeanCN();
    }


    IniFixBuilder::Factory JeanFixer::v6_1ToJeanSea() {
        return IniFixBuilderFuncs::jean6_1ToJeanSea();
    }
}
