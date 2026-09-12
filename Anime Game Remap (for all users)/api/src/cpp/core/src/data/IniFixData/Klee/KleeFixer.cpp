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

#include "AGRemapCore/data/IniFixData/Klee/KleeFixer.h"

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/ColourReplaceFilter.h"


namespace AGRemapCore {

    namespace {
        /**
         * Klee's body lightmap, greened.
         *
         * Straight from klee4_0's RegTexEdit: replace with (0, 128, 0, 177) wherever the texture
         * is black in either of two narrow ALPHA bands -- 250-255 and 125-130. The bands are the
         * maintainer's, not derived, and replaceAlpha is on (the filter's default), which is what
         * carries that 177 across.
         */
        void greenLightMap(TextureFile& texFile) {
            ColourReplaceFilter filter(
                Colour(0, 128, 0, 177),
                ColourOrRangeSet{ColourRange(Colour(0, 0, 0, 250), Colour(0, 0, 0, 255)),
                                 ColourRange(Colour(0, 0, 0, 125), Colour(0, 0, 0, 130))});
            filter.transform(texFile);
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::klee6_1ToKleeBlossomingStarlight() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // THE SPLIT. KleeBlossomingStarlight's skirt is a 'dress' object Klee has no geometry
        // for at all, so Klee's body graph is emitted twice -- once carrying the skin's body
        // index (32553) and once her dress index (82101), both under the skin's own ib hash.
        //
        // 'head' is listed even though it maps to itself: objSplits is all-or-nothing, and an
        // object left out of it is dropped from the remap entirely.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // NO objNewRegVals HERE. Jean's split writes "null" over the dress copy's inherited
        // 'ib'; klee6_1 does not, and copying Jean's row into this one was an invention rather
        // than a port. If a Klee mod ever shows a dress bound to Klee's own ib, start here.

        // Her body's lightmap is greened -- klee6_1's RegTexEdit(textures = {"GreenLightMap":
        // ["ps-t1"]}). This is what the old script edits two .dds files for on a Klee mod, and
        // its absence is how the first draft of this file was caught: the run reported a clean fix
        // while editing no textures at all.
        // BOTH COPIES, not just the body. klee6_1 sets preRegEditOldObj = True, which runs its
        // RegTexEdit on the pre-split object -- so the dress that the split creates inherits the
        // edited lightmap rather than the original one. The old script points both sections at the
        // same ...GreenLightMap...RemapTex0 resource, and so does this: one source plus one edit
        // name is one file, however many objects end up referencing it.
        //
        // This is the opposite of Jean's split, where the dress deliberately keeps the UNEDITED
        // lightmap -- her row has no preRegEditOldObj. The two look alike and are not, so check
        // that flag before copying either one.
        config.texEdits = {{"body", "ps-t1", "GreenLightMap", &greenLightMap},
                           {"dress", "ps-t1", "GreenLightMap", &greenLightMap, true, "body"}};

        // ...and her head's ps-t2 moves up to ps-t3 to make room, which the template's own
        // diffuse/lightmap handling does not do on its own.
        config.objRegRemaps = {{"head", {{"ps-t2", {{"ps-t3"}}, true}}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory KleeFixer::v6_1ToKleeBlossomingStarlight() {
        return IniFixBuilderFuncs::klee6_1ToKleeBlossomingStarlight();
    }
}
