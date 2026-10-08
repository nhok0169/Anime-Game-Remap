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
        // A normal map on ps-t0 for the head and body, pushing their diffuse and lightmap up a
        // slot. The dress has no normal map and is NOT a slot higher: see kiraraBoots5_7.
        GIMICharParserConfig config = kiraraBootsBase();
        config.objDownloadRegs = {{"head", "ps-t1", "ps-t2", "ps-t0"},
                                  {"body", "ps-t1", "ps-t2", "ps-t0"}};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::kiraraBoots5_7() {
        // Here the head and body have moved to the modern ps-t0/ps-t1; the pure-Python row says so
        // by taking them from FileDownloadData[5.7].
        //
        // ITS DRESS IS ON THE MODERN SLOTS TOO, whatever that table's [4.8] block says. Her dress
        // binds diffuse / lightmap / shadow ramp on ps-t0 / ps-t1 / ps-t2 with no normal map: the
        // assets repo's dumped hash.json labels those slots NormalMap / Diffuse / LightMap, but
        // the hashes it names are her BODY's diffuse (e3a21e6f) and lightmap (8ca27fd3) and a
        // shared ramp (7eb5b84e), and her one real mod's ps-t0 "DressNormalMap" is an sRGB BC7 the
        // size of her body diffuse. Every KiraraBoots fix row keeps the dress's diffuse on ps-t0
        // (6.1 drops the ramp on ps-t2), so the old {ps-t1, ps-t2} here left the downloaded diffuse
        // on Kirara's lightmap slot and deleted the lightmap as the ramp.
        GIMICharParserConfig config = kiraraBootsBase();

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KiraraBootsParser::v4_8() {
        return IniParseBuilderFuncs::kiraraBoots4_8();
    }


    IniParseBuilder::Factory KiraraBootsParser::v5_7() {
        return IniParseBuilderFuncs::kiraraBoots5_7();
    }
}
