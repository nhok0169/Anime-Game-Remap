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

#include "AGRemapCore/data/IniFixData/Ayaka/AyakaFixer.h"

#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/TransparencyAdjustFilter.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {

    namespace {
        // 1 / 2.2, spelled as the division so it reads as the sRGB exponent it is -- the same
        // metadata DarkDiffuse sets, and for the same reason: save() applies it through
        // GammaFilter immediately before encoding, which is the only point at which the correction
        // is wanted.
        const double Gamma = 1.0 / 2.2;

        const int NormalMapSize = 1024;

        // AyakaSpringbloom's normal map is a muted purple, Colours.NormalMapPurple1 -- NOT the flat
        // (128, 128, 255) blue most characters use. A normal map is a direction field, so the
        // colour IS the data.
        const Colour NormalMapPurple1(128, 98, 128);

        // Nearly transparent rather than fully: 1 and not 0, for the reason XianglingFixer records
        // -- a 0 can be optimised away by the encoder where a 1 survives the BC7 round trip.
        const int NearlyTransparent = 1;

        // The dress keeps most of its opacity. 177 is the pure-Python row's own number and there is
        // no derivation for it -- it is what looked right to the maintainer.
        const int DressAlpha = 177;

        // A lightmap's alpha is a material parameter, not opacity, and Ayaka's reads brighter than
        // her skin's shader expects. An ADJUSTMENT rather than a set, clamped.
        const int LightMapAlphaChange = -78;


        void makeHeadTransparent(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, NearlyTransparent);
            texFile.setGamma(Gamma);
        }


        void makeDressOpaque(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, DressAlpha);
            texFile.setGamma(Gamma);
        }


        void brightenLightMap(TextureFile& texFile) {
            TransparencyAdjustFilter filter(LightMapAlphaChange);
            filter.transform(texFile);
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::ayaka6_1() {
        // Remapped onto AyakaSpringbloom, a genuinely different model -- see makeGIMICharFixer for
        // what that shape does. Only what Ayaka does differently lives here.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // Registers the skin does not read. The dress has none of its own to lose.
        config.objRegRemovals = {{"head", {"ps-t2"}}, {"body", {"ps-t3"}}};

        // ---- everything shifts UP a slot to meet the skin ----
        //
        // AyakaSpringbloom's head and body read their diffuse from ps-t1 and their lightmap from
        // ps-t2 (see AyakaSpringbloomParser's objDownloadRegs for the same fact from the download
        // side), while Ayaka's sit on ps-t0/ps-t1.
        //
        // ps-t0 keeps a COPY of itself so the slot is still bound when the texAdd below overwrites
        // it -- the same trick Ganyu, Xiangling and KiraraBoots use.
        //
        // No value checks on these, unlike Kirara's: the pure-Python row has none either. Ayaka's
        // is the plain shift, and adding a guard the original does not have would be inventing
        // behaviour rather than porting it.
        config.objRegRemaps = {{"head", {{"ps-t1", {"ps-t2"}}, {"ps-t0", {"ps-t0", "ps-t1"}}}},
                               {"body", {{"ps-t2", {"ps-t3"}}, {"ps-t1", {"ps-t2"}},
                                         {"ps-t0", {"ps-t0", "ps-t1"}}}}};

        // ---- three texture edits, all naming the register the COLLECTORS see ----
        //
        // Which is ps-t0/ps-t1 -- where each texture sits BEFORE the shift above, not where it ends
        // up. Texture edits run before the register edits.
        config.texEdits = {{"head", "ps-t0", "TransparentDiffuse", &makeHeadTransparent},
                           {"body", "ps-t1", "BrightLightMap", &brightenLightMap},
                           {"dress", "ps-t0", "OpaqueDiffuse", &makeDressOpaque}};

        // ...and the normal map the skin reads on ps-t0, which Ayaka never had.
        config.texAdds = {{"head", "ps-t0", "NormalMap", TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)},
                          {"body", "ps-t0", "NormalMap", TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)}};

        // ORFix for the two objects that now carry a normal map; the dress takes the default NNFix.
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath}},
                              {"body", {IniKeywords::ORFixPath}}};

        // The pure-Python row's IbRemapData / IbDrawIndexedRename / IbTempToDrawIndexed plus its
        // postModel drawindexed removal are together what this flag does.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory AyakaFixer::v6_1() {
        return IniFixBuilderFuncs::ayaka6_1();
    }
}
