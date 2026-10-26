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
#include <vector>

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

            // The skin's one-sided LEFT BACK DRAPE (99 -> 104, shoulder to shin, with the circle emblem) is MODELLED
            // splayed out to his left, and the skin's simulated bones swing it to the middle of its back. On his left
            // inner cape chain it hung off to his left; on his torso further still (reverse epochs 1-3, 2026-10-25).
            // So it is pulled in, more the lower it hangs: the whole pull at its hem (y 0.4), none at its root (1.15).
            VGPushAway drapePull{};
            drapePull.groups = {99, 100, 101, 102, 103, 104};
            drapePull.side = -1;
            drapePull.from = {-3.0f, 0.0f, -0.15f};
            drapePull.distance = 0.07f;      // with the side panels below, emblem and pendant where the original's hang
            drapePull.fade = {0.4f, 1.15f};
            main.pushAway = {drapePull};

            // ...and every coat panel (front 58-65, side 66-73, back 74-81, and the drape's lowest links 103 / 104) pulled in
            // towards his body's axis, more the lower it hangs. The skin's coat is MODELLED wider than its own simulated
            // bones let it hang, and on his bones it keeps that width: its hem flared all round (reverse epochs 1-6). Which
            // bones carried the flare was found by throwing each candidate set aside in game; it sat on several at once, so
            // pulling one set at a time (side panels, front panels) changed nothing visible.
            VGPushAway coatPull{};
            for (long long group = 58; group <= 81; ++group) {
                coatPull.groups.push_back(group);
            }
            coatPull.groups.push_back(103);
            coatPull.groups.push_back(104);
            coatPull.side = 0;
            coatPull.from = {0.0f, 0.0f, 0.0f};
            coatPull.distance = -0.07f;     // negative: towards 'from'
            coatPull.fade = {0.4f, 1.0f};
            main.pushAway.push_back(std::move(coatPull));

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
