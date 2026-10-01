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

#include "AGRemapCore/data/IniParseData/LumineHeaven/LumineHeavenParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::lumineHeaven6_3() {
        // The seventh parser for a skin of SEVERAL components -- see makeGIMIComponentParser. Every value was read off
        // the prototype (Tools/Misc/Prototypes/lumineFromHeavenFix.py), which stays the oracle, and it off the skin's
        // frame dump (FrameAnalysis-LumineHeaven-2026-09-29-190651, giDrawTable.py):
        //
        //   slot        first   draw                              textures it reads
        //   main Head   0       vs 2c157719 / ps 92544cbc, LND    its own (hair, the neck scarf, sleeves, the bow)
        //   main Body   57141   vs 2c157719 / ps 6546504e, LND    its own (dress, skirt, legs)
        //   Bang A      0       vs 2c157719 / ps 92544cbc, LND    the Head's set
        //   Eye  A      0       plain vs 95aa6cdb, LD             its OWN iris atlas
        //
        // The main mesh's component name is EMPTY, so a slot borrowing its textures names its donor ";Head", and its
        // hashes are filed under LumineHeavenMain (Component::modTypeName). Lumine reads no normal map, so a borrowing
        // slot takes none of its donor's. Counts off the download folder (Data/Mod Downloads/GI/LumineHeaven/6_3);
        // every Texcoord is 12 bytes.
        GIMIComponentParserConfig config{};
        config.modTypeId = ModTypeId::LumineHeaven;
        config.downloadCharFolder = "LumineHeaven";
        config.downloadVersionFolder = "6_3";
        config.downloadPrefix = "LumineHeaven";

        GIMIComponentParserConfig::Component main{};
        main.name = "";
        main.modTypeName = ModTypeIdTools::getName(ModTypeId::LumineHeavenMain);
        main.texcoordStride = 12;
        main.vertexCount = 31179;
        main.slots = {{"Head", "0", "ps-t1", "ps-t2", "ps-t0", false},
                      {"Body", "57141", "ps-t1", "ps-t2", "ps-t0", false}};

        GIMIComponentParserConfig::Component bang{};
        bang.name = "Bang";
        bang.modTypeName = ModTypeIdTools::getName(ModTypeId::LumineHeavenBang);
        bang.texcoordStride = 12;
        bang.vertexCount = 2740;
        bang.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, ";Head", false}};

        // The Eye draws with textures of its OWN (the iris atlas), so it downloads its own when a mod lacks them.
        GIMIComponentParserConfig::Component eye{};
        eye.name = "Eye";
        eye.modTypeName = ModTypeIdTools::getName(ModTypeId::LumineHeavenEye);
        eye.texcoordStride = 12;
        eye.vertexCount = 246;
        eye.slots = {{"A", "0", "ps-t0", "ps-t1", "", false}};

        config.components = {std::move(main), std::move(bang), std::move(eye)};

        // A slot written in the GAME's register order: LumineHeaven1's Eye binds only `ps-t1 = ...Diffuse`, where the
        // skin's 6.x eye shader reads the diffuse, and a download decided per register put the game's eye diffuse at
        // ps-t0 beside it -- read by position on Lumine, the game's iris drew and the mod's became the light map (dark
        // eyes, 2026-09-29). See GIMIComponentParserConfig::downloadsByName.
        config.downloadsByName = true;

        return makeGIMIComponentParser(std::move(config));
    }


    IniParseBuilder::Factory LumineHeavenParser::v6_3() {
        return IniParseBuilderFuncs::lumineHeaven6_3();
    }
}
