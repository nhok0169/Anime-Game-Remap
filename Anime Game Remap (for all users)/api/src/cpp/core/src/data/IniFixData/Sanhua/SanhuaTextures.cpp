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

#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaTextures.h"
#include "AGRemapCore/data/IniFixData/Sanhua/SanhuaThumbprints.h"


namespace AGRemapCore {

    WWMITextureFacts sanhuaTextureFacts() {
        WWMITextureFacts facts;

        // ---- her textures by the hash the game binds them under ----
        facts.roles = {
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
                // live: what the game binds with texture quality on Ultra High, where 3DMigoto has
                // rehashed each texture as its mips loaded (Data/Mod Downloads/WuWa/Sanhua/
                // SanhuaLiveHashes.json; re-measured at 3.7 on 2026-10-05). A mod brought up to date
                // by wwmiTextureFix.py --live overrides these beside the current ones.
                {"b0828323", "bangsDiffuse"}, {"332a6aac", "bangsMask"}, {"07f0a2ea", "hairDiffuse"},
                {"466478e4", "hairNormal"}, {"bf16c0c7", "faceMask"}, {"d2765954", "faceDiffuse"},
                {"7aee2f16", "skinNormal"}, {"fa792b59", "skinDiffuse"}, {"091a8707", "t5Ramp"},
                {"8141e933", "bodiceNormal"}, {"02746d45", "bodiceMask"}, {"fcecfdcb", "bodiceDiffuse"},
                {"01ef7c05", "skirtNormal"}, {"0b5be4da", "skirtMask"}, {"1cd3181e", "skirtDiffuse"},
                {"5764478b", "irisDiffuse"}, {"d0524bfb", "eyeMask"},
            };

        // ---- which role each register binds, per component ----
        // Only RabbitFX's resource lines: a RabbitFX mod hands each draw's textures to its
        // SetTextures by NAME, which states the role outright. RabbitFX's Lightmap is her mask
        // (Component<N>_LM.dds matches her own masks by pixels), and component 1's mask role is
        // the historically named hairNormal. Component 6 is left out: Component6_Diffuse.dds is
        // her ps-t5 ramp, not the iris, so the name is no evidence there. The cloak mod binds its
        // bangs, hair and bodice this way, from a namespaced .ini one folder above the mesh's
        // (2026-10-05).
        facts.registerRoles = {
            {0, {{"Resource\\RabbitFX\\Diffuse", "bangsDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "bangsMask"}}},
            {1, {{"Resource\\RabbitFX\\Diffuse", "hairDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "hairNormal"}}},
            {2, {{"Resource\\RabbitFX\\Diffuse", "faceDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "faceMask"}}},
            {3, {{"Resource\\RabbitFX\\Diffuse", "skinDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "skinNormal"}}},
            {4, {{"Resource\\RabbitFX\\Diffuse", "bodiceDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "bodiceMask"},
                 {"Resource\\RabbitFX\\Normalmap", "bodiceNormal"}}},
            {5, {{"Resource\\RabbitFX\\Diffuse", "skirtDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "skirtMask"},
                 {"Resource\\RabbitFX\\Normalmap", "skirtNormal"}}},
        };

        // ---- and her own textures' PIXELS, for a file no hash and no register names ----
        // A mod that ships a texture dumped straight out of the game declares no hash for it and
        // binds it through no slot this table lists, so it is the only thing left that can say what
        // the file is. Read by the PARSER (`WWMIParser::buildRoles`), which is what identifies a
        // mod's textures; they sat on the fixer's config until 2026-09-29, where nothing read them
        // any more, and the parser's pass matched against an empty table on every mod.
        facts.textureThumbprints = sanhuaTextureThumbprints();

        // A planned role a mod ships no file for is bound to Sanhua's OWN texture of that role,
        // downloaded from her folder (the mod's UVs are hers). The red-camellia mod carries no
        // bodice or skirt mask, and the Exorcist's mask sampled at its UVs shaded cloth as skin: a
        // reddish hue over the whole body (2026-09-19). eyeMask (c88cc1fc) and faceMask (46177147)
        // are bound under the same hash on both skins, so they need no entry.
        facts.downloadCharFolder = "Sanhua";
        facts.downloadVersionFolder = "2_5";
        facts.downloadPrefix = "Sanhua";
        facts.fallbackTextures = {
            {"bangsDiffuse", "ae6e9014"}, {"bangsMask", "1c0c8b91"}, {"t5Ramp", "1035197c"},
            {"hairDiffuse", "68ca7071"}, {"hairNormal", "cef6494f"}, {"faceDiffuse", "881c236d"},
            {"skinNormal", "e39835c7"}, {"skinDiffuse", "fde0f298"},
            {"bodiceNormal", "efb25eb3"}, {"bodiceMask", "89ba19a1"}, {"bodiceDiffuse", "abda232b"},
            {"skirtNormal", "f3b217ab"}, {"skirtMask", "f0713dc7"}, {"skirtDiffuse", "c689a8ee"},
            {"irisDiffuse", "1dcc0f1d"},
        };

        return facts;
    }
}
