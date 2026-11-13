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
    WWMIFixerConfig ChangliNymphFixer::changli1_4Config() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::Changli;
        config.sourceVersion = "2.4";

        // a mod from before WWMI's merged skeleton (WWMI BETA-2 exports with no vg_offset) holds each
        // component's LOCAL bone ids: lifted through her Metadata.json's per-component vg_map
        // sourceVgMaps: read off VGRemaps (component -> "") at the run's versions (WWMIFixerConfig::withGameData)

        // ---- the passes the TARGET draws each slot on (Changli's dump, wwmiPassLayout.py) ----
        // slotPasses: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

        // ---- every pass gated through its VERTEX shader -- both outfits' and both dumps', per pixel shader ----
        // passVertexShaders: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

        // ---- the blindfold: Changli draws her own, black cloth on another mesh. The skin's is white lace,
        // so hers takes a recoloured copy of her own diffuse (the meshes' UVs differ, so the skin's cannot be
        // bound there), and a mod's own section on the skin's blindfold acts on hers too ----
        config.sharedMeshes = {
            {"blindfoldMesh", {}},          // Changli's blindfold mesh: its passes and bindings off the tables
        };
        config.meshCounterparts = {"blindfoldMesh"};

        // ---- and every tag NAMED: the values the forward direction and the other pairs already give a
        // shader, and 3381.521 up for the vertex shaders only Changli's own draws use ----
        // filterIndices: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

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
        config.plan.at(3).offPasses = {"other"};
        config.plan.at(7).offPasses = {"other"};

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
        WWMIFixerConfig::Shader sheer = WWMIFixerConfig::Shader::ofGame("ChangliNymphSheer");
        // (the ramp as a download too: the target draw's registers cannot be `ref`'d for them: the slot's OVERWORLD pass holds other textures there than the menu's (Changli's keeps bb73967a at t6 and the ramp at t7))
        // (each at the register her own draw binds it: her TextureRegisters, read by withGameData)
        sheer.registers = {{"", "sheerNormal"}, {"", "sheerDiffuse"}, {"", "sheerDetail"}, {"", "sheerNoise"},
                           {"", "sheerRamp"}};
        sheer.onlyWith = IniKeywords::WWMIColourTarget;
        config.plan.at(3).shader = std::move(sheer);

        WWMIFixerConfig::Shader shawl = WWMIFixerConfig::Shader::ofGame("ChangliNymphShawl");
        shawl.registers = {{"", "shawlNormal"}, {"", "shawlDiffuse"}, {"", "sheerDetail"}, {"", "sheerNoise"},
                           {"", "sheerRamp"}};
        shawl.onlyWith = IniKeywords::WWMIColourTarget;
        config.plan.at(7).shader = std::move(shawl);

        // ---- a slot's OTHER passes: her outlines bind the part's own DIFFUSE at ps-t0 (Changli binds
        // dedicated outline maps there); the eye's second pass sets ps-t1 only ----
        config.extraPassRegs = {
            {0, {{"other", {{"ps-t0", "bangsDiffuse"}}}}},
            {1, {{"other", {{"ps-t0", "hairDiffuse"}}}}},
            {4, {{"other", {{"ps-t0", "bodyDiffuse"}}}}},
            {5, {{"other", {{"ps-t0", "skirtDiffuse", 5}}}}},
            {6, {{"other", {{"ps-t1", "eyeHighlight"}}}}},
        };

        config.createdTextures = {          // their colours: Changli's MaterialColours rows
            {"flatClothMask", Colour(), 16, {"component3", "upperMask.cloth"}},
            {"flatDetail", Colour(), 16, {"component3", "upperDetail.flat"}},
        };

        config.sourceTextures = changliNymphTextureFacts();
        // the masks a flat or aliased file is refused for are the source's: see WWMITextureFacts::regionRoles

        // ---- the three lines that undo the remap on a source past 256 bones, and RabbitFX's ----

        // ---- the shape keys are RETARGETED, in Changli's slot order (wwmiShapeKeyOrder.py ChangliNymph
        // Changli: her slots 0-71 take the skin's keys 14-85 by content, 72-85 nothing), with her dispatch height ----
        // shapeKeyDispatchSize: read off ShapeKeyDispatchSizes at the run's versions (WWMIFixerConfig::withGameData)
        // shapeKeyOrder: read off ShapeKeyRemaps at the run's versions (WWMIFixerConfig::withGameData)
        config.cleanTexcoords = true;
        config.carryByRegisterRole = true;
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
        return config;
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::changli1_4() {
        return makeWWMIFixer(ChangliNymphFixer::changli1_4Config());
    }


    IniFixBuilder::Factory ChangliNymphFixer::changli1_4() {
        return IniFixBuilderFuncs::changli1_4();
    }
}
