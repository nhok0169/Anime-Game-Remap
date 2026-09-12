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
