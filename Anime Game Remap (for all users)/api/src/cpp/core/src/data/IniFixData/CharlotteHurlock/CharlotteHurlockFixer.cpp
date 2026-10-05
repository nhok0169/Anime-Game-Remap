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

#include "AGRemapCore/data/IniFixData/CharlotteHurlock/CharlotteHurlockFixer.h"

#include <utility>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIMergeFixer.h"


namespace AGRemapCore {
    namespace {
        GIMIMergeFixerConfig charlotteConfig() {
            // CharlotteHurlock -> Charlotte: the fourth remap of a skin of SEVERAL components onto
            // a target of one, and the inverse of the Charlotte -> CharlotteHurlock rows. Every
            // value here was read off the prototype (Tools/Misc/Prototypes/charlotteFromHurlockFix.py),
            // which stays the oracle -- abCharlotteRev.py proves this identical to it.
            //
            // WHICH SOURCE SLOT LANDS ON WHICH OBJECT: every one lands on `body`, and Charlotte's
            // `head` receives nothing (the whole-ib skip keeps her own head hidden). Her head and
            // body draw on the SAME shader pair (2c157719 / 6546504e), so the shader-family rule
            // that sent Citlali's fringe to her body does not choose between them, and the body
            // is where Citlali's went.
            //
            // The trailing numbers are the GAME model's index count (hash.json's
            // object_index_counts) and `outline`: every slot of the skin outlines with Charlotte's
            // own outline shader (vs 67fd126e in both dumps), so none is kept out of it.
            //
            // NO BAND MOVE (lightMapEdit stays unset): the two legends put hair, skin and cloth on
            // the same bands (Charlotte's head 126-128 hair, her body 255 skin; the skin's Body A
            // 128 hair / 255 skin, Body B 128 cloth / 255 skin), and the identity mod renders right
            // in game without one.
            GIMIMergeFixerConfig config{};

            GIMIMergeFixerConfig::Component body{};
            body.name = "Body";
            body.slots = {{"A", "0", "body", true, "", 53529, true},
                          {"B", "53529", "body", true, "", 46227, true},
                          {"C", "99756", "body", false, "Body;B", 4158, true},
                          {"D", "103914", "body", true, "Body;A", 258, true}};

            // The GAME model's vertex count, for a mod that does not carry the component at all --
            // download Blend.buf bytes / 32.
            body.vertexCount = 30214;

            GIMIMergeFixerConfig::Component bangs{};
            bangs.name = "Bangs";
            bangs.slots = {{"A", "0", "body", true, "Body;A", 7104, true}};
            bangs.vertexCount = 1840;

            GIMIMergeFixerConfig::Component eyes{};
            eyes.name = "Eyes";
            eyes.slots = {{"A", "0", "body", false, "Body;B", 468, true}};
            eyes.vertexCount = 120;

            config.components = {std::move(body), std::move(bangs), std::move(eyes)};
            config.targetObjs = {"head", "body"};

            // Charlotte reads normal map / diffuse / light map at ps-t0/1/2 under ORFix on both of
            // her objects -- see CitlaliWhisperofStarsFixer, whose target is the same layout.
            config.targetLayout = GIMIMergeFixerConfig::TargetLayout::NormalMap;

            // Every carried binding onto the register its resource NAME says: the skin's mods bind
            // in ORFix's order (CharlotteHurlock4) or through GIMI's SetTextures API
            // (CharlotteHurlock1 and 5, normalized to the same), and a name decides the role either
            // way.
            config.texRegsByName = true;

            // A mod carrying none of a component merges it from its downloads.
            config.downloadPrefix = "CharlotteHurlock";

            // Charlotte binds her face diffuse (58d9859b -- the skin's too) at ps-t1, the GI 6.x
            // layout.
            config.faceReg = "ps-t1";
            config.compressTextures = false;
            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::charlotteHurlockToCharlotte6_7() {
        return makeGIMIMergeFixer(charlotteConfig());
    }


    IniFixBuilder::Factory CharlotteHurlockFixer::toCharlotte6_7() {
        return IniFixBuilderFuncs::charlotteHurlockToCharlotte6_7();
    }
}
