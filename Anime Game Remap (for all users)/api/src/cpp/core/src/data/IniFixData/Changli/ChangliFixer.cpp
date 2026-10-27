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
#include "AGRemapCore/data/IniFixData/Changli/ChangliSkeleton.h"
#include "AGRemapCore/data/IniFixData/Changli/ChangliTattooShader.h"
#include "AGRemapCore/data/IniFixData/Changli/ChangliTextures.h"
#include "AGRemapCore/data/IniFixData/Changli/ChangliUpperShader.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>


namespace AGRemapCore {
    namespace {
        // The skin's outline pass multiplies its ps-t0 by a per-material tint (vs-cb5 / ps-cb2 rows 23-24),
        // and the skin's bangs material tints far darker than Changli's: (0.713, 0.586, 0.593) against her
        // (1, 0.823, 0.835) -- the same ratio on every channel, read off two character-menu frame dumps. Her
        // own outline map is brightened by that ratio in linear light, which puts it back to what her own
        // material draws (her outline maps peak at ~0.47 linear, so nothing clips). Her long hair is kept
        // off that pass instead (see the plan).
        constexpr double BangsOutlineGain = 1.4035;

        TexEditor::Filter outlineGainFilter(double gain) {
            std::array<std::uint8_t, 256> lut{};
            for (int v = 0; v < 256; ++v) {
                double lin = std::min(1.0, std::pow(v / 255.0, 2.2) * gain);
                lut[v] = static_cast<std::uint8_t>(std::nearbyint(255.0 * std::pow(lin, 1.0 / 2.2)));
            }

            return [lut](TextureFile& tex) {
                std::vector<std::uint8_t> px = tex.getPixels();
                for (std::size_t i = 0; i < px.size(); i += 4) {
                    px[i] = lut[px[i]];
                    px[i + 1] = lut[px[i + 1]];
                    px[i + 2] = lut[px[i + 2]];
                }

                tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
            };
        }
    }

    // Transcribed from Tools/Misc/Prototypes/changliNymphFix.py, which stays the oracle this is A/B'd
    // against. Pairing (per-component overlap of the two download folders' Position.buf, and shader
    // family, off character-menu dumps at 3.7.1): bangs, hair, face and eyes are the same mesh and the
    // same shaders on both -> the same slot; her upper body (9d4c7d96), legs and skirt (fbe2f2f0) and her
    // chest flame tattoo (bb1c5ba6) -> the skin's body slot 4 (14fee50a); her coat, shoulder drapes and
    // side sashes -> slot 4 as well: the skin's skirt slot 5 draws them with the same shader (7f639abb) in
    // the character menu, but cuts them away in the overworld (see the plan). The skin's sheer dress (3),
    // skirt (5) and shawl (7) take nothing and are skipped.
    IniFixBuilder::Factory IniFixBuilderFuncs::changliNymph2_4() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::ChangliNymph;
        config.version = "2.4";
        config.sourceVersion = "1.4";

        // ---- three mesh generations: the hash lookups follow the mod's vb0, the blend's vertex group row
        // its GEOMETRY, since the community hash tools rewrite a mod's hashes and keep its bone ids ----
        config.sourceVersionByVb0 = {{"77975500", "1.1"}, {"5f8aac45", "1.2"}, {"d14bed8b", "1.4"}};
        config.skeletonNumberings = {{"1.1", changliRenumbered11To14()}, {"1.2", changliRenumbered12To14()}, {"1.4", {}}};
        config.referenceBoneCentroids = changliBoneCentroids();
        // ...and a mod from before WWMI's merged skeleton (Changli10) holds each component's LOCAL ids
        config.sourceVgMaps = changliVgMaps();

        // ---- the passes the TARGET draws each slot on (her Laurel Nymph dump, wwmiPassLayout.py) ----
        config.slotPasses = {
            {"880e1a81d913db05", "83782bd563d3a6c9"},   // bangs (the second inherits the first's set)
            {"518dadece9a19b08"},                       // hair
            {"ed1c0f8b2ba08ac4"},                       // face
            {"d5572b1cbbba07a4"},                       // sheer dress (nothing of hers lands there)
            {"14fee50a0ce4ec91"},                       // body
            {"7f639abb370f996d"},                       // skirt and veils
            {"5616e4de7d4e70ef"},                       // eyes
            {"43b4972776737988"},                       // shawl (nothing of hers lands there)
        };

        // ---- every pass gated through its VERTEX shader, so RabbitFX's pixel-shader tags survive ----
        config.passVertexShaders = {
            {"880e1a81d913db05", {"a695e6d5f9d489cf"}},
            {"83782bd563d3a6c9", {"a695e6d5f9d489cf"}},
            {"518dadece9a19b08", {"e97b4254e5ed8812"}},
            // the face and eye-highlight shaders are shared, and other frames draw them through a second
            // vertex shader (the Sanhua, Chisa and Lynae pairs list both): gated on one, the textures
            // would flicker to the skin's
            {"ed1c0f8b2ba08ac4", {"7c0b4db32cee62d3", "4ce754b5b9574b1a"}},
            {"d5572b1cbbba07a4", {"cb05fd1251af9a73"}},
            {"14fee50a0ce4ec91", {"e00893f9d07dc9b0"}},
            {"7f639abb370f996d", {"a434a95920c66e51"}},
            // the eyes draw through a second vertex shader outside the character menu (as Lynae's do): gated
            // on the menu's alone, the overworld eyes took the game's textures
            {"5616e4de7d4e70ef", {"ceff9a81afb1de70", "c32a69851757154a"}},
            {"43b4972776737988", {"a434a95920c66e51"}},
            {"f8c96a270bf847dd", {"6a6650a9db8983ce", "e4a3da6d1d1068b9"}},
            {"705575a2e1b4a4b9", {"7148f95feba974ac"}},
            {"e04f4df80ee6b0ab", {"5fd6e5bb6ff81c53", "a54621ce48ed541b"}},
            {"57661bc080f261c6", {"a434a95920c66e51"}},
        };

        // ---- her blindfold: the skin draws its own, white lace on another mesh. Changli's is black,
        // so the skin's takes a recoloured copy of its own diffuse (the meshes' UVs differ, so hers
        // cannot be bound there), and a mod's own section on her blindfold acts on the skin's too ----
        config.sharedMeshes = {
            {"646aa163", {{"57661bc080f261c6", {{"ps-t1", "blindfold"}}}}},
        };
        config.meshCounterparts = {{"baaa6262", "646aa163"}};

        // ---- ...and every tag NAMED. A shader holds ONE filter_index across every loaded .ini, so the
        // shaders other pairs already tag keep their values (Sanhua's hair .91, Chisa's face .76 and
        // outlines .73 / .731, Lynae's eye shaders .625 / .718); the rest take 3381.51 up, a range no
        // other pair uses ----
        config.filterIndices = {
            {"a695e6d5f9d489cf", "3381.91"},
            {"7c0b4db32cee62d3", "3381.76"},
            {"6a6650a9db8983ce", "3381.73"},
            {"e4a3da6d1d1068b9", "3381.731"},
            {"ceff9a81afb1de70", "3381.625"},
            {"c32a69851757154a", "3381.615"},
            {"5fd6e5bb6ff81c53", "3381.718"},
            {"4ce754b5b9574b1a", "3381.769"},
            {"a54621ce48ed541b", "3381.616"},
            {"e00893f9d07dc9b0", "3381.51"},
            {"a434a95920c66e51", "3381.511"},
            {"e97b4254e5ed8812", "3381.512"},
            {"7148f95feba974ac", "3381.513"},
            {"cb05fd1251af9a73", "3381.517"},
        };

        // ---- source component -> target slot and the registers it binds there ----
        config.plan = {
            {0, {0, {{"ps-t0", "bangsMap"}, {"ps-t1", "bangsDiffuse"}, {"ps-t2", "bangsStrand"}}}},
            {1, {1, {{"ps-t0", "hairMap"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairStrand"}}}},
            {2, {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}}}},
            // her body shaders read normal / mask / diffuse at t0 / t1 / t2 as the skin's body shader does;
            // the skin's t3 is its UV-mapped glow map, which her parts have no counterpart for
            {3, {4, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDiffuse"}, {"ps-t3", "noGlow"}}}},
            // her legs and her tattoo share slot 4 with her upper body: drawn from the mod's OWN file, where
            // the shape-keyed positions are computed (a copy computes none), as Lynae's lower body is
            {4, {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDiffuse"}, {"ps-t3", "noGlow"}}, true}},
            // her coat, drapes and sashes go through the skin's BODY slot, in the body shader's layout with a
            // flat cloth mask: the skin's skirt slot (5) draws with the same shader as her coat in the
            // character menu, but in the overworld with a G-buffer shader whose skirt material cuts away
            // whatever the skirt is not -- every part of hers drawn there vanished outside the menu
            {5, {4, {{"ps-t0", "coatNormal"}, {"ps-t1", "flatClothMask"}, {"ps-t2", "coatDiffuse"}, {"ps-t3", "noGlow"}}, true}},
            {6, {6, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"}}}},
            // her chest flame tattoo, on the skin's body slot too, drawn by its OWN shader below (through the
            // skin's body shader the whole quad drew as a red rectangle on her sternum, in-game audit epoch 1)
            {7, {4, {{"ps-t0", "glow"}}, true}},
        };

        // her long hair is NOT drawn in the skin's outline pass: its layered strands' outline shells show
        // through the gaps between strands as a second, darker copy of the hair (Changli9's "second shadow
        // ghost") -- darker on the skin, whose hair outline tint is (0.547, 0.450, 0.457) against her
        // (1, 0.823, 0.835), but visible even with her own tint restored (in game, 2026-10-24)
        config.plan.at(1).offPasses = {"f8c96a270bf847dd"};

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
        WWMIFixerConfig::Shader upper = changliUpperShader();
        // (every texture as the mod's file or a download: the target draw's registers cannot be `ref`'d for them: the slot's OVERWORLD pass holds other textures there than the menu's (Changli's keeps bb73967a at t6 and the ramp at t7))
        upper.registers = {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDiffuse"},
                           {"ps-t3", "upperDetail"}, {"ps-t4", "upperRamp"}, {"ps-t5", "upperShared"},
                           {"ps-t6", "upperSubsurface"}};
        upper.lines = {{"depth_stencil_state_merge", "true"}, {"depth_func", "equal"},
                       {"depth_write_mask", "zero"}, {"mask[1]", "0"}, {"mask[2]", "0"}, {"mask[3]", "0"},
                       {"mask[4]", "0"}, {"mask[5]", "0"}, {"mask[6]", "0"}};
        upper.onlyWith = "o3";
        upper.yieldToRabbitFX = true;
        config.plan.at(3).overlay = std::move(upper);

        // ---- and her chest TATTOO by its own cut-out pass (bb1c5ba6), which discards whatever of the quad the glow
        // texture's alpha leaves out. Its copy takes the TEXCOORD4 / 5 inputs it does not read (the skin's body
        // vertex shader writes them, and it read one register early otherwise) and swaps its two constant buffers'
        // registers (its material at b1, which the fix binds; the skin keeps per-object constants at b2, where her
        // tattoo draw has them at b1). Its two game textures are downloads, as the overlay's ----
        WWMIFixerConfig::Shader tattoo = changliTattooShader();
        tattoo.registers = {{"ps-t0", "glow"}, {"ps-t1", "tattooNoise"}, {"ps-t2", "tattooFlow"}};
        tattoo.onlyWith = "o3";
        config.plan.at(7).shader = std::move(tattoo);

        // ---- a slot's OTHER passes: the outlines take a colour map at ps-t0, the eye's second pass sets
        // ps-t1 only. The bangs take her own outline map, brightened for the skin's darker outline tint
        // (outlineGainFilter) ----
        config.extraPassRegs = {
            {0, {{"f8c96a270bf847dd", {{"ps-t0", "bangsOutline"}}}}},
            {4, {{"f8c96a270bf847dd", {{"ps-t0", "upperDiffuse", 3}, {"ps-t0", "lowerDiffuse", 4}, {"ps-t0", "coatOutline", 5}}}}},
            {6, {{"e04f4df80ee6b0ab", {{"ps-t1", "eyeHighlight"}}}}},
        };

        config.createdTextures = {
            {"noGlow", Colour(249, 12, 4, 0)},
            // cloth in her mask legend, which the skin's body shader reads as hers (her upper body binds her own)
            {"flatClothMask", Colour(0, 128, 0, 255)},
        };

        config.sourceTextures = changliTextureFacts();
        config.flatFallsBackToSource = {"upperMask", "lowerMask", "faceMask"};

        config.texEdits = {
            {"bangsOutline", "TintGain", [](const WWMIFixerConfig::TexEditContext&) {
                 return outlineGainFilter(BangsOutlineGain);
             }, false},
        };

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

        // ---- the shape keys are RETARGETED (the Chisa decision), in the skin's slot order
        // (Tools/Misc/Diagnostics/wwmiShapeKeyOrder.py Changli ChangliNymph), with its dispatch height ----
        config.hiddenObjs = {};
        config.zeroShapeKeyStream = false;
        config.shapeKeyDispatchSize = "1072";
        config.shapeKeyOrder = changliToNymphShapeKeys();
        config.cleanTexcoords = true;
        config.carryByRegisterRole = true;
        // Changli13 draws from mod.ini and binds every texture from Textures/textures.ini
        config.readInisBelow = true;
        // the skin's slots reach bone 263 (264 x 3 float4 = 792), past the 768 default
        config.mergedSkeletonSlots = 1536;
        // INERT for every mod in hand: her slot-4 sources are drawn from the mod's own file
        // (sameFileSection), so no copy .ini is written. Kept so that a copy, should a mod's shape ever
        // produce one on this target past 256 bones, shares the mod's skeleton state
        config.copiesShareSkeleton = true;
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
        return makeWWMIFixer(std::move(config));
    }


    IniFixBuilder::Factory ChangliFixer::nymph2_4() {
        return IniFixBuilderFuncs::changliNymph2_4();
    }
}
