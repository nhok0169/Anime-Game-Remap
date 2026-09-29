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

#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistTextures.h"
#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistThumbprints.h"


namespace AGRemapCore {

    WWMITextureFacts sanhuaExorcistTextureFacts() {
        WWMITextureFacts facts;

        // ---- her textures by the hash the game binds them under ----
        facts.roles = {
                // the asset repo's (2.5 / 2.6)
                {"f8d5c991", "bangsDiffuse"}, {"63c807fe", "bangsMask"}, {"4478285f", "t5Ramp"},
                {"11171f1c", "hairDiffuse"}, {"9febd992", "hairNormal"},
                {"46177147", "faceMask"}, {"c98e83cd", "faceDiffuse"},
                {"72739d6e", "torsoNormal"}, {"e0c15187", "torsoMask"}, {"52f35e6d", "torsoDiffuse"},
                {"221b8ad6", "lowerNormal"}, {"8e5306a9", "lowerDiffuse"},
                {"c88cc1fc", "eyeMask"}, {"1dcc0f1d", "irisDiffuse"},
                // the 2.2 / 2.4 ones (sanhua_qiming carries these, and repaints most of them)
                {"31a5f36a", "bangsDiffuse"}, {"d153e37f", "bangsMask"}, {"f22348f0", "t5Ramp"},
                {"9522bbc7", "hairDiffuse"}, {"a0cf932f", "hairNormal"}, {"464256d1", "faceDiffuse"},
                {"d5a089c8", "torsoNormal"}, {"6a10c291", "torsoNormal"}, {"168462a9", "torsoDiffuse"},
                {"d96aa9b8", "lowerNormal"}, {"fd078186", "lowerDiffuse"}, {"3cd03f60", "irisDiffuse"},
                // the live ones (the 2026-09-20 max-LOD dump), each 1.00 against the asset file
                {"af3ba241", "bangsDiffuse"}, {"0442ed26", "bangsMask"}, {"38074c14", "t5Ramp"},
                {"10980f87", "hairDiffuse"}, {"77a480f9", "hairNormal"},
                {"bf16c0c7", "faceMask"}, {"280300b0", "faceDiffuse"},
                {"773ed913", "torsoNormal"}, {"bc4f430b", "torsoMask"}, {"2dad4dfe", "torsoDiffuse"},
                {"83b34054", "lowerNormal"}, {"dfd02e32", "lowerDiffuse"},
                {"d0524bfb", "eyeMask"}, {"5764478b", "irisDiffuse"},
            };

        // ---- which role each register binds, per component ----
        facts.registerRoles = {};

        // ---- and her own textures' PIXELS, for a file no hash and no register names ----
        // A mod that ships a texture dumped straight out of the game declares no hash for it and
        // binds it through no slot this table lists, so it is the only thing left that can say what
        // the file is. Read by the PARSER (`WWMIParser::buildRoles`), which is what identifies a
        // mod's textures; they sat on the fixer's config until 2026-09-29, where nothing read them
        // any more, and the parser's pass matched against an empty table on every mod.
        facts.textureThumbprints = sanhuaExorcistTextureThumbprints();

        return facts;
    }
}
