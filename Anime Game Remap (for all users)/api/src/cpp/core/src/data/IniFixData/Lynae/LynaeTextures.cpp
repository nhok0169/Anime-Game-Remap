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

#include "AGRemapCore/data/IniFixData/Lynae/LynaeTextures.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeThumbprints.h"


namespace AGRemapCore {

    WWMITextureFacts lynaeTextureFacts() {
        WWMITextureFacts facts;

        // ---- her textures by the hash the game binds them under: the 3.7 dump's... ----
        facts.roles = {
            {"0eacd18a", "jacketNormal"},
            {"0f19cf16", "faceDiffuse"},
            {"0f5d6c08", "lowerNormal"},
            {"179ec8b9", "upperDiffuse"},
            {"1998df83", "lowerDiffuse"},
            {"265d844d", "hairDiffuse"},
            {"48947f0c", "bangsStrand"},
            {"4f1d5285", "propsMask"},
            {"694d3d7f", "hairRamp"},
            {"6bf7a371", "eyeIris"},
            {"6d5f71f9", "jacketDetail"},
            {"739ef6e9", "upperNormal"},
            {"7d044e58", "bangsDiffuse"},
            {"94d7281e", "lowerMask"},
            {"950acc22", "lowerDetail"},
            {"997e0a9a", "bangsMask"},
            {"a004a399", "eyeDiffuse"},
            {"a506a70d", "eyeMask"},
            {"b2ef1928", "hairStrand"},
            {"bfa33038", "hairMask"},
            {"c3375aee", "propsDiffuse"},
            {"c49bec43", "propsDetail"},
            {"c5784ddf", "upperDetail"},
            {"cecc13eb", "propsNormal"},
            {"d2ef0438", "jacketMask"},
            {"d5dfa096", "upperMask"},
            {"d63a624a", "lowerSheer"},
            {"d7c56e9e", "hairShadeRamp"},
            {"e28662e9", "faceMask"},
            {"f94bbcf4", "jacketDiffuse"},
            // ...and every OLDER generation wwmiHashHistory.py proved from her mods: all but two of the
            // thirteen on hand carry one, and each is identified by a mod shipping the game's own pixels
            // under it (correlation 1.000 with the current texture of that role)
            {"10c8713e", "jacketMask"},
            {"1261dcbc", "bangsStrand"},
            {"17372a89", "hairStrand"},
            {"211e50f4", "propsDiffuse"},
            {"23d9b128", "lowerDiffuse"},
            {"2e60ca6c", "faceDiffuse"},
            {"2fd59f3a", "upperNormal"},
            {"347a41a2", "bangsMask"},
            {"358f8d1e", "jacketNormal"},
            {"37cdf366", "propsDiffuse"},
            {"38686e88", "upperMask"},
            {"391a7fde", "propsNormal"},
            {"47df9d42", "hairMask"},
            {"513eace9", "bangsStrand"},
            {"60caec0b", "upperDiffuse"},
            {"6c09d0d7", "lowerNormal"},
            {"7065b0be", "hairDiffuse"},
            {"70ee685f", "jacketDiffuse"},
            {"7aa57b2c", "propsMask"},
            {"7f91ca3e", "lowerSheer"},
            {"8383cbbf", "eyeMask"},
            {"879f275e", "jacketDiffuse"},
            {"8e20c909", "bangsMask"},
            {"a8e46264", "jacketNormal"},
            {"b0e8646a", "bangsDiffuse"},
            {"b5453395", "hairStrand"},
            {"ba2b09a1", "propsNormal"},
            {"c84b599c", "eyeDiffuse"},
            {"cb04a3d7", "eyeDiffuse"},
            {"dd5dceaa", "lowerMask"},
            {"e45c9797", "propsMask"},
            {"ec6b0edc", "lowerNormal"},
            {"f45cea79", "hairMask"},
        };

        // ---- which role each of HER registers binds, per component (her own draws, wwmiPassLayout.py),
        // plus RabbitFX's resource lines, which a RabbitFX mod sets instead of any ps-t ----
        facts.registerRoles = {
            {0, {{"ps-t0", "bangsMask"}, {"ps-t1", "bangsDiffuse"}, {"ps-t4", "hairShadeRamp"}, {"ps-t5", "bangsStrand"}, {"Resource\\RabbitFX\\Diffuse", "bangsDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "bangsMask"}}},
            {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"}, {"ps-t4", "hairShadeRamp"}, {"ps-t5", "hairStrand"}, {"Resource\\RabbitFX\\Diffuse", "hairDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "hairMask"}}},
            {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "faceDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "faceMask"}}},
            {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDetail"}, {"ps-t3", "upperDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "upperDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "upperMask"}, {"Resource\\RabbitFX\\Normalmap", "upperNormal"}}},
            {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDetail"}, {"ps-t3", "lowerDiffuse"}, {"ps-t8", "lowerSheer"}, {"Resource\\RabbitFX\\Diffuse", "lowerDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "lowerMask"}, {"Resource\\RabbitFX\\Normalmap", "lowerNormal"}}},
            {5, {{"ps-t0", "jacketNormal"}, {"ps-t1", "jacketMask"}, {"ps-t2", "jacketDetail"}, {"ps-t3", "jacketDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "jacketDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "jacketMask"}, {"Resource\\RabbitFX\\Normalmap", "jacketNormal"}}},
            {6, {{"ps-t0", "propsNormal"}, {"ps-t1", "propsMask"}, {"ps-t2", "propsDetail"}, {"ps-t3", "propsDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "propsDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "propsMask"}, {"Resource\\RabbitFX\\Normalmap", "propsNormal"}}},
            {7, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "eyeDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "eyeMask"}}},
        };

        // ---- her own textures' pixels, for a file no hash and no register names ----
        facts.textureThumbprints = lynaeTextureThumbprints();

        // ---- a role the mod ships no file for falls back to HER game texture ----
        // The eye textures are the same hashes on both skins, so the game already binds them on the
        // target and they have no fallback.
        facts.downloadGameFolder = "WuWa";
        facts.downloadCharFolder = "Lynae";
        facts.downloadVersionFolder = "3_7";
        facts.downloadPrefix = "Lynae";
        facts.fallbackTextures = {
            {"bangsDiffuse", "7d044e58"},
            {"bangsMask", "997e0a9a"},
            {"bangsStrand", "48947f0c"},
            {"faceDiffuse", "0f19cf16"},
            {"faceMask", "e28662e9"},
            {"hairDiffuse", "265d844d"},
            {"hairMask", "bfa33038"},
            {"hairRamp", "694d3d7f"},
            {"hairShadeRamp", "d7c56e9e"},
            {"hairStrand", "b2ef1928"},
            {"jacketDetail", "6d5f71f9"},
            {"jacketDiffuse", "f94bbcf4"},
            {"jacketMask", "d2ef0438"},
            {"jacketNormal", "0eacd18a"},
            {"lowerDetail", "950acc22"},
            {"lowerDiffuse", "1998df83"},
            {"lowerMask", "94d7281e"},
            {"lowerNormal", "0f5d6c08"},
            {"lowerSheer", "d63a624a"},
            {"propsDetail", "c49bec43"},
            {"propsDiffuse", "c3375aee"},
            {"propsMask", "4f1d5285"},
            {"propsNormal", "cecc13eb"},
            {"upperDetail", "c5784ddf"},
            {"upperDiffuse", "179ec8b9"},
            {"upperMask", "d5dfa096"},
            {"upperNormal", "739ef6e9"},
        };

        return facts;
    }
}
