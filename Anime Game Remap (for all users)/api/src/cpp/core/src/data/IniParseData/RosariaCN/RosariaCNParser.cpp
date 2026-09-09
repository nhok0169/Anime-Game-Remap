#include "AGRemapCore/data/IniParseData/RosariaCN/RosariaCNParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::rosariaCN4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what RosariaCN does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::RosariaCN;
        config.downloadCharFolder = "RosariaCN";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "RosariaCN";
        config.drawnObjs = {"head", "body", "dress", "extra"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory RosariaCNParser::v4_0() {
        return IniParseBuilderFuncs::rosariaCN4_0();
    }
}
