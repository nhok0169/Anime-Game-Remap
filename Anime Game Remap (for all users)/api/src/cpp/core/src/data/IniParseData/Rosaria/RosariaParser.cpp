#include "AGRemapCore/data/IniParseData/Rosaria/RosariaParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::rosaria4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Rosaria does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Rosaria;
        config.downloadCharFolder = "Rosaria";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Rosaria";
        config.drawnObjs = {"head", "body", "dress", "extra"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory RosariaParser::v4_0() {
        return IniParseBuilderFuncs::rosaria4_0();
    }
}
