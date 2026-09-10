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

#include "AGRemapCore/data/IniParseData/GanyuTwilight/GanyuTwilightParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::ganyuTwilight4_4() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what GanyuTwilight does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::GanyuTwilight;
        config.downloadCharFolder = "GanyuTwilight";
        config.downloadVersionFolder = "4_4";
        config.downloadPrefix = "GanyuTwilight";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    // THE ROW A 6.1 FIX ACTUALLY USES, and the reason filling only 4.4 left this character
    // producing nothing at all.
    //
    // Parse rows are version-keyed and the lookup takes the NEWEST row at or below the version
    // asked for. GanyuTwilight has two -- 4.4 and 5.7 -- so a 6.1 fix resolves to 5.7, and while
    // that was still a default-factory stub the parser classified no mod objects, the fixer had no
    // graphs to edit, and the run logged "Fixing the .ini file from GanyuTwilight to Ganyu" for
    // every file while writing not one remapped section.
    //
    // Identical to 4.4 deliberately: the pure-Python 5.7 row differs only in which FileDownloadData
    // block its object downloads come from, and that block still points at the 4_4 folder.
    IniParseBuilder::Factory IniParseBuilderFuncs::ganyuTwilight5_7() {
        return IniParseBuilderFuncs::ganyuTwilight4_4();
    }


    IniParseBuilder::Factory GanyuTwilightParser::v5_7() {
        return IniParseBuilderFuncs::ganyuTwilight5_7();
    }


    IniParseBuilder::Factory GanyuTwilightParser::v4_4() {
        return IniParseBuilderFuncs::ganyuTwilight4_4();
    }


}
