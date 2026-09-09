#include "AGRemapCore/data/IniParseData/MonaCN/MonaCNParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::monaCN4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what MonaCN does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::MonaCN;
        config.downloadCharFolder = "MonaCN";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "MonaCN";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory MonaCNParser::v4_0() {
        return IniParseBuilderFuncs::monaCN4_0();
    }
}
