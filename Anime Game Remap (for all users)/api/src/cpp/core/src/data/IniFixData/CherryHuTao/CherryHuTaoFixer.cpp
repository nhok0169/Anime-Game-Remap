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

#include "AGRemapCore/data/IniFixData/CherryHuTao/CherryHuTaoFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/ColourReplaceFilter.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/InvertAlphaFilter.h"
#include "AGRemapCore/model/textures/Colour.h"
#include "AGRemapCore/model/textures/ColourRange.h"


namespace AGRemapCore {
    namespace {
        // The green a lightmap uses to mark an ordinary, non-emissive surface.
        const Colour LightMapGreen(0, 128, 0, 255);

        // No gamma correction on the way out. Every other texture edit here leaves the metadata
        // alone and lets TextureFile decide; this one sets it to 1 explicitly, which is the
        // identity, because a LIGHTMAP is not a picture -- its channels are material parameters and
        // correcting them the way you would correct a colour makes the numbers wrong.
        const double NoGammaCorrection = 1.0;


        std::vector<std::string> reflectionKeys(const std::string& obj) {
            return {"ResourceRef" + obj + "Diffuse", "ResourceRef" + obj + "LightMap", "$CharacterIB"};
        }


        /**
         * The bands CherryHuTao's body lightmap uses for the emissive parts of her Lantern Rite
         * outfit -- the lantern glow and the two trim colours around it.
         *
         * HuTao's shader has no emission to drive, so those texels are flattened to the ordinary
         * green and the outfit stops glowing through her default one. The alpha floor of 65 on every
         * range is what keeps the match to the emissive texels: below it the channel means something
         * else entirely.
         */
        ColourOrRangeSet emissiveBands() {
            return ColourOrRangeSet{
                ColourRange(Colour(0, 120, 110, 65), Colour(255, 140, 255, 75)),
                ColourRange(Colour(0, 120, 0, 65), Colour(255, 140, 200, 75)),
                ColourRange(Colour(0, 0, 200, 65), Colour(30, 30, 255, 75))};
        }


        void invertAlpha(TextureFile& texFile) {
            InvertAlphaFilter filter;
            filter.transform(texFile);
        }


        void flattenEmission(TextureFile& texFile) {
            texFile.setGamma(NoGammaCorrection);

            ColourReplaceFilter filter(LightMapGreen, emissiveBands());
            filter.transform(texFile);
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::cherryHuTao6_1() {
        // Remapped onto HuTao -- the MERGE that undoes hutao6_1's split, and the most involved fix
        // in the repo.
        //
        // Four of the skin's objects come back through HuTao's two: her head takes the skin's head
        // and its glasses ('extra'), her body the skin's body and its skirt ('dress'). Two sources
        // on each target means TWO .ini files, and the game overlaps them.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress", "extra"};

        // Claim order: the mod's own file gets the head and the body, the generated copy the extra
        // and the dress -- which is the way round the old script's own output has it.
        config.objSplits = {{"head", {"head"}}, {"extra", {"head"}},
                            {"body", {"body"}}, {"dress", {"body"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // ---- the reflection sections ----
        //
        // UNIONED per target rather than kept per source, which is safe in a way the shift below is
        // not: a key that is not in the part is not removed, so handing the head's copy the extra's
        // keys as well costs nothing and reads more simply than four source-keyed entries.
        std::vector<std::string> headRemovals = reflectionKeys("Head");
        for (const std::string& key : reflectionKeys("Extra")) {
            headRemovals.push_back(key);
        }

        std::vector<std::string> bodyRemovals = reflectionKeys("Body");
        for (const std::string& key : reflectionKeys("Dress")) {
            bodyRemovals.push_back(key);
        }

        config.objRegRemovals = {{"head", headRemovals}, {"body", bodyRemovals}};

        // ---- the shift, and why it MUST be source-keyed ----
        //
        // The skin's head and its dress carry a normal map on ps-t0 that HuTao does not read, so
        // both lose it and shift down a slot. Its body and its extra do not -- and they land on the
        // SAME two targets, in the other group.
        //
        // Target-keyed, this would shift the body-sourced and extra-sourced copies as well: their
        // diffuse would move to a slot the shader reads as a normal map and their lightmap would
        // vanish. The .ini file would still look entirely reasonable.
        const std::vector<std::pair<std::string, std::vector<std::string>>> shift = {
            {"ps-t1", {"ps-t0"}}, {"ps-t2", {"ps-t1"}}};

        config.srcObjRegRemovals = {{"head", {"ps-t0"}}, {"dress", {"ps-t0"}}};
        config.srcObjRegRemaps = {{"head", shift}, {"dress", shift}};

        // ---- the three texture edits ----
        //
        // All three name the register the texture sits on BEFORE the shift, since the collectors run
        // first -- so the dress's diffuse is named at ps-t1 even though it ends up on ps-t0.
        //
        // Each is source-keyed. The dress's diffuse edit and the body's lightmap edit are both
        // "target body, ps-t1", and only the srcObj tells them apart.
        config.texEdits = {
            {"body", "ps-t0", "TransparentBodyDiffuse", &invertAlpha, true, "body"},
            {"body", "ps-t1", "OpaqueBodyLightMap", &flattenEmission, true, "body"},
            {"body", "ps-t1", "TransparentyDressDiffuse", &invertAlpha, true, "dress"}};

        // NNFix on both targets, and TexFx naming ps-t0 as the diffuse's home once the shift has put
        // it there (TN.0, not TN.1). TexFx is opt-in and adds nothing to a mod that never bound
        // ps-t69 or ps-t70.
        config.objFixCalls = {{"head", {IniKeywords::NNFixPath, IniKeywords::TexFxTransparency0}},
                              {"body", {IniKeywords::NNFixPath, IniKeywords::TexFxTransparency0}}};

        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries.
        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory CherryHuTaoFixer::v6_1() {
        return IniFixBuilderFuncs::cherryHuTao6_1();
    }
}
