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

#include "AGRemapCore/data/IniFixData/GanyuTwilight/GanyuTwilightFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {

    IniFixBuilder::Factory IniFixBuilderFuncs::ganyuTwilight6_1() {
        // Remapped onto Ganyu -- and this is the direction that LOSES a normal map.
        //
        // GanyuTwilight is a 4.4-era model, from after GI 3.x gave characters a normal map:
        // ps-t0 normal map, ps-t1 diffuse, ps-t2 lightmap. Ganyu predates that and reads
        // ps-t0 diffuse, ps-t1 lightmap. So the fix drops the normal map and shifts the other two
        // down a slot. See GIMICharFixerConfig::objRegRemaps.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // The head's normal map goes -- Ganyu has no slot for it -- along with the reflection
        // resources and the shared IB reference, exactly as the pure-Python row's
        // ReflectionHeadRemove/ReflectionBodyRemove/ReflectionDressRemove sets do.
        config.objRegRemovals = {
            {"head", {"ps-t0", "ResourceRefHeadDiffuse", "ResourceRefHeadLightMap", "$CharacterIB"}},
            {"body", {"ResourceRefBodyDiffuse", "ResourceRefBodyLightMap", "$CharacterIB"}},
            {"dress", {"ResourceRefDressDiffuse", "ResourceRefDressLightMap", "$CharacterIB"}}};

        // ...and what is left slides down. Both renames are in ONE entry so they are applied in a
        // single pass -- ps-t1 -> ps-t0 must not then be re-read as the input to ps-t2 -> ps-t1.
        config.objRegRemaps = {{"head", {{"ps-t1", {"ps-t0"}}, {"ps-t2", {"ps-t1"}}}}};

        // NNFix rather than ORFix on the head, because the target has no normal map -- ORFix is the
        // normal-map one. TexFx is re-issued alongside it, naming ps-t0 as the diffuse's home now
        // that the shift has put it there (TN.0 rather than TN.1).
        config.objFixCalls = {{"head", {IniKeywords::NNFixPath, IniKeywords::TexFxTransparency0}}};

        // body and dress are not listed, so they take the default: NNFix alone.

        // The pure-Python row carries IbRemapData/IbDrawIndexedRename on all three objects plus a
        // postModel drawindexed removal, which together are what this flag does.
        config.moveDrawIndexed = true;

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory GanyuTwilightFixer::v6_1() {
        return IniFixBuilderFuncs::ganyuTwilight6_1();
    }
}
