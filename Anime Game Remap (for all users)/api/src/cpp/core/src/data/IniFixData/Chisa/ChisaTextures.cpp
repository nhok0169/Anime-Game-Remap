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

#include "AGRemapCore/data/IniFixData/Chisa/ChisaTextures.h"
#include "AGRemapCore/data/IniFixData/Chisa/ChisaThumbprints.h"


namespace AGRemapCore {

    WWMITextureFacts chisaTextureFacts() {
        WWMITextureFacts facts;

        // ---- her textures by the hash the game binds them under ----
        facts.roles = {
            {"019c268e", "accessoryDiffuse"},
            {"06790f7e", "skinRamp"},
            {"165f3a1b", "upperDiffuse"},
            {"226b31fc", "irisDiffuse"},
            {"232c2dbc", "hairRamp"},
            {"2b16c5ac", "hairTipRamp"},
            {"2b6f8bcb", "lowerNormal"},
            {"3f0e6f21", "lowerMask"},
            {"40528957", "accessoryNormal"},
            {"4eaa9816", "accessorySheen"},
            {"526b9ed0", "upperNormal"},
            {"6ae8dd10", "faceMask"},
            {"90196068", "upperMask"},
            {"9ccd7ea7", "frontHairNormal"},
            {"a842d51f", "hairMask"},
            {"bb73967a", "bodySheen"},
            {"cbab5910", "hairDiffuse"},
            {"d030af95", "faceDiffuse"},
            {"d3b9ba76", "frontHairMask"},
            {"e921181d", "hairNormal"},
            {"f2646d21", "frontHairDiffuse"},
            {"f642139e", "lowerDiffuse"},
        };

        // ---- which role each register binds, per component ----
        facts.registerRoles = {
            {0, {{"ps-t0", "frontHairMask"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}, {"Resource\\RabbitFX\\Diffuse", "frontHairDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "frontHairNormal"}, {"Resource\\RabbitFX\\Lightmap", "frontHairMask"}}},
            {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"}, {"ps-t5", "hairNormal"}, {"Resource\\RabbitFX\\Diffuse", "hairDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "hairNormal"}, {"Resource\\RabbitFX\\Lightmap", "hairMask"}}},
            {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "faceDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "faceMask"}}},
            {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "upperDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "upperNormal"}, {"Resource\\RabbitFX\\Lightmap", "upperMask"}}},
            {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "lowerDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "lowerNormal"}, {"Resource\\RabbitFX\\Lightmap", "lowerMask"}}},
            {5, {{"ps-t0", "accessoryDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "accessoryDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "accessoryNormal"}}},
            {6, {{"ps-t1", "irisDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "irisDiffuse"}}},
        };

        // ---- and her own textures' PIXELS, for a file no hash and no register names ----
        // A mod that ships a texture dumped straight out of the game declares no hash for it and
        // binds it through no slot this table lists, so it is the only thing left that can say what
        // the file is. Read by the PARSER (`WWMIParser::buildRoles`), which is what identifies a
        // mod's textures; they sat on the fixer's config until 2026-09-29, where nothing read them
        // any more, and the parser's pass matched against an empty table on every mod.
        facts.textureThumbprints = chisaTextureThumbprints();

        // ---- a role the mod ships no file for falls back to HER game texture ----
        facts.downloadCharFolder = "Chisa";
        facts.downloadVersionFolder = "2_8";
        facts.downloadPrefix = "Chisa";
        facts.fallbackTextures = {
            {"accessoryDiffuse", "019c268e"},
            {"accessoryNormal", "40528957"},
            {"accessorySheen", "4eaa9816"},
            {"bodySheen", "bb73967a"},
            {"faceDiffuse", "d030af95"},
            {"faceMask", "6ae8dd10"},
            {"frontHairDiffuse", "f2646d21"},
            {"frontHairMask", "d3b9ba76"},
            {"frontHairNormal", "9ccd7ea7"},
            {"hairDiffuse", "cbab5910"},
            // `hairMask` IS downloaded, and the flat test above must not reach it (2026-09-26).
            // a842d51f is one RGBA value over all 1024x1024 texels -- (255, 0, 126, 0) -- which
            // reads as "carries no information, so binding it is pointless". It is not: that value
            // is ChisaParfait's OWN dominant hair code, 49.2% of her own hair mask's texels. Bound,
            // every texel of the mod's hair is ordinary hair material.
            //
            // Dropped, the register falls to the game, and the game binds the TARGET's structured
            // mask (129 distinct R values, 16.3% of them R = 0) sampled at CHISA's UVs -- so the
            // codes land in patches laid out for a different head. Patchy codes shade in patches.
            //
            // The "R = 255 means bare skin" reading that argued for dropping it is a fact about the
            // BODY mask and does not carry to the hair pass: 60.4% of the target's own hair mask is
            // R = 255. Ask the target's own texture at a register what its values mean.
            {"hairMask", "a842d51f"},
            {"hairNormal", "e921181d"},
            {"hairRamp", "232c2dbc"},
            {"hairTipRamp", "2b16c5ac"},
            {"irisDiffuse", "226b31fc"},
            {"lowerDiffuse", "f642139e"},
            {"lowerMask", "3f0e6f21"},
            {"lowerNormal", "2b6f8bcb"},
            {"skinRamp", "06790f7e"},
            {"upperDiffuse", "165f3a1b"},
            {"upperMask", "90196068"},
            {"upperNormal", "526b9ed0"},
        };

        return facts;
    }
}
