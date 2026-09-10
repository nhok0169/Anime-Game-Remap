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

#include "AGRemapCore/data/IniParseData/JeanCN/JeanCNParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::jeanCN4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what JeanCN does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::JeanCN;
        config.downloadCharFolder = "JeanCN";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "JeanCN";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        return makeGIMICharParser(std::move(config));
    }


    // Identical to 4.0, and deliberately so.
    //
    // The pure-Python row DID differ at 5.5: it added a 'texEdits' entry declaring the body's
    // ShadeLightMap edit. That edit still happens -- it moved to the FIXER, where
    // GIMICharFixerConfig::texEdits both makes the texture and repoints the register, the two
    // halves the original split between the parser (which declared it) and the fixer's RegTexEdit
    // (which used it). It also only ever applied to the JeanSea remap, which is a fixer's business
    // rather than a parser's.
    IniParseBuilder::Factory IniParseBuilderFuncs::jeanCN5_5() {
        return IniParseBuilderFuncs::jeanCN4_0();
    }


    IniParseBuilder::Factory JeanCNParser::v4_0() {
        return IniParseBuilderFuncs::jeanCN4_0();
    }


    IniParseBuilder::Factory JeanCNParser::v5_5() {
        return IniParseBuilderFuncs::jeanCN5_5();
    }
}
