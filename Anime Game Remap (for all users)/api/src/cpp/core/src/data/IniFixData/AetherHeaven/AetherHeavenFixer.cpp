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

#include "AGRemapCore/data/IniFixData/AetherHeaven/AetherHeavenFixer.h"

#include <utility>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"


namespace AGRemapCore {
    namespace {
        GIMIMergeFixerConfig aetherConfig() {
            // AetherHeaven -> Aether: a skin of SEVERAL components onto a target of one, the inverse of the
            // Aether -> AetherHeaven rows, and LumineHeaven -> Lumine with one target object fewer (he has no dress).
            // Every value here was read off the prototype (Tools/Misc/Prototypes/aetherFromHeavenFix.py), which
            // stays the oracle.
            //
            // WHERE EACH SLOT LANDS: the Bang and the Eye onto his head, the main Head and the Body onto his BODY, and
            // the HAIR of the main Head onto his head (LumineHeaven's lesson: his head draw shades everything as hair,
            // his body draw everything as cloth, and the skin's Head slot holds both). The trailing numbers are the
            // GAME model's index count per slot, off the download folder's index buffers. Merge order: the biggest
            // first.
            GIMIMergeFixerConfig config{};

            GIMIMergeFixerConfig::Component main{};
            main.name = "";
            main.modTypeName = ModTypeIdTools::getName(ModTypeId::AetherHeavenMain);
            main.slots = {{"Head", "0", "body", true, "", 31407, true},
                          {"Body", "31407", "body", true, "", 61062, true}};

            // The skin's hair in its Head slot is band 0 (3961 of its own 10469 head triangles, all but 4 on the
            // braid / head bones); the cloth and legs there sit on 71 and above. 0-40 takes the hair with a margin.
            // See Slot::splitFrom.
            GIMIMergeFixerConfig::Slot hair{"Head#hair", "0", "head", true, "", 0, true};
            hair.splitFrom = "Head";
            hair.splitBands = {{0, 40}};
            main.slots.push_back(std::move(hair));
            main.vertexCount = 26421;

            GIMIMergeFixerConfig::Component bang{};
            bang.name = "Bang";
            bang.modTypeName = ModTypeIdTools::getName(ModTypeId::AetherHeavenBang);
            bang.slots = {{"A", "0", "head", true, ";Head", 12087, true}};
            bang.vertexCount = 3193;

            // The Eye with its OWN textures (the skin's iris atlas), onto his head.
            GIMIMergeFixerConfig::Component eye{};
            eye.name = "Eye";
            eye.modTypeName = ModTypeIdTools::getName(ModTypeId::AetherHeavenEye);
            eye.slots = {{"A", "0", "head", false, "", 1176, true}};
            eye.vertexCount = 400;

            config.components = {std::move(main), std::move(bang), std::move(eye)};
            config.targetObjs = {"head", "body"};

            // He draws every object on the PLAIN shader (diffuse, light map, no normal map), under NNFix.
            config.targetLayout = GIMIMergeFixerConfig::TargetLayout::Plain;

            // Every carried binding onto the register its resource NAME says.
            config.texRegsByName = true;

            // A mod carrying none of a component merges it from its downloads.
            config.downloadPrefix = "AetherHeaven";

            // Both Texcoords are 12 bytes a vertex.
            config.texcoordStride = 12;

            // The face graph is left alone: the two skins draw DIFFERENT face meshes, and a face atlas does not carry
            // across them -- see AetherFixer. On Aether his own face is drawn.
            config.faceReg = "";

            config.diffuseEdits = {};
            config.lightMapEdit = nullptr;
            config.compressTextures = false;

            // A mod hiding the skin's face meshes by hash hides his too.
            config.sideMeshes = {"ib_face", "ib_headupper"};

            config.texFxGuardUnreached = true;

            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::aetherHeavenToAether6_3() {
        return makeGIMIMergeFixer(aetherConfig());
    }


    IniFixBuilder::Factory AetherHeavenFixer::toAether6_3() {
        return IniFixBuilderFuncs::aetherHeavenToAether6_3();
    }
}
