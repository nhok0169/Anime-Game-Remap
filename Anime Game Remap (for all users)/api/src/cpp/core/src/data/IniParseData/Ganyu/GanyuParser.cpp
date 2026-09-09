#include "AGRemapCore/data/IniParseData/Ganyu/GanyuParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::ganyu4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Ganyu does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Ganyu;
        config.downloadCharFolder = "Ganyu";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Ganyu";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory GanyuParser::v4_0() {
        return IniParseBuilderFuncs::ganyu4_0();
    }


}
