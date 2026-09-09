#include "AGRemapCore/data/IniFixData/KeqingOpulent/KeqingOpulentFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/TransparencyAdjustFilter.h"
#include "AGRemapCore/model/textures/Colour.h"
#include "AGRemapCore/model/textures/ColourRange.h"


namespace AGRemapCore {
    namespace {
        // Fully opaque -- an ADJUSTMENT of +255 rather than a set, which is what
        // TransparencyAdjustFilter takes. Clamped, so it saturates every matched pixel.
        const int Opaque = 255;


        /**
         * The two colour bands KeqingOpulent's head lightmap uses to mark a surface as reflective.
         *
         * A lightmap is not a picture: each channel is a material parameter, and the alpha channel
         * is the one the shader reads as "how much does this reflect". The magenta band is the
         * metal trim, the olive band the lacquered hair ornament. Keqing has neither, so the
         * remapped model ends up with a mirror finish on flat cloth.
         *
         * Forcing those pixels opaque is what switches the reflection off -- hence the resource
         * name the fix writes, NonReflectiveLightMap.
         *
         * The alpha bound of 254 on each maximum is deliberate and is copied from the pure-Python
         * row: a pixel that is ALREADY fully opaque is left out of the match, since adjusting it
         * would be a no-op anyway.
         */
        ColourOrRangeSet reflectiveColours() {
            return ColourOrRangeSet{
                ColourRange(Colour(20, 0, 20, 0), Colour(225, 0, 225, 254)),
                ColourRange(Colour(120, 120, 50, 0), Colour(140, 140, 70, 254))};
        }


        void removeReflections(TextureFile& texFile) {
            TransparencyAdjustFilter filter(Opaque, reflectiveColours());
            filter.transform(texFile);
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::keqingOpulent6_1() {
        // Remapped onto Keqing -- the direction that SPLITS, and the exact mirror of keqing6_1's
        // merge.
        //
        // KeqingOpulent's outfit is a single body mesh; Keqing draws a body AND a dress. So the one
        // body graph is emitted twice, once carrying Keqing's body index (10824) and once her dress
        // index (48216), both under Keqing's own ib hash. Both copies bind the same textures, which
        // is the point: one mesh is being shown through two of the target's draw calls.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        // 'head' is listed even though it maps to itself: objSplits is all-or-nothing, and an
        // object left out of it is dropped from the remap entirely.
        config.objSplits = {{"head", {"head"}}, {"body", {"body", "dress"}}};

        // Unlike a merge, a split needs no copyPreamble -- both targets are distinct objects and
        // fit in one .ini file.

        // See reflectiveColours above for what this is actually doing. Declared against ps-t1
        // because that is where a lightmap hangs, and against 'head' only: the pure-Python row
        // names the head and nothing else.
        config.texEdits = {{"head", "ps-t1", "NonReflectiveLightMap", &removeReflections}};

        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries that
        // switch it on.
        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory KeqingOpulentFixer::v6_1() {
        return IniFixBuilderFuncs::keqingOpulent6_1();
    }
}
