#include "AGRemapCore/data/IniParseData/Shenhe/ShenheParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::shenhe4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Shenhe does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Shenhe;
        config.downloadCharFolder = "Shenhe";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Shenhe";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory ShenheParser::v4_0() {
        return IniParseBuilderFuncs::shenhe4_0();
    }
}
