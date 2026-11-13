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

#include "AGRemapCore/data/IniFixData/Changli/ChangliFixer.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/Changli/ChangliTextures.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"



namespace AGRemapCore {
    // Transcribed from Tools/Misc/Prototypes/changliNymphFix.py, which stays the oracle this is A/B'd
    // against. Pairing (per-component overlap of the two download folders' Position.buf, and shader
    // family, off character-menu dumps at 3.7.1): bangs, hair, face and eyes are the same mesh and the
    // same shaders on both -> the same slot; her upper body (9d4c7d96), legs and skirt (fbe2f2f0) and her
    // chest flame tattoo (bb1c5ba6) -> the skin's body slot 4 (14fee50a); her coat, shoulder drapes and
    // side sashes -> slot 4 as well: the skin's skirt slot 5 draws them with the same shader (7f639abb) in
    // the character menu, but cuts them away in the overworld (see the plan). The skin's sheer dress (3),
    // skirt (5) and shawl (7) take nothing and are skipped.
    WWMIFixerConfig ChangliFixer::nymph2_4Config() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::ChangliNymph;
        config.sourceVersion = "1.4";

        // ---- three mesh generations: the hash lookups follow the mod's vb0, the blend's vertex group row
        // its GEOMETRY, since the community hash tools rewrite a mod's hashes and keep its bone ids ----
        config.sourceVersionFromVb0 = true;      // her vb0 rows' versions (WWMIFixerConfig::vb0VersionsOf)
        // ...and a mod from before WWMI's merged skeleton (Changli10) holds each component's LOCAL ids
        // sourceVgMaps: read off VGRemaps (component -> "") at the run's versions (WWMIFixerConfig::withGameData)

        // ---- the passes the TARGET draws each slot on (her Laurel Nymph dump, wwmiPassLayout.py) ----
        // slotPasses: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

        // ---- every pass gated through its VERTEX shaders, each with its NAMED filter_index ----
        // passVertexShaders and filterIndices: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData),
        // its tags the values both directions of the pair share (a shader holds ONE filter_index across every loaded
        // .ini; `Tools/Misc/Diagnostics/wwmiShaderTags.py` is the check). The literal entries this config carried for
        // passes with no row went on 2026-11-12: on all 41 Chisa / ChisaParfait / Changli mods they changed no file,
        // at 3.7 or 3.8.

        // ---- her blindfold: the skin draws its own, white lace on another mesh. Changli's is black,
        // so the skin's takes a recoloured copy of its own diffuse (the meshes' UVs differ, so hers
        // cannot be bound there), and a mod's own section on her blindfold acts on the skin's too ----
        config.sharedMeshes = {
            {"blindfoldMesh", {}},          // the skin's blindfold mesh: its passes and bindings off the tables
        };
        config.meshCounterparts = {"blindfoldMesh"};

        // ---- ...and every tag NAMED. A shader holds ONE filter_index across every loaded .ini, so the
        // shaders other pairs already tag keep their values (Sanhua's hair .91, Chisa's face .76 and
        // outlines .73 / .731, Lynae's eye shaders .625 / .718); the rest take 3381.51 up, a range no
        // other pair uses ----
        // filterIndices: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)
        // ...and her own passes' tags off her own rows

        // ---- source component -> target slot and the registers it binds there ----
        config.plan = {
            {0, {0, {{"ps-t0", "bangsMap"}, {"ps-t1", "bangsDiffuse"}, {"ps-t2", "bangsStrand"}}}},
            {1, {1, {{"ps-t0", "hairMap"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairStrand"}}}},
            {2, {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}}}},
            // her body shaders read normal / mask / diffuse at t0 / t1 / t2 as the skin's body shader does; the skin's
            // t3 is left to the game (a shared effect texture its own body binds too: a made "no glow" one changed
            // nothing in game, 2026-11-10)
            {3, {4, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDiffuse"}}}},
            // her legs and her tattoo share slot 4 with her upper body: drawn from the mod's OWN file, where
            // the shape-keyed positions are computed (a copy computes none), as Lynae's lower body is
            {4, {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDiffuse"}}, true}},
            // her coat, drapes and sashes go through the skin's BODY slot, in the body shader's layout with a
            // flat cloth mask: the skin's skirt slot (5) draws with the same shader as her coat in the
            // character menu, but in the overworld with a G-buffer shader whose skirt material cuts away
            // whatever the skirt is not -- every part of hers drawn there vanished outside the menu
            {5, {4, {{"ps-t0", "coatNormal"}, {"ps-t1", "flatClothMask"}, {"ps-t2", "coatDiffuse"}}, true}},
            {6, {6, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"}}}},
            // her chest flame tattoo, on the skin's body slot too, drawn by its OWN shader below (through the
            // skin's body shader the whole quad drew as a red rectangle on her sternum, in-game audit epoch 1)
            {7, {4, {{"ps-t0", "glow"}}, true}},
        };

        // her long hair is NOT drawn in the skin's outline pass: its layered strands' outline shells show
        // through the gaps between strands as a second, darker copy of the hair (Changli9's "second shadow
        // ghost") -- darker on the skin, whose hair outline tint is (0.547, 0.450, 0.457) against her
        // (1, 0.823, 0.835), but visible even with her own tint restored (in game, 2026-10-24)
        config.plan.at(1).offPasses = {"other"};

        // ---- her flame feathers' GLOW, from HER upper-body pass (9d4c7d96) drawn as an emissive OVERLAY. A part
        // of hers glows where its diffuse's alpha is above 230 (her shader's emissive is a colour times saturate((alpha -
        // 0.9) / 0.1); a mod's flame feathers carry 240-255, the see-through and white parts other mods paint stay
        // below 230 -- a cut-off at 117 lit Changli5's shirt and Changli2's stockings white, forward epoch 6),
        // and the skin's body shader has no input that lights a part (a t3 at alpha 255 lit nothing), so through it
        // the feathers kept their colours and never glowed. Her shader outright drew the whole upper body dark red,
        // and drawn over the skin's surface her G-buffer outputs turned the feathers black -- they mean something
        // else to the skin's lighting -- so the skin's shader draws the surface as before and a copy of hers draws it
        // again at the same depth, discarding every pixel at or below that alpha and writing only the emissive target: the
        // part's own diffuse x2, matched to the original's glow on Changli10's feathers (3182 / 4248 yellow pixels
        // against 3176 / 4430). Its vertex inputs are the skin's body slot's, register for register. A RabbitFX mod
        // keeps its look. It (and the tattoo below) drops out with the SKIN's camera-near fade -- the skin's per-object
        // cb2[43] with the skin's own dither -- since her own fade reads her material constants, from a menu dump ----
        WWMIFixerConfig::Shader upper = WWMIFixerConfig::Shader::ofGame("ChangliUpper");
        // (every texture as the mod's file or a download: the target draw's registers cannot be `ref`'d for them: the slot's OVERWORLD pass holds other textures there than the menu's (Changli's keeps bb73967a at t6 and the ramp at t7))
        // (each at the register her own draw binds it: her TextureRegisters, read by withGameData)
        upper.registers = {{"", "upperNormal"}, {"", "upperMask"}, {"", "upperDiffuse"},
                           {"", "upperDetail"}, {"", "upperRamp"}, {"", "upperShared"},
                           {"", "upperSubsurface"}};
        upper.lines = {{IniKeywords::DepthStencilStateMerge, "true"}, {IniKeywords::DepthFunc, "equal"},
                       {IniKeywords::DepthWriteMask, "zero"}, {IniKeywords::Mask + "[1]", "0"}, {IniKeywords::Mask + "[2]", "0"}, {IniKeywords::Mask + "[3]", "0"},
                       {IniKeywords::Mask + "[4]", "0"}, {IniKeywords::Mask + "[5]", "0"}, {IniKeywords::Mask + "[6]", "0"}};
        upper.onlyWith = IniKeywords::WWMIColourTarget;
        upper.yieldToRabbitFX = true;
        config.plan.at(3).overlay = std::move(upper);

        // ---- and her chest TATTOO by its own cut-out pass (bb1c5ba6), which discards whatever of the quad the glow
        // texture's alpha leaves out. Its copy takes the TEXCOORD4 / 5 inputs it does not read (the skin's body
        // vertex shader writes them, and it read one register early otherwise) and swaps its two constant buffers'
        // registers (its material at b1, which the fix binds; the skin keeps per-object constants at b2, where her
        // tattoo draw has them at b1). Its two game textures are downloads, as the overlay's ----
        WWMIFixerConfig::Shader tattoo = WWMIFixerConfig::Shader::ofGame("ChangliTattoo");
        tattoo.registers = {{"", "glow"}, {"", "tattooNoise"}, {"", "tattooFlow"}};
        tattoo.onlyWith = IniKeywords::WWMIColourTarget;
        config.plan.at(7).shader = std::move(tattoo);

        // ---- a slot's OTHER passes: the outlines take a colour map at ps-t0, the eye's second pass sets
        // ps-t1 only. The bangs take her own outline map as it is: brightened for the skin's darker
        // outline tint (x1.4035 in linear light), it changed nothing in game (2026-11-10) ----
        config.extraPassRegs = {
            {0, {{"other", {{"ps-t0", "bangsOutline"}}}}},
            {4, {{"other", {{"ps-t0", "upperDiffuse", 3}, {"ps-t0", "lowerDiffuse", 4}, {"ps-t0", "coatOutline", 5}}}}},
            {6, {{"other", {{"ps-t1", "eyeHighlight"}}}}},
        };

        // (its colour: the skin's MaterialColours row -- cloth in her mask legend, which the skin's body shader reads
        // as hers; her upper body binds her own)
        config.createdTextures = {
            {"flatClothMask", Colour(), 16, {"component4", "bodyMask.cloth"}},
        };

        config.sourceTextures = changliTextureFacts();
        // the masks a flat or aliased file is refused for are the source's: see WWMITextureFacts::regionRoles

        // ---- the three lines that undo the remap on a source past 256 bones, and RabbitFX's ----

        // ---- the shape keys are RETARGETED (the Chisa decision), in the skin's slot order
        // (Tools/Misc/Diagnostics/wwmiShapeKeyOrder.py Changli ChangliNymph), with its dispatch height ----
        // shapeKeyDispatchSize: read off ShapeKeyDispatchSizes at the run's versions (WWMIFixerConfig::withGameData)
        // shapeKeyOrder: read off ShapeKeyRemaps at the run's versions (WWMIFixerConfig::withGameData)
        config.cleanTexcoords = true;
        config.carryByRegisterRole = true;
        // her blend is FOUR influences a vertex and the skin's draws read EIGHT: unwidened, every vertex
        // was read across its neighbour's bytes and the whole model drew as giant polygons in game
        config.targetInfluences = 8;

        config.sourceLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "legs and skirt"},
            {5, "coat and sashes"}, {6, "eyes"}, {7, "chest flame tattoo"},
        };
        config.targetLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "sheer dress"}, {4, "body"},
            {5, "skirt and veils"}, {6, "eyes"}, {7, "shawl"},
        };
        return config;
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::changliNymph2_4() {
        return makeWWMIFixer(ChangliFixer::nymph2_4Config());
    }


    IniFixBuilder::Factory ChangliFixer::nymph2_4() {
        return IniFixBuilderFuncs::changliNymph2_4();
    }
}
