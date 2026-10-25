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

#include "AGRemapCore/data/IniParseData/AetherHeaven/AetherHeavenParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::aetherHeaven6_3() {
        // A parser for a skin of SEVERAL components -- see makeGIMIComponentParser -- and LumineHeaven's twin. Every
        // value was read off the prototype (Tools/Misc/Prototypes/aetherFromHeavenFix.py), which stays the oracle,
        // and it off the skin's frame dump (FrameAnalysis-AetherHeaven-2026-10-24-164142):
        //
        //   slot        first   draw                              textures it reads
        //   main Head   0       vs 2c157719 / ps 92544cbc, LND    its own (the braid, the coat's upper half, the legs)
        //   main Body   31407   vs 2c157719 / ps 6546504e, LND    its own (the coat, the pauldron, the long drape)
        //   Bang A      0       vs 2c157719 / ps 92544cbc, LND    the Head's set
        //   Eye  A      0       plain vs 95aa6cdb, LD             its OWN iris atlas
        //
        // The main mesh's component name is EMPTY, so a slot borrowing its textures names its donor ";Head", and its
        // hashes are filed under AetherHeavenMain (Component::modTypeName). Aether reads no normal map, so a borrowing
        // slot takes none of its donor's. Counts off the download folder (Data/Mod Downloads/GI/AetherHeaven/6_3);
        // every Texcoord is 12 bytes.
        GIMIComponentParserConfig config{};
        config.modTypeId = ModTypeId::AetherHeaven;
        config.downloadCharFolder = "AetherHeaven";
        config.downloadVersionFolder = "6_3";
        config.downloadPrefix = "AetherHeaven";

        GIMIComponentParserConfig::Component main{};
        main.name = "";
        main.modTypeName = ModTypeIdTools::getName(ModTypeId::AetherHeavenMain);
        main.texcoordStride = 12;
        main.vertexCount = 26421;
        main.slots = {{"Head", "0", "ps-t1", "ps-t2", "ps-t0", false},
                      {"Body", "31407", "ps-t1", "ps-t2", "ps-t0", false}};

        GIMIComponentParserConfig::Component bang{};
        bang.name = "Bang";
        bang.modTypeName = ModTypeIdTools::getName(ModTypeId::AetherHeavenBang);
        bang.texcoordStride = 12;
        bang.vertexCount = 3193;
        bang.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, ";Head", false}};

        // The Eye draws with textures of its OWN (the iris atlas), so it downloads its own when a mod lacks them.
        GIMIComponentParserConfig::Component eye{};
        eye.name = "Eye";
        eye.modTypeName = ModTypeIdTools::getName(ModTypeId::AetherHeavenEye);
        eye.texcoordStride = 12;
        eye.vertexCount = 400;
        eye.slots = {{"A", "0", "ps-t0", "ps-t1", "", false}};

        config.components = {std::move(main), std::move(bang), std::move(eye)};

        // A slot written in the GAME's register order (an Eye binding only `ps-t1 = ...Diffuse`) is read by its
        // resource names -- LumineHeaven's dark eyes. See GIMIComponentParserConfig::downloadsByName.
        config.downloadsByName = true;

        return makeGIMIComponentParser(std::move(config));
    }


    IniParseBuilder::Factory AetherHeavenParser::v6_3() {
        return IniParseBuilderFuncs::aetherHeaven6_3();
    }
}
