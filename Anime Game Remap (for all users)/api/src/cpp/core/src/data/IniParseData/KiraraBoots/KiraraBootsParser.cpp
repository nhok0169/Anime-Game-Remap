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

#include "AGRemapCore/data/IniParseData/KiraraBoots/KiraraBootsParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    namespace {
        GIMICharParserConfig kiraraBootsBase() {
            GIMICharParserConfig config{};
            config.modTypeId = ModTypeId::KiraraBoots;
            config.downloadCharFolder = "KiraraBoots";
            config.downloadVersionFolder = "4_8";
            config.downloadPrefix = "KiraraBoots";
            config.drawnObjs = {"head", "body", "dress"};
            config.texcoordStride = 20;

            return config;
        }
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::kiraraBoots4_8() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what KiraraBoots does differently lives here.
        //
        // The same three-layout split her base has: a normal map on ps-t0 for the head and body,
        // pushing their diffuse and lightmap up a slot, and a dress a slot higher with no normal
        // map.
        GIMICharParserConfig config = kiraraBootsBase();
        config.objDownloadRegs = {{"head", "ps-t1", "ps-t2", "ps-t0"},
                                  {"body", "ps-t1", "ps-t2", "ps-t0"},
                                  {"dress", "ps-t1", "ps-t2"}};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::kiraraBoots5_7() {
        // The MIRROR of her base's 5.7 row: here the head and body have moved to the modern
        // ps-t0/ps-t1 and the DRESS is the one left behind, where Kirara's own 5.7 row keeps its
        // head back instead. The pure-Python row says so by taking head and body from
        // FileDownloadData[5.7] and the dress from [4.8].
        GIMICharParserConfig config = kiraraBootsBase();
        config.objDownloadRegs = {{"dress", "ps-t1", "ps-t2"}};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KiraraBootsParser::v4_8() {
        return IniParseBuilderFuncs::kiraraBoots4_8();
    }


    IniParseBuilder::Factory KiraraBootsParser::v5_7() {
        return IniParseBuilderFuncs::kiraraBoots5_7();
    }
}
