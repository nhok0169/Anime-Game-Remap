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

#include "AGRemapCore/data/IniParseData/NeuvilletteMelusent/NeuvilletteMelusentParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMIComponentParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::neuvilletteMelusent6_3() {
        // The fifth parser for a skin of SEVERAL components -- see makeGIMIComponentParser. Every value was read
        // off the prototype (Tools/Misc/Prototypes/neuvilletteFromMelusentFix.py), which stays the oracle, and it
        // off the skin's frame dump (FrameAnalysis-NeuvilletteMelusent-2026-09-24-204316, giDrawTable.py):
        //
        //   slot           first    draw                               textures it reads
        //   main Head      0        vs 63e32ce4 / ps 883013ba, LND     its own (hair, face skin, lapels)
        //   main Body      46620    vs 63e32ce4 / ps 5f3b4260, LND     its own (waistcoat, trousers)
        //   main Dress     71025    vs 4c036e7e / ps 26dbacaa, LND     the Head's set
        //   Coat A         0        vs 63e32ce4 / ps 5f3b4260, LND     the Body's set
        //   Bang A         0        vs 63e32ce4 / ps 883013ba, LND     the Head's set
        //   Eye A          0        plain vs 95aa6cdb, LD              the Head's diffuse / light map
        //
        // The main mesh's component name is EMPTY -- its files are NeuvilletteMelusentHead.ib,
        // NeuvilletteMelusentBlend.buf -- so a slot borrowing its textures names its donor ";Head", and its hashes
        // are filed under NeuvilletteMelusentMain (Component::modTypeName). Neuvillette reads normal maps (see the
        // fixer), so a borrowing slot takes its donor's normal map too. Every component's Texcoord is 12 bytes.
        GIMIComponentParserConfig config{};
        config.modTypeId = ModTypeId::NeuvilletteMelusent;
        config.downloadCharFolder = "NeuvilletteMelusent";
        config.downloadVersionFolder = "6_3";
        config.downloadPrefix = "NeuvilletteMelusent";

        GIMIComponentParserConfig::Component main{};
        main.name = "";
        main.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentMain);
        main.texcoordStride = 12;
        main.vertexCount = 22503;
        main.slots = {{"Head", "0", "ps-t1", "ps-t2", "ps-t0", false},
                      {"Body", "46620", "ps-t1", "ps-t2", "ps-t0", false},
                      {"Dress", "71025", "ps-t1", "ps-t2", "ps-t0", true, ";Head", true}};

        GIMIComponentParserConfig::Component coat{};
        coat.name = "Coat";
        coat.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentCoat);
        coat.texcoordStride = 12;
        coat.vertexCount = 8670;
        coat.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, ";Body", true}};

        GIMIComponentParserConfig::Component bang{};
        bang.name = "Bang";
        bang.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentBang);
        bang.texcoordStride = 12;
        bang.vertexCount = 2644;
        bang.slots = {{"A", "0", "ps-t1", "ps-t2", "ps-t0", true, ";Head", true}};

        GIMIComponentParserConfig::Component eye{};
        eye.name = "Eye";
        eye.modTypeName = ModTypeIdTools::getName(ModTypeId::NeuvilletteMelusentEye);
        eye.texcoordStride = 12;
        eye.vertexCount = 168;
        eye.slots = {{"A", "0", "ps-t0", "ps-t1", "", true, ";Head", true}};

        config.components = {std::move(main), std::move(coat), std::move(bang), std::move(eye)};

        return makeGIMIComponentParser(std::move(config));
    }


    IniParseBuilder::Factory NeuvilletteMelusentParser::v6_3() {
        return IniParseBuilderFuncs::neuvilletteMelusent6_3();
    }
}
