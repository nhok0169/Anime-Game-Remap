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

#include <utility>
#include <vector>

#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphFixer.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphShawlShader.h"
#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphSheerShader.h"
#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphSkeleton.h"
#include "AGRemapCore/data/IniFixData/ChangliNymph/ChangliNymphTextures.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {
    // Transcribed from Tools/Misc/Prototypes/nymphChangliFix.py, which stays the oracle this is A/B'd against.
    // Pairing (per-component overlap of the two download folders' Position.buf, and the register layout each
    // pass SETS, off the character-menu dumps at 3.7.1): bangs, hair, face and eyes are the same mesh and the
    // same shaders on both -> the same slot; her body (14fee50a) -> Changli's legs slot (fbe2f2f0), whose
    // shader reads normal / mask / diffuse at t0-t2 as hers does; her sheer dress -> Changli's upper body,
    // drawn by her own see-through pass (Changli has none); her skirt and veils and her shawl -> Changli's coat
    // slot, the skirt's own shader (7f639abb), the shawl by its own see-through pass. Changli's chest tattoo
    // slot takes nothing.
    IniFixBuilder::Factory IniFixBuilderFuncs::changli1_4() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::Changli;
        config.version = "1.4";            // her live vb0 is 1.4's, and her bones are numbered as at 1.4
        config.sourceVersion = "2.4";

        // a mod from before WWMI's merged skeleton (WWMI BETA-2 exports with no vg_offset) holds each
        // component's LOCAL bone ids: lifted through her Metadata.json's per-component vg_map
        config.sourceVgMaps = changliNymphVgMaps();

        // ---- the passes the TARGET draws each slot on (Changli's dump, wwmiPassLayout.py) ----
        config.slotPasses = {
            {"880e1a81d913db05", "83782bd563d3a6c9"},   // bangs
            {"518dadece9a19b08"},                       // hair
            {"ed1c0f8b2ba08ac4"},                       // face
            {"9d4c7d9618ef7f45"},                       // upper body
            {"fbe2f2f061949559"},                       // legs and skirt
            {"7f639abb370f996d"},                       // coat
            {"5616e4de7d4e70ef"},                       // eyes
            {"bb1c5ba6e1be6395"},                       // chest flame tattoo (nothing of hers lands there)
        };

        // ---- every pass gated through its VERTEX shader -- both outfits' and both dumps', per pixel shader ----
        config.passVertexShaders = {
            {"880e1a81d913db05", {"a695e6d5f9d489cf"}},
            {"83782bd563d3a6c9", {"a695e6d5f9d489cf"}},
            {"518dadece9a19b08", {"e97b4254e5ed8812"}},
            {"ed1c0f8b2ba08ac4", {"7c0b4db32cee62d3", "4ce754b5b9574b1a"}},
            {"9d4c7d9618ef7f45", {"6e5d4bc00bc160ee"}},
            {"fbe2f2f061949559", {"3b36d7d284d20522"}},
            {"7f639abb370f996d", {"3ba352c448cd29cc", "a434a95920c66e51"}},
            // the eyes draw through a second vertex shader outside the character menu (as Lynae's do): gated
            // on the menu's alone, the overworld eyes took the game's textures
            {"5616e4de7d4e70ef", {"ceff9a81afb1de70", "c32a69851757154a"}},
            {"bb1c5ba6e1be6395", {"488d2eb256ff8c71"}},
            {"f8c96a270bf847dd", {"6a6650a9db8983ce", "e4a3da6d1d1068b9"}},
            {"705575a2e1b4a4b9", {"3e22b125329523fa", "7148f95feba974ac"}},
            {"e04f4df80ee6b0ab", {"5fd6e5bb6ff81c53", "a54621ce48ed541b"}},
        };

        // ---- the blindfold: Changli draws her own, black cloth on another mesh. The skin's is white lace,
        // so hers takes a recoloured copy of her own diffuse (the meshes' UVs differ, so the skin's cannot be
        // bound there), and a mod's own section on the skin's blindfold acts on hers too ----
        config.sharedMeshes = {
            {"baaa6262", {{"7f639abb370f996d", {{"ps-t1", "blindfold"}}}}},
        };
        config.meshCounterparts = {{"646aa163", "baaa6262"}};

        // ---- and every tag NAMED: the values the forward direction and the other pairs already give a
        // shader, and 3381.521 up for the vertex shaders only Changli's own draws use ----
        config.filterIndices = {
            {"a695e6d5f9d489cf", "3381.91"},
            {"7c0b4db32cee62d3", "3381.76"},
            {"4ce754b5b9574b1a", "3381.769"},
            {"6a6650a9db8983ce", "3381.73"},
            {"e4a3da6d1d1068b9", "3381.731"},
            {"ceff9a81afb1de70", "3381.625"},
            {"c32a69851757154a", "3381.615"},
            {"5fd6e5bb6ff81c53", "3381.718"},
            {"a54621ce48ed541b", "3381.616"},
            {"a434a95920c66e51", "3381.511"},
            {"e97b4254e5ed8812", "3381.512"},
            {"7148f95feba974ac", "3381.513"},
            {"6e5d4bc00bc160ee", "3381.521"},
            {"3b36d7d284d20522", "3381.522"},
            {"3ba352c448cd29cc", "3381.523"},
            {"488d2eb256ff8c71", "3381.524"},
            {"3e22b125329523fa", "3381.525"},
        };

        // ---- source component -> target slot and the registers it binds there ----
        config.plan = {
            {0, {0, {{"ps-t0", "bangsMap"}, {"ps-t1", "bangsDiffuse"}, {"ps-t2", "bangsStrand"}}}},
            {1, {1, {{"ps-t0", "hairMap"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairStrand"}}}},
            {2, {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}}}},
            // her sheer dress through Changli's upper-body shader: normal / mask / diffuse / detail at t0-t3.
            // She has no mask or detail map for it; the masks share one legend (R = 255 bare skin), so a flat
            // cloth mask, and a flat detail at the mean of Changli's own (70, alpha 0). The ramp (t4) is the game's
            {3, {3, {{"ps-t0", "sheerNormal"}, {"ps-t1", "flatClothMask"}, {"ps-t2", "sheerDiffuse"}, {"ps-t3", "flatDetail"}}}},
            // her body through Changli's legs shader: the same three inputs at the same registers
            {4, {4, {{"ps-t0", "bodyNormal"}, {"ps-t1", "bodyMask"}, {"ps-t2", "bodyDiffuse"}}}},
            // the same shader on both: normal at t0, diffuse at t1
            {5, {5, {{"ps-t0", "skirtNormal"}, {"ps-t1", "skirtDiffuse"}}}},
            {6, {6, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"}}}},
            // her shawl through the coat slot's shader, a second section in the mod's own file (where the
            // shape-keyed positions are computed; a copy computes none)
            {7, {5, {{"ps-t0", "shawlNormal"}, {"ps-t1", "shawlDiffuse"}}, true}},
        };
        // neither her sheer dress nor her shawl has an outline pass on her own draws
        config.plan.at(3).offPasses = {"f8c96a270bf847dd"};
        config.plan.at(7).offPasses = {"705575a2e1b4a4b9"};

        // ---- her sheer dress and her shawl drawn by HER see-through passes (d5572b1c, 43b49727). Changli has no
        // see-through shader, and through her opaque ones the dress, its skirt panels (and a mod's opacity toggles
        // on them), her fine chains and the shawl came out solid. Both passes are SCREEN-DOOR see-through, not a
        // blend: each pixel whose opacity (the diffuse's alpha) is under a noise threshold that moves every frame
        // is discarded, and the game's temporal anti-aliasing resolves the dither -- so they draw as opaque
        // passes, with no blend, stencil or ordering of their own (a blend over a stencil mark of the body, as
        // Lynae's sheer shirt takes, drew the dress as shards). They read the mod's normal map and diffuse at t0
        // / t1, a matcap (t2) and the dither noise (t3) no draw of Changli's binds, and the ramp at t4, which
        // her upper-body and coat slots hold at t6 in the menu (and at t7 in the overworld). Her dress pass reads a COLOR0 her upper-body slot's
        // vertex shader does not pass (every later input would land one register off: the dress drew as
        // shards), so its copy takes the dress's own value, 0 on every vertex; her coat slot passes the shawl's. And
        // both take the overworld's camera-near fade from CHANGLI's material constants, live at b4 (the skin's own
        // fade is per-object data hers leaves off), with their own material bound at b5 ----
        WWMIFixerConfig::Shader sheer = changliNymphSheerShader();
        // (the ramp as a download too: the target draw's registers cannot be `ref`'d for them: the slot's OVERWORLD pass holds other textures there than the menu's (Changli's keeps bb73967a at t6 and the ramp at t7))
        sheer.registers = {{"ps-t0", "sheerNormal"}, {"ps-t1", "sheerDiffuse"}, {"ps-t2", "sheerDetail"}, {"ps-t3", "sheerNoise"},
                           {"ps-t4", "sheerRamp"}};
        sheer.onlyWith = "o3";
        config.plan.at(3).shader = std::move(sheer);

        WWMIFixerConfig::Shader shawl = changliNymphShawlShader();
        shawl.registers = {{"ps-t0", "shawlNormal"}, {"ps-t1", "shawlDiffuse"}, {"ps-t2", "sheerDetail"}, {"ps-t3", "sheerNoise"},
                           {"ps-t4", "sheerRamp"}};
        shawl.onlyWith = "o3";
        config.plan.at(7).shader = std::move(shawl);

        // ---- a slot's OTHER passes: her outlines bind the part's own DIFFUSE at ps-t0 (Changli binds
        // dedicated outline maps there); the eye's second pass sets ps-t1 only ----
        config.extraPassRegs = {
            {0, {{"f8c96a270bf847dd", {{"ps-t0", "bangsDiffuse"}}}}},
            {1, {{"f8c96a270bf847dd", {{"ps-t0", "hairDiffuse"}}}}},
            {4, {{"705575a2e1b4a4b9", {{"ps-t0", "bodyDiffuse"}}}}},
            {5, {{"705575a2e1b4a4b9", {{"ps-t0", "skirtDiffuse", 5}}}}},
            {6, {{"e04f4df80ee6b0ab", {{"ps-t1", "eyeHighlight"}}}}},
        };

        config.createdTextures = {
            {"flatClothMask", Colour(0, 128, 0, 255)},
            {"flatDetail", Colour(70, 70, 70, 0)},
        };

        config.sourceTextures = changliNymphTextureFacts();
        config.flatFallsBackToSource = {"bodyMask", "faceMask"};

        // ---- the three lines that undo the remap on a source past 256 bones, and RabbitFX's ----
        config.removedRegs = {
            {"ResourceBlendBufferOverride", "ref"},
            {"ResourceMergedSkeletonOverride", "ref"},
            {"ResourceExtraMergedSkeletonOverride", "ref"},
            {"Resource\\RabbitFX\\Diffuse", ""},
            {"Resource\\RabbitFX\\Lightmap", ""},
            {"Resource\\RabbitFX\\Normalmap", ""},
            {"Resource\\RabbitFX\\Materialmap", ""},
            {"Resource\\RabbitFX\\Cutoutmap", ""},
            {"Resource\\RabbitFX\\Specialmap", ""},
            {"run", "commandlist\\rabbitfx\\settextures"},
        };

        // ---- the shape keys are RETARGETED, in Changli's slot order (wwmiShapeKeyOrder.py ChangliNymph
        // Changli: her slots 0-71 take the skin's keys 14-85 by content, 72-85 nothing), with her dispatch height ----
        config.hiddenObjs = {};
        config.zeroShapeKeyStream = false;
        config.shapeKeyDispatchSize = "1097";
        config.shapeKeyOrder = {};
        for (long long key = 14; key < 86; ++key) {
            config.shapeKeyOrder.push_back(key);
        }
        config.shapeKeyOrder.insert(config.shapeKeyOrder.end(), 14, -1);
        config.cleanTexcoords = true;
        config.carryByRegisterRole = true;
        config.readInisBelow = true;
        // her slots reach bone 275 (276 x 3 float4 = 828), past the 768 default
        config.mergedSkeletonSlots = 1536;
        config.copiesShareSkeleton = true;
        // the skin's blend is EIGHT influences a vertex and Changli's draws read FOUR
        config.targetInfluences = 4;

        config.sourceLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "sheer dress"}, {4, "body"},
            {5, "skirt and veils"}, {6, "eyes"}, {7, "shawl"},
        };
        config.targetLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "legs and skirt"},
            {5, "coat and sashes"}, {6, "eyes"}, {7, "chest flame tattoo"},
        };
        return makeWWMIFixer(std::move(config));
    }


    IniFixBuilder::Factory ChangliNymphFixer::changli1_4() {
        return IniFixBuilderFuncs::changli1_4();
    }
}
