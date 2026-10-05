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

#include "AGRemapCore/data/IniFixData/Kaeya/KaeyaFixer.h"

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {

    namespace {
        // Colours.NormalMapYellow -- the flat yellow Ganyu and KiraraBoots are given too, at the
        // 1024x1024 every RegTexAdd in the pure-Python tables uses.
        const int NormalMapSize = 1024;
        const Colour NormalMapYellow(128, 128, 0);
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::kaeya4_0() {
        // THE 4.0 FIX for Kaeya -> KaeyaSailwind, verified only against the old script at
        // --version 4.0 --fromVersion 4.0 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        config.objRegRemaps = {{"body", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t2", {{"ps-t3"}}, true}}}};

        config.texAdds = {{"body", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapYellow)}};

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;
        //   ^ no ORFix/NNFix entry in its removal set, so the mod's own survive.
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // The external libraries this row re-issues, keyed by TARGET. An empty list means
        // none, and REPLACES the template's default NNFix.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1Pre5_0}},
                              {"dress", std::vector<std::string>{}},
                              {"extra", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }
    IniFixBuilder::Factory IniFixBuilderFuncs::kaeya5_0() {
        // THE 5.0 FIX, and it is kaeya4_0 with ONE change: TexFx renamed its
        // transparency sub-commands at 5.0, T.0/T.1 becoming the Natlan TN.0/TN.1. That
        // rename is the entire difference between the pure-Python rows' ...4_0 and ...5_0
        // value-rename constants, and issuing the wrong one names a sub-command that
        // version of the library does not have -- which shows up as an effect that
        // quietly does nothing.
        //
        // kaeya5_0 and kaeya4_0 are otherwise character-for-character identical.
        //
        // Originally for Kaeya -> KaeyaSailwind, verified only against the old script at
        // --version 4.0 --fromVersion 4.0 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body"};

        config.objRegRemaps = {{"body", {{"ps-t0", {{"ps-t0"}, {"ps-t1"}}, true},
                                         {"ps-t1", {{"ps-t2"}}, true},
                                         {"ps-t2", {{"ps-t3"}}, true}}}};

        config.texAdds = {{"body", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapYellow)}};

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = false;
        //   ^ no ORFix/NNFix entry in its removal set, so the mod's own survive.
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // The external libraries this row re-issues, keyed by TARGET. An empty list means
        // none, and REPLACES the template's default NNFix.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}},
                              {"dress", std::vector<std::string>{}},
                              {"extra", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::kaeya6_1ToKaeyaSailwind() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // NO objSplits: this direction is one-to-one, the only pair in this batch that is.
        // kaeya6_1 is a GIMIObjRegEditFixer, not a split or a merge.

        // ---- THE BODY GAINS A NORMAL MAP ----
        //
        // KaeyaSailwind's body reads ps-t0 normal map, ps-t1 diffuse, ps-t2 lightmap; Kaeya's reads
        // ps-t0 diffuse, ps-t1 lightmap, ps-t2 shadow. So the body shifts up a slot and the vacated
        // ps-t0 is filled with an invented flat normal map -- the same thing Ganyu -> GanyuTwilight
        // does, and for the same reason.
        //
        // TWO TARGETS ON ps-t0 is what makes room for it: the diffuse lands on ps-t1 where the
        // target reads it AND stays on ps-t0 for the texAdd below to overwrite. Both renames of the
        // object in ONE entry, so ps-t0 -> ps-t1 is not re-read as the input to ps-t1 -> ps-t2.
        //
        // head and dress are not listed: only the body has a normal map to gain.
        config.objRegRemaps = {{"body", {{"ps-t0", {"ps-t0", "ps-t1"}}, {"ps-t1", {"ps-t2"}},
                                         {"ps-t2", {"ps-t3"}}}}};

        config.texAdds = {{"body", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapYellow)}};

        // ORFix on the body because it now HAS a normal map -- ORFix is the normal-map library --
        // and TexFx alongside, naming ps-t1 as the diffuse's home now that the shift has put it
        // there (TN.1, which is what IniKeywords::TexFxTransparency1 spells). head and dress take
        // the default, NNFix alone.
        config.objFixCalls = {{"body", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory KaeyaFixer::v6_1ToKaeyaSailwind() {
        return IniFixBuilderFuncs::kaeya6_1ToKaeyaSailwind();
    }
}
