#include "AGRemapCore/data/IniParseData/AmberCN/AmberCNParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::amberCN4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what AmberCN does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::AmberCN;
        config.downloadCharFolder = "AmberCN";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "AmberCN";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory AmberCNParser::v4_0() {
        return IniParseBuilderFuncs::amberCN4_0();
    }
}
