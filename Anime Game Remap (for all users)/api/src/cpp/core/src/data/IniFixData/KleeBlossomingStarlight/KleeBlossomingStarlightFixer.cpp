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

#include "AGRemapCore/data/IniFixData/KleeBlossomingStarlight/KleeBlossomingStarlightFixer.h"

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/InvertAlphaFilter.h"


namespace AGRemapCore {

    namespace {
        // Her dress diffuse, alpha inverted -- kleeBlossomingStarlight4_0's
        // TexEditor(filters = [InvertAlphaFilter()]), with no arguments of its own.
        void invertAlpha(TextureFile& texFile) {
            InvertAlphaFilter filter;
            filter.transform(texFile);
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::kleeBlossomingStarlight4_0() {
        // THE 4.0 FIX for KleeBlossomingStarlight -> Klee. Kept for the historical record: the game cannot be
        // rolled back, so an A/B against the old script at --version 4.0 --fromVersion 4.0 is
        // the whole of its verification.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE, the inverse of klee4_0's split. Head twice -- omitted from the
        // pure-Python map, which means "into every generated file"; see fischl4_0.
        config.objSplits = {{"head", {"head", "head"}}, {"body", {"body"}}, {"dress", {"body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // Her body diffuse's alpha is inverted.
        config.texEdits = {{"body", "ps-t0", "TransparentDiffuse", &invertAlpha}};

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
                              {"body", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::kleeBlossomingStarlight6_1ToKlee() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE MERGE, and the mirror of KleeFixer's split. Klee has nowhere to put a dress, so
        // the skin's dress is drawn through Klee's BODY -- two sources landing on one target,
        // which is why the fix writes more than one .ini file and lets the game overlap them.
        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}};

        // The generated second file explains itself -- see IniComments::GIMIObjMergerPreamble,
        // which is the paragraph the pure-Python merge wrote for exactly this.
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // Her dress diffuse has its alpha inverted -- kleeBlossomingStarlight6_1's
        // RegTexEdit(textures = {"TransparentDiffuse": ["ps-t0"]}).
        //
        // srcObj is named as well as obj because this is a MERGE: the edit belongs to HER dress,
        // and that dress is drawn through Klee's BODY. Naming only the object would collect the
        // wrong texture -- the mistake that rendered AyakaSpringbloom's neck pale.
        config.texEdits = {{"body", "ps-t0", "TransparentDiffuse", &invertAlpha,
                            true, "dress"}};

        // Her head binds a ps-t2 Klee has no use for, and its ps-t3 takes that slot instead.
        config.objRegRemovals = {{"head", {"ps-t2"}}};
        config.objRegRemaps = {{"head", {{"ps-t3", {{"ps-t2"}}, true}}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory KleeBlossomingStarlightFixer::v6_1ToKlee() {
        return IniFixBuilderFuncs::kleeBlossomingStarlight6_1ToKlee();
    }
}
