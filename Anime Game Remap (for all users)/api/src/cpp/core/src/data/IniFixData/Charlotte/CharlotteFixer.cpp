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

#include "AGRemapCore/data/IniFixData/Charlotte/CharlotteFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"


namespace AGRemapCore {
    namespace {
        GIMIComponentFixerConfig charlotteHurlockConfig() {
            // Remapped onto CharlotteHurlock ("Hurlock Variations") -- the fourth skin of SEVERAL
            // components. Every value was read off the prototype
            // (Tools/Misc/Prototypes/charlotteHurlockFix.py), confirmed in game on her identity mod
            // and eight real mods, and off the two 6.7 frame dumps (Tools/Misc/Diagnostics/giDrawTable.py).
            GIMIComponentFixerConfig config{};
            config.targetSkin = ModTypeIdTools::getName(ModTypeId::CharlotteHurlock);
            config.drawnObjs = {"head", "body"};

            // Her own sections are the normal-map layout (her head and body draw on shader
            // 2c157719180b096c / 6546504e7a226d3a, bound LND): pass through; a mod still on the plain
            // layout (no ps-t2) gets the shift up. Per object, read off the mod's own section.
            config.sourceLayout = GIMIComponentFixerConfig::SourceLayout::Detect;

            // Both characters read the face diffuse at ps-t1 (GI 6.x). Swap only a mod still on ps-t0.
            config.faceSwapOnlyFromDiffuseReg = true;

            // Her HEAD draws through Body slot A and her BODY through slot B, spelled out rather than
            // read from IndexData -- see Component::slotIndex / objSlotIndices. The slot decides the
            // PIXEL SHADER: the skin draws slot A (hair, skin) on 92544cbc, a hair shader, and slot B
            // (the outfit) on 6546504e -- Charlotte's own. Her body through slot A put black shards
            // over Charlotte8's dark cardigan, and they went the moment it was drawn through slot B
            // (in game, 2026-09-24). The band legends line up either way: her head's hair (126-128)
            // on slot A's hair band (128); her body's skin (255) and cloth (126-128) on slot B's skin
            // (255) and cloth (128) bands. Each object is its own .ini group (the merge's second
            // file), so each takes its own slot. No band table.
            GIMIComponentFixerConfig::Component body{};
            body.name = "Body";
            body.modTypeName = ModTypeIdTools::getName(ModTypeId::CharlotteHurlockBody);
            body.slot = "A";
            body.slotIndex = "0";
            body.objSlotIndices = {{"body", "53529"}};
            body.negativeIndex = false;
            body.normalMap = true;
            body.face = true;
            body.texcoordStride = 20;
            body.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // Her eyes (vertex groups 13 / 14) are in her HEAD object. The skin draws its Eyes on the
            // PLAIN shader 95aa6cdb84eb7b99 with a diffuse / light map at ps-t0 / ps-t1 under NNFix, so
            // her normal-map sections are shifted DOWN there (buildPlainSlotShift).
            GIMIComponentFixerConfig::Component eyes{};
            eyes.name = "Eyes";
            eyes.modTypeName = ModTypeIdTools::getName(ModTypeId::CharlotteHurlockEyes);
            eyes.slot = "A";
            eyes.slotIndex = "0";
            eyes.negativeIndex = false;
            eyes.normalMap = false;
            eyes.face = false;
            eyes.texcoordStride = 12;
            eyes.slotRegisters = {"ps-t0", "ps-t1"};

            config.components = {body, eyes};

            // No forward vertex-group row reaches the skin's Bangs (her fringe is on her head bone),
            // and the skin's Camera is an ACCESSORY rather than part of the character -- the
            // maintainer's call, 2026-09-23: base Charlotte's camera is a separate mesh her hash.json
            // does not list, drawn by the game in both previews, so neither is remapped. Both are
            // hidden, or the skin's own would draw over her.
            config.hiddenComponents = {ModTypeIdTools::getName(ModTypeId::CharlotteHurlockBangs),
                                       ModTypeIdTools::getName(ModTypeId::CharlotteHurlockCamera)};

            // Body slots C, D and E -- every Body draw but slot A's (her head) and slot B's (her
            // body), so a TexFx request made on either is not served on one of them (Citlali's shiny
            // web, 2026-09-22). Eyes is one slot.
            config.unremappedSlots = {{ModTypeIdTools::getName(ModTypeId::CharlotteHurlockBody),
                                       {"99756", "103914", "104172"}}};
            config.diffuseEdits = {};
            config.lightMapObjs = {};

            // NOT BC7-compressed, as for every component config since Bennett's.
            config.compressTextures = false;

            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::charlotteHurlockBody6_7() {
        return makeGIMIComponentFixer(charlotteHurlockConfig(), "Body");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::charlotteHurlockEyes6_7() {
        return makeGIMIComponentFixer(charlotteHurlockConfig(), "Eyes");
    }


    IniFixBuilder::Factory CharlotteFixer::body6_7() {
        return IniFixBuilderFuncs::charlotteHurlockBody6_7();
    }


    IniFixBuilder::Factory CharlotteFixer::eyes6_7() {
        return IniFixBuilderFuncs::charlotteHurlockEyes6_7();
    }
}
