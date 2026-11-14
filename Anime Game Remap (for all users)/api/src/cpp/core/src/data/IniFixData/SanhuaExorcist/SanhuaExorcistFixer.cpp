#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistFixer.h"
#include "AGRemapCore/data/IniFixData/SanhuaExorcist/SanhuaExorcistTextures.h"

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

            // Sanhua's pixel shaders that bind CHARACTER textures, per draw slot, off a frame dump of
            // her (wwmiDrawTable.py). Her bangs slot has two of its own, and her bodice and skirt slots
            // share the body shader. Confirmed unchanged on the 2026-09-20 max-LOD dump.
            // slotPasses: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

            // ...each gated by the VERTEX shaders it is drawn with, never by its own pixel shader:
            // RabbitFX patches these, and a [ShaderOverride] on one of them takes the filter_index
            // RabbitFX needs away from it -- see the forward direction's note for the measurement
            // (2026-10-13; six of Sanhua's seven passes are proven patched, the eye pass unproven
            // either way). Her hair slot and both bangs passes share one vertex shader, so those
            // conditions collapse; each slot's list is in its own section, and nothing is routed
            // onto her bangs slot anyway.
            // passVertexShaders: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

            // The shaders Sanhua's own direction already tags keep ITS values (a [ShaderOverride] is
            // keyed by shader hash across every loaded .ini, so a mod fixed each way, both installed,
            // must agree); the shaders only this direction tags take values that direction never uses.
            // The face pair is ChisaParfait's and LynaePeppermint's value, for the same reason across
            // characters.
            // filterIndices: read off DrawPasses at the run's versions (WWMIFixerConfig::withGameData)

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


            // Her own vg_map per component (WWMI-Assets' SanhuaSkin1 Metadata.json): that component's
            // bone indices to the merged skeleton's. Only a mod from before WWMI's merged skeleton needs
            // it -- its blend holds the component's OWN indices, and without this the fix refuses such a
            // mod rather than remapping them as if they were merged (SanhuaExorcist3, 2026-09-19).
            // sourceVgMaps: read off VGRemaps (component -> "") at the run's versions (WWMIFixerConfig::withGameData)

            config.createdTextures = {{ClothMask, Colour(), 16, {"component5", "skirtMask.cloth"}}};     // its colour: MaterialColours

            config.sourceTextures = sanhuaExorcistTextureFacts();

            // ---- the mod's SHAPE KEYS are kept (2026-10-10). Hidden (the default), her face neither
            // blinked nor moved on Sanhua, and the hiding commented the overrides out of the mod's OWN
            // text, so its own character read the game's offsets by the mod's vertex ids -- every
            // expression on the wrong vertices there. The two number their keys identically
            // (Tools/Misc/Diagnostics/wwmiShapeKeyOrder.py: every key at its own index), so no
            // shapeKeyOrder; the asset remap moves the overrides' hashes and checksum, and a batched
            // export's dispatch height is the target's (its Metadata.json's dispatch_y) ----
            // her mods' RabbitFX routing is kept (a RabbitFX cloak draws through it), and no source of hers is past
            // 256 bones
            config.removedRegs = {};
            // shapeKeyDispatchSize: read off ShapeKeyDispatchSizes at the run's versions (WWMIFixerConfig::withGameData)


            // Her own textures' thumbprints, so a file no hash names is placed by what it IS

            config.sourceLabels = {{0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "torso, arms, ribbons"},
                                   {4, "hair bun, trousers"}, {5, "eyes"}};
            config.targetLabels = {{0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "arm skin"},
                                   {4, "bodice, hat, ribbons, boots"}, {5, "skirt"}, {6, "eyes"}};

            config.copyPreamble = IniComments::GIMIObjMergerPreamble;
            return config;
        }
    }


    WWMIFixerConfig SanhuaExorcistFixer::sanhua2_5Config() {
        return sanhuaConfig();
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::sanhua2_5() {
        return makeWWMIFixer(SanhuaExorcistFixer::sanhua2_5Config());
    }


    IniFixBuilder::Factory SanhuaExorcistFixer::sanhua2_5() {
        return IniFixBuilderFuncs::sanhua2_5();
    }
}
