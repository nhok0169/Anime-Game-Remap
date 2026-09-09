#include "AGRemapCore/data/IniParseData/Jean/JeanParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::jean4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Jean does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Jean;
        config.downloadCharFolder = "Jean";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Jean";
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
    IniParseBuilder::Factory IniParseBuilderFuncs::jean5_5() {
        return IniParseBuilderFuncs::jean4_0();
    }


    IniParseBuilder::Factory JeanParser::v4_0() {
        return IniParseBuilderFuncs::jean4_0();
    }


    IniParseBuilder::Factory JeanParser::v5_5() {
        return IniParseBuilderFuncs::jean5_5();
    }
}
