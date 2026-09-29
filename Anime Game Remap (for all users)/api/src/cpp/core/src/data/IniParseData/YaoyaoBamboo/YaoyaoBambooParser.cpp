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

#include "AGRemapCore/data/IniParseData/YaoyaoBamboo/YaoyaoBambooParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::yaoyaoBamboo6_3() {
        // The sixth parser for a skin of SEVERAL components -- see makeGIMIComponentParser. Every value was read
        // off the prototype (Tools/Misc/Prototypes/yaoyaoFromBambooFix.py), which stays the oracle, and it off the
        // skin's frame dump (FrameAnalysis-YaoyaoBamboo-2026-09-27-093332, giDrawTable.py):
        //
        //   slot        first   draw                              textures it reads
        //   main Head   0       vs 2c157719 / ps 92544cbc, LND    its own (hair, face skin, the crate, the umbrella)
        //   main Body   43092   vs 2c157719 / ps 6546504e, LND    its own (coat, shorts, legs)
        //   Bang A      0       vs 2c157719 / ps 92544cbc, LND    the Head's set
        //   Eye  A      0       plain vs 95aa6cdb, LD             the Head's diffuse / light map
        //
        // The main mesh's component name is EMPTY -- its files are YaoyaoBambooHead.ib, YaoyaoBambooPosition.buf --
        // so a slot borrowing its textures names its donor ";Head", and its hashes are filed under YaoyaoBambooMain
        // (Component::modTypeName). Yaoyao reads no normal map, so a borrowing slot takes none of its donor's.
        // Counts off the download folder (Data/Mod Downloads/GI/YaoyaoBamboo/6_3); every Texcoord is 12 bytes.
        GIMIComponentParserConfig config{};
        config.modTypeId = ModTypeId::YaoyaoBamboo;
        config.downloadCharFolder = "YaoyaoBamboo";
        config.downloadVersionFolder = "6_3";
        config.downloadPrefix = "YaoyaoBamboo";

        GIMIComponentParserConfig::Component main{};
        main.name = "";
        main.modTypeName = ModTypeIdTools::getName(ModTypeId::YaoyaoBambooMain);
        main.texcoordStride = 12;
        main.vertexCount = 27836;
        main.slots = {{"Head", "0", "ps-t1", "ps-t2", "ps-t0", false},
                      {"Body", "43092", "ps-t1", "ps-t2", "ps-t0", false}};

        GIMIComponentParserConfig::Component bang{};
        bang.name = "Bang";
        bang.modTypeName = ModTypeIdTools::getName(ModTypeId::YaoyaoBambooBang);
        bang.texcoordStride = 12;
        bang.vertexCount = 2829;
        bang.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, ";Head", false}};

        GIMIComponentParserConfig::Component eye{};
        eye.name = "Eye";
        eye.modTypeName = ModTypeIdTools::getName(ModTypeId::YaoyaoBambooEye);
        eye.texcoordStride = 12;
        eye.vertexCount = 238;
        eye.slots = {{"A", "0", "ps-t0", "ps-t1", "", true, ";Head", false}};

        config.components = {std::move(main), std::move(bang), std::move(eye)};

        return makeGIMIComponentParser(std::move(config));
    }


    IniParseBuilder::Factory YaoyaoBambooParser::v6_3() {
        return IniParseBuilderFuncs::yaoyaoBamboo6_3();
    }
}
