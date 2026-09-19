#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaFixer.h"

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
                {"69e3d3219c979981", "8fbb55320274f090"},   // 0: bangs
                {"69e3d3219c979981"},                       // 1: hair
                {"374a4f8fc9a5ea6a"},                       // 2: face
                {"3093e3c72c791456"},                       // 3: torso, arms, ribbons
                {"5cc08ed68341dd38"},                       // 4: hair bun, trousers
                {"056f9f3c356ff96e"},                       // 5: eyes
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
            config.roles = {
                // current (2.5)
                {"c88cc1fc", "eyeMask"}, {"ae6e9014", "bangsDiffuse"}, {"1dcc0f1d", "irisDiffuse"}, {"1035197c", "t5Ramp"},
                {"1c0c8b91", "bangsMask"},
                {"68ca7071", "hairDiffuse"}, {"cef6494f", "hairNormal"},
                {"46177147", "faceMask"}, {"881c236d", "faceDiffuse"},
                {"e39835c7", "skinNormal"}, {"fde0f298", "skinDiffuse"},
                {"efb25eb3", "bodiceNormal"}, {"89ba19a1", "bodiceMask"}, {"abda232b", "bodiceDiffuse"},
                {"f3b217ab", "skirtNormal"}, {"f0713dc7", "skirtMask"}, {"c689a8ee", "skirtDiffuse"},
                // older
                {"2584190a", "hairNormal"}, {"98b9635b", "hairDiffuse"}, {"aa70ef15", "faceDiffuse"},
                {"03d9850b", "skinNormal"}, {"4b6d52b9", "skinDiffuse"},
                {"0521977a", "bodiceNormal"}, {"ebeeda8c", "bodiceDiffuse"}, {"5efe7892", "bodiceMask"},
                {"16695017", "skirtNormal"}, {"2c0c2728", "skirtDiffuse"}, {"11b9cadd", "skirtMask"},
                {"1bdd0987", "t5Ramp"},
                {"48616ac9", "bangsDiffuse"}, {"3cd03f60", "irisDiffuse"}, {"345368c9", "bangsMask"},
            };

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

            // A planned role a mod ships no file for is bound to Sanhua's OWN texture of that role,
            // downloaded from her folder (the mod's UVs are hers). The red-camellia mod carries no
            // bodice or skirt mask, and the Exorcist's mask sampled at its UVs shaded cloth as skin: a
            // reddish hue over the whole body (2026-09-19). eyeMask (c88cc1fc) and faceMask (46177147)
            // are bound under the same hash on both skins, so they need no entry.
            config.downloadCharFolder = "Sanhua";
            config.downloadVersionFolder = "2_5";
            config.downloadPrefix = "Sanhua";
            config.fallbackTextures = {
                {"bangsDiffuse", "ae6e9014"}, {"bangsMask", "1c0c8b91"}, {"t5Ramp", "1035197c"},
                {"hairDiffuse", "68ca7071"}, {"hairNormal", "cef6494f"}, {"faceDiffuse", "881c236d"},
                {"skinNormal", "e39835c7"}, {"skinDiffuse", "fde0f298"},
                {"bodiceNormal", "efb25eb3"}, {"bodiceMask", "89ba19a1"}, {"bodiceDiffuse", "abda232b"},
                {"skirtNormal", "f3b217ab"}, {"skirtMask", "f0713dc7"}, {"skirtDiffuse", "c689a8ee"},
                {"irisDiffuse", "1dcc0f1d"},
            };

            // Her own textures' thumbprints, so a file named by nothing (Component6_Diffuse.dds: the
            // ps-t5 ramp, not the iris its name says) is placed by what it IS.
            config.textureThumbprints = sanhuaTextureThumbprints();

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
