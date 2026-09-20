#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistFixer.h"

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
#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistThumbprints.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {
    namespace {
        using Binding = WWMIFixerConfig::Binding;

        // The role this direction invents: SanhuaExorcist's slot 4 (her hair bun and trousers) is drawn
        // by a shader that reads NO material mask, and every slot of Sanhua's body family needs one.
        // (0, 0, 0) is her plain-cloth code -- 96% of her own bodice mask and 75% of her skirt's, measured.
        const std::string ClothMask = "ClothMask";

        WWMIFixerConfig sanhuaConfig() {
            // Every value here was read off Tools/Misc/Prototypes/exorcistToSanhuaFix.py, which was
            // confirmed in game on the SanhuaExorcist identity mod and four real mods over 2026-09-19/20.
            // See WWMIFixerConfig for what each field does, and the VGRemaps guide's
            // "THE REVERSE DIRECTION IS PROTOTYPED" for why the plan is what it is.
            WWMIFixerConfig config{};
            config.targetId = ModTypeId::Sanhua;
            config.version = "2.5";

            // Sanhua's pixel shaders that bind CHARACTER textures, per draw slot, off a frame dump of
            // her (wwmiDrawTable.py). Her bangs slot has two of its own, and her bodice and skirt slots
            // share the body shader. Confirmed unchanged on the 2026-09-20 max-LOD dump.
            config.slotPasses = {
                {"a512f04f32f6aa26", "f6bc3927337f5b8c"},   // 0: bangs
                {"69e3d3219c979981"},                       // 1: hair
                {"374a4f8fc9a5ea6a"},                       // 2: face
                {"7a0ab7c3ffbea13c"},                       // 3: arm skin
                {"96356f03a963d1d2"},                       // 4: bodice, hat, ribbons, boots
                {"96356f03a963d1d2"},                       // 5: skirt
                {"056f9f3c356ff96e"},                       // 6: eyes
            };

            // The shaders Sanhua's own direction already tags keep ITS values (a [ShaderOverride] is
            // keyed by shader hash across every loaded .ini, so a mod fixed each way, both installed,
            // must agree); the shaders only this direction tags take values that direction never uses.
            config.filterIndices = {
                {"69e3d3219c979981", "3381.91"},   // the hair shader, which both directions tag
                {"374a4f8fc9a5ea6a", "3381.93"},   // the face shader
                {"056f9f3c356ff96e", "3381.96"},   // the eye shader
                {"a512f04f32f6aa26", "3381.81"},   // Sanhua's own bangs shaders and body shader: only
                {"f6bc3927337f5b8c", "3381.82"},   //   this direction tags these, so they take values
                {"7a0ab7c3ffbea13c", "3381.83"},   //   the forward direction's numbering never reaches
                {"96356f03a963d1d2", "3381.84"},
            };

            // Source component -> target slot, and the registers each pass reads. Shader families decide
            // the slot. The BANGS go through Sanhua's HAIR slot rather than her bangs slot: Exorcist draws
            // her bangs with the hair shader, and Sanhua's own bangs shaders add a warm term that turns a
            // dark-recoloured fringe brown (in game, 2026-09-19). Her torso -- coat, arms and ribbons in
            // one component -- goes through the bodice slot, and her hair bun + trousers through the
            // skirt slot, both of Sanhua's body family (ps-t0 normal, ps-t1 mask, ps-t2 diffuse). Nothing
            // goes through Sanhua's arm-skin slot: its shader reads no mask and would shade the whole
            // coat as skin. Two sources on the hair slot is the merge: one extra .ini file.
            config.plan = {
                {0, {1, {{"ps-t0", "bangsDiffuse"}, {"ps-t1", "bangsMask"}, {"ps-t5", "t5Ramp"}}}},
                {1, {1, {{"ps-t0", "hairDiffuse"}, {"ps-t1", "hairNormal"}, {"ps-t5", "t5Ramp"}}}},
                {2, {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}}}},
                {3, {4, {{"ps-t0", "torsoNormal"}, {"ps-t1", "torsoMask"}, {"ps-t2", "torsoDiffuse"}}}},
                {4, {5, {{"ps-t0", "lowerNormal"}, {"ps-t1", ClothMask}, {"ps-t2", "lowerDiffuse"}}}},
                {5, {6, {{"ps-t1", "eyeMask"}, {"ps-t2", "irisDiffuse"}, {"ps-t5", "t5Ramp"}}}},
            };

            // SanhuaExorcist's textures by the hash they are filed under, with the role each plays in its
            // component's main pass -- every one measured, by correlating what the pass binds in the dump
            // against the asset file. Three sets of hashes, because a mod carries whichever its author
            // exported from: the asset repo's (WWMI-Assets' SanhuaSkin1), the 2.2 / 2.4 ones out of that
            // repo's git history, and the LIVE ones the game binds at LOD bias Ultra High, which is what a
            // mod extracted from a frame dump today carries. All of them are in
            // Data/Mod Downloads/WuWa/SanhuaExorcist/SanhuaExorcistHashLineage.json.
            config.roles = {
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

            // A file named by component and type and by nothing else (Component4_NM.dds: the
            // RabbitFX / WWMI-Tools export names)
            config.typeRoles = {
                {0, {{"diffuse", "bangsDiffuse"}, {"mask", "bangsMask"}}},
                {1, {{"diffuse", "hairDiffuse"}, {"mask", "hairNormal"}, {"normal", "hairNormal"}}},
                {2, {{"diffuse", "faceDiffuse"}, {"mask", "faceMask"}}},
                {3, {{"diffuse", "torsoDiffuse"}, {"mask", "torsoMask"}, {"normal", "torsoNormal"}}},
                {4, {{"diffuse", "lowerDiffuse"}, {"normal", "lowerNormal"}}},
                {5, {{"diffuse", "irisDiffuse"}, {"mask", "eyeMask"}}},
            };

            // Her own vg_map per component (WWMI-Assets' SanhuaSkin1 Metadata.json): that component's
            // bone indices to the merged skeleton's. Only a mod from before WWMI's merged skeleton needs
            // it -- its blend holds the component's OWN indices, and without this the fix refuses such a
            // mod rather than remapping them as if they were merged (SanhuaExorcist3, 2026-09-19).
            config.sourceVgMaps = {
                {0, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                16, 17, 18}},
                {1, {1, 20}},
                {2, {1}},
                {3, {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
                38, 39, 40, 41, 42, 20, 44, 45, 1, 47, 48, 49, 50, 51, 52, 53,
                54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
                70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
                86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101,
                102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117,
                118, 119, 120, 121, 122, 123, 124, 125, 126}},
                {4, {1, 4, 6, 5, 131, 132, 133, 134, 75, 80, 137, 138, 74, 79, 141, 142,
                143, 144, 68, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
                159, 160, 161, 162, 73, 164, 72, 24, 47, 44, 169, 170, 65, 172, 173, 174,
                175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189}},
                {5, {1}},
            };

            config.createdTextures = {{ClothMask, Colour(0, 0, 0, 255), 16}};

            // A planned role a mod ships no file for is bound to SanhuaExorcist's OWN texture of that
            // role, downloaded from her folder -- the mod's UVs are hers, so the target's texture is
            // wrong by construction. eyeMask (c88cc1fc) and faceMask (46177147) are the same texture on
            // both skins, and so is the iris (1dcc0f1d): they need no entry.
            config.downloadCharFolder = "SanhuaExorcist";
            config.downloadVersionFolder = "2_5";
            config.downloadPrefix = "SanhuaExorcist";
            config.fallbackTextures = {
                {"bangsDiffuse", "f8d5c991"}, {"bangsMask", "63c807fe"}, {"t5Ramp", "4478285f"},
                {"hairDiffuse", "11171f1c"}, {"hairNormal", "9febd992"}, {"faceDiffuse", "c98e83cd"},
                {"torsoNormal", "72739d6e"}, {"torsoMask", "e0c15187"}, {"torsoDiffuse", "52f35e6d"},
                {"lowerNormal", "221b8ad6"}, {"lowerDiffuse", "8e5306a9"},
            };

            // Her own textures' thumbprints, so a file no hash names is placed by what it IS
            config.textureThumbprints = sanhuaExorcistTextureThumbprints();

            config.sourceLabels = {{0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "torso, arms, ribbons"},
                                   {4, "hair bun, trousers"}, {5, "eyes"}};
            config.targetLabels = {{0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "arm skin"},
                                   {4, "bodice, hat, ribbons, boots"}, {5, "skirt"}, {6, "eyes"}};

            config.copyPreamble = IniComments::GIMIObjMergerPreamble;
            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::sanhua2_5() {
        return makeWWMIFixer(sanhuaConfig());
    }


    IniFixBuilder::Factory SanhuaExorcistFixer::sanhua2_5() {
        return IniFixBuilderFuncs::sanhua2_5();
    }
}
