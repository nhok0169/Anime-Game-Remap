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

#include "AGRemapCore/data/IniFixData/YelanTranquil/YelanTranquilFixer.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {
    namespace {
        // ---- the band legend, the other way round ----
        //
        // A lightmap's alpha is a material band, and the legend differs per skin. Both were
        // re-measured off the two identity mods -- an alpha histogram of each lightmap with the mean
        // diffuse colour under each band -- rather than taken from the forward direction's comment:
        //
        //   YelanTranquil Body A   0 white fur (diffuse 187,204,221) | 64-89 silver | 115-128 hair
        //                          (55,65,105) | 165-189 silk and lace | 255 SKIN (243,216,197)
        //   YelanTranquil Body B/C 0 white trim | 64-89 silver | 115-128 dark navy CLOTH (45,64,88)
        //                          | 165-189 teal silk | no 255 band at all
        //   Yelan Head/Extra       0 hair (57,65,95) | 115-128 skin | 255 FUR (190,201,215), 65% of it
        //   Yelan Body/Dress       0 hair and dark cloth | 64-89 metal | 115-128 SKIN (244,224,206)
        //                          | 165-189 ornaments | 255 fur
        //
        // Three bands move and the rest already line up (silver/metal at 64-89, the dark 165-189).
        // They are applied SIMULTANEOUSLY -- every decision is made from the ORIGINAL alpha -- because
        // they are a permutation: 0 -> 255 -> 121 -> ... would otherwise chase itself.
        //
        // Each move is conditioned on the DIFFUSE under the pixel, for the reason the forward
        // direction found the hard way: a mod need not follow its character's legend at all, because
        // a PORT keeps its SOURCE character's bands.
        //
        // One asymmetry with the forward direction worth knowing: there the fur move is
        // unconditional, because Yelan's 255 is never Tranquil-legend skin. Here it cannot be, since
        // the band in question is 0 -- the DEFAULT a lazy or ported mod leaves everything on -- and
        // Yelan's own band 0 is her hair-and-all-cloth band, i.e. the safe no-op. So band 0 moves to
        // her fur only where the diffuse actually looks like white fur.
        // Read as a MaterialBandRemapFilter table: source band (or range) -> target band, gated on
        // the diffuse under the pixel. The filter applies them SIMULTANEOUSLY -- every decision
        // from the ORIGINAL alpha -- which is what these three need, since they are a permutation
        // and in sequence a permutation chases itself.
        const std::vector<MaterialBandRemapFilter::Band> Bands = {
            // Tranquil's skin onto the middle of Yelan's 115-127.
            {255, 121, &MaterialBandRemapFilter::skinColoured},

            // Tranquil's white fur onto Yelan's. Gated, unlike the forward direction's fur move,
            // because the source band here is 0 -- the DEFAULT a lazy or ported mod leaves
            // everything on -- and Yelan's own band 0 is her hair-and-dark-cloth band, the safe
            // no-op. So it moves only where the diffuse really does look like white fur.
            {0, 255, &MaterialBandRemapFilter::whiteFurColoured},

            // Tranquil's hair onto Yelan's hair band, wherever the diffuse is NOT skin.
            {115, 128, 0, &MaterialBandRemapFilter::skinColoured, true},
        };



        GIMIMergeFixerConfig yelanConfig() {
            // YelanTranquil -> Yelan: the first remap of a skin of SEVERAL components onto a target
            // of one, and the inverse of Yelan -> YelanTranquil. See GIMIMergeFixerConfig for what
            // each field does; every value here was read off the prototype
            // (Tools/Misc/Prototypes/tranquilToYelanFix.py), confirmed in game on the identity mod
            // and two downloads.
            //
            // Which source slot lands on which of Yelan's objects was decided by ISOLATING every
            // draw out of the two frame analyses -- each draw's render target minus the one before
            // it -- rather than from the slot names:
            //
            //   Body A  17877 tris  the body: skin, legs, arms, back hair, shoes   -> body   (20913)
            //   Body B   4581 tris  jewellery and ornaments                        -> extra  (54042)
            //   Body C   6780 tris  the outfit: bodice, sash, skirt, gloves        -> dress  (51759)
            //   Bang     2564 tris  the front fringe                               -> head   (0)
            //   Eye       156 tris  the eyes                                       -> head   (0)
            //
            // The Bang and the Eye both land on the head, which the MERGE turns into one SECTION
            // rather than a second .ini file. The Body goes first in merge order so its three index
            // buffers pass through untouched.
            //
            // It is not one draw call, though, and assuming so cost Yelan her eyes. The head's
            // index buffer is the Bang's followed by the Eye's, and a mod that issues its own
            // `drawindexed` lines addresses its own Bang buffer -- covering the Bang exactly and
            // stopping where the Eye begins. The fix appends a draw for every member after the
            // first; see GIMIMergeFixer's extra-draw loop.
            //
            // All four of Yelan's draws are the no-normal-map shader family (vs 95aa6cdb84eb7b99 and
            // d4c01363144d79d6, both falling to ORFix's LDX branch), so every remapped object is the
            // two-register NNFix layout and the normal map Tranquil's A/B/Bang carry is dropped.
            GIMIMergeFixerConfig config{};

            GIMIMergeFixerConfig::Component body{};
            body.name = "Body";
            // The trailing number is the GAME model's index count for the slot, read off the
            // identity mod's own .ib files (R32, so bytes / 4). It is the fallback for a slot whose
            // ib has to be downloaded -- see GIMIMergeFixerConfig::Slot::indexCount -- and is only
            // ever read for a target object several slots merge onto, which here is the head.
            body.slots = {{"A", "0", "body", true, "", 53631},
                          {"B", "53631", "extra", true, "", 13743},
                          {"C", "67374", "dress", false, "", 20340}};

            // The GAME model's counts, for a mod that does not have the component at all -- see
            // GIMIMergeFixerConfig::Component::vertexCount. The same three numbers the parse row
            // gives its downloads, read off the 5.7 frame analysis.
            body.vertexCount = 25954;

            // No textures of their own: the game draws both with the Body slot A set (dump draws
            // 49-52). A mod may leave these sections with an ib and nothing else.
            GIMIMergeFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.slots = {{"A", "0", "head", true, "Body;A", 7692}};
            bang.vertexCount = 2256;

            GIMIMergeFixerConfig::Component eye{};
            eye.name = "Eye";
            eye.slots = {{"A", "0", "head", false, "Body;A", 468}};
            eye.vertexCount = 120;

            config.components = {std::move(body), std::move(bang), std::move(eye)};
            config.targetObjs = {"head", "body", "dress", "extra"};

            // As BennettAdventure: a mod carrying none of a component merges it from its downloads.
            // Inert for a mod that has all three, which is every YelanTranquil mod measured so far.
            config.downloadPrefix = "YelanTranquil";

            // Both skins bind the face diffuse at ps-t1 on the main pass (dump draws 39-41 for
            // Yelan, 42-48 for Tranquil) -- GI 6.x swapped it with the light map. A MOD may still
            // write the pre-6.x ps-t0 though, and one of the two downloads does.
            config.faceReg = "ps-t1";
            config.lightMapEdit = MaterialBandRemapFilter::lightMapEdit(Bands);
            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::yelanTranquilToYelan6_1() {
        return makeGIMIMergeFixer(yelanConfig());
    }


    IniFixBuilder::Factory YelanTranquilFixer::toYelan6_1() {
        return IniFixBuilderFuncs::yelanTranquilToYelan6_1();
    }
}
