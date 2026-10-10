#include "AGRemapCore/data/IniFixData/Lynae/LynaeFixer.h"

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

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeSkeleton.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeSkinCodes.h"
#include "AGRemapCore/data/IniFixData/Lynae/LynaeTextures.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {
    // Transcribed from Tools/Misc/Prototypes/lynaePeppermintFix.py, which stays the oracle this is
    // A/B'd against. Pairing (per-component overlap of the two download folders' Position.buf, and
    // shader family): 0-4 and 7 one to one (the face and eyes are the same mesh), her jacket (5)
    // onto the skin's jacket (6), her props (6) onto the skin's hip pouch (5) -- same register order,
    // and the skin has no special-material shader, so the props' glass / foil passes do not carry.
    IniFixBuilder::Factory IniFixBuilderFuncs::lynaePeppermint3_7() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::LynaePeppermint;
        config.version = "3.7";

        // Hers, for a mod whose vb0 is not in sourceVersionByVb0. Both skins share cb4 f02baf77,
        // which at 3.6 only Lynae has
        config.sourceVersion = "3.6";

        // ---- her 3.7 update moved her vb0 AND renumbered her skeleton ----
        // The hash lookups follow the mod's vb0; the blend's vertex group row follows its GEOMETRY,
        // because a hash-update tool rewrote one mod's vb0 to 3.7's and kept its 3.6 bone ids
        config.sourceVersionByVb0 = {{"0c33d628", "3.6"}, {"7e400733", "3.7"}};
        config.skeletonNumberings = {{"3.6", lynaeRenumbered36To37()}, {"3.7", {}}};
        config.referenceBoneCentroids = lynaeBoneCentroids();

        // ---- the passes the TARGET draws each slot on (her Peppermint dump, wwmiDrawTable.py) ----
        config.slotPasses = {
            {"dfeea2d5f7210740", "8485dc12c5851fa9"},   // bangs (the second pass inherits the first's set)
            {"dfeea2d5f7210740"},                       // hair
            {"640fac991b3599a7"},                       // face
            {"ed4fe222718a4497"},                       // upper body
            {"0a52e215e81518f3"},                       // lower body
            {"12fcca8e31aad1f5", "4470be479e212ca1"},   // hip pouch (both passes set the whole set)
            {"efc2690a4acd3c11", "0b6e3ef7b7c30cd3"},   // jacket (the second inherits the first's)
            {"fa9e4d98ed0a570e"},                       // eyes
        };

        // ---- every pass gated through its VERTEX shaders, so RabbitFX's pixel-shader tags survive ----
        config.passVertexShaders = {
            {"dfeea2d5f7210740", {"c30da0cb86079064"}},
            {"8485dc12c5851fa9", {"c30da0cb86079064"}},
            {"640fac991b3599a7", {"1c42858ceb438917"}},
            {"ed4fe222718a4497", {"1f324d354402418c"}},
            {"0a52e215e81518f3", {"8af0aa3dcb3903ee"}},
            {"12fcca8e31aad1f5", {"fd92e896d5503d96"}},
            {"4470be479e212ca1", {"fd92e896d5503d96"}},
            {"efc2690a4acd3c11", {"1f324d354402418c"}},
            {"0b6e3ef7b7c30cd3", {"1f324d354402418c"}},
            {"fa9e4d98ed0a570e", {"c32a69851757154a"}},
            {"e04f4df80ee6b0ab", {"a54621ce48ed541b"}},
            {"bce1512f1c6b82fe", {"a57b6349c93b6107"}},
            {"f8c96a270bf847dd", {"6a6650a9db8983ce", "e4a3da6d1d1068b9"}},
            {"0fbe7ebba08cd1b0", {"641c11c9ee112caf", "d87c4657c08f2cdd"}},
        };

        // ---- ...and every tag NAMED. A shader holds ONE filter_index across every loaded .ini, so
        // the three outline vertex shaders Chisa -> ChisaParfait already tags keep its values; the
        // rest take 3381.610 up, a range no other pair uses (Chisa .710-.734 / .76-.768, Sanhua
        // .81-.84 / .91-.96). The reverse direction must reuse every value here it also tags ----
        config.filterIndices = {
            {"6a6650a9db8983ce", "3381.73"},
            {"e4a3da6d1d1068b9", "3381.731"},
            {"641c11c9ee112caf", "3381.715"},
            {"c30da0cb86079064", "3381.61"},
            {"1c42858ceb438917", "3381.611"},
            {"1f324d354402418c", "3381.612"},
            {"8af0aa3dcb3903ee", "3381.613"},
            {"fd92e896d5503d96", "3381.614"},
            {"c32a69851757154a", "3381.615"},
            {"a54621ce48ed541b", "3381.616"},
            {"a57b6349c93b6107", "3381.617"},
            {"d87c4657c08f2cdd", "3381.618"},
        };

        // ---- source component -> target slot and the registers it binds there ----
        config.plan = {
            // ps-t2 (36c90686) and ps-t3 (4eaa9816) are the same texture on both skins: left to the game
            {0, {0, {{"ps-t0", "bangsMask"}, {"ps-t1", "bangsDiffuse"}, {"ps-t4", "hairShadeRamp"}, {"ps-t5", "bangsStrand"}}}},
            {1, {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"},
                     {"ps-t4", "hairShadeRamp"}, {"ps-t5", "hairStrand"}}}},
            // The face is the same mesh on both, and the skin's face shader reads its diffuse at ps-t2
            // where hers reads it at ps-t1. The two face MASKS are packed differently (hers R 246 /
            // G 28 / B 0, the skin's R 118 / B 101 / A 202), so the skin's own mask stays -- its UVs
            // are hers
            {2, {2, {{"ps-t2", "faceDiffuse"}}}},
            {3, {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDetail"}, {"ps-t3", "upperDiffuse"}}}},
            {4, {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDetail"}, {"ps-t3", "lowerDiffuse"}}}},
            // The skin's jacket shader reads mask / diffuse / detail at ps-t1 / t2 / t3 -- hers reads
            // the detail at t2 and the diffuse at t3
            {5, {6, {{"ps-t0", "jacketNormal"}, {"ps-t1", "jacketMask"}, {"ps-t2", "jacketDiffuse"}, {"ps-t3", "jacketDetail"}}}},
            {6, {5, {{"ps-t0", "propsNormal"}, {"ps-t1", "propsMask"}, {"ps-t2", "propsDetail"}, {"ps-t3", "propsDiffuse"}}}},
            {7, {7, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"}}}},
        };

        // ---- a slot's OTHER passes: the outlines take the slot's diffuse at ps-t0; the eye's second
        // pass sets ps-t0 only ----
        config.extraPassRegs = {
            {0, {{"bce1512f1c6b82fe", {{"ps-t0", "bangsDiffuse"}}}}},
            {1, {{"f8c96a270bf847dd", {{"ps-t0", "hairDiffuse"}}}}},
            {3, {{"f8c96a270bf847dd", {{"ps-t0", "upperDiffuse"}}}}},
            {4, {{"0fbe7ebba08cd1b0", {{"ps-t0", "lowerDiffuse"}}}}},
            {5, {{"0fbe7ebba08cd1b0", {{"ps-t0", "propsDiffuse"}}}}},
            {6, {{"0fbe7ebba08cd1b0", {{"ps-t0", "jacketDiffuse"}}}}},
            {7, {{"e04f4df80ee6b0ab", {{"ps-t0", "eyeMask"}}}}},
        };

        config.sourceTextures = lynaeTextureFacts();

        // ---- the body's material-code maps (ps-t2) mark skin with 0 on Lynae and 4 on the skin, so
        // a map carried across unchanged shaded her body's skin as cloth -- whiter, with lavender
        // shadows, under a warm face: see lynaeSkinCodeSwap ----
        config.texEdits = {
            {"upperDetail", "SkinCode", [](const WWMIFixerConfig::TexEditContext&) { return lynaeSkinCodeSwap(); }},
            {"lowerDetail", "SkinCode", [](const WWMIFixerConfig::TexEditContext&) { return lynaeSkinCodeSwap(); }},
        };

        // Masks mark regions, and her own are flat in places on purpose (her hair mask is one value
        // over the whole texture, her lower-body and jacket masks R = 255 throughout), so a flat mask
        // is replaced by HER game texture rather than left to the game: the skin's hair mask is
        // structured and laid out for the skin's UVs, and would land in patches on hers
        config.flatFallsBackToSource = {"upperMask", "lowerMask", "jacketMask", "propsMask", "faceMask", "hairMask", "bangsMask"};

        // ---- the three lines that undo the remap on a source past 256 bones, and RabbitFX's ----
        config.removedRegs = {
            {"ResourceBlendBufferOverride", "ref"},
            {"ResourceMergedSkeletonOverride", "ref"},
            {"ResourceExtraMergedSkeletonOverride", "ref"},
            {"Resource\\RabbitFX\\Diffuse", ""},
            {"Resource\\RabbitFX\\Lightmap", ""},
            {"Resource\\RabbitFX\\Normalmap", ""},
            {"Resource\\RabbitFX\\Materialmap", ""},
            {"Resource\\RabbitFX\\Cutoutmap", ""},
            {"Resource\\RabbitFX\\Specialmap", ""},
            {"run", "commandlist\\rabbitfx\\settextures"},
        };

        // ---- the shape keys are RETARGETED: the overrides keep firing and the asset remap moves
        // their hashes and checksum onto the skin's; a batched export's dispatch height is the skin's
        // (her Metadata.json's dispatch_y), or WWMI loads too few of the mod's offsets on her draws ----
        config.hiddenObjs = {};
        config.zeroShapeKeyStream = false;
        config.shapeKeyDispatchSize = "1854";
        config.cleanTexcoords = true;

        // A mod's own line that puts a texture into another kind of slot on purpose keeps that slot:
        // Lynae3's `$Wish` (U) binds each part's diffuse into the hair ramp / detail slot, `ps-t2`,
        // which the two skins read alike -- moved by the texture's role it landed on the diffuse
        // register and the toggle did nothing
        config.carryByRegisterRole = true;

        // The fix's own merged skeleton is sized for the TARGET: LynaePeppermint's slots reach bone 314
        // (315 x 3 float4 = 945), past the 768 default
        config.mergedSkeletonSlots = 1536;

        // ---- a mod from before WWMI's merged skeleton: every such mod of hers is a 3.6 export ----
        config.sourceVgMaps = lynae36VgMaps();

        config.sourceLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "lower body"},
            {5, "jacket"}, {6, "props (headphones, pin, ID card)"}, {7, "eyes"},
        };
        config.targetLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "lower body"},
            {5, "hip pouch"}, {6, "jacket"}, {7, "eyes"},
        };

        return makeWWMIFixer(std::move(config));
    }


    IniFixBuilder::Factory LynaeFixer::peppermint3_7() {
        return IniFixBuilderFuncs::lynaePeppermint3_7();
    }
}
