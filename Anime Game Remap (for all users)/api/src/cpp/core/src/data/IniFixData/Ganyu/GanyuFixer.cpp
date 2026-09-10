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

#include "AGRemapCore/data/IniFixData/Ganyu/GanyuFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/DarkDiffuse.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {
    namespace {
        // The flat normal map a character with none of her own is given -- 1024x1024 of the yellow
        // that dominates a real one, matching the pure-Python row's
        // TexCreator(1024, 1024, colour = Colours.NormalMapYellow) exactly.
        //
        // Flat rather than derived from the model: nothing in the mod carries the normal data to
        // derive it FROM. What this buys is a slot the shader can read that does not perturb the
        // lighting, which is the whole requirement.
        const int NormalMapSize = 1024;
        const Colour NormalMapYellow(128, 128, 0);
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::ganyu6_1() {
        // Remapped onto GanyuTwilight -- and this is the direction that GAINS a normal map, the
        // exact mirror of ganyuTwilight6_1.
        //
        // Ganyu predates GI 3.x's normal maps and reads ps-t0 diffuse, ps-t1 lightmap.
        // GanyuTwilight is a 4.4-era model and reads ps-t0 normal map, ps-t1 diffuse, ps-t2
        // lightmap. So the fix shifts both textures up a slot and invents the normal map that
        // belongs in the hole it leaves.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // TWO TARGETS ON ps-t0, and that duplication is what makes room for the normal map: the
        // diffuse lands on ps-t1 where the target reads it AND stays on ps-t0, where the texAdd
        // below overwrites it. Writing only {"ps-t0", {"ps-t1"}} leaves ps-t0 unbound in a part the
        // texAdd never reaches, and the head renders with whatever the game had there.
        //
        // Both renames in ONE entry so they are applied in a single pass -- ps-t0 -> ps-t1 must not
        // then be re-read as the input to ps-t1 -> ps-t2.
        config.objRegRemaps = {{"head", {{"ps-t0", {"ps-t0", "ps-t1"}}, {"ps-t1", {"ps-t2"}}}}};

        // ORFix rather than NNFix on the head, because the target HAS a normal map -- ORFix is the
        // normal-map one, and this is the one line that differs from ganyuTwilight6_1's mirror
        // image of it. TexFx is re-issued alongside, naming ps-t1 as the diffuse's home now that
        // the shift has put it there (TN.1 rather than TN.0).
        //
        // body and dress are not listed, so they take the default: NNFix alone. They keep their
        // registers where they are -- only the head gains a normal map.
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}}};

        // The head's diffuse is darkened on the way over. Declared against ps-t0, which is where it
        // sits when the collectors run -- they go before the register edits, so this names the
        // register BEFORE the shift, not the ps-t1 the pure-Python row names after it.
        config.texEdits = {{"head", "ps-t0", "DarkDiffuse", &DarkDiffuse::edit}};

        // ...and the normal map that fills the slot the shift vacated.
        config.texAdds = {{"head", "ps-t0", "NormalMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapYellow)}};

        // The pure-Python row carries IbRemapData/IbDrawIndexedRename on all three objects plus a
        // postModel drawindexed removal, which together are what this flag does.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory GanyuFixer::v6_1() {
        return IniFixBuilderFuncs::ganyu6_1();
    }
}
