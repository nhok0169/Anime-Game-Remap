#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintFixer.h"

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

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintSkeleton.h"
#include "AGRemapCore/data/IniFixData/LynaePeppermint/LynaePeppermintTextures.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"


namespace AGRemapCore {
    namespace {
        /**
         * @brief A material mask of the skin's, repacked for Lynae's shaders
         *
         * The skin marks the region under a SHEER garment with R = 0 (the bikini under her see-through
         * shirt, a coat's translucent panels); Lynae's masks are R = 255 almost everywhere, and her
         * shaders do not draw an R = 0 texel as cloth -- a skin mod's coat vanished from Lynae. Every
         * R = 0 becomes 255, the cloth both legends agree on. A is set to 0: the skin's is 255 over the
         * translucent regions (40% of her jacket mask), Lynae's is 0 almost everywhere. G (how shiny a
         * surface is, the author's to choose) and B stay.
         */
        TexEditor::Filter maskRepackFilter() {
            return [](TextureFile& tex) {
                tex.setGamma(std::nullopt);          // these bytes are data, not colour
                std::vector<std::uint8_t> px = tex.getPixels();
                for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                    if (px[i] == 0) {
                        px[i] = 255;
                    }
                    px[i + 3] = 0;
                }

                tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
            };
        }
    }


    // Transcribed from Tools/Misc/Prototypes/peppermintLynaeFix.py, which stays the oracle this is
    // A/B'd against. Pairing: 0-4 and 7 one to one (the face and eyes are the same mesh); the skin's
    // coat (5) and her shirt / jacket / shoes (6) BOTH onto Lynae's jacket (5), the second through a
    // generated copy .ini. Lynae's props slot (6) renders through a special-material (glass / foil)
    // layer that barely reaches the G-buffer -- 137 pixels for her OWN props in a vanilla frame dump --
    // so nothing of the skin goes there.
    //
    // (The skin's component 5 roles are named "pouch" after the asset folder's first reading of that
    // component; it is her coat.)
    IniFixBuilder::Factory IniFixBuilderFuncs::lynae3_7() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::Lynae;
        config.version = "3.7";             // her live vb0 is 3.7's, and her bones are numbered as at 3.7
        config.sourceVersion = "3.7";

        // ---- the TARGET draws her early depth passes with her skeleton in vs-cb3 alone ----
        config.currentPoseInCb3Only = true;
        // ...and two source components share her jacket slot through a copy .ini, which has to share the
        // mod's skeleton state: merge-only sections for the slots the other file draws, and the
        // bone-data marker and shape-key overrides in the mod's own file only
        config.copiesShareSkeleton = true;
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // ---- the passes the TARGET (Lynae) draws each slot on (her dumps, wwmiDrawTable.py) ----
        config.slotPasses = {
            {"d4252cf5b68968eb", "5eb19d847b81ed33"},   // bangs (the second pass inherits the first's set)
            {"d4252cf5b68968eb"},                       // hair
            {"97ce9ee79fd51349"},                       // face
            {"a175ff3bd01feb29", "8d8250706d224af7"},   // upper body (the second inherits)
            {"a68c6144f18d6caf"},                       // lower body
            {"404f5464fdffc665"},                       // jacket
            // props: nothing of the skin is drawn there, so only the hide section uses it
            {"0f6f8facde912ff4", "2d6d59feda76cef9", "208e7eadc60a7abd", "0fc420109de57a0e"},
            {"fa9e4d98ed0a570e"},                       // eyes (and e04f4df8, in extraPassRegs)
        };

        // ---- every pass gated through its VERTEX shaders, so RabbitFX's pixel-shader tags survive ----
        config.passVertexShaders = {
            {"d4252cf5b68968eb", {"3e7bb648e306c671"}},
            {"5eb19d847b81ed33", {"3e7bb648e306c671"}},
            {"97ce9ee79fd51349", {"7c0b4db32cee62d3"}},
            {"a175ff3bd01feb29", {"1f324d354402418c"}},
            {"8d8250706d224af7", {"1f324d354402418c"}},
            {"a68c6144f18d6caf", {"343a49bd31719ade", "c30da0cb86079064"}},
            {"404f5464fdffc665", {"6ffcf365937e50bd"}},
            {"0f6f8facde912ff4", {"d4edc0d609271c1a"}},
            {"2d6d59feda76cef9", {"3424c4bc44c29aad"}},
            {"208e7eadc60a7abd", {"759b7b30c86ca081"}},
            {"0fc420109de57a0e", {"e2cb95b268cbb51e"}},
            {"bb718d7619295f25", {"99b91a71ed833cb8"}},
            {"9a01e7bd0aeff915", {"3b8be044e56e8c14"}},
            {"fa9e4d98ed0a570e", {"c32a69851757154a", "ceff9a81afb1de70"}},
            {"e04f4df80ee6b0ab", {"5fd6e5bb6ff81c53", "a54621ce48ed541b"}},
            {"bce1512f1c6b82fe", {"a57b6349c93b6107"}},
            {"f8c96a270bf847dd", {"6a6650a9db8983ce", "e4a3da6d1d1068b9"}},
            {"30ab50e715dce218", {"6a6650a9db8983ce", "e4a3da6d1d1068b9"}},
        };

        // ---- ...and every tag NAMED. A shader holds ONE filter_index across every loaded .ini, so
        // every shader another pair tags keeps THAT value: Chisa's 3e7bb648 .71, 343a49bd .713,
        // 5fd6e5bb .718, 6a6650a9 .73, e4a3da6d .731, ChisaParfait -> Chisa's 7c0b4db3 .76, and this
        // pair's forward direction's .61x. The shaders only this direction tags take 3381.62x ----
        config.filterIndices = {
            {"3e7bb648e306c671", "3381.71"},
            {"343a49bd31719ade", "3381.713"},
            {"5fd6e5bb6ff81c53", "3381.718"},
            {"6a6650a9db8983ce", "3381.73"},
            {"e4a3da6d1d1068b9", "3381.731"},
            {"7c0b4db32cee62d3", "3381.76"},
            {"c30da0cb86079064", "3381.61"},
            {"1f324d354402418c", "3381.612"},
            {"c32a69851757154a", "3381.615"},
            {"a54621ce48ed541b", "3381.616"},
            {"a57b6349c93b6107", "3381.617"},
            {"6ffcf365937e50bd", "3381.62"},
            {"d4edc0d609271c1a", "3381.621"},
            {"3424c4bc44c29aad", "3381.622"},
            {"759b7b30c86ca081", "3381.623"},
            {"e2cb95b268cbb51e", "3381.624"},
            {"ceff9a81afb1de70", "3381.625"},
            {"99b91a71ed833cb8", "3381.626"},
            {"3b8be044e56e8c14", "3381.627"},
        };

        // ---- source component -> Lynae's slot and the registers it binds there ----
        config.plan = {
            // ps-t2 (36c90686) and ps-t3 (4eaa9816) are the same texture on both skins: left to the game
            {0, {0, {{"ps-t0", "bangsMask"}, {"ps-t1", "bangsDiffuse"}, {"ps-t4", "hairShadeRamp"}, {"ps-t5", "bangsStrand"}}}},
            {1, {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"},
                     {"ps-t4", "hairShadeRamp"}, {"ps-t5", "hairStrand"}}}},
            // Lynae's face shader reads the diffuse at ps-t1; her mask (packed differently) stays -- the
            // face is the same mesh, so its UVs are hers
            {2, {2, {{"ps-t1", "faceDiffuse"}}}},
            {3, {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDetail"}, {"ps-t3", "upperDiffuse"}}}},
            {4, {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDetail"}, {"ps-t3", "lowerDiffuse"}}}},
            // the coat AND the jacket onto Lynae's jacket (her shader reads detail at t2, diffuse at t3);
            // the coat, first in order, stays in the mod's own file and the jacket goes to the copy
            {5, {5, {{"ps-t0", "pouchNormal"}, {"ps-t1", "pouchMask"}, {"ps-t2", "pouchDetail"}, {"ps-t3", "pouchDiffuse"}}}},
            {6, {5, {{"ps-t0", "jacketNormal"}, {"ps-t1", "jacketMask"}, {"ps-t2", "jacketDetail"}, {"ps-t3", "jacketDiffuse"}}}},
            {7, {7, {{"ps-t0", "eyeIris"}, {"ps-t1", "eyeMask"}, {"ps-t2", "eyeDiffuse"}}}},
        };

        // ---- a slot's OTHER passes: the outlines take the slot's diffuse at ps-t0 ----
        config.extraPassRegs = {
            {0, {{"bce1512f1c6b82fe", {{"ps-t0", "bangsDiffuse"}}}}},
            {1, {{"f8c96a270bf847dd", {{"ps-t0", "hairDiffuse"}}}}},
            {3, {{"f8c96a270bf847dd", {{"ps-t0", "upperDiffuse"}}}}},
            // her lower body's outline reads a mask at ps-t0 and the diffuse at ps-t1. Her own mask there
            // is a flat white ("outline everywhere"); the skin's outline reads its DIFFUSE at ps-t0, and
            // with the white the outline shell of the skin's layered thigh overlay drew as two black
            // bands down the backs of her thighs. So the skin's diffuse goes at both
            {4, {{"30ab50e715dce218", {{"ps-t0", "lowerDiffuse"}, {"ps-t1", "lowerDiffuse"}}}}},
            // her jacket slot takes TWO sources, and "the diffuse" is a different file for each:
            // without srcComponent both land in one list and the jacket's wins
            {5, {{"f8c96a270bf847dd", {{"ps-t0", "pouchDiffuse", 5}, {"ps-t0", "jacketDiffuse", 6}}}}},
            // her second eye pass SETS its own t0, and it is the eye MASK (a506a70d in her dump)
            {7, {{"e04f4df80ee6b0ab", {{"ps-t0", "eyeMask"}}}}},
        };

        config.sourceTextures = lynaePeppermintTextureFacts();

        // ---- every BODY mask is repacked into Lynae's legend: see maskRepackFilter ----
        config.texEdits = {
            {"upperMask", "Repack", [](const WWMIFixerConfig::TexEditContext&) { return maskRepackFilter(); }},
            {"lowerMask", "Repack", [](const WWMIFixerConfig::TexEditContext&) { return maskRepackFilter(); }},
            {"jacketMask", "Repack", [](const WWMIFixerConfig::TexEditContext&) { return maskRepackFilter(); }},
            {"pouchMask", "Repack", [](const WWMIFixerConfig::TexEditContext&) { return maskRepackFilter(); }},
        };

        // Masks mark regions: a flat one from a mod is replaced by HER game texture
        config.flatFallsBackToSource = {"upperMask", "lowerMask", "jacketMask", "pouchMask", "hairMask", "bangsMask"};

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

        // ---- the shape keys are RETARGETED; a batched export's dispatch height is LYNAE's (her
        // Metadata.json's dispatch_y) ----
        config.hiddenObjs = {};
        config.zeroShapeKeyStream = false;
        config.shapeKeyDispatchSize = "1631";
        config.cleanTexcoords = true;

        // Lynae's merged skeleton is 401 slots (1203 float4), past the 768 default
        config.mergedSkeletonSlots = 1536;

        // ---- a mod from before WWMI's merged skeleton: the skin was never renumbered ----
        config.sourceVgMaps = lynaePeppermintVgMaps();

        config.sourceLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "lower body"},
            {5, "coat"}, {6, "shirt, jacket and shoes"}, {7, "eyes"},
        };
        config.targetLabels = {
            {0, "bangs"}, {1, "hair"}, {2, "face"}, {3, "upper body"}, {4, "lower body"},
            {5, "jacket"}, {6, "props (headphones, pin, ID card)"}, {7, "eyes"},
        };

        return makeWWMIFixer(std::move(config));
    }


    IniFixBuilder::Factory LynaePeppermintFixer::lynae3_7() {
        return IniFixBuilderFuncs::lynae3_7();
    }
}
