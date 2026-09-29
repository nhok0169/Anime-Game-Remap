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

#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitTextures.h"
#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitThumbprints.h"


namespace AGRemapCore {

    WWMITextureFacts chisaParfaitTextureFacts() {
        WWMITextureFacts facts;

        // ---- her textures by the hash the game binds them under ----
        facts.roles = {
            // shared with Chisa outright
            {"d3b9ba76", "frontHairMask"}, {"f2646d21", "frontHairDiffuse"}, {"9ccd7ea7", "frontHairNormal"},
            {"226b31fc", "irisDiffuse"},
            // hers
            {"3f433212", "hairMask"}, {"a94ee44f", "hairDiffuse"}, {"81f48e54", "hairRamp"},
            {"57aa5a71", "hairTipRamp"}, {"d547f3c6", "hairNormal"},
            {"226d9bc4", "faceMask"}, {"53e96488", "faceDiffuse"},
            {"3c4279a9", "upperNormal"}, {"6b7ae743", "upperMask"}, {"4c420ea9", "upperDiffuse"},
            {"b9a888ec", "lowerNormal"}, {"4668fce8", "lowerMask"}, {"1d79fc96", "lowerDiffuse"},
            {"74a761f5", "lowerSheen"},
            {"4bee4070", "bodySheen"},              // the holographic foil -- see the plan's note
            {"2f911db8", "panelMask"}, {"56e725c4", "panelNormal"}, {"1e1b7bbc", "panelDiffuse"},
            {"00e3f13b", "panelMatcap"},
            {"2c990f51", "propMask"}, {"71a6e63f", "propDiffuse"}, {"e4463fca", "propNormal"},
        };

        // ---- which role each register binds, per component ----
        facts.registerRoles = {
            {0, {{"ps-t0", "frontHairMask"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"},
                 {"Resource\\RabbitFX\\Diffuse", "frontHairDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "frontHairNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "frontHairMask"}}},
            {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"}, {"ps-t4", "hairTipRamp"},
                 {"ps-t5", "hairNormal"}, {"Resource\\RabbitFX\\Diffuse", "hairDiffuse"},
                 {"Resource\\RabbitFX\\Normalmap", "hairNormal"}, {"Resource\\RabbitFX\\Lightmap", "hairMask"}}},
            {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"},
                 {"Resource\\RabbitFX\\Diffuse", "faceDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "faceMask"}}},
            {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t3", "upperDiffuse"}, {"ps-t8", "bodySheen"},
                 {"Resource\\RabbitFX\\Diffuse", "upperDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "upperNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "upperMask"}}},
            {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t3", "lowerDiffuse"}, {"ps-t5", "lowerSheen"},
                 {"Resource\\RabbitFX\\Diffuse", "lowerDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "lowerNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "lowerMask"}}},
            {5, {{"ps-t1", "panelMask"}, {"ps-t2", "panelNormal"}, {"ps-t3", "panelDiffuse"},
                 {"Resource\\RabbitFX\\Diffuse", "panelDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "panelNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "panelMask"}}},
            {6, {{"ps-t1", "irisDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "irisDiffuse"}}},
            {7, {{"ps-t0", "propMask"}, {"ps-t2", "propDiffuse"}, {"ps-t4", "propNormal"},
                 {"Resource\\RabbitFX\\Diffuse", "propDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "propNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "propMask"}}},
        };

        // ---- and her own textures' PIXELS, for a file no hash and no register names ----
        // A mod that ships a texture dumped straight out of the game declares no hash for it and
        // binds it through no slot this table lists, so it is the only thing left that can say what
        // the file is. Read by the PARSER (`WWMIParser::buildRoles`), which is what identifies a
        // mod's textures; they sat on the fixer's config until 2026-09-29, where nothing read them
        // any more, and the parser's pass matched against an empty table on every mod.
        facts.textureThumbprints = chisaParfaitTextureThumbprints();

        return facts;
    }
}
