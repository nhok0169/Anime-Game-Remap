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

#include "AGRemapCore/data/IniFixData/NilouBreeze/NilouBreezeFixer.h"

#include <utility>

#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/textures/Colour.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include <vector>
#include <string>
#include "AGRemapCore/constants/IniKeywords.h"


namespace AGRemapCore {
    namespace {
        // NilouBreeze's 5.4 normal map is a DIFFERENT purple from Ayaka's -- (128, 114, 128)
        // rather than (128, 98, 128). Colours.NormalMapPurple2, not Purple1.
        const int NormalMapSize5_4 = 1024;
        const Colour NormalMapPurple2(128, 114, 128);
        // The flat normal map these rows invent, matching TexCreator(1024, 1024,
        // colour = Colours.NormalMapYellow) exactly.
        const int NormalMapSize4x = 1024;
        const Colour NormalMapYellow4x(128, 128, 0);
        // The 3dmigoto reflection-support keys. They name the SOURCE's texture slots, so
        // carrying them into a remapped section aims the reflection pass at the wrong textures.
        std::vector<GIMICharFixerConfig::RegRef> reflectionKeys(const std::string& obj) {
            return {"ResourceRef" + obj + "Diffuse", "ResourceRef" + obj + "LightMap", "$CharacterIB"};
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::nilouBreeze5_7() {
        // THE 5.7 FIX for NilouBreeze -> Nilou, verified only against the old script at
        // --version 5.7 --fromVersion 5.7 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // Three removals, and that is the entire row. Everything nilouBreeze5_4 does -- the
        // shift, the purple normal map, ORFix, TexFx -- is gone by 5.7.
        config.objRegRemovals = {{"head", {"ps-t3"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t3"}}};

        // ---- the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;

        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::nilouBreeze5_4() {
        // THE 5.4 FIX for NilouBreeze -> Nilou, verified only against the old script at
        // --version 5.4 --fromVersion 5.4 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        config.objRegRemovals = {{"head", {"ps-t3"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t3"}}};

        config.objRegRemaps = {{"head", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t2", {{"ps-t3"}}, true}}},
                               {"body", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t2", {{"ps-t3"}}, true}}},
                               {"dress", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                          {"ps-t1", {{"ps-t2"}}, true},
                                          {"ps-t2", {{"ps-t3"}}, true}}}};

        config.texAdds = {{"head", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize5_4, NormalMapSize5_4, NormalMapPurple2)},
                          {"body", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize5_4, NormalMapSize5_4, NormalMapPurple2)},
                          {"dress", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize5_4, NormalMapSize5_4, NormalMapPurple2)}};

        // ---- the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        config.objFixCalls = {{"head", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}},
                              {"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}},
                              {"dress", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::nilouBreeze4_8() {
        // THE 4.8 FIX for NilouBreeze -> Nilou, verified only against the old script at
        // --version 4.8 --fromVersion 4.8 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // All three the same: drop ps-t3, then make room at ps-t0 for a normal map by
        // duplicating ps-t0 onto ps-t1 and shifting the rest down.
        config.objRegRemovals = {{"head", {"ps-t3"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t3"}}};

        config.objRegRemaps = {{"head", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t2", {{"ps-t3"}}, true}}},
                               {"body", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t2", {{"ps-t3"}}, true}}},
                               {"dress", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                          {"ps-t1", {{"ps-t2"}}, true},
                                          {"ps-t2", {{"ps-t3"}}, true}}}};

        config.texAdds = {{"head", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize4x, NormalMapSize4x, NormalMapYellow4x)},
                          {"body", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize4x, NormalMapSize4x, NormalMapYellow4x)},
                          {"dress", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize4x, NormalMapSize4x, NormalMapYellow4x)}};

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // The external libraries this row re-issues, keyed by TARGET. An empty list means
        // none, and REPLACES the template's default NNFix.
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1Pre5_0}},
                              {"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1Pre5_0}},
                              {"dress", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1Pre5_0}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::nilouBreeze6_1() {
        // Remapped onto Nilou, a genuinely different model -- see makeGIMICharFixer for what that
        // shape does. Only what NilouBreeze does differently lives here.
        //
        // The plainest fix in this batch, and the counterpart to her base's: Nilou -> NilouBreeze
        // re-issues ORFix and therefore needs no 6.1 row, while this direction re-issues NNFix and
        // is registered at 6.1 precisely because NNFix is what GI 6.1 introduced.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // Nilou does not read ps-t3 on any of the three. NilouBreeze binds one, and a register the
        // target's shader never samples is at best ignored and at worst read as something else.
        config.objRegRemovals = {{"head", {"ps-t3"}}, {"body", {"ps-t3"}}, {"dress", {"ps-t3"}}};

        // No objFixCalls: every object re-issues NNFix, which is makeGIMICharFixer's default. The
        // pure-Python row says the same thing the long way round, threading a 'tempNNFix' register
        // through a remap and renaming it to 'run' -- the template owns that now, including
        // stripping the mod's own ORFix/NNFix calls first (the row's ORFixCompleteRemoval).

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory NilouBreezeFixer::v6_1() {
        return IniFixBuilderFuncs::nilouBreeze6_1();
    }
}
