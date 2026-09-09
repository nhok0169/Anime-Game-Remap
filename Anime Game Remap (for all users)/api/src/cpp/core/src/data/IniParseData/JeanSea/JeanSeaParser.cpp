#include "AGRemapCore/data/IniParseData/JeanSea/JeanSeaParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::jeanSea4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what JeanSea does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::JeanSea;
        config.downloadCharFolder = "JeanSea";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "JeanSea";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory JeanSeaParser::v4_0() {
        return IniParseBuilderFuncs::jeanSea4_0();
    }


}
