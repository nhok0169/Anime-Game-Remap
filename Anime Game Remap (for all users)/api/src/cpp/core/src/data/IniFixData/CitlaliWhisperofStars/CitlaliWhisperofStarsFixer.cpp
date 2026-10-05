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

#include "AGRemapCore/data/IniFixData/CitlaliWhisperofStars/CitlaliWhisperofStarsFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"


namespace AGRemapCore {
    namespace {
        // ---- the band legend, skin -> Citlali ----
        //
        // A light map's alpha is a material band. Measured off the two identity mods, an alpha
        // histogram per light map with the mean diffuse under each band:
        //
        //   CitlaliWhisperofStars  126-127 and 177-178 both carry SKIN on the body slots
        //   Citlali                255 is her skin band
        //
        // Both of the skin's skin bands move to 255 and the rest already line up. Gated on the
        // diffuse under the pixel for the reason the other two directions found the hard way: a mod
        // need not follow its character's legend at all, because a PORT keeps its SOURCE
        // character's bands, and an ungated move would repaint a ported mod's cloth as flesh.
        //
        // Applied SIMULTANEOUSLY by the filter -- every decision from the ORIGINAL alpha -- which
        // these two need like any permutation.
        const std::vector<MaterialBandRemapFilter::Band> Bands = {
            {177, 178, 255, &MaterialBandRemapFilter::skinColoured},
            {126, 127, 255, &MaterialBandRemapFilter::skinColoured},
        };


        GIMIMergeFixerConfig citlaliConfig() {
            // CitlaliWhisperofStars -> Citlali: the third remap of a skin of SEVERAL components
            // onto a target of one, and the inverse of the Citlali -> CitlaliWhisperofStars rows.
            // Every value here was read off the prototype
            // (Tools/Misc/Prototypes/citlaliFromWhisperFix.py), which stays the oracle -- it is the
            // only thing that can tell a transcription error here from a template gap.
            //
            // WHICH SOURCE SLOT LANDS ON WHICH OBJECT: all six land on `body`, including the Bangs,
            // and Citlali's `head` receives nothing. That is not an oversight. The slot was placed
            // by SHADER FAMILY first and bones second (the fringe weights 35% to head-only bones,
            // which argued for the head and was wrong in game): the skin draws its Bangs on the
            // body's shader pass, and sending it to the head drew it with the head's registers.
            GIMIMergeFixerConfig config{};

            // The trailing number of each slot is the GAME model's index count, off the download
            // .ib files (R32, so bytes / 4) -- the fallback for a slot whose ib is downloaded, read
            // only for a target object several slots merge onto. Which is all of them here.
            //
            // `outline` is the last field, and false keeps a slot OUT of Citlali's outline pass.
            // The skin outlines its dress -- Body B and C -- with outline shaders nothing else of
            // hers uses (077848d1 / 57c73a33 in the 6.7 dump, where Body A, D and the Bangs share
            // Citlali's 67fd126e). Through Citlali's own outline shader the hull of the skirt's far
            // panel covered the lining in black and showed as a dark edge by the hair: the identity
            // mod's dump had that draw write 4890 pixels outside the silhouette and turn 7141
            // inside it near-black, and gating exactly these two cleared both in game.
            GIMIMergeFixerConfig::Component body{};
            body.name = "Body";
            body.slots = {{"A", "0", "body", true, "", 60888, true},
                          {"B", "60888", "body", true, "", 50208, false},
                          {"C", "111096", "body", true, "", 11820, false},
                          {"D", "122916", "body", false, "", 1935, true}};

            // The GAME model's vertex count, for a mod that does not carry the component at all --
            // download Blend.buf bytes / 32.
            body.vertexCount = 37631;

            // No textures of their own: the game draws both with the Body slot A set (they share
            // its hashes), so a mod may leave these sections with an ib and nothing else.
            GIMIMergeFixerConfig::Component bangs{};
            bangs.name = "Bangs";
            bangs.slots = {{"A", "0", "body", true, "Body;A", 11328, true}};
            bangs.vertexCount = 3210;

            GIMIMergeFixerConfig::Component eyes{};
            eyes.name = "Eyes";
            eyes.slots = {{"A", "0", "body", true, "Body;A", 864, true}};
            eyes.vertexCount = 255;

            config.components = {std::move(body), std::move(bangs), std::move(eyes)};
            config.targetObjs = {"head", "body"};

            // Citlali reads normal map / diffuse / light map at ps-t0/1/2 under ORFix -- unlike
            // Yelan and Bennett, whose targets are the two-register NNFix layout. Without this the
            // template drops every normal map the skin's mods carry and issues NNFix on a shader
            // that wants ORFix.
            config.targetLayout = GIMIMergeFixerConfig::TargetLayout::NormalMap;

            // AND EVERY CARRIED BINDING ONTO THE REGISTER ITS NAME SAYS.
            //
            // The skin's mods bind (light map, normal map, diffuse) at ps-t0/1/2 -- the order the
            // GAME's own draw of that ib binds, read off a frame dump -- while ORFix reads the
            // normal map out of ps-t0, the diffuse out of ps-t1 and the light map out of ps-t2.
            // Carried across unchanged and handed to ORFix, every role comes out of the wrong slot:
            // one mod's shoes, eyes and sleeping mask were wrong in game while every slot the fix
            // had downloaded and bound itself was right. Citlali's OWN mods are written in ORFix's
            // order, which is why the forward direction never needed this.
            config.texRegsByName = true;

            // A mod carrying none of a component merges it from its downloads.
            config.downloadPrefix = "CitlaliWhisperofStars";

            // Citlali's identity mod binds her face diffuse (9fb78572) at ps-t1, the GI 6.x layout.
            // A face draw takes no fix library call at all -- the merge strips any the mod carried.
            config.faceReg = "ps-t1";
            config.lightMapEdit = MaterialBandRemapFilter::lightMapEdit(Bands);

            // NOT BC7: the alpha being edited is a band SELECTOR, and BC7 blurs a band across its
            // boundary while a band is read as an exact value.
            config.compressTextures = false;
            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::citlaliWhisperofStarsToCitlali6_7() {
        return makeGIMIMergeFixer(citlaliConfig());
    }


    IniFixBuilder::Factory CitlaliWhisperofStarsFixer::toCitlali6_7() {
        return IniFixBuilderFuncs::citlaliWhisperofStarsToCitlali6_7();
    }
}
