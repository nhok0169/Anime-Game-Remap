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

#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintTextures.h"
#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintThumbprints.h"


namespace AGRemapCore {

    WWMITextureFacts lynaePeppermintTextureFacts() {
        WWMITextureFacts facts;

        // ---- her textures by the hash the game binds them under: the 3.7 dump's... ----
        facts.roles = {
            {"0fa3ab67", "lowerDetail"},
            {"14b3b873", "upperNormal"},
            {"15f5a1c4", "pouchDetail"},
            {"16e48d10", "lowerDiffuse"},
            {"180c8e8a", "bangsDiffuse"},
            {"1dbd2313", "bangsStrand"},
            {"34926f91", "hairMask"},
            {"4644dbe8", "hairShadeRamp"},
            {"46bb3c2d", "pouchDiffuse"},
            {"4f2871af", "upperMask"},
            {"59324a0f", "bangsMask"},
            {"5a750238", "hairRamp"},
            {"6bc6b4c8", "faceDiffuse"},
            {"6bf7a371", "eyeIris"},
            {"6f56f1a3", "pouchNormal"},
            {"7003edde", "upperDetail"},
            {"701b1015", "faceMask"},
            {"9617b922", "jacketNormal"},
            {"a004a399", "eyeDiffuse"},
            {"a1fa0024", "jacketDetail"},
            {"a299690c", "hairStrand"},
            {"a506a70d", "eyeMask"},
            {"b66b4c2a", "lowerNormal"},
            {"bf5c5233", "faceMap"},
            {"c56aa503", "hairDiffuse"},
            {"ca537274", "jacketMask"},
            {"cb051033", "jacketDiffuse"},
            {"e546902d", "upperDiffuse"},
            {"ee1f2bc0", "pouchMask"},
            {"fef651e2", "lowerMask"},
            // ...and the OLDER generation wwmiHashHistory.py proved from LynaePeppermint2 (a mod
            // shipping the game's own pixels under each)
            {"2b8fb2f2", "lowerDiffuse"},
            {"33594042", "jacketMask"},
            {"561382b1", "jacketDiffuse"},
            {"68a0988e", "upperMask"},
            {"6b7842d9", "bangsDiffuse"},
            {"73975da7", "lowerNormal"},
            {"a37e527f", "upperNormal"},
            {"ad3621fc", "upperDiffuse"},
            {"b7820acf", "hairDiffuse"},
            {"e3466a1e", "lowerMask"},
            {"fa1f94a9", "jacketNormal"},
        };

        // ---- which role each of HER registers binds, per component (her own draws, wwmiPassLayout.py),
        // plus RabbitFX's resource lines, which a RabbitFX mod sets instead of any ps-t. Her jacket
        // shader reads mask / diffuse / detail at ps-t1 / t2 / t3 ----
        facts.registerRoles = {
            {0, {{"ps-t0", "bangsMask"}, {"ps-t1", "bangsDiffuse"}, {"ps-t4", "hairShadeRamp"}, {"ps-t5", "bangsStrand"}, {"Resource\\RabbitFX\\Diffuse", "bangsDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "bangsMask"}}},
            {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"}, {"ps-t4", "hairShadeRamp"}, {"ps-t5", "hairStrand"}, {"Resource\\RabbitFX\\Diffuse", "hairDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "hairMask"}}},
            {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceMap"}, {"ps-t2", "faceDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "faceDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "faceMask"}}},
            {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDetail"}, {"ps-t3", "upperDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "upperDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "upperMask"}, {"Resource\\RabbitFX\\Normalmap", "upperNormal"}}},
            {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDetail"}, {"ps-t3", "lowerDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "lowerDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "lowerMask"}, {"Resource\\RabbitFX\\Normalmap", "lowerNormal"}}},
            {5, {{"ps-t0", "pouchNormal"}, {"ps-t1", "pouchMask"}, {"ps-t2", "pouchDetail"}, {"ps-t3", "pouchDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "pouchDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "pouchMask"}, {"Resource\\RabbitFX\\Normalmap", "pouchNormal"}}},
            {6, {{"ps-t0", "jacketNormal"}, {"ps-t1", "jacketMask"}, {"ps-t2", "jacketDiffuse"}, {"ps-t3", "jacketDetail"}, {"Resource\\RabbitFX\\Diffuse", "jacketDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "jacketMask"}, {"Resource\\RabbitFX\\Normalmap", "jacketNormal"}}},
            {7, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "eyeDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "eyeMask"}}},
        };

        // ---- her own textures' pixels, for a file no hash and no register names ----
        facts.textureThumbprints = lynaePeppermintTextureThumbprints();

        // ---- a role the mod ships no file for falls back to HER game texture ----
        // The eye textures are the same hashes on both skins, and her face map and mask are not bound
        // on Lynae, so none of the five has a fallback.
        facts.downloadGameFolder = "WuWa";
        facts.downloadCharFolder = "LynaePeppermint";
        facts.downloadVersionFolder = "3_7";
        facts.downloadPrefix = "LynaePeppermint";
        facts.fallbackTextures = {
            {"bangsDiffuse", "180c8e8a"},
            {"bangsMask", "59324a0f"},
            {"bangsStrand", "1dbd2313"},
            {"faceDiffuse", "6bc6b4c8"},
            {"hairDiffuse", "c56aa503"},
            {"hairMask", "34926f91"},
            {"hairRamp", "5a750238"},
            {"hairShadeRamp", "4644dbe8"},
            {"hairStrand", "a299690c"},
            {"jacketDetail", "a1fa0024"},
            {"jacketDiffuse", "cb051033"},
            {"jacketMask", "ca537274"},
            {"jacketNormal", "9617b922"},
            {"lowerDetail", "0fa3ab67"},
            {"lowerDiffuse", "16e48d10"},
            {"lowerMask", "fef651e2"},
            {"lowerNormal", "b66b4c2a"},
            {"pouchDetail", "15f5a1c4"},
            {"pouchDiffuse", "46bb3c2d"},
            {"pouchMask", "ee1f2bc0"},
            {"pouchNormal", "6f56f1a3"},
            {"upperDetail", "7003edde"},
            {"upperDiffuse", "e546902d"},
            {"upperMask", "4f2871af"},
            {"upperNormal", "14b3b873"},
        };

        return facts;
    }
}
