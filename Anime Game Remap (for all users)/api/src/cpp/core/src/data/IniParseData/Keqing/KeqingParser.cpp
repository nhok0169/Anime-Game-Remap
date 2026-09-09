#include "AGRemapCore/data/IniParseData/Keqing/KeqingParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::keqing4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Keqing does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Keqing;
        config.downloadCharFolder = "Keqing";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Keqing";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KeqingParser::v4_0() {
        return IniParseBuilderFuncs::keqing4_0();
    }
}
