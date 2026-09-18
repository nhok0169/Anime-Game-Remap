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
        // The flat normal map a 4.0-era row invents, matching that row's
        // TexCreator(1024, 1024, colour = Colours.NormalMapYellow) exactly.
        const int NormalMapSize4_0 = 1024;
        const Colour NormalMapYellow4_0(128, 128, 0);
    }


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


    IniFixBuilder::Factory IniFixBuilderFuncs::ayaka5_7() {
        // THE 5.7 FIX for Ayaka -> AyakaSpringbloom, verified only against the old script at
        // --version 5.7 --fromVersion 5.7 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        // head, body AND DRESS: drawnObjs is the PARSE row's object set
        // ([{"head", "body", "dress"}]), not the list of objects the fixer row happens to
        // name in its removals. Reading the fixer row alone drops her dress remap entirely.
        config.drawnObjs = {"head", "body", "dress"};

        // NO REGISTER SHIFT, and that is the whole difference from ayaka5_6. By 5.7 the normal
        // map binds straight at ps-t0 with no ps-t0 -> ps-t1 duplication; ayaka6_1 then puts the
        // shift BACK, so interpolating between the neighbours gives the wrong row.
        //
        // NO TexFx EITHER: ayaka5_7 drops the TexFxRemove and the re-issue that ayaka5_6 has.
        config.objRegRemovals = {{"head", {"ps-t2"}}, {"body", {"ps-t3"}}};

        config.texEdits = {{"head", "ps-t0", "TransparentDiffuse", &makeHeadTransparent},
                           {"body", "ps-t1", "BrightLightMap", &brightenLightMap},
                           {"dress", "ps-t0", "OpaqueDiffuse", &makeDressOpaque}};

        config.texAdds = {{"head", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)},
                          {"body", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)}};

        // ---- the 6.1-era defaults, and 5.x's own draw-call move ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;

        // IbRemapData + IbDrawIndexedRename + IbTempToDrawIndexed + the postModel
        // drawindexed removal, all four of which this flag is.
        config.moveDrawIndexed = true;

        config.objFixCalls = {{"head", {IniKeywords::ORFixPath}},
                              {"body", {IniKeywords::ORFixPath}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::ayaka5_6() {
        // THE 5.6 FIX for Ayaka -> AyakaSpringbloom, verified only against the old script at
        // --version 5.6 --fromVersion 5.6 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        // head, body AND DRESS: drawnObjs is the PARSE row's object set
        // ([{"head", "body", "dress"}]), not the list of objects the fixer row happens to
        // name in its removals. Reading the fixer row alone drops her dress remap entirely.
        config.drawnObjs = {"head", "body", "dress"};

        config.objRegRemovals = {{"head", {"ps-t2"}}, {"body", {"ps-t3"}}};

        config.objRegRemaps = {{"head", {{"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true}}},
                               {"body", {{"ps-t2", {{"ps-t3"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true}}}};

        config.texEdits = {{"head", "ps-t0", "TransparentDiffuse", &makeHeadTransparent},
                           {"body", "ps-t1", "BrightLightMap", &brightenLightMap},
                           {"dress", "ps-t0", "OpaqueDiffuse", &makeDressOpaque}};

        // PURPLE from 5.6 on, where ayaka4_0 invents a yellow one.
        config.texAdds = {{"head", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)},
                          {"body", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)}};

        // ---- the 6.1-era defaults, and 5.x's own draw-call move ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // IbRemapData + IbDrawIndexedRename + IbTempToDrawIndexed + the postModel
        // drawindexed removal, all four of which this flag is.
        config.moveDrawIndexed = true;

        config.objFixCalls = {{"head", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}},
                              {"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }
    IniFixBuilder::Factory IniFixBuilderFuncs::ayaka5_4() {
        // THE 5.4 FIX for Ayaka -> AyakaSpringbloom, verified only against the old script at
        // --version 5.4 --fromVersion 5.4 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        // head, body AND DRESS: drawnObjs is the PARSE row's object set
        // ([{"head", "body", "dress"}]), not the list of objects the fixer row happens to
        // name in its removals. Reading the fixer row alone drops her dress remap entirely.
        config.drawnObjs = {"head", "body", "dress"};

        config.objRegRemovals = {{"head", {"ps-t2"}}, {"body", {"ps-t3"}}};

        config.objRegRemaps = {{"head", {{"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true}}},
                               {"body", {{"ps-t2", {{"ps-t3"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true}}}};

        config.texEdits = {{"head", "ps-t0", "TransparentDiffuse", &makeHeadTransparent},
                           {"body", "ps-t1", "BrightLightMap", &brightenLightMap},
                           {"dress", "ps-t0", "OpaqueDiffuse", &makeDressOpaque}};

        // PURPLE from 5.6 on, where ayaka4_0 invents a yellow one.
        config.texAdds = {{"head", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)},
                          {"body", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapPurple1)}};

        // ---- the 6.1-era defaults, and 5.x's own draw-call move ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // The draw call does not move until 5.6, so this row does NOT set
        // moveDrawIndexed. That flag is the only difference between it and ayaka5_6.

        config.objFixCalls = {{"head", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}},
                              {"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::ayaka4_0() {
        // THE 4.0 FIX for Ayaka -> AyakaSpringbloom, verified only against the old script at
        // --version 4.0 --fromVersion 4.0 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        // head, body AND DRESS: drawnObjs is the PARSE row's object set
        // ([{"head", "body", "dress"}]), not the list of objects the fixer row happens to
        // name in its removals. Reading the fixer row alone drops her dress remap entirely.
        config.drawnObjs = {"head", "body", "dress"};

        config.objRegRemovals = {{"head", {"ps-t2"}}, {"body", {"ps-t3"}}};

        config.objRegRemaps = {{"head", {{"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true}}},
                               {"body", {{"ps-t2", {{"ps-t3"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true}}}};

        // Three edits, all named separately by the pure-Python row's single RegTexEdit.
        // One edit per OBJECT, as ayaka4_0's PARSE row declares them -- head/ps-t0,
        // body/ps-t1, dress/ps-t0. The fix row's RegTexEdit names only the registers; which
        // object owns each edit is the parser's half of the pair, and reading only the fixer
        // row puts all three on the head.
        config.texEdits = {{"head", "ps-t0", "TransparentDiffuse", &makeHeadTransparent},
                           {"body", "ps-t1", "BrightLightMap", &brightenLightMap},
                           {"dress", "ps-t0", "OpaqueDiffuse", &makeDressOpaque}};

        config.texAdds = {{"head", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize4_0, NormalMapSize4_0, NormalMapYellow4_0)},
                          {"body", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize4_0, NormalMapSize4_0, NormalMapYellow4_0)}};

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;
        //   ^ no ORFix/NNFix entry in its removal set, so the mod's own survive.
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // The external libraries this row re-issues, keyed by TARGET. An empty list means
        // none, and REPLACES the template's default NNFix.
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1Pre5_0}},
                              {"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1Pre5_0}}};

        return makeGIMICharFixer(std::move(config));
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
