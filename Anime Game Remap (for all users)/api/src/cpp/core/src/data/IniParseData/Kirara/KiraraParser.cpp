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

#include "AGRemapCore/data/IniParseData/Kirara/KiraraParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    namespace {
        GIMICharParserConfig kiraraBase() {
            GIMICharParserConfig config{};
            config.modTypeId = ModTypeId::Kirara;
            config.downloadCharFolder = "Kirara";

            // 4_0 for both versions -- the 5.7 entry in the pure-Python table points at the same
            // folder. Only the registers moved.
            config.downloadVersionFolder = "4_0";
            config.downloadPrefix = "Kirara";
            config.drawnObjs = {"head", "body", "dress"};
            config.texcoordStride = 20;

            return config;
        }

        // Her head and body shipped a NORMAL MAP, which is what pushes their diffuse and lightmap
        // up a slot. Most characters here have none and the fix invents one instead (texAdds).
        const GIMICharParserConfig::ObjDownloadRegs WithNormalMap(const std::string& obj) {
            return {obj, "ps-t1", "ps-t2", "ps-t0"};
        }
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::kirara4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Kirara does differently lives here.
        GIMICharParserConfig config = kiraraBase();

        // THREE DIFFERENT LAYOUTS IN ONE CHARACTER, which is why these are per object rather than
        // per version: head and body carry a normal map on ps-t0 and sit a slot higher, the dress
        // sits a slot higher with no normal map at all.
        config.objDownloadRegs = {WithNormalMap("head"), WithNormalMap("body"),
                                  {"dress", "ps-t1", "ps-t2"}};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory IniParseBuilderFuncs::kirara5_7() {
        // Her body and dress have moved to the modern ps-t0/ps-t1 by 5.7 -- but her HEAD has not,
        // and the pure-Python row says so by reaching back into FileDownloadData[4.0] for it while
        // taking the other two from [5.7]. Getting this wrong is silent: the head's diffuse would
        // be fetched onto the slot its normal map is still bound to.
        GIMICharParserConfig config = kiraraBase();
        config.objDownloadRegs = {WithNormalMap("head")};

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KiraraParser::v4_0() {
        return IniParseBuilderFuncs::kirara4_0();
    }


    IniParseBuilder::Factory KiraraParser::v5_7() {
        return IniParseBuilderFuncs::kirara5_7();
    }
}
