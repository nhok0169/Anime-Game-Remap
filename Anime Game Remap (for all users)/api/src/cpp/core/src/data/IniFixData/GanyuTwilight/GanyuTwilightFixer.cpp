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
    namespace {
        // The 3dmigoto reflection-support keys. They name the SOURCE's texture slots, so
        // carrying them into a remapped section aims the reflection pass at the wrong textures.
        std::vector<GIMICharFixerConfig::RegRef> reflectionKeys(const std::string& obj) {
            return {"ResourceRef" + obj + "Diffuse", "ResourceRef" + obj + "LightMap", "$CharacterIB"};
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::ganyuTwilight5_7() {
        // THE 5.7 FIX for GanyuTwilight -> Ganyu, verified only against the old script at
        // --version 5.7 --fromVersion 5.7 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // Only the head loses its normal map; body and dress lose the reflection keys alone.
        // NO SHIFT, unlike ganyuTwilight4_4 -- by 5.7 the registers stay where they are.
        std::vector<GIMICharFixerConfig::RegRef> headRem = reflectionKeys("Head");
        headRem.push_back({"ps-t0"});

        config.objRegRemovals = {{"head", headRem},
                                 {"body", reflectionKeys("Body")},
                                 {"dress", reflectionKeys("Dress")}};

        // ---- the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = true;
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match
        config.moveDrawIndexed = true;

        config.objFixCalls = {{"head", {IniKeywords::TexFxTransparency0}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::ganyuTwilight4_4() {
        // THE 4.4 FIX for GanyuTwilight -> Ganyu, verified only against the old script at
        // --version 4.4 --fromVersion 4.4 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // Only the HEAD loses its normal map and shifts. Body and dress lose the reflection
        // keys and nothing else -- easy to make symmetric by eye and wrong.
        std::vector<GIMICharFixerConfig::RegRef> headRem = reflectionKeys("Head");
        headRem.push_back({"ps-t0"});

        config.objRegRemovals = {{"head", headRem},
                                 {"body", reflectionKeys("Body")},
                                 {"dress", reflectionKeys("Dress")}};

        config.objRegRemaps = {{"head", {{"ps-t1", {{"ps-t0"}}, true}, {"ps-t2", {{"ps-t1"}}, true}}}};

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = true;
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // The external libraries this row re-issues, keyed by TARGET. An empty list means
        // none, and REPLACES the template's default NNFix.
        config.objFixCalls = {{"head", {IniKeywords::TexFxTransparency0Pre5_0}},
                              {"body", std::vector<std::string>{}},
                              {"dress", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

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
