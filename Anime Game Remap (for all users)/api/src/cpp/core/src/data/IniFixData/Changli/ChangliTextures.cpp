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

#include "AGRemapCore/data/IniFixData/Changli/ChangliTextures.h"
#include "AGRemapCore/data/IniFixData/Changli/ChangliThumbprints.h"


namespace AGRemapCore {

    WWMITextureFacts changliTextureFacts() {
        WWMITextureFacts facts;

        // ---- her textures by hash: WWMI-Assets' (2.7, which most of her mods carry), the LIVE twin of
        // each (what the game binds at its highest texture quality, Data/Mod Downloads/WuWa/Changli/
        // ChangliLiveHashes.json), and the textures only the 3.7 dump holds, by their live hash. Generated
        // by Tools/Misc/Prototypes/changliPortTables.py from changliNymphFix.py's allRoles() ----
        facts.roles = {
            {"259c280f", "bangsDiffuse"},
            {"89da6405", "bangsDiffuse"},
            {"25e6decf", "bangsMap"},
            {"96177e42", "bangsMap"},
            {"dc8bd99e", "bangsOutline"},
            {"fcec9218", "bangsOutline"},
            {"06fbc0a7", "bangsStrand"},
            {"7eb470fc", "bangsStrand"},
            {"258c919f", "coatDiffuse"},
            {"2d7babdb", "coatDiffuse"},
            {"1061f8d7", "coatNormal"},
            {"59b52b44", "coatNormal"},
            {"5a9f3650", "coatOutline"},
            {"1bc98900", "eyeDiffuse"},
            {"f0536df1", "eyeDiffuse"},
            {"dd9d28ce", "eyeHighlight"},
            {"6bf7a371", "eyeIris"},
            {"8383cbbf", "eyeMask"},
            {"a506a70d", "eyeMask"},
            {"a97f587b", "faceDiffuse"},
            {"eefe376d", "faceDiffuse"},
            {"05187cd9", "faceMask"},
            {"e0fa653e", "faceMask"},
            {"21f813ba", "glow"},
            {"742c5c7b", "glow"},
            {"3800a5cc", "hairDiffuse"},
            {"9a3f146d", "hairDiffuse"},
            {"0484ab73", "hairMap"},
            {"fcf01372", "hairMap"},
            {"48e4d268", "hairOutline"},
            {"8b0bbdb6", "hairStrand"},
            {"fde34d42", "hairStrand"},
            {"0a1ba232", "lowerDiffuse"},
            {"3e6d3c8e", "lowerDiffuse"},
            {"4da65048", "lowerMask"},
            {"f3245be3", "lowerMask"},
            {"7f609046", "lowerNormal"},
            {"ee867ac2", "lowerNormal"},
            {"5ed7e436", "upperDetail"},
            {"cce7a5fa", "upperDetail"},
            {"8407b021", "upperDiffuse"},
            {"90aaf120", "upperDiffuse"},
            {"3ff7cad2", "upperMask"},
            {"9e3e0ad8", "upperMask"},
            {"69994531", "upperNormal"},
            {"f27be670", "upperNormal"},
            {"2ce995ee", "upperRamp"},
        };

        // ---- which role each of HER registers binds, per component (her own draws, wwmiPassLayout.py),
        // plus RabbitFX's resource names, which a RabbitFX mod sets instead of a ps-t (Changli13's
        // Textures/textures.ini binds her bodies only as Resource\RabbitFX\Diffuse); the Lightmap is the
        // mask, as on every pair so far ----
        facts.registerRoles = {
            {0, {{"ps-t0", "bangsMap"}, {"ps-t1", "bangsDiffuse"}, {"ps-t2", "bangsStrand"},
                 {"Resource\\RabbitFX\\Diffuse", "bangsDiffuse"}}},
            {1, {{"ps-t0", "hairMap"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairStrand"},
                 {"Resource\\RabbitFX\\Diffuse", "hairDiffuse"}}},
            {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"},
                 {"Resource\\RabbitFX\\Diffuse", "faceDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "faceMask"}}},
            {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDiffuse"}, {"ps-t3", "upperDetail"},
                 {"ps-t4", "upperRamp"}, {"Resource\\RabbitFX\\Diffuse", "upperDiffuse"},
                 {"Resource\\RabbitFX\\Lightmap", "upperMask"}, {"Resource\\RabbitFX\\Normalmap", "upperNormal"}}},
            {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDiffuse"},
                 {"Resource\\RabbitFX\\Diffuse", "lowerDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "lowerMask"},
                 {"Resource\\RabbitFX\\Normalmap", "lowerNormal"}}},
            {5, {{"ps-t0", "coatNormal"}, {"ps-t1", "coatDiffuse"},
                 {"Resource\\RabbitFX\\Diffuse", "coatDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "coatNormal"}}},
            {6, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"},
                 {"Resource\\RabbitFX\\Diffuse", "eyeDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "eyeMask"}}},
            {7, {{"ps-t0", "glow"}}},
        };

        // ---- her own textures' pixels, for a file no hash and no register names ----
        facts.textureThumbprints = changliTextureThumbprints();

        // ---- a role the mod ships no file for falls back to HER game texture ----
        // The hair map, the eye textures and the glow are the same hashes on both skins, so the game
        // already binds them on the target and they have no fallback.
        facts.downloadGameFolder = "WuWa";
        facts.downloadCharFolder = "Changli";
        facts.downloadVersionFolder = "3_7";
        facts.downloadPrefix = "Changli";
        facts.fallbackTextures = {
            {"bangsMap", "25e6decf"},
            {"bangsDiffuse", "89da6405"},
            {"bangsStrand", "7eb470fc"},
            {"bangsOutline", "fcec9218"},
            {"hairDiffuse", "9a3f146d"},
            {"hairStrand", "fde34d42"},
            {"hairOutline", "48e4d268"},
            {"faceMask", "05187cd9"},
            {"faceDiffuse", "a97f587b"},
            {"upperNormal", "69994531"},
            {"upperMask", "3ff7cad2"},
            {"upperDiffuse", "8407b021"},
            {"upperDetail", "5ed7e436"},
            {"upperRamp", "2ce995ee"},
            {"lowerNormal", "ee867ac2"},
            {"lowerMask", "4da65048"},
            {"lowerDiffuse", "3e6d3c8e"},
            {"coatNormal", "59b52b44"},
            {"coatDiffuse", "258c919f"},
            {"coatOutline", "5a9f3650"},
            {"eyeHighlight", "dd9d28ce"},
        };

        return facts;
    }
}
