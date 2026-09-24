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

#include "AGRemapCore/data/IniParseData/CharlotteHurlock/CharlotteHurlockParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::charlotteHurlock6_7() {
        // The fourth parser for a skin of SEVERAL components -- see makeGIMIComponentParser. Every
        // value was read off the prototype (Tools/Misc/Prototypes/charlotteFromHurlockFix.py), which
        // stays the oracle, and it off the skin's 6.7 frame dump
        // (FrameAnalysis-CharlotteHurlock-2026-09-23-195553 through giDrawTable.py):
        //
        //   slot       first    draw (G-buffer)                     textures it reads
        //   Body A     0        vs 2c157719 / ps 92544cbc, LND      its own (hair, skin, some cloth)
        //   Body B     53529    vs 2c157719 / ps 6546504e, LND      its own (the outfit)
        //   Body C     99756    plain vs d4c01363, LD               Body B's diffuse / light map
        //   Body D     103914   vs b466a89c, LND                    Body A's set
        //   Bangs A    0        vs 2c157719 / ps 92544cbc, LND      Body A's set
        //   Eyes A     0        plain vs 95aa6cdb, LD               Body B's diffuse / light map
        //
        // So the normal-map layout is ps-t0 normal map / ps-t1 diffuse / ps-t2 light map, and the
        // two plain slots read diffuse / light map at ps-t0 / ps-t1. A slot the game draws with
        // ANOTHER slot's textures names it as its donor ("Body;B"): its textures are downloaded
        // from the donor rather than read out of the mod, and -- Charlotte reading normal maps --
        // so is the donor's normal map (the trailing `true`, donorNormalMap).
        //
        // NOT CONFIGURED: Body E (504 indices, drawn only in a special pass on shader c4a3e42f -- a
        // lens) and the whole Camera component, an accessory rather than part of her (the
        // maintainer, 2026-09-23). Base Charlotte's own camera is a separate mesh the game draws on
        // her anyway.
        GIMIComponentParserConfig config{};
        config.modTypeId = ModTypeId::CharlotteHurlock;
        config.downloadCharFolder = "CharlotteHurlock";
        config.downloadVersionFolder = "6_7";
        config.downloadPrefix = "CharlotteHurlock";

        GIMIComponentParserConfig::Component body{};
        body.name = "Body";
        body.modTypeName = ModTypeIdTools::getName(ModTypeId::CharlotteHurlockBody);
        body.texcoordStride = 20;
        body.vertexCount = 30214;
        body.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", false},
                      {"B", "53529", "ps-t1", "ps-t2", "ps-t0", false},
                      {"C", "99756", "ps-t0", "ps-t1", "", true, "Body;B", true},
                      {"D", "103914", "ps-t1", "ps-t2", "ps-t0", true, "Body;A", true}};

        GIMIComponentParserConfig::Component bangs{};
        bangs.name = "Bangs";
        bangs.modTypeName = ModTypeIdTools::getName(ModTypeId::CharlotteHurlockBangs);
        bangs.texcoordStride = 12;
        bangs.vertexCount = 1840;
        bangs.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, "Body;A", true}};

        GIMIComponentParserConfig::Component eyes{};
        eyes.name = "Eyes";
        eyes.modTypeName = ModTypeIdTools::getName(ModTypeId::CharlotteHurlockEyes);
        eyes.texcoordStride = 12;
        eyes.vertexCount = 120;
        eyes.slots = {{"A", "0", "ps-t0", "ps-t1", "", true, "Body;B", true}};

        config.components = {std::move(body), std::move(bangs), std::move(eyes)};

        return makeGIMIComponentParser(std::move(config));
    }


    IniParseBuilder::Factory CharlotteHurlockParser::v6_7() {
        return IniParseBuilderFuncs::charlotteHurlock6_7();
    }
}
