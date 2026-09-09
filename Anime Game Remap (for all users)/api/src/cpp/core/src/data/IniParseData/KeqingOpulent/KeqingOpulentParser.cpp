#include "AGRemapCore/data/IniParseData/KeqingOpulent/KeqingOpulentParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::keqingOpulent4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what KeqingOpulent does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::KeqingOpulent;
        config.downloadCharFolder = "KeqingOpulent";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "KeqingOpulent";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KeqingOpulentParser::v4_0() {
        return IniParseBuilderFuncs::keqingOpulent4_0();
    }
}
