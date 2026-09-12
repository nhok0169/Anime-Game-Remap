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
