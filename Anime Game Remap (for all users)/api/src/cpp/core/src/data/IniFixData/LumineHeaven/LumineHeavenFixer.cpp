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

#include "AGRemapCore/data/IniFixData/LumineHeaven/LumineHeavenFixer.h"

#include <utility>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"


namespace AGRemapCore {
    namespace {
        GIMIMergeFixerConfig lumineConfig() {
            // LumineHeaven -> Lumine: the seventh remap of a skin of SEVERAL components onto a target of one, and the
            // inverse of the Lumine -> LumineHeaven rows. Every value here was read off the prototype
            // (Tools/Misc/Prototypes/lumineFromHeavenFix.py), which stays the oracle, confirmed in game on the skin's
            // identity mod and its three real mods.
            //
            // WHERE EACH SLOT LANDS: the Bang and the Eye onto her head, the main Head and the Body onto her BODY; her
            // dress receives nothing, and the whole-ib skip keeps her own dress hidden. The main Head is NOT on her head
            // although its textures are the Head set's: that slot carries the skin's sleeves, neck scarf and bow (band 0
            // of its light map) beside the back hair, and her head draw shades everything as HAIR -- in the overworld's
            // shade the white cloth took her hair's warm shadow, yellow beside the cool white of the dress
            // (LumineHeaven2, 2026-09-30). On her body draw the cloth shades as her own white cloth, and the back hair
            // looked the same both ways in sun and shade. A member binds its own textures, so the Head set's still go
            // with it. The trailing numbers are the GAME model's index count per slot, off the download folder's index
            // buffers. Merge order: the biggest first.
            GIMIMergeFixerConfig config{};

            GIMIMergeFixerConfig::Component main{};
            main.name = "";
            main.modTypeName = ModTypeIdTools::getName(ModTypeId::LumineHeavenMain);
            main.slots = {{"Head", "0", "body", true, "", 57141, true},
                          {"Body", "57141", "body", true, "", 47298, true}};
            main.vertexCount = 31179;

            GIMIMergeFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.modTypeName = ModTypeIdTools::getName(ModTypeId::LumineHeavenBang);
            bang.slots = {{"A", "0", "head", true, ";Head", 9096, true}};
            bang.vertexCount = 2740;

            // The Eye with its OWN textures (the skin's iris atlas), onto her head.
            GIMIMergeFixerConfig::Component eye{};
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::LumineHeavenEye);
            eye.slots = {{"A", "0", "head", false, "", 720, true}};
            eye.vertexCount = 246;

            config.components = {std::move(main), std::move(bang), std::move(eye)};
            config.targetObjs = {"head", "body", "dress"};

            // She draws every object on the PLAIN shader (diffuse, light map, no normal map), under NNFix.
            config.targetLayout = GIMIMergeFixerConfig::TargetLayout::Plain;

            // Every carried binding onto the register its resource NAME says.
            config.texRegsByName = true;

            // A mod carrying none of a component merges it from its downloads.
            config.downloadPrefix = "LumineHeaven";

            // Lumine's Texcoord is 20 bytes a vertex (a second UV set); every one of the skin's components carries 12.
            // The merge pads each line at its end up to her width, and the copied section declares it.
            config.texcoordStride = 20;

            // The face graph is left alone: the two skins draw DIFFERENT face meshes, and a face atlas does not carry
            // across them -- see LumineFixer. On Lumine her own face is drawn.
            config.faceReg = "";

            // No texture edits: the skin's hair on 126-128 and its head alpha ~0 render as hair on her head shader --
            // neither moved to her 255 changed anything the eye could tell from the pose (2026-09-29).
            config.diffuseEdits = {};
            config.lightMapEdit = nullptr;
            config.compressTextures = false;

            // A mod hiding the skin's face meshes by hash hides hers too.
            config.sideMeshes = {"ib_face", "ib_headupper"};

            // Her dress is never reached, and may still hold a TexFx request.
            config.texFxGuardUnreached = true;

            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::lumineHeavenToLumine6_3() {
        return makeGIMIMergeFixer(lumineConfig());
    }


    IniFixBuilder::Factory LumineHeavenFixer::toLumine6_3() {
        return IniFixBuilderFuncs::lumineHeavenToLumine6_3();
    }
}
