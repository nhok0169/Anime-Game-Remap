#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaFixer.h"
#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaTextures.h"

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

#include <string>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaThumbprints.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {
    namespace {
        using Binding = WWMIFixerConfig::Binding;

        // The role the fix invents: Exorcist's material mask reads skin off ONE colour, measured
        // at (255, 77, 0) on her own texture at her vertices (the maintainer's hand-made mask
        // agrees), and Sanhua's arm skin ships no mask at all.
        const std::string SkinMask = "SkinMask";

        WWMIFixerConfig sanhuaExorcistConfig() {
            // Every value here was read off the prototype (Tools/Misc/Prototypes/sanhuaExorcistFix.py),
            // which four mods confirmed in game on 2026-09-19: Sanhua's identity mod, the maintainer's
            // simplified export, a merged NSFW mod with a clothes toggle, and a RabbitFX cloak mod
            // with LOD folders. See WWMIFixerConfig for what each field does.
            WWMIFixerConfig config{};
            config.targetId = ModTypeId::SanhuaExorcist;
            config.version = "2.5";

            // The target's pixel shaders that bind CHARACTER textures, per draw slot, off a frame
            // dump of SanhuaExorcist (wwmiDrawTable.py). Slot 0 (the bangs) has two -- the hair
            // shader and the see-through pass over the eyes -- and its third pass (94d9d5e9, the
            // eye region) binds only globals, so it is not here.
            config.slotPasses = {
                {"0f1752e476c3804d", "394feb3747bda7a0"},   // 0: bangs
                {"0f1752e476c3804d"},                       // 1: hair
                {"ed1c0f8b2ba08ac4"},                       // 2: face
                {"fbe2f2f061949559"},                       // 3: torso, arms, ribbons
                {"348ac69e57e38686"},                       // 4: hair bun, trousers
                {"0512a7e61d6bd01d"},                       // 5: eyes
            };

            // ...each gated by the VERTEX shaders it is drawn with rather than by its own pixel
            // shader, because RabbitFX patches these pixel shaders. Measured 2026-10-13 over the 3.7
            // frame dumps of both characters (`FrameAnalysis-Sanhua37*`, read the way
            // wwmiDrawTable.py reads them): NINE of the ten pixel shaders this pair used to tag run
            // RabbitFX's [ShaderRegexMain] command list, the eye pass being the only one with no
            // RabbitFX file in any dump -- and absence is not proof there, since all but one of its
            // regexes have their dump lines commented out. A [ShaderOverride] is keyed by shader hash
            // across every loaded .ini and a shader holds ONE filter_index, so tagging those pixel
            // shaders replaced RabbitFX's own `filter_index = 1718.1` and every RabbitFX
            // `SetTextures` / `Run` read `if ps == 1718.1` as false -- for EVERY mod drawing with
            // them, not only a remapped one. The face shader alone (ed1c0f8b2ba08ac4) is shared with
            // ChisaParfait and appears in 19 of the dumps on disk.
            //
            // Sanhua's hair slot and both of her bangs passes are drawn with ONE vertex shader
            // (a695e6d5f9d489cf), so their conditions collapse into one -- which is harmless here
            // because each slot's list lives in its own remapped section and both bind the same
            // roles. The face pass carries ChisaParfait's second vertex shader too: it is the same
            // pass, and naming only the one this character's dumps show is how Chisa's eye list
            // ended up gated on a draw nobody sees (2026-10-01).
            config.passVertexShaders = {
                {"0f1752e476c3804d", {"a695e6d5f9d489cf"}},                         // hair, both characters
                {"394feb3747bda7a0", {"a695e6d5f9d489cf"}},                         // Exorcist's bangs see-through pass
                {"ed1c0f8b2ba08ac4", {"7c0b4db32cee62d3", "4ce754b5b9574b1a"}},     // face, both characters
                {"fbe2f2f061949559", {"02f5c4b333a78fa0"}},                         // Exorcist's torso, arms, ribbons
                {"348ac69e57e38686", {"15a2c90dd35b8c07"}},                         // Exorcist's hair bun, trousers
                {"0512a7e61d6bd01d", {"a79e471015b2a16c"}},                         // eyes, both characters
            };

            // Every value named rather than derived from filterBase/filterStep: a derived value is
            // numbered in map-iteration order, so adding a pass renumbers every shader after it and
            // silently breaks the agreement these shaders are under. The face pair must match
            // ChisaParfait's and LynaePeppermint's values -- a Chisa mod and a Sanhua mod installed
            // together is the ordinary case, and whichever .ini loads last wins.
            config.filterIndices = {
                {"a695e6d5f9d489cf", "3381.91"},    // hair + bangs; the reverse direction tags it too
                {"7c0b4db32cee62d3", "3381.76"},    // the face, as ChisaParfait and LynaePeppermint tag it
                {"4ce754b5b9574b1a", "3381.769"},   //   and its second vertex shader, likewise
                {"02f5c4b333a78fa0", "3381.94"},    // Exorcist's own slots: values the reverse never uses
                {"15a2c90dd35b8c07", "3381.95"},
                {"a79e471015b2a16c", "3381.96"},    // the eyes; the reverse tags this one too
            };

            // Source component -> target slot, and the registers each pass reads. Shader families
            // decide the slot (bones do not, under a merged skeleton): the bangs, hair and face onto
            // their own slots, the eyes onto the eye slot (which reads the iris at ps-t2 and the eye
            // mask at ps-t1; ps-t0 is a global the game keeps), and everything of the BODY -- the arm
            // skin, the bodice with its hat, ribbons and boots, and the skirt -- through the torso
            // slot, whose normal-map layout is ps-t0 normal, ps-t1 mask, ps-t2 diffuse. Three
            // sources on one slot is the merge: two extra .ini files.
            config.plan = {
                {0, {0, {{"ps-t0", "bangsDiffuse"}, {"ps-t1", "bangsMask"}, {"ps-t5", "t5Ramp"}}}},
                {1, {1, {{"ps-t0", "hairDiffuse"}, {"ps-t1", "hairNormal"}, {"ps-t5", "t5Ramp"}}}},
                {2, {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}}}},
                {3, {3, {{"ps-t0", "skinNormal"}, {"ps-t1", SkinMask}, {"ps-t2", "skinDiffuse"}}}},
                {4, {3, {{"ps-t0", "bodiceNormal"}, {"ps-t1", "bodiceMask"}, {"ps-t2", "bodiceDiffuse"}}}},
                {5, {3, {{"ps-t0", "skirtNormal"}, {"ps-t1", "skirtMask"}, {"ps-t2", "skirtDiffuse"}}}},
                {6, {5, {{"ps-t1", "eyeMask"}, {"ps-t2", "irisDiffuse"}, {"ps-t5", "t5Ramp"}}}},
            };

            // Sanhua's textures by the hash WWMI Tools filed them under (WWMI-Assets, TextureUsage.json)
            // with the role each plays in its component's main pass, read off the 2026-09-19 dump --
            // and the same textures under the hashes of the 2024-2025 game versions the maintainer's
            // mods were exported from (Data/Mod Downloads/WuWa/Sanhua/SanhuaHashLineage.json, each pair
            // measured by pixel identity or taken from the community's hash maps).

            // A file named by component and type and by nothing else (Component4_NM.dds: the
            // RabbitFX / WWMI-Tools export names). cef6494f reads as a mask by its pixels; the
            // "hairNormal" label is historical.
            config.typeRoles = {
                {0, {{"diffuse", "bangsDiffuse"}, {"mask", "bangsMask"}}},
                {1, {{"diffuse", "hairDiffuse"}, {"mask", "hairNormal"}, {"normal", "hairNormal"}}},
                {2, {{"diffuse", "faceDiffuse"}, {"mask", "faceMask"}}},
                {3, {{"diffuse", "skinDiffuse"}, {"normal", "skinNormal"}}},
                {4, {{"diffuse", "bodiceDiffuse"}, {"mask", "bodiceMask"}, {"normal", "bodiceNormal"}}},
                {5, {{"diffuse", "skirtDiffuse"}, {"mask", "skirtMask"}, {"normal", "skirtNormal"}}},
                {6, {{"diffuse", "irisDiffuse"}, {"mask", "eyeMask"}}},
            };

            config.createdTextures = {{SkinMask, Colour(255, 77, 0, 255), 16}};

            config.sourceTextures = sanhuaTextureFacts();


            // Her own textures' thumbprints, so a file named by nothing (Component6_Diffuse.dds: the
            // ps-t5 ramp, not the iris its name says) is placed by what it IS.

            config.sourceLabels = {{0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "arm skin"}, {4, "bodice, hat, ribbons, boots"},
                                   {5, "skirt"}, {6, "eyes"}};
            config.targetLabels = {{0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "torso, arms, ribbons"}, {4, "hair bun, trousers"},
                                   {5, "eyes"}};

            // The generated second and third files explain themselves.
            config.copyPreamble = IniComments::GIMIObjMergerPreamble;
            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::sanhuaExorcist2_5() {
        return makeWWMIFixer(sanhuaExorcistConfig());
    }


    IniFixBuilder::Factory SanhuaFixer::exorcist2_5() {
        return IniFixBuilderFuncs::sanhuaExorcist2_5();
    }
}
